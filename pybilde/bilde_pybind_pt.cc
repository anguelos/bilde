// bilde_pybind_torch.cc
#include <torch/extension.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <vector>
#include <cmath>
#include <stdexcept>
#include <thread>
#include <type_traits>

#include "../include/bilde.hpp"

namespace py = pybind11;

// ---------------------------- helpers ----------------------------

static inline void require_2d_cpu_contig(const torch::Tensor& t, const char* name) {
    if (!t.defined()) throw std::runtime_error(std::string(name) + " must be defined");
    if (t.device().type() != torch::kCPU) throw std::runtime_error(std::string(name) + " must be a CPU tensor");
    if (t.dim() != 2) throw std::runtime_error(std::string(name) + " must be a 2D tensor");
    if (!t.is_contiguous()) throw std::runtime_error(std::string(name) + " must be contiguous (C-order)");
}

template <typename T>
static inline py::buffer_info tensor_as_buffer_info(const torch::Tensor& t) {
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    if (t.scalar_type() != torch::CppTypeToScalarType<T>::value) {
        throw std::runtime_error("Tensor has wrong dtype for requested buffer view");
    }

    const auto itemsize = static_cast<ssize_t>(t.element_size());
    const auto rows = static_cast<ssize_t>(t.size(0));
    const auto cols = static_cast<ssize_t>(t.size(1));

    // Strides are in elements in torch; py::buffer_info wants bytes.
    const ssize_t s0 = static_cast<ssize_t>(t.stride(0)) * itemsize;
    const ssize_t s1 = static_cast<ssize_t>(t.stride(1)) * itemsize;

    return py::buffer_info(
        t.data_ptr<T>(),
        itemsize,
        py::format_descriptor<T>::format(),
        /*ndim*/ 2,
        /*shape*/ std::vector<ssize_t>{rows, cols},
        /*strides*/ std::vector<ssize_t>{s0, s1}
    );
}

static inline std::string __get_version__() { return BILDE_VERSION; }

// ---------------------------- bindings (torch) ----------------------------

torch::Tensor lbp_transform_torch(
    torch::Tensor img,
    int nb_samples = 8,
    double radius = 1.0,
    std::string interpolation = "bilinear",
    std::string cmp_operation = "one-tail",
    std::string cmp_threshold = "otsu"
) {
    require_2d_cpu_contig(img, "img");
    if (img.scalar_type() != torch::kUInt8) throw std::runtime_error("img must be torch.uint8");

    auto rows = static_cast<int>(img.size(0));
    auto cols = static_cast<int>(img.size(1));

    if (((radius + 3) * 2 >= rows) || ((radius + 3) * 2 >= cols)) {
        throw std::runtime_error("The radius is too big for the image size.");
    }

    auto out = torch::empty({rows, cols}, torch::TensorOptions().dtype(torch::kUInt8).device(torch::kCPU));

    py::buffer_info in_buf  = tensor_as_buffer_info<bilde::t_uint8>(img);
    py::buffer_info out_buf = tensor_as_buffer_info<bilde::t_uint8>(out);

    bilde::Buffer<bilde::t_uint8> inputBuffer(in_buf);
    bilde::Buffer<bilde::t_uint8> outputBuffer(out_buf);

    auto lbp = bilde::operations::lbp::__lbp_util__::LbpIterator<bilde::t_uint8>(
        inputBuffer, nb_samples, radius, interpolation, cmp_operation, cmp_threshold
    );
    lbp.applyLBPTransform(outputBuffer);

    return out;
}

torch::Tensor lbp_features_torch(
    torch::Tensor img,
    int nb_samples = 8,
    std::vector<double> radii = {1., 2., 3.},
    std::string interpolation = "bilinear",
    std::string cmp_operation = "one-tail",
    std::string cmp_threshold = "otsu",
    int num_threads = 20
) {
    require_2d_cpu_contig(img, "img");
    if (img.scalar_type() != torch::kUInt8) throw std::runtime_error("img must be torch.uint8");
    if (nb_samples > 8 || nb_samples < 1) throw std::runtime_error("not_implemented for more than 8 samples.");

    py::buffer_info in_buf = tensor_as_buffer_info<bilde::t_uint8>(img);
    const int rows = static_cast<int>(img.size(0));
    const int cols = static_cast<int>(img.size(1));

    for (double radius : radii) {
        if (((radius + 3) * 2 >= rows) || ((radius + 3) * 2 >= cols)) {
            throw std::runtime_error("The radius is too big for the image size.");
        }
    }

    std::vector<std::thread> threads;
    std::vector<std::vector<int>> indexed_results(radii.size());

    int current_radius_idx = 0;
    while (current_radius_idx < static_cast<int>(radii.size())) {
        for (int i = 0; i < num_threads && (current_radius_idx + i) < static_cast<int>(radii.size()); i++) {
            threads.emplace_back([&, i, current_radius_idx]() {
                double radius = radii[current_radius_idx + i];
                int erode = static_cast<int>(std::ceil(radius));

                auto lbp = bilde::operations::lbp::__lbp_util__::LbpIterator<bilde::t_uint8>(
                    in_buf, nb_samples, radius, interpolation, cmp_operation, cmp_threshold
                );

                // Match your numpy binding behavior: write into a chopped view.
                bilde::Buffer<bilde::t_uint8> chopped_out_buf =
                    bilde::Buffer<bilde::t_uint8>(in_buf, erode, erode, cols - (erode + 1), rows - (erode + 1));

                lbp.applyLBPTransform(chopped_out_buf);
                indexed_results[current_radius_idx + i] = bilde::operations::essential::getHistogram(chopped_out_buf);
            });
        }

        for (auto& t : threads) t.join();
        threads.clear();
        current_radius_idx += num_threads;
    }

    std::vector<int> results;
    results.reserve(indexed_results.size() * 256); // typical for 8-sample LBP hist, ok as a hint
    for (auto& r : indexed_results) results.insert(results.end(), r.begin(), r.end());

    auto out = torch::empty({static_cast<long>(results.size())},
                            torch::TensorOptions().dtype(torch::kInt32).device(torch::kCPU));
    std::memcpy(out.data_ptr<int>(), results.data(), results.size() * sizeof(int));
    return out;
}

