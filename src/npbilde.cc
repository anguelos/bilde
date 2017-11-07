#include <boost/python/tuple.hpp>
#include <boost/python.hpp>
#include <boost/numpy.hpp>
#include <iostream>

#include "bilde.hpp"
namespace p = boost::python;
namespace np = boost::numpy;
//char const* greet(){
//   return "hello, world";
//}

void translate_str_to_exception(const char* msg){
    // Use the Python 'C' API to set up an exception object
    PyErr_SetString(PyExc_RuntimeError, msg);
}

std::string greet(){
    return "hello world";
}

p::tuple getConnectedComponents(np::ndarray inputImage,int neighbors=8){
    if(neighbors!=4 && neighbors!=8){
        throw "getConnectedComponents neighbors must be 4 or 8";
    }
    np::ndarray res=np::empty(p::make_tuple(inputImage.shape(0),inputImage.shape(1)),np::dtype::get_builtin<bilde::t_label>());
    int nbComponents=bilde::operations::components::__labelConnectedComponents__<bilde::t_uint8>(&res,&inputImage,neighbors);
    return p::make_tuple(res,nbComponents);
}
BOOST_PYTHON_FUNCTION_OVERLOADS(getConnectedComponents_overloads, getConnectedComponents,1, 2)


const char* getLbpTransformDocString="Returns the lbp transform of a grayscale image.\n\n"
        "param inputImage numpy.ndarray(,dtype='uint8'): a two dimentional numpy array of uint8 containing a grayscale image.\n"
        "param nbSamples int : an integer depicting the number of samples must be less than 64.\n"
        "param radius float : The radius of the transform.\n"
        "param sampling string : one of ['bilinear','nearEuclidean'].\n"
        "param threshold string : one of ['otsu','0','1',...,'256'].\n"
        ;
//np::ndarray getLbpTransform(np::ndarray& inputImage,int nbSamples=8,double radius=3,std::string sampling="bilinear",std::string threshold="otsu"){
np::ndarray getLbpTransform(np::ndarray& inputImage,int nbSamples,double radius,std::string sampling,std::string threshold){
    if(nbSamples<=8){
        np::ndarray res=np::empty(p::make_tuple(inputImage.shape(0),inputImage.shape(1)),np::dtype::get_builtin<bilde::t_uint8>());
        bilde::operations::lbp::__lbpTransform__<bilde::t_uint8>(&res,&inputImage,nbSamples,radius,sampling,"one-tail",threshold);
        return res;
    }else if(nbSamples<=16){
        np::ndarray res=np::empty(p::make_tuple(inputImage.shape(0),inputImage.shape(1)),np::dtype::get_builtin<bilde::t_uint16>());
        bilde::operations::lbp::__lbpTransform__<bilde::t_uint16>(&res,&inputImage,nbSamples,radius,sampling,"one-tail",threshold);
        return res;
    }else if(nbSamples<=32){
        np::ndarray res=np::empty(p::make_tuple(inputImage.shape(0),inputImage.shape(1)),np::dtype::get_builtin<bilde::t_uint32>());
        bilde::operations::lbp::__lbpTransform__<bilde::t_uint32>(&res,&inputImage,nbSamples,radius,sampling,"one-tail",threshold);
        return res;
    }else if(nbSamples<=64){
        np::ndarray res=np::empty(p::make_tuple(inputImage.shape(0),inputImage.shape(1)),np::dtype::get_builtin<bilde::t_uint64>());
        bilde::operations::lbp::__lbpTransform__<bilde::t_uint64>(&res,&inputImage,nbSamples,radius,sampling,"one-tail",threshold);
        return res;
    }else{
        throw "getLbpTransform, nbSamples must be less than 64!";
    }
}
//BOOST_PYTHON_FUNCTION_OVERLOADS(getLbpTransform_overloads, getLbpTransform,1, 5)

BOOST_PYTHON_MODULE(npbilde){
    Py_Initialize();
    np::initialize();
    p::register_exception_translator<const char*>(&translate_str_to_exception);
    p::def("greet", greet);
    //p::def("getLbpTransform", getLbpTransform,getLbpTransform_overloads());
    p::def("getLbpTransform", getLbpTransform);
    p::def("getConnectedComponents", getConnectedComponents,getConnectedComponents_overloads());
    //p::def("getLbpTransform", getLbpTransform,getLbpTransformDocString);
}
