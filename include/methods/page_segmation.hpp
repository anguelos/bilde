#ifndef PAGE_SEGMATION_HPP
#define PAGE_SEGMATION_HPP

#include <iostream>

namespace bilde{
namespace methods {



namespace textline_segmentation{

struct TextlineSegmenter{
    static const int BOX_FILTER_PAD=400;
    const int windowWidth;
    const int windowHeight;
    const int tracerDencity;
    const int compareDistance;
    const int minimumLetterHeight;
    const int maximumLetterHeight;

    const int verbose;
    std::ostream & err_stream;

    TextlineSegmenter(int windowWidth_=600, int windowHeight_=100, int tracerDencity_=1, 
            int compareDistance_=50, int minimumLetterHeight_=10, int maximumLetterHeight_=500,
            int verbose_=0, std::ostream & err_stream_=std::cerr):
        windowWidth(windowWidth_), windowHeight(windowHeight_),
        tracerDencity(tracerDencity_), compareDistance(compareDistance_), minimumLetterHeight(minimumLetterHeight_),
        maximumLetterHeight(maximumLetterHeight_), verbose(verbose_), err_stream(err_stream_){
    }

    void __traceMaximaLeftToRight__(bilde::Buffer<bilde::t_uint8> out,bilde::Buffer<bilde::t_uint8> blurred,int compaireDistance,int trDencity,bilde::t_uint8 shreder){
        std::vector<int> tracers(blurred.height/tracerDencity,0);
        int pos=0;
        for(int k=1;k<tracers.size()-1;k++){
            tracers[k]=pos;
            pos+=trDencity;
        }
        tracers[0]=tracers[1];
        tracers[tracers.size()-2]=tracers[tracers.size()-1];
        int top,bottom;
        for(int x=0;x<blurred.width;x++){
            for(int tr=0;tr<tracers.size();tr++){
                top=tracers[tr]>compaireDistance?blurred.getRow(tracers[tr]-compaireDistance)[x]:blurred.getRow(0)[x];
                bottom=tracers[tr]+compaireDistance<blurred.height?blurred.getRow(tracers[tr]+compaireDistance)[x]:blurred.getRow(blurred.height-1)[x];
                if(top>bottom && tracers[tr]<(blurred.height-2)){
                    tracers[tr]++;
                }else if(top<bottom && tracers[tr]>0){
                    tracers[tr]--;
                }
                out.getRow(tracers[tr])[x]=shreder;
            }
        }
    }

    void __traceMaximaRightToLeft__(bilde::Buffer<bilde::t_uint8> out,bilde::Buffer<bilde::t_uint8> blurred,int compaireDistance,int trDencity,bilde::t_uint8 shreder){
        std::vector<int> tracers(blurred.height/tracerDencity,0);
        int pos=0;
        for(int k=1;k<tracers.size()-1;k++){
            tracers[k]=pos;
            pos+=trDencity;
        }
        tracers[0]=tracers[1];
        tracers[tracers.size()-2]=tracers[tracers.size()-1];
        int top,bottom;
        for(int x=blurred.width-1;x>=0;x--){
            for(int tr=0;tr<tracers.size();tr++){
                top=tracers[tr]>compaireDistance?blurred.getRow(tracers[tr]-compaireDistance)[x]:blurred.getRow(0)[x];
                bottom=tracers[tr]+compaireDistance<blurred.height?blurred.getRow(tracers[tr]+compaireDistance)[x]:blurred.getRow(blurred.height-1)[x];
                if(top>bottom && tracers[tr]<(blurred.height-2)){
                    tracers[tr]++;
                }else if(top<bottom && tracers[tr]>0){
                    tracers[tr]--;
                }
                out.getRow(tracers[tr])[x]=shreder;
            }
        }
    }

