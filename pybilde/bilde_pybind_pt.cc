// pybilde/bilde_pybind_pt.cc
// Torch-tensor bindings for bilde, independent of NumPy.
// C++17, CPU only. Requires pybind11 + LibTorch and your torch_pybind11_container.hpp.

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <torch/extension.h>

#include <stdexcept>
#include <string>
#include <vector>
#include <type_traits>

//#include "torch_pybind11_container.hpp"  // from your earlier header
#include "../include/bilde.hpp"


namespace py = pybind11;
namespace pt = torch;
using bilde::container::TorchImageContainer;

// ========================= Shape & checks =====================================

struct BCHW {
    int64_t B{1}, C{1}, H{0}, W{0};
    int     rank{0}; // 2, 3, or 4
};


static inline void require_cpu(pt::Tensor const& t) {
    if (!t.defined()) throw std::invalid_argument("Tensor is undefined.");
    if (!t.device().is_cpu()) throw std::invalid_argument("Only CPU tensors are supported.");
}


static inline BCHW parse_bchw(pt::Tensor const& t) {
    require_cpu(t);
    const int d = t.dim();
    if (d == 2) return BCHW{1, 1, t.size(0), t.size(1), 2};
    if (d == 3) return BCHW{1, t.size(0), t.size(1), t.size(2), 3};
    if (d == 4) return BCHW{t.size(0), t.size(1), t.size(2), t.size(3), 4};
    throw std::invalid_argument("Expected HxW, CxHxW, or BxCxHxW.");
}


static inline pt::Tensor ensure_contiguous_cpu(pt::Tensor t) {
    require_cpu(t);
    return t.is_contiguous() ? t : t.contiguous();
}


static inline pt::Tensor slice_to_2d(pt::Tensor t, BCHW s, int64_t b, int64_t c) {
    // Reduce to HxW (single channel) view, then ensure contiguous
    if (s.rank == 4) {
        if (s.B > 1) t = t.select(0, b);  // -> CxHxW
        if (s.C > 1) t = t.select(0, c);  // -> HxW
    } else if (s.rank == 3) {
        if (s.C > 1) t = t.select(0, c);  // -> HxW
    } // rank 2 is already HxW
    return t.is_contiguous() ? t : t.contiguous();
}

// ========================= Dtype dispatch =====================================

template <typename F>
static inline void dispatch_dtype(pt::ScalarType st, F&& f) {
    switch (st) {
        case pt::kUInt8:  f.template operator()<uint8_t>();  break;
        case pt::kInt8:   f.template operator()<int8_t>();   break;
        case pt::kInt16:  f.template operator()<int16_t>();  break;
        case pt::kInt32:  f.template operator()<int32_t>();  break;
        case pt::kInt64:  f.template operator()<int64_t>();  break;
        case pt::kFloat:  f.template operator()<float>();    break;
        case pt::kDouble: f.template operator()<double>();   break;
        case pt::kBool:   f.template operator()<bool>();     break;
        default:
            throw std::invalid_argument("Unsupported tensor dtype.");
    }
}

// ========================= Broadcasting for (B, C) ============================

struct BCPlan {
    int64_t B{1}, C{1};
    int     out_rank{2};
};

static inline int max_rank(const std::vector<BCHW>& xs) {
    int r = 2;
    for (auto& s : xs) r = std::max(r, s.rank);
    return r;
}

static inline BCPlan plan_broadcast(const std::vector<BCHW>& xs) {
    if (xs.empty()) return {1, 1, 2};
    BCPlan p;
    p.out_rank = max_rank(xs);
    // H/W equality check
    const auto H = xs.front().H, W = xs.front().W;
    for (auto& s : xs) {
        if (s.H != H || s.W != W) {
            throw std::invalid_argument("All input tensors must have the same H and W.");
        }
    }
    // B/C: each must be 1 or the max
    p.B = 1; p.C = 1;
    for (auto& s : xs) { p.B = std::max(p.B, s.B); p.C = std::max(p.C, s.C); }
    for (auto& s : xs) {
        if (!(s.B == 1 || s.B == p.B))
            throw std::invalid_argument("Batch sizes (B) do not broadcast.");
        if (!(s.C == 1 || s.C == p.C))
            throw std::invalid_argument("Channel sizes (C) do not broadcast.");
    }
    return p;
}


static inline pt::Tensor allocate_like_rank(pt::ScalarType dt, BCPlan p, int64_t H, int64_t W) {
    auto opts = pt::TensorOptions().dtype(dt).device(pt::kCPU);
    if (p.out_rank == 4) return pt::empty({p.B, p.C, H, W}, opts);
    if (p.out_rank == 3) return pt::empty({p.C, H, W}, opts);
    return pt::empty({H, W}, opts);
}


