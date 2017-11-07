/*
 * wrapping_api.hpp
 *
 *  Created on: Mar 3, 2012
 *      Author: anguelos
 */

#ifndef WRAPPING_API_HPP_
#define WRAPPING_API_HPP_


#include <boost/math/special_functions/round.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>

#include <climits>
#include <sstream>
#include <vector>
#include <map>
#include <list>
#include <iostream>
#include <cmath>
#include <cstdio>

namespace bilde {

///unsigned 64bit integer intended to index pixels in Buffers
typedef unsigned long long t_sz;

///unsigned char. Default pixel type
typedef unsigned char t_byte;

///Pixel type
typedef unsigned char t_uint8;
///Pixel type
typedef unsigned short int t_uint16;
///Pixel type
typedef unsigned int t_uint32;
///Pixel type
typedef unsigned long long int t_uint64;


///Pixel type
typedef signed char t_sint8;
///Pixel type
typedef signed short int t_sint16;
///Pixel type
typedef signed int t_sint32;
///Pixel type
typedef signed long long int t_sint64;

///Pixel type
typedef float t_real32;
///Pixel type
typedef double t_real64;
///Pixel type
typedef long double t_real128;

///Signed 32 bit interger. The convention is that labels are in [1,(2^31-1)]. Zero indicates no label. Negative values are for internal usage
typedef t_sint32 t_label;

/// Templated type that wraps various image types
template<typename T> struct Buffer;

namespace pixeltypes {
template<typename T> T minValue();
template<typename T> T maxValue();

}



namespace container {

/**
 * Template struct that assists the wrapping of an arbitrary image type to a Buffer.
 * The the wrapping is implemented with template instantiation in order to avoid deep
 * hierarchies.
 * In order to define a new type wrapper one has to specialize the template for each image-type one wants.
 * In order to realize the interface One must define all member functions
 */

template <typename CONTAINER,typename PIXELT> struct ContainerWrapper{
/*
//BASIC API CLASS METHODS
	static 	bool isDefined();
//BASIC API INSTANCE METHODS
	static bool canWrap(const CONTAINER& cont);
	static bool canTakeDataOwnership(const CONTAINER& cont);
	static boost::shared_ptr<PIXELT> takeStorageOwnership(CONTAINER& cont);
	static boost::shared_ptr<PIXELT> deepCopyStorage(const CONTAINER& cont);
	static const PIXELT* getFirstRow(const CONTAINER&);
	static PIXELT* getFirstRow(CONTAINER&);
	static t_sz getWidth(const CONTAINER&);
	static t_sz getHeight(const CONTAINER&);
	static t_sz getByteLinestride(const CONTAINER&);
*/
};


}

template <typename A,typename B> struct __type_comparisson__{
	static bool sameTypes(){
		return false;
	}
};

template <typename A> struct __type_comparisson__<A,A>{
	static bool sameTypes(){
		return true;
	}
};

template <typename A,typename B> bool compareTypes(){
	return __type_comparisson__<A,B>::sameTypes();
}


template<typename T> struct Buffer {
    ///This is the fundamental class of the hole framework. It is the abstaction
    ///that unifies all image types contained by containers. A buffer points to
    ///a specified location in memory that is constant a cross the lifecycle of
    ///a Buffer. A buffer has constant width and constant height.
    ///A Buffer can either own its memory, or wrap externally allocated memory
    ///depending on the constructor used. Buffers implement reference counting
    ///with boost::smart_ptr in the case they own their memory. Buffers can refer
    ///to any rectangular sub-region of an image. When a Buffer is created from
    ///an existing Buffer even as a subregion, a shallow copy is performed.
	typedef T t_pixel;
	typedef boost::shared_ptr<T> t_pix_smartptr;
	boost::shared_ptr<t_uint8> __ownedStorage__;
	t_uint8* const __data__;
	const t_sz width; ///Number of pixels in a horizontal line of the image
	const t_sz height; ///Number of pixels in a vertical line of the image
	const t_sz __linestride__; ///Distance between to vertically consecutive pixels in bytes

	template<typename K>
	Buffer(const K& obj) :
        ///This is where the magic happens. A Buffer is created as long as there is a container type
        ///for typename K. The buffer will only wrap the memory storage. It is up to the user of the
        /// library to make sure, that the memory beeing wrapped is not dealocated.
			__data__((t_byte*) (container::ContainerWrapper<K, T>::getFirstRow(obj))),
					width(container::ContainerWrapper<K, T>::getWidth(obj)),
					height(container::ContainerWrapper<K, T>::getHeight(obj)),
					__linestride__(container::ContainerWrapper<K, T>::getByteLinestride(obj)),
					__ownedStorage__(container::ContainerWrapper<K, T>::takeStorageOwnership(obj))
					{
		if(container::ContainerWrapper<K, T>::canWrap(obj)){
			//std::cerr<<"Can wrap\n";
		}else{
			std::cerr<<"Cant wrap\n";
			std::cerr<<"Cant wrap, throwing";
			throw "cant wrap";
		}
	}
	Buffer(t_sz w, t_sz h) :
        ///This constructor creates a Buffer that owns its memory. Memory is allocated with standard con
			__ownedStorage__((t_uint8*)(new T[w * h])), __data__(
					(t_byte*) (__ownedStorage__.get())), width(w), height(h), __linestride__(
					w * sizeof(T)) {
	}
	Buffer(void* d, t_sz w, t_sz h, t_sz ls) :
            __data__((t_uint8*)d), width(w), height(h), __linestride__(ls) {
	}
    Buffer(Buffer<T> img,int leftMost,int topMost,int rightMost,int bottomMost):__ownedStorage__(img.__ownedStorage__),
        __data__(img.__data__+sizeof(T)*(leftMost+topMost*img.getPixelLinestride())),
        width(1+rightMost-leftMost),height(1+bottomMost-topMost),__linestride__(img.__linestride__){}

    void copyFrom(Buffer<T> in){
        int x,y;
        for(y=0;y<height;y++){
            memcpy(in.getRow(y),this->getRow(y),width*sizeof(T));
        }
    }
    void copyTo(Buffer<T> in){
        int x,y;
        for(y=0;y<height;y++){
            memcpy(in.getRow(y),this->getRow(y),width*sizeof(T));
        }
    }
	Buffer<T> deepCopy(){
		Buffer<T> res(width,height);
        res.copyFrom(*this);
		return res;
	}

	bool ownsMemory() {
		return bool(__ownedStorage__.get());
	}
	T* getRow(t_sz row) {
		return (T*) (this->__data__ + __linestride__ * row);
	}
	T* operator[](t_sz rowNum) {
		return (T*) (__data__ + __linestride__ * rowNum);
	}
	std::string getDescription(){
		std::stringstream tmp;
		tmp<<"width : "<<width<<"\nheight : "<<height<<"\nlinestride : "<<__linestride__<<"\n__data__ : "<<((void*)(__data__))<<"\n__ownedStorage__ : "<<((void*)(__ownedStorage__.get()))<<"\n";
		return tmp.str();
	}
	bool isMemContinious(){
		return __linestride__==sizeof(T)*width;
	}
	t_uint32 getByteLinestride(){
		return __linestride__;
	}
	t_uint32 getPixelLinestride(){
		return __linestride__/sizeof(T);
	}
	~Buffer() {
	}
};



}

#endif /* WRAPPING_API_HPP_ */
