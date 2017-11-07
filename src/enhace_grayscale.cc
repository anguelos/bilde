/*
 * illumination_adjustment.cc
 *
 *  Created on: Dec 4, 2013
 *      Author: anguelos
 */
#include "config.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include "bilde.hpp"


std::vector<std::string> inFnames;
std::vector<std::string> outFnames;
std::string mode;
int topCentile;
int bottomCentile;
int bitDepth;
int windowWidth;
int windowHeight;
int globalHistogramCoeficient;
int localHistogramCoeficient;

typedef bilde::t_uint32 BINT;
const int MAX_HISTOGRAM_SIZE=256;
BINT globalCummulativeHistogram[MAX_HISTOGRAM_SIZE];



template <typename BINT,int NBBINS> int enhaceGrayLinear(int val,const BINT* histogram){
    typedef typename bilde::operations::integral_histograms::IntegralHistogram<BINT,NBBINS,true>::HistogramOperations operations;
    int sum=histogram[NBBINS-1];
    //int top=operations::__getCentilePos__(.01*topCentile,histogram);
    int top=operations::__getCentilePos__(.01*topCentile,histogram);
    int bottom=operations::__getCentilePos__(.01*bottomCentile,histogram);
    if(VERBOSE>5){
        std::cerr<<"Top:"<<top<<" Bottom:"<<bottom<<" { "<<histogram[0];
        for(int k=1;k<NBBINS;k++){std::cerr<<","<<histogram[k];}std::cerr<<"}\n";
    }
    return (NBBINS-1)*((val-top)/double(top-bottom));
}


template <typename BINT,int NBBINS> int enhaceGrayHistEqualise(int val,const BINT* histogram){
    //return round((NBBINS-1)*((histogram[val])/double( histogram[NBBINS-1])));
    int naturalRation=(globalCummulativeHistogram[NBBINS-1]/histogram[NBBINS-1])*localHistogramCoeficient;
    return round((NBBINS-1)*((naturalRation*histogram[val]+globalHistogramCoeficient*globalCummulativeHistogram[val])/double(naturalRation* histogram[NBBINS-1]+globalHistogramCoeficient*globalCummulativeHistogram[NBBINS-1])));
}


template <typename BINT,int NBBINS> int enhaceGrayNull(int val,const BINT* histogram){
    return val;
}


template <int BITDEPTH> void enhaceGrayMode(bilde::Buffer<bilde::t_uint8> in,bilde::Buffer<bilde::t_uint8> out){
    const int NBBINS=256/(1<<(8-BITDEPTH));
    bilde::Buffer<bilde::t_uint8> inCp(in.width,in.height);
    bilde::operations::essential::__copy__(inCp,in);
    bilde::operations::essential::__divideBy__<bilde::t_uint8>(inCp,inCp,1<<(8-BITDEPTH));

    bilde::operations::__histogram__::__getCummulativeHistogram__<BINT,NBBINS>(inCp,globalCummulativeHistogram);
    bilde::util::timer::Timer ihtimer;
    bilde::operations::integral_histograms::IntegralHistogram<BINT,NBBINS,true> ih(inCp);
    if(VERBOSE>4){
        std::cerr<<"Calculated cummulative integral histogram for "<<in.width<<"x"<<in.height<<" x "<<NBBINS<<" Bins.  Duration "<<ihtimer.getSec()<<" sec.\n";
    }
    auto iter=ih.getIterator(windowWidth,windowHeight);    
    if(mode=="linear"){
        iter.applyFilter(out,inCp,enhaceGrayLinear<BINT,NBBINS>);
        bilde::operations::essential::__multiplyBy__<bilde::t_uint8>(out,out,1<<(8-BITDEPTH));
        return;
    }
    if(mode=="null"){
        iter.applyFilter(out,inCp,enhaceGrayNull<BINT,NBBINS>);
        bilde::operations::essential::__multiplyBy__<bilde::t_uint8>(out,out,1<<(8-BITDEPTH));
        return;
    }
    if(mode=="equalise"){
        if(VERBOSE>3){
            bilde::util::timer::Timer t;
            iter.applyFilter(out,inCp,enhaceGrayHistEqualise<BINT,NBBINS>);
            std::cerr<<"Equalised histogram locally duration "<<t.getSec()<<" sec.\n";
        }else{
            iter.applyFilter(out,inCp,enhaceGrayHistEqualise<BINT,NBBINS>);
        }
        bilde::operations::essential::__multiplyBy__<bilde::t_uint8>(out,out,1<<(8-BITDEPTH));
        return;
    }

    throw "enhaceGray inrecognised mode";
}

