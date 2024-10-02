#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>  // <-- Include this for std::vector support
#include <vector>
#include <stdexcept>
#include "../include/bilde.hpp"

//namespace py = pybind11;
#include <iostream>


/* 
pybind11::array_t<bilde::t_label> bwlabel(pybind11::array_t<uint8_t> img, int mode=8) {
    // Ensure the input is a 2D array
    if (img.ndim() != 2) {
        throw std::runtime_error("Input image must be a 2D array");
    }

    // Ensure the input array is of type uint8
    if (img.dtype().kind() != 'u' || img.dtype().itemsize() != 1) {
        throw std::runtime_error("Input image must be of type uint8");
    }

    // Get the shape of the array (rows and columns)
    pybind11::buffer_info in_buf = img.request();
    int rows = in_buf.shape[0];
    int cols = in_buf.shape[1];
    
    pybind11::array_t<uint32_t> output_img = pybind11::array_t<uint32_t>({rows, cols});
    pybind11::buffer_info out_buf = output_img.request();

    bilde::Buffer<bilde::t_uint8> inputBuffer(in_buf);
    //bilde::Buffer<bilde::t_uint8> outputBuffer(img);
    bilde::Buffer<bilde::t_uint8> outputBuffer(out_buf);
    //bilde::operations::components::__labelConnectedComponents__<bilde::uint8>()
    __labelConnectedComponents__<T>(out, in, neighborhood)
    return output_img;
}
*/

pybind11::array_t<uint8_t> lbp_image(pybind11::array_t<uint8_t> img, int nb_samples=8, double radius=1.0, std::string interpolation="bilinear", std::string cmp_operation="one-tail", std::string cmp_threshold="otsu") {
    // Ensure the input is a 2D array
    if (img.ndim() != 2) {
        throw std::runtime_error("Input image must be a 2D array");
    }

    // Ensure the input array is of type uint8
    if (img.dtype().kind() != 'u' || img.dtype().itemsize() != 1) {
        throw std::runtime_error("Input image must be of type uint8");
    }

    // Get the shape of the array (rows and columns)
    pybind11::buffer_info in_buf = img.request();
    int rows = in_buf.shape[0];
    int cols = in_buf.shape[1];
    
    pybind11::array_t<uint8_t> output_img = pybind11::array_t<uint8_t>({rows, cols});
    pybind11::buffer_info out_buf = output_img.request();

    bilde::Buffer<bilde::t_uint8> inputBuffer(in_buf);
    bilde::Buffer<bilde::t_uint8> outputBuffer(out_buf);

    auto lbp=bilde::operations::lbp::__lbp_util__::LbpIterator<bilde::t_uint8>(inputBuffer, nb_samples,radius,interpolation,cmp_operation,cmp_threshold);
    lbp.applyLBPTransform(outputBuffer);

    return output_img;
}


std::vector<int> lbp_features(pybind11::array_t<uint8_t> img, int nb_samples, std::vector<double> radii, std::string interpolation, std::string cmp_operation, std::string cmp_threshold) {
    // Ensure the input is a 2D array
    if (img.ndim() != 2) {
        throw std::runtime_error("Input image must be a 2D array");
    }

    // Ensure the input array is of type uint8
    if (img.dtype().kind() != 'u' || img.dtype().itemsize() != 1) {
        throw std::runtime_error("Input image must be of type uint8");
    }
    if (nb_samples > 8 || nb_samples < 1){
        throw std::runtime_error("not_implemented for more than 8 samples.");
    }

    // Get the shape of the array (rows and columns)
    pybind11::buffer_info in_buf = img.request();
    int rows = in_buf.shape[0];
    int cols = in_buf.shape[1];
    
    pybind11::array_t<uint8_t> output_img = pybind11::array_t<uint8_t>({rows, cols});  // creating a temporary buffer
    pybind11::buffer_info out_buf = output_img.request();
    bilde::container::ContainerWrapper<pybind11::buffer_info, bilde::t_uint8>::getFirstRow(in_buf);
    bilde::Buffer<bilde::t_uint8> inputBuffer(in_buf);
    bilde::Buffer<bilde::t_uint8> outputBuffer(out_buf);
    std::vector<int> results;
    for (const double& radius : radii){
        auto lbp=bilde::operations::lbp::__lbp_util__::LbpIterator<bilde::t_uint8>(inputBuffer, nb_samples,radius,interpolation,cmp_operation,cmp_threshold);
        lbp.applyLBPTransform(outputBuffer);
        std::vector<int> h=bilde::operations::essential::getHistogram(outputBuffer);
        results.insert(results.end(), h.begin(), h.end());
    }
    return results;
}



PYBIND11_MODULE(bilde, m) {
    // Expose the function to Python
    m.def("lbp_transform", &lbp_image, "A function that processes a 2D uint8 NumPy array");
    m.def("lbp_features", &lbp_features, "A function that processes a 2D uint8 NumPy array");
    //m.def("test_arg", &test_params, "A function that prints info about an array");
}