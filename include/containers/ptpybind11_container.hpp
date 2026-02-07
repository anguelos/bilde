/*
 * torch_pybind11_container.hpp
 *
 *  Created: Sep 09, 2025
 *  Purpose: Header-only helpers to wrap torch::Tensor images with pybind11.
 *           Supports HxW, CxHxW, BxCxHxW (CPU only), and enforces C == 1.
 *           Works with C++17 and pybind11. No RTTI or exceptions beyond std::invalid_argument.
 */

#ifndef TORCHPYBIND11_CONTAINER_HPP_
#define TORCHPYBIND11_CONTAINER_HPP_

// pybind11 + libtorch
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <torch/extension.h>

// std
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <string>

namespace bilde {
namespace container {

// ---- small traits / aliases --------------------------------------------------

using t_sz    = std::size_t;
using t_uint8 = unsigned char;

namespace pt_detail {

// Map C++ pixel type -> torch::ScalarType (CPU only)
template <class T> struct ctype_to_scalar;

template <> struct ctype_to_scalar<uint8_t>  { static constexpr torch::ScalarType value = torch::kUInt8;  };
template <> struct ctype_to_scalar<int8_t>   { static constexpr torch::ScalarType value = torch::kInt8;   };
template <> struct ctype_to_scalar<int16_t>  { static constexpr torch::ScalarType value = torch::kInt16;  };
template <> struct ctype_to_scalar<int32_t>  { static constexpr torch::ScalarType value = torch::kInt32;  };
template <> struct ctype_to_scalar<int64_t>  { static constexpr torch::ScalarType value = torch::kInt64;  };
template <> struct ctype_to_scalar<float>    { static constexpr torch::ScalarType value = torch::kFloat;  };
template <> struct ctype_to_scalar<double>   { static constexpr torch::ScalarType value = torch::kDouble; };
template <> struct ctype_to_scalar<bool>     { static constexpr torch::ScalarType value = torch::kBool;   };

// true if T is a supported element type
template <class T>
constexpr bool is_supported_pixel_v =
    std::is_same<T,uint8_t>::value  || std::is_same<T,int8_t>::value  ||
    std::is_same<T,int16_t>::value  || std::is_same<T,int32_t>::value ||
    std::is_same<T,int64_t>::value  || std::is_same<T,float>::value   ||
    std::is_same<T,double>::value   || std::is_same<T,bool>::value;

// Check the tensor is CPU, not sparse, not quantized
inline void require_plain_cpu_tensor(const torch::Tensor& t) {
    if (!t.defined())
        throw std::invalid_argument("Torch tensor is not defined.");
    if (!t.device().is_cpu())
        throw std::invalid_argument("Only CPU tensors are supported.");
    if (t.is_sparse())
        throw std::invalid_argument("Sparse tensors are not supported.");
    if (t.is_quantized())
        throw std::invalid_argument("Quantized tensors are not supported.");
}

// Returns {B, C, H, W} with B defaulted to 1 for 3D and 1 for 2D inputs.
// Enforces single-channel (C==1) for 3D/4D inputs. For 2D (H×W), C is set to 1.
struct BCHW {
    int64_t B{1}, C{1}, H{0}, W{0};
};

inline BCHW parse_image_shape(const torch::Tensor& t) {
    const int64_t ndim = t.dim();
    if (ndim == 2) {
        // H x W
        return BCHW{1, 1, t.size(0), t.size(1)};
    } else if (ndim == 3) {
        // C x H x W
        const int64_t C = t.size(0);
        if (C != 1) {
            throw std::invalid_argument("Only single-channel images are allowed (C==1 for CxHxW).");
        }
        return BCHW{1, C, t.size(1), t.size(2)};
    } else if (ndim == 4) {
        // B x C x H x W
        const int64_t C = t.size(1);
        if (C != 1) {
            throw std::invalid_argument("Only single-channel images are allowed (C==1 for BxCxHxW).");
        }
        return BCHW{t.size(0), C, t.size(2), t.size(3)};
    } else {
        throw std::invalid_argument("Unsupported tensor rank. Expected HxW, CxHxW, or BxCxHxW.");
    }
}

// Ensure the last spatial dimension is width for 2D/3D/4D layouts.
// We additionally accept non-contiguous tensors by using strides.
inline void require_last_dim_is_width(const torch::Tensor& t) {
    const int64_t ndim = t.dim();
    if (ndim == 2) return;        // [H, W] OK
    if (ndim == 3) return;        // [C, H, W] OK
    if (ndim == 4) return;        // [B, C, H, W] OK
    // parse_image_shape already guards ranks; this is defensive.
    throw std::invalid_argument("Tensor must have W as the last dimension.");
}

// Bytes per element for given dtype (host)
inline t_sz dtype_nbytes(torch::ScalarType st) {
    switch (st) {
        case torch::kUInt8:  return sizeof(uint8_t);
        case torch::kInt8:   return sizeof(int8_t);
        case torch::kInt16:  return sizeof(int16_t);
        case torch::kInt32:  return sizeof(int32_t);
        case torch::kInt64:  return sizeof(int64_t);
        case torch::kFloat:  return sizeof(float);
        case torch::kDouble: return sizeof(double);
        case torch::kBool:   return sizeof(bool);
        default: throw std::invalid_argument("Unsupported tensor dtype for image view.");
    }
}

} // namespace detail

// ---- Primary container adaptor ----------------------------------------------
//
// TorchImageContainer<T> is a lightweight, non-owning view over a torch::Tensor.
// It enforces CPU tensors, accepts HxW / CxHxW / BxCxHxW (with C==1), and exposes
// convenient accessors (width, height, batch, strides, data pointer, etc).
//
template <class PIXELT>
class TorchImageContainer {
    static_assert(pt_detail::is_supported_pixel_v<PIXELT>,
                  "TorchImageContainer<PIXELT>: PIXELT not supported.");

public:
    using pixel_type = PIXELT;