void enhaceGray(bilde::Buffer<bilde::t_uint8> in,bilde::Buffer<bilde::t_uint8> out){
    switch(bitDepth){
        case 8:enhaceGrayMode<8>(in,out);break;
        case 7:enhaceGrayMode<7>(in,out);break;
        case 6:enhaceGrayMode<6>(in,out);break;
        case 5:enhaceGrayMode<5>(in,out);break;
        case 4:enhaceGrayMode<4>(in,out);break;
        case 3:enhaceGrayMode<3>(in,out);break;
    default:
        throw "ditDept out of range";
    }
}

void declareVariables(int argc,char** argv){
	bilde::util::Args("Illumination adjustment","anguelos.nicolaou@gmail.com");
    bilde::util::Args().addIntParam(false, "w", "window-width"," The width of the local window.",&windowWidth).setDefault(51);
    bilde::util::Args().addIntParam(false, "H", "window-height"," The height of the local window.",&windowHeight).setDefault(51);
	bilde::util::Args().addIntParam(false, "d", "bit-depth"," The bitdepth of the input image. Any pixel less significant than this will be ignored.",&bitDepth).setDefault(6);
    bilde::util::Args().addIntParam(false, "t", "top-centile"," The top centile to be considerd as white. ",&topCentile).setDefault(95);
    bilde::util::Args().addIntParam(false, "b","bottom-centile"," The bottom centile to be considerd as black. ",&bottomCentile).setDefault(5);
    bilde::util::Args().addStringListParam(false, "i","input-images"," A list of images to be processed. ",1,&inFnames);
    bilde::util::Args().addStringListParam(false, "o","output-images"," A list of files where the outputs will be saved. ",1,&outFnames);
    bilde::util::Args().addStringSelectParam(false, "m", "mode"," The mode under which the is going to be .", {"linear","null","equalise"}, &mode).setDefault("equalise");
    bilde::util::Args().addIntParam(false, "g","global-coefficient"," In equalise mode, the factor by which the global histogram will contribute to the computation ",&globalHistogramCoeficient).setDefault(0);
    bilde::util::Args().addIntParam(false, "l","local-coefficient"," In equalise mode, the factor by which the local histogram will contribute to the computation ",&localHistogramCoeficient).setDefault(1);
	bilde::util::Args().parseArgv(argc, argv);
	if(!bilde::util::Args().validate()){
		exit(1);
	}
    if(inFnames.size()!=outFnames.size()||inFnames.size()==0){
        std::cerr<<"error : input filenames and output filenames must have the same number of files and at least one.\nAborting\n";
        exit(1);
    }
}

int main(int argc,char** argv){
    try{
        declareVariables(argc,argv);
        if(bilde::util::Args().verboseLevel>3){
            cv::namedWindow("in");
            cv::namedWindow("out");
        }
        for(int k=0;k<inFnames.size();k++){
            cv::Mat inImg=cv::imread(inFnames[k], 0);
            cv::Mat outImg=inImg.clone();


            enhaceGray(inImg,outImg);

            imwrite(outFnames[k], outImg );            
            if(VERBOSE>3){
                cv::imshow("in",inImg);
                cv::imshow("out",outImg);
                cv::waitKey();
            }
        }
    }catch(const char* msg){
        std::cout<<"Aborted because of exception:\n"<<msg<<"\n";
    }
}




