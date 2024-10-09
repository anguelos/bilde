#ifndef ENHANCE_GRAYSCALE_HPP_
#define ENHANCE_GRAYSCALE_HPP_

#include <string>
#include <functional>
#include "wrapping_api.hpp"
#include "operations/integral_histograms.hpp"

namespace bilde {
namespace methods {
namespace enhance_grayscale {


template <typename BINT,int BITDEPTH> struct __GrayEnhancer__{
    static constexpr int NBBINS = 1 << BITDEPTH;
    
    BINT globalCummulativeHistogram[NBBINS];
    std::string mode;
    const int topCentile;
    const int bottomCentile;
    const int windowWidth;
    const int windowHeight;
    const int globalHistogramCoeficient;
    const int localHistogramCoeficient;
    __GrayEnhancer__(std::string mode="equalise", int top_centile=95, int bottom_centile=5, int window_width=51, int window_height=51,
                int global_histogram_coeficient=0, int local_histogram_coeficient=1):
            mode(mode), topCentile(top_centile), bottomCentile(bottom_centile),
            windowWidth(window_width), windowHeight(window_height), 
            globalHistogramCoeficient(global_histogram_coeficient), 
            localHistogramCoeficient(local_histogram_coeficient){
        if(mode != "linear" && mode != "null" && mode != "equalise"){
            throw "mode must be 'linear', 'null', or 'equalise'";
        }
    }

    void enhaceGrayMode(bilde::Buffer<bilde::t_uint8> in,bilde::Buffer<bilde::t_uint8> out){
        // Handling variable bit depths
        bilde::Buffer<bilde::t_uint8> inCp(in.width,in.height);
        bilde::operations::essential::__copy__(inCp,in);
        bilde::operations::essential::__divideBy__<bilde::t_uint8>(inCp,inCp,1<<(8-BITDEPTH)); 


        bilde::operations::__histogram__::__getCummulativeHistogram__<BINT,NBBINS>(inCp,globalCummulativeHistogram);
        bilde::operations::integral_histograms::IntegralHistogram<BINT,NBBINS,true> ih(inCp);  // ISCUMULATIVE = true
        auto iter=ih.getIterator(windowWidth,windowHeight);

        auto linear = [this](BINT val, const BINT* histogram) {
            double e = 0.00000000001;
            typedef typename bilde::operations::integral_histograms::IntegralHistogram<BINT,NBBINS,true>::HistogramOperations operations;
            int top=operations::__getCentilePos__(.01*this->topCentile, histogram);
            int bottom=operations::__getCentilePos__(.01*this->bottomCentile, histogram);
            return BINT((NBBINS-1)*((val-top)/(double(top-bottom))+ e));
        };

        auto equalise = [this](BINT val,const BINT* histogram){
            double e = 0.00000000001;
            //double naturalRatio = (this->globalCummulativeHistogram[NBBINS-1] / histogram[NBBINS-1]) * this->localHistogramCoeficient;
            double naturalRatio = (this->globalCummulativeHistogram[NBBINS-1] / (histogram[NBBINS-1]+e)) * this->localHistogramCoeficient;
            double numerator = (naturalRatio*histogram[val] + this->globalHistogramCoeficient * this->globalCummulativeHistogram[val]);
            double denominator = (naturalRatio* histogram[NBBINS-1]+globalHistogramCoeficient*globalCummulativeHistogram[NBBINS-1]);
            double res = round((NBBINS-1)*(numerator/(denominator+e)));
            //double res = round((NBBINS-1)*1);
            return BINT(res);
        };

        auto null = [this](BINT val,const BINT* histogram){
            typedef typename bilde::operations::integral_histograms::IntegralHistogram<BINT,NBBINS,true>::HistogramOperations operations;
            return histogram[NBBINS-1];
        };

        if(mode=="linear"){
            iter.applyLambdaFilter(out,inCp, linear);
            bilde::operations::essential::__multiplyBy__<bilde::t_uint8>(out,out,1<<(8-BITDEPTH));
            return;
        }
        if(mode=="null"){
            iter.applyLambdaFilter(out,inCp, null);
            bilde::operations::essential::__multiplyBy__<bilde::t_uint8>(out,out,1<<(8-BITDEPTH));
            return;
        }
        if(mode=="equalise"){
            iter.applyLambdaFilter(out,inCp, equalise);
            bilde::operations::essential::__multiplyBy__<bilde::t_uint8>(out,out,1<<(8-BITDEPTH));
            return;
        }
        throw "enhaceGray inrecognised mode";
    }
};


inline void enhaceGray(bilde::Buffer<bilde::t_uint8> in,bilde::Buffer<bilde::t_uint8> out, int bitDepth, std::string mode="equalise", int windowWidth=51, int windowHeight=51,
                int globalHistogramCoeficient=0, int localHistogramCoeficient=1, int topCentile=95, int bottomCentile=5){
    if (bitDepth>8 || bitDepth<3){
        throw "bitdepth shoud be in 3-8 range";
    }
    switch(bitDepth){
        case 8:__GrayEnhancer__<t_uint32, 8>(mode, topCentile, bottomCentile, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient).enhaceGrayMode(in,out);break;
        case 7:__GrayEnhancer__<t_uint32, 7>(mode, topCentile, bottomCentile, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient).enhaceGrayMode(in,out);break;
        case 6:__GrayEnhancer__<t_uint32, 6>(mode, topCentile, bottomCentile, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient).enhaceGrayMode(in,out);break;
        case 5:__GrayEnhancer__<t_uint32, 5>(mode, topCentile, bottomCentile, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient).enhaceGrayMode(in,out);break;
        case 4:__GrayEnhancer__<t_uint32, 4>(mode, topCentile, bottomCentile, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient).enhaceGrayMode(in,out);break;
        case 3:__GrayEnhancer__<t_uint32, 3>(mode, topCentile, bottomCentile, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient).enhaceGrayMode(in,out);break;
        default:
            throw "ditDept out of range";
    }
}


}
}
}

#endif /* ENHANCE_GRAYSCALE_HPP_ */