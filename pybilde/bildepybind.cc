#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>  // <-- Include this for std::vector support
#include <vector>
#include <cmath>
#include <stdexcept>
#include <thread>

#include "../include/bilde.hpp"

namespace py = pybind11;


std::string __version__(){
    return BILDE_VERSION;
}


py::array_t<uint8_t> lbp_image(py::array_t<uint8_t> img, int nb_samples=8, double radius=1.0, std::string interpolation="bilinear", std::string cmp_operation="one-tail", std::string cmp_threshold="otsu") {
    // Ensure the input is a 2D array
    if (img.ndim() != 2) {
        throw std::runtime_error("Input image must be a 2D array");
    }

    // Ensure the input array is of type uint8
    if (img.dtype().kind() != 'u' || img.dtype().itemsize() != 1) {
        throw std::runtime_error("Input image must be of type uint8");
    }

    // Get the shape of the array (rows and columns)
    py::buffer_info in_buf = img.request();
    int rows = in_buf.shape[0];
    int cols = in_buf.shape[1];

    if (((radius + 3) * 2 >= rows ) || ((radius + 3) * 2 >= cols)){
        throw std::runtime_error("The radius is too big for the image size.");
    }

    py::array_t<uint8_t> output_img = py::array_t<uint8_t>({rows, cols});
    py::buffer_info out_buf = output_img.request();

    bilde::Buffer<bilde::t_uint8> inputBuffer(in_buf);
    bilde::Buffer<bilde::t_uint8> outputBuffer(out_buf);

    auto lbp=bilde::operations::lbp::__lbp_util__::LbpIterator<bilde::t_uint8>(inputBuffer, nb_samples,radius,interpolation,cmp_operation,cmp_threshold);
    lbp.applyLBPTransform(outputBuffer);

    return output_img;
}


py::array_t<int> lbp_features(py::array_t<uint8_t> img, int nb_samples, std::vector<double> radii, std::string interpolation, std::string cmp_operation, std::string cmp_threshold) {
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
    py::buffer_info in_buf = img.request();
    int rows = in_buf.shape[0];
    int cols = in_buf.shape[1];
    
    py::array_t<uint8_t> output_img = py::array_t<uint8_t>({rows, cols});  // creating a temporary buffer
    py::buffer_info out_buf = output_img.request();

    bilde::Buffer<bilde::t_uint8> inputBuffer(in_buf);
    std::vector<int> results;
    for (const double& radius : radii){
        if (((radius + 3) * 2 >= rows ) || ((radius + 3) * 2 >= cols)){
            throw std::runtime_error("The radius is too big for the image size.");
        }
        int erode = int(std::ceil(radius));
        auto lbp=bilde::operations::lbp::__lbp_util__::LbpIterator<bilde::t_uint8>(in_buf, nb_samples, radius, interpolation, cmp_operation, cmp_threshold);
        bilde::Buffer<bilde::t_uint8> chopped_out_buf=bilde::Buffer<bilde::t_uint8>(out_buf, erode, erode, cols - (erode + 1), rows - (erode + 1));
        lbp.applyLBPTransform(chopped_out_buf);
        std::vector<int> h=bilde::operations::essential::getHistogram(chopped_out_buf);
        results.insert(results.end(), h.begin(), h.end());
    }
    return py::array_t<int>(results.size(), results.data());
}


