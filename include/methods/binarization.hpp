#ifndef LOF_BINARIZATION_HPP
#define LOF_BINARIZATION_HPP
#include "bilde.hpp"

namespace bilde{
namespace methods{
namespace binarization{


inline Buffer<t_uint8> getOtsuBinarization(Buffer<t_uint8> in) {
    int verbose=10;
    Buffer<t_uint8> res(in.width, in.height);
    int threshold=operations::histogram::Histogram<t_uint32,256,false>(in).getOtsu();
    if(VERBOSE>9){
        std::cerr<<operations::histogram::Histogram<t_uint32,256,false>(in)<<"\n";
        std::cerr<<"Otsu Threshold:"<<threshold<<"\n";
    }
    bilde::operations::essential::__threshold__<t_uint8>(res,in,threshold,255,0);
    return res;
}

template <typename PIXELT, int NBBINS> Buffer<PIXELT> getLofFilter(operations::integral_histograms::IntegralHistogram<PIXELT,t_uint32,NBBINS,false>& ih,int radius ){
    Buffer<PIXELT>res(ih.width, ih.height);
    ih.getIterator(radius).applyFilter(res,operations::__histogram__::__HistogramOperations__<PIXELT,t_uint32,NBBINS,false>::__getOtsu__);
    return res;
}

template <typename PIXELT, int BITDEPTH> void __lofBinarization__(Buffer<PIXELT> outImg,Buffer<PIXELT> inImg){
    int verbose=10;
    constexpr const int NBBINS=1<<(BITDEPTH);
    constexpr const int DIVIDEBY=1<<(8-BITDEPTH);
    Buffer<PIXELT> dividedImg(inImg.width,inImg.height);
    operations::essential::__divideBy__<PIXELT>(dividedImg,inImg,DIVIDEBY);
    if(verbose>6){
        std::cerr<<"__lofBinarization__: NBBINS="<<NBBINS<<" DIVIDEBY="<<DIVIDEBY<<"\n";
    }
    //bilde::operations::essential::__divideBy__<t_uint8>(inImg,inImg,DIVIDEBY);
    operations::integral_histograms::IntegralHistogram<PIXELT, t_uint32,NBBINS,false> ih(dividedImg);
    Buffer<PIXELT> filter(dividedImg.width,dividedImg.height);
    Buffer<PIXELT> thresholded(dividedImg.width,dividedImg.height);
    bilde::operations::essential::__setTo__<PIXELT>(outImg, 0);
    //operations::essential::__incrementBy__<t_uint8>(dividedImg,dividedImg,1);
    for(int radius=((dividedImg.width>dividedImg.height)?dividedImg.width/4:dividedImg.height/4);radius>10;radius=radius/2){

        bilde::util::timer::Timer tLof;
        ih.getIterator(radius).applyFilterFunctor(filter, operations::__histogram__::__HistogramOperations__<PIXELT,t_uint32,NBBINS,false>::__getOtsu__);
        if(verbose>6){
            std::cerr<<"__lofBinarization__ lof rendering time "<<tLof.getMsec()<<"\n";
        }
        //operations::essential::__incrementBy__<t_uint8>(filter,filter,1);
        operations::essential::__pixelWiseThreshold__<PIXELT>(thresholded,dividedImg,filter,255,0);
        operations::essential::__pixelWiseMax__<PIXELT>(outImg,thresholded,outImg);
    }

    //operations::essential::__pixelWiseMin__<t_uint8>(outImg,outImg,getOtsuBinarization(filter));
    //getOtsuBinarization(filter).copyTo(outImg);
    //filter.copyTo(outImg);
    //filter.copyTo(outImg);
    //operations::essential::__multiplyBy__<t_uint8>(outImg,outImg,DIVIDEBY);
}

inline Buffer<t_uint8> getLofBinarization(Buffer<t_uint8> inimg, Buffer<t_uint8> outimg, int bitdepth){
    switch (bitdepth) {
        case 3:__lofBinarization__<t_uint8, 3>(outimg,inimg); break;
        case 4:__lofBinarization__<t_uint8, 4>(outimg,inimg); break;
        case 5:__lofBinarization__<t_uint8, 5>(outimg,inimg); break;
        case 6:__lofBinarization__<t_uint8, 6>(outimg,inimg); break;
        case 7:__lofBinarization__<t_uint8, 7>(outimg,inimg); break;
        case 8:__lofBinarization__<t_uint8, 8>(outimg,inimg); break;
        default:
            std::cerr<<"LOF Binarization for bitdepth "<<bitdepth<<" not implemented.\nAborting!";
        throw "LOF Binarize, bad bitdepth!"; break;
    }
}

}
}
}

#endif // LOF_BINARIZATION_HPP