    void __traceMinimaLeftToRight__(bilde::Buffer<bilde::t_uint8> out,bilde::Buffer<bilde::t_uint8> blurred,int compaireDistance,int trDencity,bilde::t_uint8 shreder){
        std::vector<int> tracers(blurred.height/tracerDencity,0);
        int pos=0;
        for(int k=1;k<tracers.size()-1;k++){
            tracers[k]=pos;
            pos+=trDencity;
        }
        tracers[0]=tracers[1];
        tracers[tracers.size()-2]=tracers[tracers.size()-1];
        int top,bottom;
        for(int x=0;x<blurred.width;x++){
            for(int tr=0;tr<tracers.size();tr++){
                top=tracers[tr]>compaireDistance?blurred.getRow(tracers[tr]-compaireDistance)[x]:blurred.getRow(0)[x];
                bottom=tracers[tr]+compaireDistance<blurred.height?blurred.getRow(tracers[tr]+compaireDistance)[x]:blurred.getRow(blurred.height-1)[x];
                if(top<bottom && tracers[tr]<(blurred.height-2)){
                    tracers[tr]++;
                }else if(top>bottom && tracers[tr]>0){
                    tracers[tr]--;
                }
                out.getRow(tracers[tr])[x]=shreder;
            }
        }
    }

    void __traceMinimaRightToLeft__(bilde::Buffer<bilde::t_uint8> out,bilde::Buffer<bilde::t_uint8> blurred,int compaireDistance,int trDencity,bilde::t_uint8 shreder){
        std::vector<int> tracers(blurred.height/tracerDencity,0);
        int pos=0;
        for(int k=1;k<tracers.size()-1;k++){
            tracers[k]=pos;
            pos+=trDencity;
        }
        tracers[0]=tracers[1];
        tracers[tracers.size()-2]=tracers[tracers.size()-1];
        int top,bottom;
        for(int x=blurred.width-1;x>=0;x--){
            for(int tr=0;tr<tracers.size();tr++){
                top=tracers[tr]>compaireDistance?blurred.getRow(tracers[tr]-compaireDistance)[x]:blurred.getRow(0)[x];
                bottom=tracers[tr]+compaireDistance<blurred.height?blurred.getRow(tracers[tr]+compaireDistance)[x]:blurred.getRow(blurred.height-1)[x];
                if(top<bottom && tracers[tr]<(blurred.height-2)){
                    tracers[tr]++;
                }else if(top>bottom && tracers[tr]>0){
                    tracers[tr]--;
                }
                out.getRow(tracers[tr])[x]=shreder;
            }
        }
    }
    int getLetterHeight(bilde::Buffer<bilde::t_sint32> components,int nbComponents,int minLetterHeight,int maxLetterHeight){
        bilde::Buffer<bilde::t_real32> features(10,nbComponents);
        bilde::operations::components::__getLabeledComponentFeatures__(features,components);
        int topColumn=bilde::operations::components::LabeledComponentsFeatures::LC_TOP_POS;
        int bottomColumn=bilde::operations::components::LabeledComponentsFeatures::LC_BOTTOM_POS;
        int nbPixels=bilde::operations::components::LabeledComponentsFeatures::LC_NBPIXELS_POS;
        std::vector<int> heights(maxLetterHeight+1,0);
        int label,height;
        for(label=1;label<features.height;label++){
            if(features.getRow(label)[nbPixels]>0){
                height=1+features.getRow(label)[bottomColumn]-features.getRow(label)[topColumn];
                //std::cerr<<"height:"<<height<<"\n";
                if(height>4 && height<maximumLetterHeight-4){
                    heights[height]+=5;
                    heights[height-1]+=4;
                    heights[height+1]+=4;
                    heights[height-2]+=3;
                    heights[height+2]+=3;
                    heights[height-3]+=2;
                    heights[height+3]+=2;
                    heights[height-4]+=1;
                    heights[height+4]+=1;
                }
            }
        }
        int maxFound=heights[minimumLetterHeight];
        int maxFoundPos=minimumLetterHeight;
        for(height=minimumLetterHeight+1;height<=maxLetterHeight;height++){
            //std::cerr<<height<<":"<<heights[height]<<"\n";
            if(heights[height]>maxFound){
                maxFoundPos=height;
                maxFound=heights[height];
            }
        }
        return maxFoundPos;
    }
    int call(Buffer<t_uint8> out, Buffer<t_uint8> in_bin){
        std::cerr<<"CALL(1,2) 1"<<std::endl;
        Buffer<t_uint8> blurred(in_bin.width, in_bin.height);
        std::cerr<<"CALL(1,2) 2"<<std::endl;
        bilde::operations::integral_images::getBoxFilter<bilde::t_uint8,TextlineSegmenter::BOX_FILTER_PAD>(in_bin, blurred, this->windowWidth, this->windowHeight);
        std::cerr<<"CALL(1,2) 3"<<std::endl;
        return call(out, in_bin, blurred);
    }