py::array_t<int> lbp_features_multithreaded(py::array_t<uint8_t> img, int nb_samples, std::vector<double> radii, std::string interpolation, std::string cmp_operation, std::string cmp_threshold, int n_threads) {
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

    py::buffer_info in_buf = img.request();
    int rows = in_buf.shape[0];
    int cols = in_buf.shape[1];

    std::vector<std::thread> threads;
    std::vector<std::vector<int>> indexed_results(radii.size());

    int current_radius_idx = 0;

    for(double radius : radii){
        if (((radius + 3) * 2 >= rows ) || ((radius + 3) * 2 >= cols)){
            throw std::runtime_error("The radius is too big for the image size.");
        }
    }

    while(current_radius_idx < radii.size()){
        for (int i = 0; i < n_threads && current_radius_idx+i < radii.size() ; i++){
            threads.push_back(std::thread([n_threads, current_radius_idx, i, &indexed_results, &in_buf, nb_samples, radii, interpolation, cmp_operation, cmp_threshold](){
                int rows = in_buf.shape[0];
                int cols = in_buf.shape[1];
                // Get the shape of the array (rows and columns)
                double radius = radii[current_radius_idx+i];
                int erode = int(std::ceil(radius));
                auto lbp=bilde::operations::lbp::__lbp_util__::LbpIterator<bilde::t_uint8>(in_buf, nb_samples, radius, interpolation, cmp_operation, cmp_threshold);
                bilde::Buffer<bilde::t_uint8> chopped_out_buf=bilde::Buffer<bilde::t_uint8>(in_buf, erode, erode, cols - (erode + 1), rows - (erode + 1));
                lbp.applyLBPTransform(chopped_out_buf);
                indexed_results[current_radius_idx+i]=bilde::operations::essential::getHistogram(chopped_out_buf);
                //std::cerr<<"Thread "<<i<<"/"<<n_threads<<" Radius:"<<radius<<" finished"<<std::endl;
            }));
        }
        //std::cerr<<"Before Join "<<current_radius_idx<<std::endl;
        for (std::thread &t : threads){
            t.join();
        }
        //std::cerr<<"After Join "<<current_radius_idx<<std::endl;
        threads.clear();
        //std::cerr<<"After clear "<<current_radius_idx<<std::endl;
        current_radius_idx+=n_threads;
    }
    std::vector<int> results;
    for (auto& r : indexed_results){
        results.insert(results.end(), r.begin(), r.end());
    }
    return py::array_t<int>(results.size(), results.data());
}


py::array_t<bilde::t_uint8> enhance_grayscale(py::array_t<bilde::t_uint8> img, int bitDepth, std::string mode, int windowWidth, int windowHeight, int globalHistogramCoeficient, int localHistogramCoeficient) {
    //throw std::runtime_error("not_implemented");
    // Ensure the input is a 2D array
    if (img.ndim() != 2) {
        throw std::runtime_error("Input image must be a 2D array");
    }

    // Ensure the input array is of type uint8
    if (img.dtype().kind() != 'u' || img.dtype().itemsize() != 1) {
        throw std::runtime_error("Input image must be of type uint8");
    }

    // Get the shape of the array (rows and columns)
    py::buffer_info in_buf = img.request();
    int rows = in_buf.shape[0];
    int cols = in_buf.shape[1];
    
    py::array_t<bilde::t_uint8> output_img = py::array_t<bilde::t_uint8>({rows, cols});  // creating a temporary buffer
    py::buffer_info out_buf = output_img.request();

    bilde::Buffer<bilde::t_uint8> inputBuffer(in_buf);
    bilde::Buffer<bilde::t_uint8> outputBuffer(out_buf);

    //bilde::methods::enhance_grayscale::__GrayEnhancer__<bilde::t_uint8, 8> enhancer(mode, bitDepth, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient);
    bilde::methods::enhance_grayscale::enhaceGray(inputBuffer, outputBuffer, bitDepth, mode, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient);
    //enhancer.enhaceGray(inputBuffer, outputBuffer);
    return output_img;
}


py::array_t<bilde::t_label> label_connected_components(py::array_t<uint8_t> img, int neighborhood) {
    // Ensure the input is a 2D array
    if (img.ndim() != 2) {
        throw std::runtime_error("Input image must be a 2D array");
    }

    // Ensure the input array is of type uint8
    if (img.dtype().kind() != 'u' || img.dtype().itemsize() != 1) {
        throw std::runtime_error("Input image must be of type uint8");
    }
    if (neighborhood != 4 && neighborhood != 8){
        throw std::runtime_error("not_implemented for other than 4 or 8 neighborhoods.");
    }

    // Get the shape of the array (rows and columns)
    py::buffer_info in_buf = img.request();
    bilde::Buffer<bilde::t_uint8> inputBuffer(in_buf);

    int rows = in_buf.shape[0];
    int cols = in_buf.shape[1];
    
    py::array_t<bilde::t_label> output_img = py::array_t<bilde::t_label>({rows, cols});  // creating a temporary buffer
    py::buffer_info out_buf = output_img.request();
    bilde::Buffer<bilde::t_label> outputBuffer(out_buf);
    bilde::operations::components::__labelConnectedComponents__<bilde::t_uint8>(outputBuffer, inputBuffer, neighborhood);
    return output_img;    
}


