#ifndef ENHANCE_GRAYSCALE_HPP_
#define ENHANCE_GRAYSCALE_HPP_

#include <string>
#include <functional>
#include "wrapping_api.hpp"
#include "operations/integral_histograms.hpp"

namespace bilde {
namespace methods {
namespace enhance_grayscale {



template<typename PIXELT, typename BINT, int BITDEPTH> struct __GrayEnhancer__{
    inline static const int NBBINS=256/(1<<(8-BITDEPTH));
    typedef typename bilde::operations::integral_histograms::IntegralHistogram<PIXELT,BINT,NBBINS,true>::HistogramOperations operations;
    inline static const double E = 0.00000000000001;

    const t_sz windowWidth;
    const t_sz windowHeight;
    const float topCentile;
    const float bottomCentile;
    const int globalHistogramCoeficient;
    const int localHistogramCoeficient;
    std::string mode;
    int verbose;
    
    __GrayEnhancer__(std::string mode, float topCentile, float bottomCentile, int windowWidth, int windowHeight, 
            int globalHistogramCoeficient, int localHistogramCoeficient, int verbose=0):
        windowWidth(windowWidth), windowHeight(windowHeight), topCentile(topCentile),
        bottomCentile(bottomCentile), globalHistogramCoeficient(globalHistogramCoeficient), localHistogramCoeficient(localHistogramCoeficient), mode(mode), verbose(10){
    }

/*     static int enhaceGrayHistEqualiseStatic(int val ,const BINT* histogram){
        int localHistogramCoeficient=1;
        int globalHistogramCoeficient=0;
        int naturalRation=(globalCummulativeHistogram[NBBINS-1]/(E+histogram[NBBINS-1]))*localHistogramCoeficient;
        return round((NBBINS-1)*((naturalRation*histogram[val]+globalHistogramCoeficient*globalCummulativeHistogram[val])/double(E+naturalRation* histogram[NBBINS-1]+globalHistogramCoeficient*globalCummulativeHistogram[NBBINS-1])));
    }
 */
    struct EqualiserLocalGlobal{
        BINT globalCummulativeHistogram[NBBINS];
        const int localHistogramCoeficient;
        const int globalHistogramCoeficient;
        EqualiserLocalGlobal(BINT* globalHistogram, int localCoefficient, int globalCoefficient):globalHistogramCoeficient(globalCoefficient),localHistogramCoeficient(localCoefficient){
            std::memcpy(this->globalCummulativeHistogram, globalHistogram, sizeof(BINT)*NBBINS);
        }
        PIXELT operator()(PIXELT val, const BINT* histogram){
            float naturalRatio=(this->globalCummulativeHistogram[NBBINS-1]/(E+histogram[NBBINS-1]))*localHistogramCoeficient;
            return round((NBBINS-1)*(
                (naturalRatio*histogram[val]+globalHistogramCoeficient*this->globalCummulativeHistogram[val])
            /(E+naturalRatio* histogram[NBBINS-1]+globalHistogramCoeficient*this->globalCummulativeHistogram[NBBINS-1])));
        }
    };


    struct LinearOutlierRemover{
        const float top;
        const float bottom;
        LinearOutlierRemover(float topQuantile, float bottomQuantile):top(topQuantile),bottom(bottomQuantile){}
        PIXELT operator()(PIXELT val, const BINT* histogram){
            PIXELT top_bin = operations::__getCentilePos__(top, histogram);
            PIXELT bottom_bin = operations::__getCentilePos__(bottom, histogram);            
            return (NBBINS-1)*((val-top_bin)/double(top_bin-bottom_bin));
        }
    };
    
    static PIXELT enhaceGrayNullStatic(PIXELT val,const BINT* histogram){
        double local_ratio = histogram[val]/(E+histogram[NBBINS-1]);
        PIXELT res= round((NBBINS-1)*local_ratio);
        //std::cerr<<"("<<int(val)<<", 0["<<int(histogram[0])<<"], V["<<int(histogram[val])<<"], "<<NBBINS-1<<"["<<int(histogram[NBBINS-1])<<"], r="<<int(res)<<"), ";
        return res;
    }

    void enhaceGrayMode(bilde::Buffer<PIXELT> out, bilde::Buffer<PIXELT> in){
        //const int NBBINS=256/(1<<(8-BITDEPTH));
        BINT globalCummulativeHistogram[NBBINS];
        bilde::Buffer<PIXELT> inCp(in.width, in.height);
        bilde::operations::essential::__copy__(inCp,in);
        bilde::operations::essential::__divideBy__<PIXELT>(inCp,inCp,1<<(8-BITDEPTH));
        if(verbose>5){
            std::cerr<<"INPUT MAX:"<<int(bilde::operations::essential::getMaxValue(inCp))<<" MIN:"<<int(bilde::operations::essential::getMinValue(inCp))<<"  Bitdepth: "<<BITDEPTH<<" NBBINS:"<<NBBINS<<"\n";
        }

        bilde::operations::__histogram__::__getCummulativeHistogram__<BINT,NBBINS>(inCp,globalCummulativeHistogram);
        bilde::util::timer::Timer ihtimer;
        bilde::operations::integral_histograms::IntegralHistogram<PIXELT, BINT,NBBINS,true> ih(inCp);
        //bilde::operations::integral_histograms::IntegralHistogram<BINT,NBBINS,true> ih(in);

        if(this->verbose >9){
            std::cerr<<"@ 0,0 [";
            for(int n ;n<NBBINS-1;n++){
                std::cerr<<ih.__getHistogramAt__(0,0)[n]<<", ";
                
            }
            std::cerr<<ih.__getHistogramAt__(0,0)[NBBINS-1]<<"]\n\n";   

            std::cerr<<"@ 0,"<<in.height-1<<" [";
            for(int n ;n<NBBINS-1;n++){
                std::cerr<<ih.__getHistogramAt__(0, in.height-1)[n]<<", ";
            }
            std::cerr<<ih.__getHistogramAt__(0, in.height-1)[NBBINS-1]<<"]\n\n";

            std::cerr<<"@ "<<in.width-1<<", 0 [";
            for(int n ;n<NBBINS-1;n++){
                std::cerr<<ih.__getHistogramAt__(in.width-1, 0)[n]<<", ";
            }
            std::cerr<<ih.__getHistogramAt__(in.width-1, 0)[NBBINS-1]<<"]\n\n";

            std::cerr<<"@ "<<in.width-1<<","<<1<<" [";
            for(int n ;n<NBBINS-1;n++){
                std::cerr<<ih.__getHistogramAt__(in.width-1, 1)[n]<<", ";
            }
            std::cerr<<ih.__getHistogramAt__(in.width-1, 1)[NBBINS-1]<<"]\n\n";

            std::cerr<<"@ "<<0<<","<<1<<" [";
            for(int n ;n<NBBINS-1;n++){
                std::cerr<<ih.__getHistogramAt__(0, 1)[n]<<", ";
            }
            std::cerr<<ih.__getHistogramAt__(0, 1)[NBBINS-1]<<"]\n\n";

            std::cerr<<"@ "<<in.width-1<<","<<in.height-1<<" [";
            for(int n ;n<NBBINS-1;n++){
                std::cerr<<ih.__getHistogramAt__(in.width-1, in.height-1)[n]<<", ";
            }
            std::cerr<<ih.__getHistogramAt__(in.width-1, in.height-1)[NBBINS-1]<<"]\n\n";
        }
        if(this->verbose>4){
            std::cerr<<"Calculated cummulative integral histogram for "<<in.width<<"x"<<in.height<<" x "<<NBBINS<<" Bins.  Duration "<<ihtimer.getSec()<<" sec.\n";
        }
        auto iter=ih.getIterator(windowWidth,windowHeight);
        if(mode=="linear"){
            LinearOutlierRemover enhaceGrayLinearStatic(topCentile, bottomCentile);
            iter.applyFilterContextualFunctor(out,inCp,enhaceGrayLinearStatic);
            bilde::operations::essential::__multiplyBy__<PIXELT>(out,out,1<<(8-BITDEPTH));
            return;
        }
        if(mode=="null"){
            iter.applyFilterContextualFunctor(out,inCp,enhaceGrayNullStatic);
            bilde::operations::essential::__multiplyBy__<PIXELT>(out,out,1<<(8-BITDEPTH));
            return;
        }
        if(mode=="equalise"){
            EqualiserLocalGlobal equaliser(globalCummulativeHistogram, localHistogramCoeficient, globalHistogramCoeficient);
            if(this->verbose>3){
                bilde::util::timer::Timer t;
                
                iter.applyFilterContextualFunctor(out, inCp, equaliser);
                std::cerr<<"Equalised histogram locally duration "<<t.getSec()<<" sec.\n";
            }else{
                iter.applyFilterContextualFunctor(out, inCp, equaliser);
            }
            bilde::operations::essential::__multiplyBy__<PIXELT>(out,out,1<<(8-BITDEPTH));
            return;
        }
        throw std::runtime_error("enhaceGray inrecognised mode");
    }


};

inline void enhaceGray(bilde::Buffer<bilde::t_uint8> in,bilde::Buffer<bilde::t_uint8> out, int bitDepth, std::string mode="equalise", int windowWidth=51, int windowHeight=51,
                int globalHistogramCoeficient=0, int localHistogramCoeficient=1, float topCentile=.95, float bottomCentile=.05){
    if (bitDepth>8 || bitDepth<3){
        throw "bitdepth shoud be in 3-8 range";
    }
    switch(bitDepth){
        case 8:__GrayEnhancer__<bilde::t_uint8, t_uint32, 8>(mode, topCentile, bottomCentile, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient).enhaceGrayMode(out,in);break;
        case 7:__GrayEnhancer__<bilde::t_uint8, t_uint32, 7>(mode, topCentile, bottomCentile, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient).enhaceGrayMode(out,in);break;
        case 6:__GrayEnhancer__<bilde::t_uint8, t_uint32, 6>(mode, topCentile, bottomCentile, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient).enhaceGrayMode(out,in);break;
        case 5:__GrayEnhancer__<bilde::t_uint8, t_uint32, 5>(mode, topCentile, bottomCentile, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient).enhaceGrayMode(out,in);break;
        case 4:__GrayEnhancer__<bilde::t_uint8, t_uint32, 4>(mode, topCentile, bottomCentile, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient).enhaceGrayMode(out,in);break;
        case 3:__GrayEnhancer__<bilde::t_uint8, t_uint32, 3>(mode, topCentile, bottomCentile, windowWidth, windowHeight, globalHistogramCoeficient, localHistogramCoeficient).enhaceGrayMode(out,in);break;
        default:
            throw std::runtime_error("ditDepth out of range");
    }
}


}
}
}

#endif /* ENHANCE_GRAYSCALE_HPP_ */