    int call(Buffer<t_uint8> out, Buffer<t_uint8> in_bin, Buffer<t_uint8> blurred){
        if(verbose > 5){
            err_stream<<"Textlinesegment Loaded Image of size ("<<in_bin.width<<","<< in_bin.height<<")\n";
        }
        std::cerr<<"CALL 1"<<std::endl;
        Buffer<t_label> CC(in_bin.width,in_bin.height);
        //cv::Mat LLA(inImg.rows,inImg.cols,CV_32SC1);
        Buffer<t_label> LLA(in_bin.width,in_bin.height);

        //cv::Mat LLC(inImg.rows,inImg.cols,CV_32SC1);
        Buffer<t_label> LLC(in_bin.width,in_bin.height);
        //int nbComponents=bilde::operations::components::__labelConnectedComponents__<bilde::t_uint8>(CC,inImg,8);
        int nbComponents=bilde::operations::components::__labelConnectedComponents__<bilde::t_uint8>(CC,in_bin,8);
        std::cerr<<"CALL 2"<<std::endl;
        int letterHeight=getLetterHeight(CC, nbComponents, minimumLetterHeight, maximumLetterHeight);
        std::cerr<<"CALL 3"<<std::endl;
        //cv::Mat LA(inImg.rows,inImg.cols,inImg.type());
        Buffer<t_uint8> LA(in_bin.width, in_bin.height);
        //cv::Mat LC(inImg.rows,inImg.cols,inImg.type());
        Buffer<t_uint8> LC(in_bin.width, in_bin.height);
        //cv::Mat outImg(inImg.rows,inImg.cols,inImg.type());
        //cv::Mat blurred(inImg.rows,inImg.cols,inImg.type());
        //blurred=boxFilter(inImg,windowWidth*compareDistance/200.0,windowHeight*compareDistance/200.0);
        
        bilde::operations::essential::__setTo__<bilde::t_uint8>(LA,255);
        std::cerr<<"CALL 4"<<std::endl;
        bilde::operations::essential::__setTo__<bilde::t_uint8>(LC,0);
        std::cerr<<"CALL 5"<<std::endl;
        __traceMaximaLeftToRight__(LA,blurred,letterHeight*compareDistance/100.0,tracerDencity,0);
        std::cerr<<"CALL 6"<<std::endl;
        __traceMaximaRightToLeft__(LA,blurred,letterHeight*compareDistance/100.0,tracerDencity,0);
        std::cerr<<"CALL 7"<<std::endl;
        __traceMinimaLeftToRight__(LC,blurred,letterHeight*compareDistance/100.0,tracerDencity,255);
        std::cerr<<"CALL 8"<<std::endl;

        if(verbose>3){
            //cv::imwrite(outFname+".la.png",LA);
            //cv::imwrite(outFname+".lc.png",LC);
            //cv::imwrite(outFname+".blurred.png",blurred);
            //cv::imwrite(outFname+".blurred.png",blurred);
        }
        //cv::imwrite(outFname,LA);
        //std::cerr<<"height : "<<getLetterHeight(CC,nbComponents,minimumLetterHeight,maximumLetterHeight)<<"\n";
        return 1;
    }

};



/*
cv::Mat boxFilter(Buffer<uint8_t> out, Buffer<uint8_t> in,int width,int height){
    if(inImg.type()!=CV_8UC1){
        std::cerr<<"error : boxFilter only works with 8bit single channel images.\n";
        exit(1);
    }
    bilde::Buffer<bilde::t_uint8> inBuf(inImg);
    bilde::Buffer<bilde::t_uint8> outBuf(outImg);
    bilde::operations::integral_images::getBoxFilter<bilde::t_uint8,TextlineSegmenter::BOX_FILTER_PAD>(inBuf,outBuf,width,height);
    return outImg;
}
*/













}
}
}

#endif // PAGE_SEGMATION_HPP
