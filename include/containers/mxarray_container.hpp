/*
 * iplimage_container.hpp
 *
 *  Created on: Mar 5, 2012
 *      Author: anguelos
 */

#ifndef IPLIMAGE_CONTAINER_HPP_
#define IPLIMAGE_CONTAINER_HPP_

#include "mex.h"

namespace bilde {
namespace container {

namespace __mx_bilde_util__ {

template<typename B> int c2mxType() {
	return -1;
}

template<> int c2mxType<unsigned char>() {
	return mxUINT8_CLASS;
}

template<> int c2mxType<signed char>() {
	return mxINT8_CLASS;
}

template<> int c2mxType<unsigned short int>() {
	return mxUINT16_CLASS;
}

template<> int c2mxType<signed short int>() {
	return mxINT16_CLASS;
}

template<> int c2mxType<signed int>() {
	return mxINT32_CLASS;
}

template<> int c2mxType<unsigned int>() {
        return mxUINT32_CLASS;
}

template<> int c2mxType<signed long int>() {
    return mxINT64_CLASS;
}

template<> int c2mxType<unsigned long int>() {
    return mxUINT64_CLASS;
}

template<> int c2mxType<float>() {
	return mxSINGLE_CLASS;
}

template<> int c2mxType<double>() {
	return mxDOUBLE_CLASS;
}

}

template<typename PIXELT> struct ContainerWrapper<mxArray, PIXELT> {
//Wrapper example.
//This wrapper is used to "wrap" an IplImage (opencv legacy type image) in a Buffer
//AKA used for copy constructors

//BASIC API CLASS METHODS
static bool isDefined() {
	int depth = __mx_bilde_util__::c2mxType<PIXELT>();
	return depth==mxUINT8_CLASS||depth==mxINT8_CLASS||
        mxUINT16_CLASS||depth==mxINT16_CLASS||
        mxUINT32_CLASS||depth==mxINT32_CLASS||
        mxUINT64_CLASS||depth==mxINT64_CLASS||
        mxSINGLE_CLASS||depth==mxDOUBLE_CLASS;
}

//BASIC API INSTANCE METHODS
static bool canWrap(const mxArray& cont) {return (__mx_bilde_util__::c2mxType<PIXELT>()==mxGetClassID(&cont))&&(mxGetNumberOfDimensions(&cont)>2);}
    
    static bool canTakeDataOwnership(const mxArray& cont) {return false;}

    static boost::shared_ptr<t_uint8> takeStorageOwnership(const mxArray& cont) {return boost::shared_ptr<t_uint8>();}

static boost::shared_ptr<t_uint8> deepCopyStorage(const mxArray& cont) {
	//its a lot simpler to deep copy a buffer to a buffer than an ipl image to a buffer
	if(!canWrap(cont)){
		throw "deepCopyStorage for mxArray only deepcopies wrapable types";
	}
	Buffer<PIXELT> tmp(cont);
	//dont forget to set Buffer.data to point to the apropriate place
	t_uint8* outdata=new t_uint8[tmp.height*tmp.__linestride__*sizeof(PIXELT)];
	for(int y=0;y<tmp.height;y++){
		std::memcpy(outdata+tmp.__linestride__*y,tmp.getRow(y),tmp.width*sizeof(PIXELT));
	}
	return boost::shared_ptr<PIXELT>(outdata);
}
    
static const PIXELT* getFirstRow(const mxArray& cont) {
    return (PIXELT*)(mxGetData(&cont));
}

static PIXELT* getFirstRow(mxArray& cont) {
    return mxGetData(cont);
}

static t_sz getWidth(const mxArray& cont) {
    return mxGetDimensions(&cont)[0];
}
static t_sz getHeight(const mxArray&cont) {
    return mxGetDimensions(&cont)[1];
}
static t_sz getByteLinestride(const mxArray&cont) {
    return getWidth(cont)*sizeof(PIXELT);
}
};


}

}

#endif /* IPLIMAGE_CONTAINER_HPP_ */
