#ifndef ENHANCE_GRAYSCALE_HPP_
#define ENHANCE_GRAYSCALE_HPP_

#include <string>
#include <functional>
#include "wrapping_api.hpp"
#include "operations/integral_histograms.hpp"

namespace bilde {
namespace methods {
namespace enhance_grayscale {


template<typename BINT, int BITDEPTH> struct __GrayEnhancer__{
    inline static const int NBBINS=256/(1<<(8-BITDEPTH));
    inline static BINT globalCummulativeHistogram[NBBINS];
    inline static const double E = 0.00000000000001;

    const t_sz windowWidth;
    const t_sz windowHeight;
    const int topCentile;
    const int bottomCentile;
    const int globalHistogramCoeficient;
    const int localHistogramCoeficient;
    std::string mode;
    int verbose;
    __GrayEnhancer__(std::string mode, int topCentile, int bottomCentile, int windowWidth, int windowHeight, 
            int globalHistogramCoeficient, int localHistogramCoeficient):
        windowWidth(windowWidth), windowHeight(windowHeight), topCentile(topCentile),
        bottomCentile(bottomCentile), globalHistogramCoeficient(globalHistogramCoeficient), localHistogramCoeficient(localHistogramCoeficient), mode(mode){
        verbose=10;

    }

    int enhaceGrayHistEqualise(int val,const BINT* histogram){
        //return round((NBBINS-1)*((histogram[val])/double( histogram[NBBINS-1])));
        int naturalRation=(globalCummulativeHistogram[NBBINS-1]/histogram[NBBINS-1])*localHistogramCoeficient;
        return round((NBBINS-1)*((naturalRation*histogram[val]+globalHistogramCoeficient*globalCummulativeHistogram[val])/double(naturalRation* histogram[NBBINS-1]+globalHistogramCoeficient*globalCummulativeHistogram[NBBINS-1])));
    }

/*     static int enhaceGrayHistEqualiseStatic(int val ,const BINT* histogram){
        int localHistogramCoeficient=1;
        int globalHistogramCoeficient=0;
        int naturalRation=(globalCummulativeHistogram[NBBINS-1]/(E+histogram[NBBINS-1]))*localHistogramCoeficient;
        return round((NBBINS-1)*((naturalRation*histogram[val]+globalHistogramCoeficient*globalCummulativeHistogram[val])/double(E+naturalRation* histogram[NBBINS-1]+globalHistogramCoeficient*globalCummulativeHistogram[NBBINS-1])));
    }
 */

    static int enhaceGrayHistEqualiseStatic(int val ,const BINT* histogram){
        //double localToGlobalRatio = (globalCummulativeHistogram[NBBINS-1]/(E+histogram[NBBINS-1]));

        double global_ratio = globalCummulativeHistogram[val]/(E+globalCummulativeHistogram[NBBINS-1]);
        double local_ratio = histogram[val]/(E+histogram[NBBINS-1]);
        //std::cerr<<histogram[val]<<",";
        return round((NBBINS-1)*local_ratio);
        //int naturalRation=(globalCummulativeHistogram[NBBINS-1]/(E+histogram[NBBINS-1]))*localHistogramCoeficient;
        //return round((NBBINS-1)*((naturalRation*histogram[val]+globalHistogramCoeficient*globalCummulativeHistogram[val])/double(E+naturalRation* histogram[NBBINS-1]+globalHistogramCoeficient*globalCummulativeHistogram[NBBINS-1])));
    }

    int enhaceGrayNull(int val,const BINT* histogram){
        return val;
    }
    
    static int enhaceGrayNullStatic(int val,const BINT* histogram){
        std::cerr<<"("<<val<<","<<histogram[0]<<"-"<<histogram[NBBINS-1]<<"), ";
        return val;
    }

    int enhaceGrayLinear(int val,const BINT* histogram){
        typedef typename bilde::operations::integral_histograms::IntegralHistogram<BINT,NBBINS,true>::HistogramOperations operations;
        int sum=histogram[NBBINS-1];
        int top = operations::__getCentilePos__(.01*topCentile,histogram);
        int bottom = operations::__getCentilePos__(.01*bottomCentile,histogram);
        if(this->verbose > 5){
            std::cerr<<"Top:"<<top<<" Bottom:"<<bottom<<" { "<<histogram[0];
            for(int k=1;k<NBBINS;k++){std::cerr<<","<<histogram[k];}std::cerr<<"}\n";
        }
        return (NBBINS-1)*((val-top)/double(top-bottom));
    }

    static int enhaceGrayLinearStatic(int val,const BINT* histogram){
        typedef typename bilde::operations::integral_histograms::IntegralHistogram<BINT,NBBINS,true>::HistogramOperations operations;
        int topCentile = 95;
        int bottomCentile = 5;
        int sum=histogram[NBBINS-1];
        int top = operations::__getCentilePos__(.01*topCentile,histogram);
        int bottom = operations::__getCentilePos__(.01*bottomCentile,histogram);
        //if(this->verbose > 5){
        //    std::cerr<<"Top:"<<top<<" Bottom:"<<bottom<<" { "<<histogram[0];
        //    for(int k=1;k<NBBINS;k++){std::cerr<<","<<histogram[k];}std::cerr<<"}\n";
        //}
        return (NBBINS-1)*((val-top)/double(top-bottom));
    }

    void enhaceGrayMode(bilde::Buffer<bilde::t_uint8> in,bilde::Buffer<bilde::t_uint8> out){
        //const int NBBINS=256/(1<<(8-BITDEPTH));
        bilde::Buffer<bilde::t_uint8> inCp(in.width, in.height);
        bilde::operations::essential::__copy__(inCp,in);
        bilde::operations::essential::__divideBy__<bilde::t_uint8>(inCp,inCp,1<<(8-BITDEPTH));

        bilde::operations::__histogram__::__getCummulativeHistogram__<BINT,NBBINS>(inCp,globalCummulativeHistogram);
        bilde::util::timer::Timer ihtimer;
        bilde::operations::integral_histograms::IntegralHistogram<BINT,NBBINS,true> ih(inCp);
        //bilde::operations::integral_histograms::IntegralHistogram<BINT,NBBINS,true> ih(in);
        /*
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
        */
        if(this->verbose>4){
            std::cerr<<"Calculated cummulative integral histogram for "<<in.width<<"x"<<in.height<<" x "<<NBBINS<<" Bins.  Duration "<<ihtimer.getSec()<<" sec.\n";
        }
        auto iter=ih.getIterator(windowWidth,windowHeight);    
        if(mode=="linear"){
            iter.applyFilter(out,in,enhaceGrayLinearStatic);
            bilde::operations::essential::__multiplyBy__<bilde::t_uint8>(out,out,1<<(8-BITDEPTH));
            return;
        }
        if(mode=="null"){
            iter.applyFilter(out,in,enhaceGrayNullStatic);
            bilde::operations::essential::__multiplyBy__<bilde::t_uint8>(out,out,1<<(8-BITDEPTH));
            return;
        }
        if(mode=="equalise"){
            if(this->verbose>3){
                bilde::util::timer::Timer t;
                iter.applyFilter(out,in,enhaceGrayHistEqualiseStatic);
                std::cerr<<"Equalised histogram locally duration "<<t.getSec()<<" sec.\n";
            }else{
                iter.applyFilter(out,in,enhaceGrayHistEqualiseStatic);
            }
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
            throw "ditDepth out of range";
    }
}


}
}
}

#endif /* ENHANCE_GRAYSCALE_HPP_ */