    // ---- static API (type-level) --------------------------------------------
    static constexpr torch::ScalarType scalar_type() {
        return pt_detail::ctype_to_scalar<PIXELT>::value;
    }

    static bool isDefined() {
        // we consider "defined" as: our pixel type maps to a torch scalar type
        return true;
    }

    // ---- construction --------------------------------------------------------
    TorchImageContainer() = default;

    explicit TorchImageContainer(const torch::Tensor& t)
        : _t(ensure_valid(t)) {}

    // Bind from a pybind11 object that should be a torch.Tensor
    explicit TorchImageContainer(const pybind11::object& obj)
        : TorchImageContainer(obj.cast<torch::Tensor>()) {}

    // Non-owning reference (shallow)
    const torch::Tensor& tensor() const { return _t; }
    torch::Tensor& tensor() { return _t; }

    // ---- validation helpers --------------------------------------------------
    static bool canWrap(const torch::Tensor& t) {
        try {
            ensure_valid(t);
            return true;
        } catch (...) {
            return false;
        }
    }

    // We never take data ownership away from a torch::Tensor
    static bool canTakeDataOwnership(const torch::Tensor&) { return false; }

    // Deep copy storage (returns a new tensor with cloned storage)
    static torch::Tensor deepCopyStorage(const torch::Tensor& t) {
        const auto v = ensure_valid(t);
        return v.clone();
    }

    // ---- geometry introspection ---------------------------------------------
    // Returns sizes in BCHW normalized form (B defaults to 1 for 2D/3D).
    int64_t batch()  const { return _shape.B; }
    int64_t channels() const { return _shape.C; } // guaranteed to be 1
    int64_t height() const { return _shape.H; }
    int64_t width()  const { return _shape.W;  }

    // byte stride for stepping by one pixel along W (i.e., last dim)
    t_sz bytePixelStride() const {
        // stride in elements along the last dimension, multiplied by bytes/element
        const auto s = _t.strides().back(); // elements
        return static_cast<t_sz>(s) * pt_detail::dtype_nbytes(_t.scalar_type());
    }

    // byte stride for stepping one row (H dimension)
    t_sz byteRowStride() const {
        // For H, the index depends on rank:
        // H index is:
        //   HxW        -> dim 0
        //   CxHxW      -> dim 1
        //   BxCxHxW    -> dim 2
        const int64_t h_dim = (_t.dim()==2) ? 0 : (_t.dim()==3 ? 1 : 2);
        const auto s = _t.stride(h_dim);
        return static_cast<t_sz>(s) * pt_detail::dtype_nbytes(_t.scalar_type());
    }

    // total number of elements for a single (B=1) image plane (C==1)
    int64_t numel_per_image() const {
        return _shape.H * _shape.W;
    }