static inline pt::Tensor out_slice_2d(pt::Tensor& out, BCPlan p, int64_t b, int64_t c) {
    if (p.out_rank == 4) {
        return out.select(0, (p.B == 1 ? 0 : b)).select(0, (p.C == 1 ? 0 : c)); // -> HxW
    } else if (p.out_rank == 3) {
        return out.select(0, (p.C == 1 ? 0 : c)); // -> HxW
    }
    return out; // 2D
}

// ========================= Core call helpers ==================================
//
// Each wrapper builds 2D Torch slices, wraps them with TorchImageContainer<T>,
// and invokes your underlying bilde::... function which takes Buffer<T> params.
// No NumPy conversion at any point.
//

// Unary op producing a new tensor: dst = F(src, extra...)
template <typename Caller, typename... A>
static pt::Tensor call_unary_new(pt::Tensor src, A... extra) {
    src = ensure_contiguous_cpu(src);
    auto s = parse_bchw(src);

    BCPlan p = plan_broadcast({s});
    auto dtype = src.scalar_type();
    pt::Tensor out = allocate_like_rank(dtype, p, s.H, s.W);

    dispatch_dtype(dtype, [&](auto TagT) {
        using T = typename decltype(TagT)::type; // trick for lambda template param extraction
    });

    // Workaround to extract T cleanly without the 'Tag' dance:
    auto runner = [&](auto* typed) {
        using T = std::remove_pointer_t<decltype(typed)>;

        const int64_t Bloop = p.B;
        const int64_t Cloop = p.C;

        for (int64_t b = 0; b < Bloop; ++b) {
            for (int64_t c = 0; c < Cloop; ++c) {
                pt::Tensor in2d  = slice_to_2d(src, s, b, c);
                pt::Tensor out2d = out_slice_2d(out, p, b, c);
                // Wrap as image containers (single-channel HxW):
                auto in_view  = TorchImageContainer<T>::from(in2d);
                auto out_view = TorchImageContainer<T>::from(out2d);
                // Call your C++ op:
                Caller::template run<T>(in_view, out_view, extra...);
            }
        }
    };

    switch (dtype) {
        case pt::kUInt8:  runner((uint8_t*)nullptr);  break;
        case pt::kInt8:   runner((int8_t*)nullptr);   break;
        case pt::kInt16:  runner((int16_t*)nullptr);  break;
        case pt::kInt32:  runner((int32_t*)nullptr);  break;
        case pt::kInt64:  runner((int64_t*)nullptr);  break;
        case pt::kFloat:  runner((float*)nullptr);    break;
        case pt::kDouble: runner((double*)nullptr);   break;
        case pt::kBool:   runner((bool*)nullptr);     break;
        default: throw std::invalid_argument("Unsupported tensor dtype.");
    }

    return out;
}

// Unary op writing into a provided dst tensor (shape/dtype must be compatible).
template <typename Caller, typename... A>
static void call_unary_into(pt::Tensor src, pt::Tensor dst, A... extra) {
    src = ensure_contiguous_cpu(src);
    dst = ensure_contiguous_cpu(dst);

    auto s  = parse_bchw(src);
    auto sd = parse_bchw(dst);

    // Require same rank/shape for output rank and H/W; broadcast B/C from src into dst
    if (s.H != sd.H || s.W != sd.W) throw std::invalid_argument("dst H/W mismatch.");
    BCPlan p = plan_broadcast({s, sd});
    if (sd.rank != p.out_rank)
        throw std::invalid_argument("dst rank mismatch (must match broadcasted rank).");
    if (dst.scalar_type() != src.scalar_type())
        throw std::invalid_argument("dst dtype must match src dtype.");

    auto dtype = src.scalar_type();

    auto runner = [&](auto* typed) {
        using T = std::remove_pointer_t<decltype(typed)>;

        const int64_t Bloop = p.B;
        const int64_t Cloop = p.C;

        for (int64_t b = 0; b < Bloop; ++b) {
            for (int64_t c = 0; c < Cloop; ++c) {
                pt::Tensor in2d  = slice_to_2d(src, s, b, c);
                pt::Tensor out2d = out_slice_2d(dst, p, b, c);
                auto in_view  = TorchImageContainer<T>::from(in2d);
                auto out_view = TorchImageContainer<T>::from(out2d);
                Caller::template run<T>(in_view, out_view, extra...);
            }
        }
    };

    switch (dtype) {
        case pt::kUInt8:  runner((uint8_t*)nullptr);  break;
        case pt::kInt8:   runner((int8_t*)nullptr);   break;
        case pt::kInt16:  runner((int16_t*)nullptr);  break;
        case pt::kInt32:  runner((int32_t*)nullptr);  break;
        case pt::kInt64:  runner((int64_t*)nullptr);  break;
        case pt::kFloat:  runner((float*)nullptr);    break;
        case pt::kDouble: runner((double*)nullptr);   break;
        case pt::kBool:   runner((bool*)nullptr);     break;
        default: throw std::invalid_argument("Unsupported tensor dtype.");
    }
}

