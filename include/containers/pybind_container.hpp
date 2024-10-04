/*
 * iplimage_container.hpp
 *
 *  Created on: Mar 5, 2012
 *      Author: anguelos
 */

#ifndef PYBIND11_CONTAINER_HPP_
#define PYBIND11_CONTAINER_HPP_

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

namespace bilde {
namespace container {

namespace __pybind11_util__ {

template<typename B> int c2iplType() {
        return -1;
}

template<> int c2iplType<unsigned char>() {
        return IPL_DEPTH_8U;
}

template<> int c2iplType<signed char>() {
        return IPL_DEPTH_8S;
}

template<> int c2iplType<unsigned short int>() {
        return IPL_DEPTH_16U;
}

template<> int c2iplType<signed short int>() {
        return IPL_DEPTH_16S;
}

template<> int c2iplType<signed int>() {
        return IPL_DEPTH_32S;
}

template<> int c2iplType<float>() {
        return IPL_DEPTH_32F;
}

template<> int c2iplType<double>() {
        return IPL_DEPTH_64F;
}

}



template<typename PIXELT> struct ContainerWrapper<pybind11::array_t<PIXELT>, PIXELT> {
//Wrapper example.
//This wrapper is used to "wrap" an IplImage (opencv legacy type image) in a Buffer
//AKA used for copy constructors

//BASIC API CLASS METHODS
static bool isDefined() {
	int depth = __ipl_util__::c2iplType<PIXELT>();
	return depth==IPL_DEPTH_8U||depth==IPL_DEPTH_8S||depth==IPL_DEPTH_16S||depth==IPL_DEPTH_16U||depth==IPL_DEPTH_32S||depth==IPL_DEPTH_32F||depth==IPL_DEPTH_64F;
}

//BASIC API INSTANCE METHODS
static bool canWrap(const py::array_t<PIXELT, py::array::c_style | py::array::forcecast> array& cont) {return cont.ndim()== 2; }
static bool canTakeDataOwnership(const py::array_t<PIXELT, py::array::c_style | py::array::forcecast> array& cont) {return false;}
static boost::shared_ptr<t_uint8> takeStorageOwnership(const py::array_t<PIXELT, py::array::c_style | py::array::forcecast> array& cont) {return boost::shared_ptr<t_uint8>();}
static boost::shared_ptr<t_uint8> deepCopyStorage(const py::array_t<PIXELT, py::array::c_style | py::array::forcecast> array& cont) {
	//its a lot simpler to deep copy a buffer to a buffer than an ipl image to a buffer
	if(!canWrap(cont)){
		throw "deepCopyStorage for boost::numpy::ndarray only deepcopies wrapable types";
	}
	Buffer<PIXELT> tmp(cont);
	//dont forget to set Buffer.data to point to the apropriate place
	t_uint8* outdata=new t_uint8[tmp.height*tmp.__linestride__*sizeof(PIXELT)];
	for(int y=0;y<tmp.height;y++){
		std::memcpy(outdata+tmp.__linestride__*y,tmp.getRow(y),tmp.width*sizeof(PIXELT));
	}
	return boost::shared_ptr<PIXELT>(outdata);
}

static const PIXELT* getFirstRow(const IplImage&cont) {
    return (const PIXELT*)(cont.data());
}

static PIXELT* getFirstRow(IplImage&cont) {
    return (PIXELT*)cont.data();
}

static t_sz getWidth(const IplImage& cont) {
    return array.shape()[1];
}

static t_sz getHeight(const IplImage&cont) {
    return array.shape()[0];
}

static t_sz getByteLinestride(const IplImage&cont) {
    array.shape()[1]*sizeof(PIXELT);
}

};


}

}

#endif /* IPLIMAGE_CONTAINER_HPP_ */
