#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>


#include "bilde.hpp"

static const int BOX_FILTER_PAD=100;

std::vector<std::string> inFnames;
std::vector<std::string> outFnames;

int windowWidth;
int windowHeight;
int tracerDencity;
int compareDistance;
int minimumLetterHeight;
int maximumLetterHeight;

void declareVariables(int argc,char** argv){
    bilde::util::Args("Textline segmentation","anguelos.nicolaou@gmail.com");
    bilde::util::Args().addStringListParam(false, "i","input-images"," A list of binary images to be processed. ",1,&inFnames);
    bilde::util::Args().addStringListParam(false, "o","output-images"," A list of output filenames. ",1,&outFnames);
    bilde::util::Args().addIntParam(false, "w", "window-width"," The width of the local window % of the estimated letter height.",&windowWidth).setDefault(600);
    bilde::util::Args().addIntParam(false, "H", "window-height"," The height of the local window.",&windowHeight).setDefault(100);
    bilde::util::Args().addIntParam(false, "t", "nb-tracres"," The number of skiped lines between tracers. 1 is the densest.",&tracerDencity).setDefault(1);
    bilde::util::Args().addIntParam(false, "d", "comparisson-distance"," The number the distance from a tracer as a percentage of the letter height.",&compareDistance).setDefault(50);
    bilde::util::Args().addIntParam(false, "m", "minimum-letter-height"," The minimum acceptable height for letter height estimation.",&minimumLetterHeight).setDefault(10);
    bilde::util::Args().addIntParam(false, "M", "maximum-letter-height"," The maximum acceptable height for letter height estimation.",&maximumLetterHeight).setDefault(500);
    bilde::util::Args().parseArgv(argc, argv);
    if(!bilde::util::Args().validate()){
        exit(1);
    }
    if(inFnames.size()!=outFnames.size()||inFnames.size()==0){
        std::cerr<<"error : input filenames and output filenames must have the same number of files and at least one.\nAborting\n";
        exit(1);
    }
}


cv::Mat boxFilter(cv::Mat inImg,int width,int height){
    if(inImg.type()!=CV_8UC1){
        std::cerr<<"error : boxFilter only works with 8bit single channel images.\n";
        exit(1);
    }
    cv::Mat outImg(inImg.rows,inImg.cols,CV_8UC1);
    bilde::Buffer<bilde::t_uint8> inBuf(inImg);
    bilde::Buffer<bilde::t_uint8> outBuf(outImg);
    bilde::operations::integral_images::getBoxFilter<bilde::t_uint8,BOX_FILTER_PAD>(inBuf,outBuf,width,height);
    return outImg;
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

int main(int argc,char** argv){
    try{
        declareVariables(argc,argv);
    }catch(const char* msg){
        std::cerr<<"Threw exception "<<msg<<"\naborting.\n";
        exit(1);
    }

    for(int fileCount=0;fileCount<inFnames.size();fileCount++){
        std::string inFname=inFnames[fileCount];
        std::string outFname=outFnames[fileCount];
        cv::Mat inImg=cv::imread(inFname, 0);
        inImg.convertTo(inImg, CV_8U);
        if(VERBOSE>5){
            std::cerr<<"Loaded Image of size ("<<inImg.cols<<","<< inImg.rows<<")\n";
        }

        cv::Mat CC(inImg.rows,inImg.cols,CV_32SC1);
        bilde::Buffer<bilde::t_sint32> ccBuf(CC);
        cv::Mat LLA(inImg.rows,inImg.cols,CV_32SC1);
        cv::Mat LLC(inImg.rows,inImg.cols,CV_32SC1);
        //int nbComponents=bilde::operations::components::__labelConnectedComponents__<bilde::t_uint8>(CC,inImg,8);
        int nbComponents=bilde::operations::components::__labelConnectedComponents__<bilde::t_uint8>(ccBuf,inImg,8);
        int letterHeight=getLetterHeight(CC,nbComponents,minimumLetterHeight,maximumLetterHeight);
        cv::Mat LA(inImg.rows,inImg.cols,inImg.type());
        cv::Mat LC(inImg.rows,inImg.cols,inImg.type());
        cv::Mat outImg(inImg.rows,inImg.cols,inImg.type());
        cv::Mat blurred(inImg.rows,inImg.cols,inImg.type());
        blurred=boxFilter(inImg,windowWidth*compareDistance/200.0,windowHeight*compareDistance/200.0);
        bilde::operations::essential::__setTo__<bilde::t_uint8>(LA,255);
        bilde::operations::essential::__setTo__<bilde::t_uint8>(LC,0);
        __traceMaximaLeftToRight__(LA,blurred,letterHeight*compareDistance/100.0,tracerDencity,0);
        __traceMaximaRightToLeft__(LA,blurred,letterHeight*compareDistance/100.0,tracerDencity,0);
        __traceMinimaLeftToRight__(LC,blurred,letterHeight*compareDistance/100.0,tracerDencity,255);

        if(VERBOSE>3){
            cv::imwrite(outFname+".la.png",LA);
            cv::imwrite(outFname+".lc.png",LC);
            cv::imwrite(outFname+".blurred.png",blurred);
        }
        cv::imwrite(outFname,LA);
        //std::cerr<<"height : "<<getLetterHeight(CC,nbComponents,minimumLetterHeight,maximumLetterHeight)<<"\n";
    }
}