// Binary op producing a new tensor: dst = F(a, b, extra...)
template <typename Caller, typename... A>
static pt::Tensor call_binary_new(pt::Tensor a, pt::Tensor b, A... extra) {
    a = ensure_contiguous_cpu(a);
    b = ensure_contiguous_cpu(b);

    auto sa = parse_bchw(a);
    auto sb = parse_bchw(b);
    if (sa.H != sb.H || sa.W != sb.W)
        throw std::invalid_argument("Inputs must have the same H and W.");
    if (a.scalar_type() != b.scalar_type())
        throw std::invalid_argument("Inputs must have the same dtype.");

    BCPlan p = plan_broadcast({sa, sb});
    pt::ScalarType dt = a.scalar_type();
    pt::Tensor out = allocate_like_rank(dt, p, sa.H, sa.W);

    auto runner = [&](auto* typed) {
        using T = std::remove_pointer_t<decltype(typed)>;

        const int64_t Bloop = p.B;
        const int64_t Cloop = p.C;

        for (int64_t bi = 0; bi < Bloop; ++bi) {
            for (int64_t ci = 0; ci < Cloop; ++ci) {
                pt::Tensor a2d  = slice_to_2d(a,  sa, (sa.B==1?0:bi), (sa.C==1?0:ci));
                pt::Tensor b2d  = slice_to_2d(b,  sb, (sb.B==1?0:bi), (sb.C==1?0:ci));
                pt::Tensor o2d  = out_slice_2d(out, p, bi, ci);

                auto av = TorchImageContainer<T>::from(a2d);
                auto bv = TorchImageContainer<T>::from(b2d);
                auto ov = TorchImageContainer<T>::from(o2d);

                Caller::template run<T>(av, bv, ov, extra...);
            }
        }
    };

    switch (dt) {
        case pt::kUInt8:  runner((uint8_t*)nullptr);  break;
        case pt::kInt8:   runner((int8_t*)nullptr);   break;
        case pt::kInt16:  runner((int16_t*)nullptr);  break;
        case pt::kInt32:  runner((int32_t*)nullptr);  break;
        case pt::kInt64:  runner((int64_t*)nullptr);  break;
        case pt::kFloat:  runner((float*)nullptr);    break;
        case pt::kDouble: runner((double*)nullptr);   break;
        case pt::kBool:   runner((bool*)nullptr);     break;
        default: throw std::invalid_argument("Unsupported tensor dtype.");
    }

    return out;
}

// Unary op returning a tuple of K tensors: (o0, o1, ... oK-1) = F(src, extra...)
template <size_t K, typename Caller, typename... A>
static auto call_unary_multi_new(pt::Tensor src, A... extra) {
    src = ensure_contiguous_cpu(src);
    auto s = parse_bchw(src);
    BCPlan p = plan_broadcast({s});
    pt::ScalarType dt = src.scalar_type();

    std::array<pt::Tensor, K> outs;
    for (size_t i = 0; i < K; ++i) outs[i] = allocate_like_rank(dt, p, s.H, s.W);

    auto runner = [&](auto* typed) {
        using T = std::remove_pointer_t<decltype(typed)>;

        const int64_t Bloop = p.B;
        const int64_t Cloop = p.C;

        for (int64_t b = 0; b < Bloop; ++b) {
            for (int64_t c = 0; c < Cloop; ++c) {
                pt::Tensor in2d = slice_to_2d(src, s, b, c);
                auto in_view = TorchImageContainer<T>::from(in2d);

                std::array<TorchImageContainer<T>, K> out_views;
                for (size_t i = 0; i < K; ++i) {
                    pt::Tensor o2d = out_slice_2d(outs[i], p, b, c);
                    out_views[i] = TorchImageContainer<T>::from(o2d);
                }

                Caller::template run<T>(in_view, out_views, extra...);
            }
        }
    };

    switch (dt) {
        case pt::kUInt8:  runner((uint8_t*)nullptr);  break;
        case pt::kInt8:   runner((int8_t*)nullptr);   break;
        case pt::kInt16:  runner((int16_t*)nullptr);  break;
        case pt::kInt32:  runner((int32_t*)nullptr);  break;
        case pt::kInt64:  runner((int64_t*)nullptr);  break;
        case pt::kFloat:  runner((float*)nullptr);    break;
        case pt::kDouble: runner((double*)nullptr);   break;
        case pt::kBool:   runner((bool*)nullptr);     break;
        default: throw std::invalid_argument("Unsupported tensor dtype.");
    }

    // Convert to Python tuple
    py::tuple tup(K);
    for (size_t i = 0; i < K; ++i) tup[i] = outs[i];
    return tup;
}

