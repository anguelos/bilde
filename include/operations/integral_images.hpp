#ifndef INTEGRAL_IMAGES_HPP
#define INTEGRAL_IMAGES_HPP

namespace bilde{
namespace operations{
namespace integral_images{

template <typename T> struct IntegralImage: public Buffer<T>{
    IntegralImage(int x,int y):Buffer<T>(x,y){
    }


};


}
}
}
#endif // INTEGRAL_IMAGES_HPP