torch::Tensor enhance_grayscale_torch(
    torch::Tensor img,
    int bitDepth = 8,
    std::string mode = "equalise",
    int windowWidth = 51,
    int windowHeight = 51,
    int globalHistogramCoeficient = 0,
    int localHistogramCoeficient = 1,
    float topQuantile = .95f,
    float bottomQuantile = .05f
) {
    require_2d_cpu_contig(img, "img");
    if (img.scalar_type() != torch::kUInt8) throw std::runtime_error("img must be torch.uint8");

    const int rows = static_cast<int>(img.size(0));
    const int cols = static_cast<int>(img.size(1));

    auto out = torch::empty({rows, cols}, torch::TensorOptions().dtype(torch::kUInt8).device(torch::kCPU));

    py::buffer_info in_buf  = tensor_as_buffer_info<bilde::t_uint8>(img);
    py::buffer_info out_buf = tensor_as_buffer_info<bilde::t_uint8>(out);

    bilde::Buffer<bilde::t_uint8> inputBuffer(in_buf);
    bilde::Buffer<bilde::t_uint8> outputBuffer(out_buf);

    bilde::methods::enhance_grayscale::enhaceGray(
        inputBuffer, outputBuffer,
        bitDepth, mode,
        windowWidth, windowHeight,
        globalHistogramCoeficient, localHistogramCoeficient,
        topQuantile, bottomQuantile
    );

    return out;
}

std::pair<torch::Tensor, int> label_connected_components_torch(torch::Tensor img, int neighborhood = 8) {
    require_2d_cpu_contig(img, "img");
    if (img.scalar_type() != torch::kUInt8) throw std::runtime_error("img must be torch.uint8");
    if (neighborhood != 4 && neighborhood != 8) throw std::runtime_error("not_implemented for other than 4 or 8 neighborhoods.");

    const int rows = static_cast<int>(img.size(0));
    const int cols = static_cast<int>(img.size(1));

    // Use int32 for labels unless your bilde::t_label differs.
    auto labels = torch::empty({rows, cols},
                               torch::TensorOptions().dtype(torch::kInt32).device(torch::kCPU));

    py::buffer_info in_buf    = tensor_as_buffer_info<bilde::t_uint8>(img);
    py::buffer_info label_buf = tensor_as_buffer_info<bilde::t_label>(labels);

    bilde::Buffer<bilde::t_uint8> inputBuffer(in_buf);
    bilde::Buffer<bilde::t_label> outputBuffer(label_buf);

    int nb_labels = bilde::operations::components::__labelConnectedComponents__<bilde::t_uint8>(
        outputBuffer, inputBuffer, neighborhood
    );

    return {labels, nb_labels};
}

std::pair<torch::Tensor, int> label_connected_components_equal_torch(torch::Tensor img, int neighborhood = 8) {
    require_2d_cpu_contig(img, "img");
    if (img.scalar_type() != torch::kUInt8) throw std::runtime_error("img must be torch.uint8");
    if (neighborhood != 4 && neighborhood != 8) throw std::runtime_error("not_implemented for other than 4 or 8 neighborhoods.");

    const int rows = static_cast<int>(img.size(0));
    const int cols = static_cast<int>(img.size(1));

    auto labels = torch::empty({rows, cols},
                               torch::TensorOptions().dtype(torch::kInt32).device(torch::kCPU));

    py::buffer_info in_buf    = tensor_as_buffer_info<bilde::t_uint8>(img);
    py::buffer_info label_buf = tensor_as_buffer_info<bilde::t_label>(labels);

    bilde::Buffer<bilde::t_uint8> inputBuffer(in_buf);
    bilde::Buffer<bilde::t_label> outputBuffer(label_buf);

    int nb_labels = bilde::operations::components::__labelEqualConnectedComponents__<bilde::t_uint8>(
        outputBuffer, inputBuffer, neighborhood
    );

    return {labels, nb_labels};
}