// ========================= Registration helpers ===============================
//
// Define tiny "Caller" structs for each bilde function you want to expose.
// Each must provide: template<typename T> static void run(...)
// with Buffer<T>-compatible parameter types (via TorchImageContainer<T>).
//
// Examples shown below use hypothetical signatures; adapt to yours.
//

// Example unary: dst = bilde::gaussian(src, sigma)
struct CallGaussian {
    template <typename T, typename BufIn, typename BufOut>
    static inline void invoke(BufIn const& in, BufOut& out, double sigma) {
        // Replace with your real call:
        // bilde::gaussian(in, out, sigma);
        (void)in; (void)out; (void)sigma;
        throw std::logic_error("CallGaussian not wired — replace with your bilde:: function.");
    }

    template <typename T>
    static inline void run(const TorchImageContainer<T>& in,
                           TorchImageContainer<T>& out,
                           double sigma) {
        invoke<T>(in, out, sigma);
    }
};

// Example binary: dst = bilde::add(a, b)
struct CallAdd {
    template <typename T, typename BufA, typename BufB, typename BufOut>
    static inline void invoke(BufA const& a, BufB const& b, BufOut& out) {
        // bilde::add(a, b, out);
        (void)a; (void)b; (void)out;
        throw std::logic_error("CallAdd not wired — replace with your bilde:: function.");
    }

    template <typename T>
    static inline void run(const TorchImageContainer<T>& a,
                           const TorchImageContainer<T>& b,
                           TorchImageContainer<T>& out) {
        invoke<T>(a, b, out);
    }
};

// ========================= pybind module ======================================

PYBIND11_MODULE(ptbilde, m) {
    py::module_::import("torch"); // This does not work , torch must be manually imported in python before ptbilde
    m.doc() = "bilde PyTorch bindings (BxCxHxW/CxHxW/HxW, CPU, contiguous). "
              "Loops over (B,C) and dispatches dtype; no NumPy involved.";
    m.def("__version__", []() { return std::string(BILDE_VERSION); },
          "bilde version string");
    m.def("label_connected_components", [](pt::Tensor img, int neighborhood) {
        // Example of a multi-output unary call:
        return call_unary_multi_new<5, bilde::operations::components::__labelConnectedComponents__<bilde::t_uint8>(img, neighborhood);
    }, py::arg("img"), py::arg("neighborhood")=8,
       "Label connected components in a binary image. "
       "Returns tuple of 5 tensors: (labeled_image, component_count, leftmost_labels, rightmost_labels, bottommost_labels). "
       "Input image must be 2D uint8 (0=background, nonzero=foreground). "
       "neighborhood: 4 or 8.");
    // ---- Example bindings (replace with your real ops) -----------------------
    //
    // 1) Unary, returns new tensor
    // m.def("gaussian",
    //       [](pt::Tensor src, double sigma) {
    //           return call_unary_new<CallGaussian>(src, sigma);
    //       },
    //       py::arg("src"), py::arg("sigma"),
    //       "Gaussian blur: out = gaussian(src, sigma)");
    //
    // 2) Unary, writes into provided dst (shape/dtype checked)
    // m.def("gaussian_into",
    //       [](pt::Tensor src, pt::Tensor dst, double sigma) {
    //           call_unary_into<CallGaussian>(src, dst, sigma);
    //       },
    //       py::arg("src"), py::arg("dst"), py::arg("sigma"),
    //       "Gaussian blur into preallocated dst.");
    //
    // 3) Binary, returns new tensor
    // m.def("add",
    //       [](pt::Tensor a, pt::Tensor b) {
    //           return call_binary_new<CallAdd>(a, b);
    //       },
    //       py::arg("a"), py::arg("b"),
    //       "Elementwise add: out = a + b");
    //
    // -------------------------------------------------------------------------

    // Tip: replicate your np bindings’ function list here by defining small
    // Call* structs that call the corresponding bilde:: functions. Because
    // TorchImageContainer<T> implicitly converts to bilde::Buffer<T>, you
    // don’t need any NumPy conversion at all.
}