std::pair<py::array_t<bilde::t_label>, py::array_t<bilde::t_real32>> get_components_and_features(py::array_t<uint8_t> img, int neighborhood){
    throw std::runtime_error("not_implemented");
    // Ensure the input is a 2D array
    if (img.ndim() != 2) {
        throw std::runtime_error("Input image must be a 2D array");
    }

    // Ensure the input array is of type uint8
    if (img.dtype().kind() != 'u' || img.dtype().itemsize() != 1) {
        throw std::runtime_error("Input image must be of type uint8");
    }
    if (neighborhood != 4 && neighborhood != 8){
        throw std::runtime_error("not_implemented for other than 4 or 8 neighborhoods.");
    }

    // Get the shape of the array (rows and columns)
    py::buffer_info in_buf = img.request();
    bilde::Buffer<bilde::t_uint8> inputBuffer(in_buf);

    int rows = in_buf.shape[0];
    int cols = in_buf.shape[1];
    
    py::array_t<bilde::t_label> label_img = py::array_t<bilde::t_label>({rows, cols});  // creating a temporary buffer
    py::buffer_info label_buf = label_img.request();
    bilde::Buffer<bilde::t_label> labelBuffer(label_buf);
    int nb_components = bilde::operations::components::__labelConnectedComponents__<bilde::t_uint8>(labelBuffer, inputBuffer, neighborhood);
    
    py::array_t<bilde::t_real32> output_array = py::array_t<bilde::t_real32>({10, nb_components});  // creating a temporary buffer
    py::buffer_info out_features_buf = output_array.request();
    bilde::Buffer<bilde::t_real32> outputFeaturesBuffer(out_features_buf);
    bilde::operations::components::__getLabeledComponentFeatures__(outputFeaturesBuffer, labelBuffer);

    //label, nb_pixels, left, right, top, bottom, sum_x, sum_y, last_x, last_y
    return std::pair(label_img, output_array);
}


py::array_t<bilde::t_uint8> lof_binarize(py::array_t<bilde::t_uint8> img, int bitDepth) {
    throw std::runtime_error("not_implemented");
    // Ensure the input is a 2D array
    if (img.ndim() != 2) {
        throw std::runtime_error("Input image must be a 2D array");
    }

    // Ensure the input array is of type uint8
    if (img.dtype().kind() != 'u' || img.dtype().itemsize() != 1) {
        throw std::runtime_error("Input image must be of type uint8");
    }

    // Get the shape of the array (rows and columns)
    py::buffer_info in_buf = img.request();
    int rows = in_buf.shape[0];
    int cols = in_buf.shape[1];
    
    py::array_t<bilde::t_uint8> output_img = py::array_t<bilde::t_uint8>({rows, cols});  // creating a temporary buffer
    py::buffer_info out_buf = output_img.request();

    bilde::Buffer<bilde::t_uint8> inputBuffer(in_buf);
    bilde::Buffer<bilde::t_uint8> outputBuffer(out_buf);

    bilde::methods::binarization::getLofBinarization(inputBuffer, outputBuffer, bitDepth);
    return output_img;
}


