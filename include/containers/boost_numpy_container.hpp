#ifndef BOOST_NUMPY_HPP
#define BOOST_NUMPY_HPP

#ifdef BOOST_NUMPY_HPP_INCLUDED

namespace bilde{
namespace container{


namespace __boost_numpy_util__ {
    template<typename T>bool isDefinedInNumpy(){return false;}
    template<> bool isDefinedInNumpy<bool>(){return true;}
    template<> bool isDefinedInNumpy<t_uint8>(){return true;}
    template<> bool isDefinedInNumpy<t_sint8>(){return true;}
    template<> bool isDefinedInNumpy<t_uint16>(){return true;}
    template<> bool isDefinedInNumpy<t_sint16>(){return true;}
    template<> bool isDefinedInNumpy<t_uint32>(){return true;}
    template<> bool isDefinedInNumpy<t_sint32>(){return true;}
    template<> bool isDefinedInNumpy<t_real32>(){return true;}
    template<> bool isDefinedInNumpy<t_real64>(){return true;}
}
template<typename PIXELT> struct ContainerWrapper<boost::numpy::ndarray*, PIXELT> {
//Wrapper example.
//This wrapper is used to "wrap" a Buffer in an other Buffer
//AKA used for copy constructors

//BASIC API CLASS METHODS
        static bool isDefined() {
                return __boost_numpy_util__::isDefinedInNumpy<PIXELT>();
        }

//BASIC API INSTANCE METHODS
        static bool canWrap(const boost::numpy::ndarray* array) {
                //std::cerr<<"PIXELSZ"<<sizeof(PIXELT)<<"\n\tIs Defined:"<<ContainerWrapper<boost::numpy::ndarray*, PIXELT>::isDefined()<<"\n\tarray.nd:"<<array->get_nd()<<"\n";
                return (ContainerWrapper<boost::numpy::ndarray*, PIXELT>::isDefined())
                        && (array->get_dtype() == boost::numpy::dtype::get_builtin<PIXELT>() ||((array->get_dtype() == boost::numpy::dtype::get_builtin<bool>())&& sizeof(PIXELT)==1))
                        && (array->get_nd() == 2)
                        && ((array->get_flags() & boost::numpy::ndarray::C_CONTIGUOUS))
                        && (array->strides(1)==sizeof(PIXELT));
        }
        static bool canTakeDataOwnership(const boost::numpy::ndarray* array) {
                return false;
        }
        static boost::shared_ptr<t_uint8> takeStorageOwnership(
                        const boost::numpy::ndarray* array) {
                return boost::shared_ptr<t_uint8>();
        }
        static boost::shared_ptr<t_uint8> deepCopyStorage(const boost::numpy::ndarray* array) {
                //its a lot simpler to deep copy a buffer to a buffer than an ipl image to a buffer
                if (!canWrap(array)) {
                        throw "deepCopyStorage for cv::Mat only deepcopies matrices with 1 channel";
                }
                Buffer<PIXELT> tmp(array);
                //dont forget to set Buffer.data to point to the apropriate place
                t_uint8* outdata = new t_uint8[tmp.height * tmp.__linestride__
                                * sizeof(PIXELT)];
                for (int y = 0; y < tmp.height; y++) {
                        std::memcpy(outdata + tmp.__linestride__ * y, tmp.getRow(y),
                                        tmp.width * sizeof(PIXELT));
                }
                return boost::shared_ptr < t_uint8 > (outdata);
        }

        static PIXELT* getFirstRow(const boost::numpy::ndarray* array) {
                return (PIXELT*) (array->get_data());
        }
        static t_sz getWidth(const boost::numpy::ndarray* array) {
                return array->shape(1);
        }
        static t_sz getHeight(const boost::numpy::ndarray* array) {
                return array->shape(0);
        }
        static t_sz getByteLinestride(const boost::numpy::ndarray*array) {
                return array->strides(0);//((t_uint8*) (cont.ptr(1))) - ((t_uint8*) (cont.ptr(0)));
        }
};



}
}

#endif //boost_numpy_HPP_INCLUDED

#endif // boost_numpy_HPP

