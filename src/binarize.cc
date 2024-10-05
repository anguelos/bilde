#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "bilde.hpp"

//commetn

int bitDepth;
std::vector<std::string> inFilenames;
std::vector<std::string> outFilenames;
std::string method;

bool registerParameters(int argc,char** argv){
bilde::util::Args("Binarization bsaed on the Local Otsu Filter","anguelos.nicolaou@gmail.com");
bilde::util::Args().addIntParam(false,"d","bit-depth","The bit-depth that will be taken in to account when computing integral histograms",&bitDepth).setDefault(5);
bilde::util::Args().addStringListParam(false,"i","input","The input file-name list files of a valid graphics format.",-1,&inFilenames);
bilde::util::Args().addStringListParam(false,"o","output","The output file-name list files of a valid graphics format. The name ",-1,&outFilenames);
bilde::util::Args().addStringSelectParam(false,"m","method","The method that will be used for binarization.",{"otsu","lof"},&method).setDefault("otsu");
bilde::util::Args().parseArgv(argc,argv);
if(bilde::util::Args().validate()){
    if(argc==1 || bilde::util::Args().getBoolParam("-h")){
        std::cerr<<bilde::util::Args().getHelpMessage();
        exit(0);
    }
    if(inFilenames.size()!=outFilenames.size() ||outFilenames.size()<1){
        std::cerr<<"-i and -o must have the same number of parameters at least one each.\nAborting.\n";
        exit(0);
    }
    if(bitDepth>8 ||bitDepth<=2){
        std::cerr<<"-d must be grater than 2 and equal or lesser to 8.\nAborting.\n";
        exit(0);
    }
    return true;
}else{
    throw "Argumnts could not be validated!";
    return false;
}

}

int main(int argc,char** argv){
    registerParameters(argc,argv);
    if(VERBOSE>5){
        cv::namedWindow("Input Image",0);
        cv::resizeWindow("Input Image",350,500);
        cv::moveWindow("Input Image",50,50);
        cv::namedWindow("Output Image",0);
        cv::resizeWindow("Output Image",350,500);
        cv::moveWindow("Output Image",450,50);
    }
    for(int i=0;i<inFilenames.size();i++){
        std::string inFname=inFilenames[i];
        std::string outFname=outFilenames[i];
        if(method=="otsu"){
            cv::Mat inImg=cv::imread(inFname, cv::IMREAD_GRAYSCALE);
            cv::Mat outImg=cv::Mat(inImg.rows,inImg.cols, CV_8UC1 );
            cv::Mat lofImg=cv::Mat(inImg.rows,inImg.cols, CV_8UC1 );
            bilde::methods::binarization::getOtsuBinarization(inImg).copyTo(outImg);
            //bilde::methods::binarization::getLofFilter(ih,20).copyTo(lofImg);
            if(VERBOSE>5){
                cv::imshow("Input Image",inImg);
                cv::imshow("Output Image",outImg);
                std::cerr<<"Displaying Images, press any key to continue.\n";
                cv::waitKey();
            }
            if(VERBOSE>2){std::cerr<<"Saving to :"<<outFname<<"\n";}
            cv::imwrite(outFname,outImg);
        }else if(method=="lof"){
            cv::Mat inImg=cv::imread(inFname, cv::IMREAD_GRAYSCALE);
            cv::Mat outImg=cv::Mat(inImg.rows,inImg.cols, CV_8UC1 );
            bilde::methods::binarization::getLofBinarization(inImg,bitDepth).copyTo(outImg);
            if(VERBOSE>5){
                cv::imshow("Input Image",inImg);
                cv::imshow("Output Image",outImg);
                std::cerr<<"Displaying Images, press any key to continue.\n";
                cv::waitKey();
            }
            if(VERBOSE>2){std::cerr<<"Saving to :"<<outFname<<"\n";}
            cv::imwrite(outFname,outImg);
        }else{
            std::cerr<<"method:"<<method<<"\n";
        }
    }
    if(VERBOSE>5){
        cv::destroyWindow("Input Image");
        cv::destroyWindow("Output Image");
    }
}
