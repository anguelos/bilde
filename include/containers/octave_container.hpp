/*
 * octave_container.hpp
 *
 *  Created on: Jul 7, 2013
 *      Author: anguelos
 */

#ifndef OCTAVE_CONTAINER_HPP_
#define OCTAVE_CONTAINER_HPP_

//#define octave_oct_h

#ifdef octave_oct_h
namespace bilde {
namespace container {

namespace __oct_util__ {


template <typename T> struct __OctavePixelType__{
};



template <> struct __OctavePixelType__<signed char>{
	typedef octave_int8 t;
	typedef intNDArray<octave_int8> cont_t;
};


template <> struct __OctavePixelType__<signed short int>{
	typedef octave_int16 t;
	typedef intNDArray<octave_int16> cont_t;
};

template <> struct __OctavePixelType__<signed int>{
	typedef octave_int32 t;
	typedef intNDArray<octave_int32> cont_t;
};

template <> struct __OctavePixelType__<int64_t>{
	typedef octave_int64 t;
	typedef intNDArray<octave_int64> cont_t;
};

template <> struct __OctavePixelType__<unsigned char>{
	typedef octave_uint8 t;
	typedef intNDArray<octave_uint8> cont_t;
};

template <> struct __OctavePixelType__<unsigned short int>{
	typedef octave_uint16 t;
	typedef intNDArray<octave_uint16> cont_t;
};

template <> struct __OctavePixelType__<unsigned int>{
	typedef octave_uint32 t;
	typedef intNDArray<octave_uint32> cont_t;
};

template <> struct __OctavePixelType__<uint64_t>{
	typedef octave_uint64 t;
	typedef intNDArray<octave_uint64> cont_t;
};

template <> struct __OctavePixelType__<float>{
	typedef float t;
	typedef FloatNDArray cont_t;
};

template <> struct __OctavePixelType__<double>{
	typedef double t;
	typedef NDArray cont_t;
};



}

template<typename PIXELT> struct ContainerWrapper<typename __oct_util__::__OctavePixelType__<PIXELT>::cont_t , PIXELT> {

typedef intNDArray<typename __oct_util__::__OctavePixelType__<PIXELT>::t > CONT_TYPE;

//BASIC API CLASS METHODS
static bool isDefined(){
	return true;
}
//BASIC API INSTANCE METHODS
static bool canWrap(const CONT_TYPE& cont) {return cont.dims().length()==2;}

static bool canTakeDataOwnership(const CONT_TYPE& cont) {return false;}

static boost::shared_ptr<t_uint8> takeStorageOwnership(const CONT_TYPE& cont) {return boost::shared_ptr<t_uint8>();}

static boost::shared_ptr<t_uint8> deepCopyStorage(const CONT_TYPE& cont) {
	//its a lot simpler to deep copy a buffer to a buffer than an ipl image to a buffer
	if(!canWrap(cont)){
        throw "ocyave_container.hpp can't wrap image";
	}
	Buffer<PIXELT> tmp(cont);
	//dont forget to set Buffer.data to point to the apropriate place
	t_uint8* outdata=new t_uint8[tmp.height*tmp.__linestride__*sizeof(PIXELT)];
	for(int y=0;y<tmp.height;y++){
		std::memcpy(outdata+tmp.__linestride__*y,tmp.getRow(y),tmp.width*sizeof(PIXELT));
	}
	return boost::shared_ptr<PIXELT>(outdata);
}
static const PIXELT* getFirstRow(const CONT_TYPE&cont) {
    return (PIXELT*)(cont.data());
}
static PIXELT* getFirstRow(CONT_TYPE&cont) {
    return cont.data();
}
static t_sz getWidth(const CONT_TYPE& cont) {
    return cont.dim2();
}
static t_sz getHeight(const CONT_TYPE&cont) {
    return cont.dim1();
}
static t_sz getByteLinestride(const CONT_TYPE&cont) {
    return cont.dim2()*sizeof(PIXELT);
}
};

}
}
#endif /* octave_oct_h */


#endif /* OCTAVE_CONTAINER_HPP_ */
