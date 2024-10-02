/*
 * numpy_pybind11_container.hpp
 *
 *  Created on: Sep 30, 2024
 *      Author: anguelos
 */

#ifndef NPPYBIND11_CONTAINER_HPP_
#define NPPYBIND11_CONTAINER_HPP_


#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>  // <-- Include this for std::vector support
#include <vector>
#include <stdexcept>

namespace bilde {
namespace container {

namespace __nppybind11_util__ {

template<typename B> int c2typeNum() {
        return -1;
}

template<> int c2typeNum<unsigned char>() {
        return 1;
}

template<> int c2typeNum<signed char>() {
        return 2;
}

template<> int c2typeNum<unsigned short int>() {
        return 3;
}

template<> int c2typeNum<signed short int>() {
        return 4;
}

template<> int c2typeNum<signed int>() {
        return 5;
}

template<> int c2typeNum<float>() {
        return 6;
}

template<> int c2typeNum<double>() {
        return 7;
}

}



template<typename PIXELT> struct ContainerWrapper<pybind11::array_t<PIXELT>, PIXELT> {
//Wrapper example.
//This wrapper is used to "wrap" an IplImage (opencv legacy type image) in a Buffer
//AKA used for copy constructors

        //BASIC API CLASS METHODS
        static bool isDefined() {
                int depth = __nppybind11_util__::c2typeNum<PIXELT>();
                return depth>=1 and depth <=7;
        }

        //BASIC API INSTANCE METHODS
        static bool canWrap(const pybind11::array_t<PIXELT> & cont) {return cont.ndim()== 2; }
        static bool canTakeDataOwnership(const pybind11::array_t<PIXELT> & cont) {return false;}
        static boost::shared_ptr<t_uint8> takeStorageOwnership(const pybind11::array_t<PIXELT> & cont) {return boost::shared_ptr<t_uint8>();}
        static boost::shared_ptr<t_uint8> deepCopyStorage(const pybind11::array_t<PIXELT> & cont) {
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

        static const PIXELT* getFirstRow(const pybind11::array_t<PIXELT>&cont) {
                return (const PIXELT*)(cont.request().ptr);
        }

        static PIXELT* getFirstRow(pybind11::array_t<PIXELT>&cont) {
                return (PIXELT*)(cont.request().ptr);
        }

        static t_sz getWidth(const pybind11::array_t<PIXELT>& cont) {
                return (cont.request().shape[1]);
        }

        static t_sz getHeight(const pybind11::array_t<PIXELT>&cont) {
                return cont.request.shape[0];
        }

        static t_sz getByteLinestride(const pybind11::array_t<PIXELT>&cont) {
                return cont.shape()[1]*sizeof(PIXELT);
        }

};


template<typename PIXELT> struct ContainerWrapper<pybind11::buffer_info, PIXELT> {
        //Wrapper example.
        //This wrapper is used to "wrap" an IplImage (opencv legacy type image) in a Buffer
        //AKA used for copy constructors

        //BASIC API CLASS METHODS
        static bool isDefined() {
                int depth = __nppybind11_util__::c2typeNum<PIXELT>();
                return depth>=1 and depth <=7;
        }

        //BASIC API INSTANCE METHODS
        static bool canWrap(const pybind11::buffer_info & cont) {return true;}  // TODO(anguelos) make some extra tests
        static bool canTakeDataOwnership(const pybind11::buffer_info & cont) {return false;}
        static boost::shared_ptr<t_uint8> takeStorageOwnership(const pybind11::buffer_info & cont) {return boost::shared_ptr<t_uint8>();}
        static boost::shared_ptr<t_uint8> deepCopyStorage(const pybind11::buffer_info & cont) {
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

        static const PIXELT* getFirstRow(const pybind11::buffer_info&cont) {
                return (const PIXELT*)(cont.ptr);
        }

        static PIXELT* getFirstRow(pybind11::buffer_info&cont) {
                return (PIXELT*)(cont.ptr);
        }

        static t_sz getWidth(const pybind11::buffer_info& cont) {
                return (cont.shape[1]);
        }

        static t_sz getHeight(const pybind11::buffer_info&cont) {
                return cont.shape[0];
        }

        static t_sz getByteLinestride(const pybind11::buffer_info&cont) {
                return cont.shape[1]*sizeof(PIXELT);
        }

};


}

}

#endif /* NPPYBIND11_CONTAINER_HPP_ */