std::tuple<torch::Tensor, torch::Tensor, std::vector<std::string>>
get_connected_components_and_features_torch(torch::Tensor img, int neighborhood = 8) {
    require_2d_cpu_contig(img, "img");
    if (img.scalar_type() != torch::kUInt8) throw std::runtime_error("img must be torch.uint8");
    if (neighborhood != 4 && neighborhood != 8) throw std::runtime_error("not_implemented for other than 4 or 8 neighborhoods.");

    const int rows = static_cast<int>(img.size(0));
    const int cols = static_cast<int>(img.size(1));

    static_assert(sizeof(bilde::t_real32) == 4, "t_real32 must be 4 bytes");

    auto labels = torch::empty({rows, cols},
                               torch::TensorOptions().dtype(torch::kInt32).device(torch::kCPU));

    py::buffer_info in_buf    = tensor_as_buffer_info<bilde::t_uint8>(img);
    py::buffer_info label_buf = tensor_as_buffer_info<bilde::t_label>(labels);

    bilde::Buffer<bilde::t_uint8> inputBuffer(in_buf);
    bilde::Buffer<bilde::t_label> labelBuffer(label_buf);

    int nb_components = bilde::operations::components::__labelConnectedComponents__<bilde::t_uint8>(
        labelBuffer, inputBuffer, neighborhood
    );

    auto feats = torch::empty({nb_components, 10},
                              torch::TensorOptions().dtype(torch::kFloat32).device(torch::kCPU));

    py::buffer_info feats_buf = tensor_as_buffer_info<bilde::t_real32>(feats);
    bilde::Buffer<bilde::t_real32> outputFeaturesBuffer(feats_buf);

    bilde::operations::components::__getLabeledComponentFeatures__(outputFeaturesBuffer, labelBuffer);

    std::vector<std::string> feature_names = {
        "label", "nb_pixels", "left", "right", "top", "bottom", "sum_x", "sum_y", "last_x", "last_y"
    };

    return {labels, feats, feature_names};
}

// ---------------------------- module ----------------------------

// Use TORCH_EXTENSION_NAME so this compiles cleanly via torch.utils.cpp_extension.load / load_inline.
//PYBIND11_MODULE(TORCH_EXTENSION_NAME, m) {
PYBIND11_MODULE(ptbilde, m) {
    m.def("__get_version__", &__get_version__, "Return Bilde version string");

    m.def("lbp_transform", &lbp_transform_torch,
          py::arg("img"),
          py::arg("nb_samples") = 8,
          py::arg("radius") = 1.0,
          py::arg("interpolation") = "bilinear",
          py::arg("cmp_operation") = "one-tail",
          py::arg("cmp_threshold") = "otsu",
          "LBP transform for a 2D uint8 CPU contiguous torch.Tensor");

    m.def("lbp_features", &lbp_features_torch,
          py::arg("img"),
          py::arg("nb_samples") = 8,
          py::arg("radii") = std::vector<double>({1., 2., 3.}),
          py::arg("interpolation") = "bilinear",
          py::arg("cmp_operation") = "one-tail",
          py::arg("cmp_threshold") = "otsu",
          py::arg("num_threads") = 20,
          "LBP hist features for a 2D uint8 CPU contiguous torch.Tensor");

    m.def("enhance_grayscale", &enhance_grayscale_torch,
          py::arg("img"),
          py::arg("bitDepth") = 8,
          py::arg("mode") = "equalise",
          py::arg("windowWidth") = 51,
          py::arg("windowHeight") = 51,
          py::arg("globalHistogramCoeficient") = 0,
          py::arg("localHistogramCoeficient") = 1,
          py::arg("topQuantile") = .95f,
          py::arg("bottomQuantile") = .05f,
          "Local histogram equalisation for 2D uint8 CPU contiguous torch.Tensor");

    m.def("label_connected_components", &label_connected_components_torch,
          py::arg("img"),
          py::arg("neighborhood") = 8,
          "Connected components labeling for 2D uint8 CPU contiguous torch.Tensor");

    m.def("label_connected_components_equal", &label_connected_components_equal_torch,
          py::arg("img"),
          py::arg("neighborhood") = 8,
          "Equal-value connected components labeling for 2D uint8 CPU contiguous torch.Tensor");

    m.def("get_connected_components_and_features", &get_connected_components_and_features_torch,
          py::arg("img"),
          py::arg("neighborhood") = 8,
          "Return (labels:int32 [H,W], features:float32 [N,10], feature_names:list[str])");
}