    // pointer to the start of the (possibly non-contiguous) storage of the first image
    // NOTE: This returns the raw data pointer of the underlying tensor at its current offset.
    const PIXELT* data() const { return _t.data_ptr<PIXELT>(); }
    PIXELT*       data()       { return _t.data_ptr<PIXELT>(); }

    // Return a view for a given batch index (no copy). Valid only if dim==4 or dim==3/2 (returns self for non-batched).
    TorchImageContainer atBatch(int64_t b) const {
        if (_t.dim() == 4) {
            if (b < 0 || b >= _shape.B) throw std::out_of_range("Batch index out of range.");
            auto slice = _t.select(0, b);
            return TorchImageContainer(slice);
        }
        // Non-batched -> return self
        return *this;
    }

    // Ensure we have contiguous memory in standard layout [B,C,H,W] or [C,H,W] / [H,W].
    // Returns a contiguous clone if necessary, otherwise returns the original tensor.
    torch::Tensor contiguous() const {
        return _t.is_contiguous() ? _t : _t.contiguous();
    }

    // ---- utility creators ----------------------------------------------------
    // Create a tensor view around external data (no copy). You own the memory lifetime.
    // The returned tensor has shape HxW (single-channel). For batched, build a stack later.
    static torch::Tensor wrapExternal(void* data, int64_t H, int64_t W) {
        if (H <= 0 || W <= 0) throw std::invalid_argument("H and W must be positive.");
        auto options = torch::TensorOptions().dtype(scalar_type()).device(torch::kCPU);
        // Make a 2D tensor [H, W] with custom deleter = no-op (since we don't own).
        // PyTorch C++ API provides from_blob for non-owning views.
        return torch::from_blob(data, {H, W}, options);
    }

    // Convenience constructor from (possibly) non-contiguous tensor
    static TorchImageContainer from(const torch::Tensor& t) {
        return TorchImageContainer(t);
    }

private:
    static torch::Tensor ensure_valid(const torch::Tensor& t) {
        pt_detail::require_plain_cpu_tensor(t);

        // dtype check
        if (t.scalar_type() != scalar_type()) {
            // Allow bool<->uint8 friendly path? Stick to strict: exact match
            throw std::invalid_argument("Tensor dtype does not match PIXELT.");
        }

        // shape & channels
        auto s = pt_detail::parse_image_shape(t);
        pt_detail::require_last_dim_is_width(t);

        // Everything OK
        TorchImageContainer tmp;
        tmp._t = t;
        tmp._shape = s;
        return tmp._t;
    }

    torch::Tensor _t;
    pt_detail::BCHW  _shape{1,1,0,0};
};

// ---- pybind11 helpers --------------------------------------------------------
//
// If you expose functions that take TorchImageContainer<T> directly, you can
// register minimal type casters here. In most projects it’s enough to accept
// torch::Tensor in your bindings and construct TorchImageContainer<T> inside.
//
template <class PIXELT>
inline TorchImageContainer<PIXELT> make_image_view(const torch::Tensor& t) {
    return TorchImageContainer<PIXELT>::from(t);
}

// Example: validate and return H, W
template <class PIXELT>
inline std::pair<int64_t,int64_t> image_hw(const torch::Tensor& t) {
    TorchImageContainer<PIXELT> v(t);
    return {v.height(), v.width()};
}

// Example: get byte row stride
template <class PIXELT>
inline t_sz byte_row_stride(const torch::Tensor& t) {
    TorchImageContainer<PIXELT> v(t);
    return v.byteRowStride();
}

// Example: deep copy
template <class PIXELT>
inline torch::Tensor deep_copy(const torch::Tensor& t) {
    return TorchImageContainer<PIXELT>::deepCopyStorage(t);
}


}}  // namespace bilde::container

// ---- Suggested pybind11 usage (snippet) -------------------------------------
//
// PYBIND11_MODULE(my_ext, m) {
//   m.def("check_single_channel_u8",
//         [](const torch::Tensor& t) {
//             auto view = torchpybind::make_image_view<uint8_t>(t);
//             return std::vector<int64_t>{view.batch(), view.channels(), view.height(), view.width()};
//         },
//         "Validate tensor (CPU, single-channel) and return BCHW.");
// }
//
// Keep your binding layer minimal and do the heavy lifting in your framework.
//

#endif // TORCHPYBIND11_CONTAINER_HPP_
