#ifndef INTEGRAL_IMAGES_HPP
#define INTEGRAL_IMAGES_HPP

#include <type_traits>
#include <cmath>

#include "wrapping_api.hpp"

namespace bilde{
namespace operations{
namespace integral_images{


//bilde::operations::integral_images::__getBoxFilter__<bilde::t_uint8,bilde::t_sint32>(outImg,inImg,width,height,tmpImg);

template<typename T, unsigned int PAD> struct IntegralImage{
    using ACCUMULATE_T = typename std::conditional<std::is_integral<T>::value, long, double>::type;
    Buffer<ACCUMULATE_T> __storage__;
    Buffer<ACCUMULATE_T> __centered_storage__;
    const t_sz width;
    const t_sz height;
    const t_sz __lineStride__;

    IntegralImage(const Buffer<T> &in):
        __storage__(in.width + 2 * PAD,in.height + 2 * PAD),
        __centered_storage__(__storage__, PAD, PAD, in.width,in.height),
        width(in.width),
        height(in.height),
        __lineStride__(__storage__.__linestride__){
        std::fill(__storage__.getRow(0), __storage__.getRow(height*PAD*2) + 2*PAD+ in.width, ACCUMULATE_T{});
        ACCUMULATE_T *accRow;
        ACCUMULATE_T *prevRow;
        const T* inRow;
        accRow = __storage__.getRow(0);
        inRow = in.getConstRow(0);
        accRow[0] = inRow[0];
        for (t_sz x = 1; x < width; x++) {
            accRow[x] = accRow[x-1] + inRow[x];
        }
        for(t_sz y=1;y<height;y++){
            prevRow = __storage__.getRow(y-1);
            accRow = __storage__.getRow(y);
            inRow = in.getConstRow(y);
            accRow[0] = inRow[0] + prevRow[0];
            for(t_sz x=1;x<width;x++){
                accRow[x] = prevRow[x] + inRow[x];
            }
        }
    }
    struct IntegralImageConstIterator{
        const IntegralImage & iimg;
        const int x0;
        const int y0;
        const int x1;
        const int y1;
        const Buffer<ACCUMULATE_T> __top_left__;
        const Buffer<ACCUMULATE_T> __top_right__;
        const Buffer<ACCUMULATE_T> __bottom_left__;
        const Buffer<ACCUMULATE_T> __bottom_right__;
        IntegralImageConstIterator(const IntegralImage & iimg,int x0,int y0,int x1,int y1):
            iimg(iimg),
            x0(x0),
            y0(y0),
            x1(x1),
            y1(y1),
            __top_left__(iimg.__storage__.getConstRow(y0) + x0, iimg.width, iimg.height, iimg.__storage__.getPixelLinestride()),
            __top_right__(iimg.__storage__.getConstRow(y0) + x1, iimg.width, iimg.height, iimg.__storage__.getPixelLinestride()),
            __bottom_left__(iimg.__storage__.getConstRow(y0) + x0, iimg.width, iimg.height, iimg.__storage__.getPixelLinestride()),
            __bottom_right__(iimg.__storage__.getConstRow(y0) + x1, iimg.width, iimg.height, iimg.__storage__.getPixelLinestride()){
                if (std::abs(x0) >= PAD || std::abs(x1) >= PAD || std::abs(y0) >= PAD || std::abs(y1) >= PAD || x0 >= x1 || y0 >= y1){
                    throw "IntegralImageConstIterator: x0>=x1 || y0>=y1";
                }
        }
        ACCUMULATE_T operator()(int x,int y){
            ACCUMULATE_T tl = __top_left__(x,y);
            ACCUMULATE_T tr = __top_right__(x,y);
            ACCUMULATE_T bl = __bottom_left__(x,y);
            ACCUMULATE_T br = __bottom_right__(x,y);
            return (tl + br) - (tr + bl);
        }
        void getSumImage(Buffer<T> & out){
            const ACCUMULATE_T * tlRow;
            const ACCUMULATE_T * trRow;
            const ACCUMULATE_T * blRow;
            const ACCUMULATE_T * brRow;
            T * outRow;
            for(t_sz y=0;y<out.height;y++){
                tlRow = __top_left__.getConstRow(y);
                trRow = __top_right__.getConstRow(y);
                blRow = __bottom_left__.getConstRow(y);
                brRow = __bottom_right__.getConstRow(y);
                outRow = out.getRow(y);
                for(t_sz x=0;x<out.width;x++){
                    outRow[x] = T((tlRow[x] + brRow[x]) - (trRow[x] + blRow[x]));
                }
            }
        }
        void getSumImage(Buffer<ACCUMULATE_T> & out){
            const ACCUMULATE_T * tlRow;
            const ACCUMULATE_T * trRow;
            const ACCUMULATE_T * blRow;
            const ACCUMULATE_T * brRow;
            T * outRow;
            for(t_sz y=0;y<out.height;y++){
                tlRow = __top_left__.getConstRow(y);
                trRow = __top_right__.getConstRow(y);
                blRow = __bottom_left__.getConstRow(y);
                brRow = __bottom_right__.getConstRow(y);
                outRow = out.getRow(y);
                for(t_sz x=0;x<out.width;x++){
                    outRow[x] = (tlRow[x] + brRow[x]) - (trRow[x] + blRow[x]);
                }
            }
        }
        void getAvgImage(Buffer<T> & out){
            const ACCUMULATE_T * tlRow;
            const ACCUMULATE_T * trRow;
            const ACCUMULATE_T * blRow;
            const ACCUMULATE_T * brRow;
            T * outRow;
            t_sz area = (x1 - x0) * (y1 - y0);
            for(t_sz y=0;y<out.height;y++){
                tlRow = __top_left__.getConstRow(y);
                trRow = __top_right__.getConstRow(y);
                blRow = __bottom_left__.getConstRow(y);
                brRow = __bottom_right__.getConstRow(y);
                outRow = out.getRow(y);
                for(t_sz x=0;x<out.width;x++){
                    outRow[x] = T(((tlRow[x] + brRow[x]) - (trRow[x] + blRow[x])) / area);
                }
            }
        }
        void getAvgImage(Buffer<ACCUMULATE_T> & out){
            const ACCUMULATE_T * tlRow;
            const ACCUMULATE_T * trRow;
            const ACCUMULATE_T * blRow;
            const ACCUMULATE_T * brRow;
            T * outRow;
            t_sz area = (x1 - x0) * (y1 - y0);
            for(t_sz y=0;y<out.height;y++){
                tlRow = __top_left__.getConstRow(y);
                trRow = __top_right__.getConstRow(y);
                blRow = __bottom_left__.getConstRow(y);
                brRow = __bottom_right__.getConstRow(y);
                outRow = out.getRow(y);
                for(t_sz x=0;x<out.width;x++){
                    outRow[x] = ((tlRow[x] + brRow[x]) - (trRow[x] + blRow[x])) / area;
                }
            }
        }
    };
};

template<typename T, unsigned int PAD> void getBoxFilter(const Buffer<T> & in, Buffer<T> & out, t_sz width, t_sz height){
    if (width / 2 >= PAD || height / 2 >= PAD){
        throw "getBoxFilter: width/2 >= PAD || height/2 >= PAD";
    }
    IntegralImage<T,PAD> iimg(in);
    typename IntegralImage<T,PAD>::IntegralImageConstIterator iimgIt(iimg, -width/2, -height/2, width/2, height/2);
    iimgIt.getAvgImage(out);
}


}
}
}
#endif // INTEGRAL_IMAGES_HPP
