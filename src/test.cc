#include <boost/python.hpp>
#include <boost/numpy.hpp>
#include <iostream>
#include "bilde.hpp"
namespace p = boost::python;
namespace np = boost::numpy;
//char const* greet(){
//   return "hello, world";
//}

std::string greet(){
    return "hello, world";
}

np::ndarray get123(){
    //p::object tu = p::make_tuple('a','b','c') ;
    //np::ndarray example_tuple = np::array (tu) ;
    //p::object tu = p::make_tuple(1,2,3,4,5) ;
    p::object tu = p::make_tuple(1,2,3,4,5) ;
    np::ndarray example_tuple = np::array (tu) ;
    return example_tuple;
}


BOOST_PYTHON_MODULE(test){
    np::initialize();
    p::def("greet", greet);
    p::def("get123", get123);

}