std::tuple<py::array_t<bilde::t_label>, py::array_t<bilde::t_label>, py::array_t<bilde::t_label>, 
py::array_t<bilde::t_label>, py::array_t<bilde::t_uint8> > 
//py::array_t<bilde::t_label> 
textline_segment(py::array_t<bilde::t_uint8> img,int windowWidth, int windowHeight,int tracerDencity,int compareDistance,int minimumLetterHeight,int maximumLetterHeight){
    throw std::runtime_error("not_implemented");
    // Ensure the input is a 2D array
    if (img.ndim() != 2) {
        throw std::runtime_error("Input image must be a 2D array");
    }

    // Ensure the input array is of type uint8
    if (img.dtype().kind() != 'u' || img.dtype().itemsize() != 1) {
        throw std::runtime_error("Input image must be of type uint8");
    }
    std::cerr<<"SEGM 1"<<std::endl;
    // Get the shape of the array (rows and columns)
    py::buffer_info in_buf = img.request();
    int rows = in_buf.shape[0];
    int cols = in_buf.shape[1];
    
    py::array_t<bilde::t_uint8> output_img = py::array_t<bilde::t_label>({rows, cols});  // creating a temporary buffer
    std::cerr<<"SEGM 2"<<std::endl;

    py::buffer_info out_buf = output_img.request();
    std::cerr<<"SEGM 3"<<std::endl;
    bilde::methods::textline_segmentation::TextlineSegmenter segm;
    std::cerr<<"SEGM 3.1"<<std::endl;

    auto [res, CC, LLA, LLC, blurred]=segm.call(out_buf, in_buf);
        std::cerr<<"SEGM 3.2"<<std::endl;
    
    py::array_t<bilde::t_label> res_array = py::array_t<bilde::t_label>({rows, cols});

    std::cerr<<"SEGM 3.22"<<std::endl;
    auto res_buf = res_array.request();
    
    std::cerr<<"SEGM 3.23"<<std::endl;
    std::cerr<<"PTR:"<<res_buf.ptr<<std::endl;
    std::cerr<<"Itemsize:"<<res_buf.itemsize<<std::endl;
    std::cerr<<"Format:"<<res_buf.format<<std::endl;
    std::cerr<<"NDIM:"<<res_buf.ndim<<", sz:"<<res_buf.shape.size()<<", str"<<res_buf.strides.size()<<std::endl;
    for(int n=0; n<res_buf.ndim; n++){
        std::cerr<<n<<": sz:"<<res_buf.shape[n]<<" str:"<<res_buf.strides[n]<<std::endl;
    }
    std::cerr<<"Buffer(res_buf):"<<bilde::Buffer<bilde::t_label>(res_buf)<<std::endl;
    std::cerr<<"SEGM 3.26"<<std::endl;
    res.copyTo(res_buf);
    std::cerr<<"SEGM 3.3"<<std::endl;
    
    py::array_t<bilde::t_label> CC_array = py::array_t<bilde::t_label>({rows, cols});
    CC.copyTo(CC_array.request());
    
    py::array_t<bilde::t_label> LLA_array = py::array_t<bilde::t_label>({rows, cols});
    LLA.copyTo(LLA_array.request());

    py::array_t<bilde::t_label> LLC_array = py::array_t<bilde::t_label>({rows, cols});
    LLC.copyTo(LLC_array.request());
    
    py::array_t<bilde::t_uint8> blurred_array = py::array_t<bilde::t_uint8>({rows, cols});
    blurred.copyTo(blurred_array.request());

    std::cerr<<"SEGM 3.3"<<std::endl;
    std::cerr<<"SEGM 3.5"<<std::endl;
    //segm.call(out_buf, in_buf);
    std::cerr<<"SEGM 4"<<std::endl;

    return {res_array, CC_array, LLA_array, LLC_array, blurred_array};
    //return res_array;
}


PYBIND11_MODULE(bilde, m) {
    // Expose the function to Python
    m.def("__version__", &__version__, "A function that returns the version of the Bilde library taken from the version.hpp file");
    m.def("lbp_transform", &lbp_image, py::arg("img"),  py::arg("nb_samples")=8, py::arg("radius")=1., py::arg("interpolation")="bilinear",
        py::arg("cmp_operation")="one-tail", py::arg("cmp_threshold")="otsu", "A function that creates an LBP image out of a Graylevel (uint8) ");
    //m.def("lbp_features", &lbp_features, py::arg("img"),  py::arg("nb_samples")=8, py::arg("radii")=std::vector<double>({1.,2.,3.}), py::arg("interpolation")="bilinear",
    //    py::arg("cmp_operation")="one-tail", py::arg("cmp_threshold")="otsu", "A function that processes a 2D uint8 NumPy array");
    m.def("lbp_features", &lbp_features_multithreaded, py::arg("img"),  py::arg("nb_samples")=8, py::arg("radii")=std::vector<double>({1.,2.,3.}), py::arg("interpolation")="bilinear",
        py::arg("cmp_operation")="one-tail", py::arg("cmp_threshold")="otsu", py::arg("num_threads")=20, "A function that processes a 2D uint8 NumPy array");
    m.def("enhance_grayscale", &enhance_grayscale, py::arg("img"),  py::arg("bitDepth")=8, py::arg("mode")="equalise", py::arg("windowWidth")=51, py::arg("windowHeight")=51, py::arg("globalHistogramCoeficient")=0, py::arg("localHistogramCoeficient")=1, "A function that doeas local histogram equlisation in order to enhance  a graylevel image");
    m.def("label_connected_components", &label_connected_components, py::arg("img"),  py::arg("neighborhood")=8, "A function that processes a 2D uint8 NumPy array");
    m.def("get_connected_components_and_features", &get_components_and_features, py::arg("img"),  py::arg("neighborhood")=8, "A function that returns labeled connected components and their features [label, nb_pixels, left, right, top, bottom, sum_x, sum_y, last_x, last_y]");
    m.def("lof_binarize", &lof_binarize, py::arg("img"),  py::arg("bitDepth")=8, "A binarization method based on the local otsu filter.");
    m.def("textline_segment", &textline_segment, py::arg("img"),  py::arg("windowWidth")=51, py::arg("windowHeight")=51, py::arg("tracerDencity")=1, py::arg("compareDistance")=50, py::arg("minimumLetterHeight")=10, py::arg("maximumLetterHeight")=500, "A function that processes a 2D uint8 NumPy array");
}
