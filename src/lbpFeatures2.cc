#include <ctime>
#include <boost/algorithm/string/replace.hpp>

//#include <opencv2/core/core.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "bilde.hpp"

#include <cstdlib>

/*
cv::Mat getRGB2PCA(std::string fname){
    cv::Mat floatColors;cv::Mat uint8Colors;cv::Mat floatGraylevels;cv::Mat uint8Graylevels;cv::Mat average;
    std::vector<cv::Mat> rgbImages;
    cv::Mat inputImg=cv::imread(fname,CV_LOAD_IMAGE_COLOR);
    std::cerr<<"Input:\t"<<inputImg.reshape(0,1).size()<<"\n";
    cv::split(inputImg.reshape(0,1),rgbImages);
    std::cerr<<"rgb[0]:\t"<<rgbImages[0].size()<<"\n";
    cv::hconcat(std::vector<cv::Mat>({rgbImages[0].reshape(0,1),rgbImages[1].reshape(0,1),rgbImages[2].reshape(0,1)}),uint8Colors);
    uint8Colors.convertTo(floatColors, CV_32FC1);
    floatColors=(floatColors/255);

    std::cerr<<"uint8Colors:\t"<<uint8Colors.size()<<"\n";

    return inputImg;


    cv::Mat result=cv::Mat::ones(inputImg.cols,inputImg.rows,CV_8U);
    std::cerr<<"loaded: inputImg ["<<inputImg.cols<<","<<inputImg.rows<<"]\n";
    inputImg.reshape(inputImg.cols*inputImg.rows,3).convertTo(floatColors, CV_32FC1);
    std::cerr<<"reshape: floatColors ["<<floatColors.cols<<","<<floatColors.rows<<"]\n";
    cv::PCA pcaObj=cv::PCA( floatColors,average, CV_PCA_DATA_AS_ROW,1);
    std::cerr<<"pca created!\n";
    pcaObj.project(floatColors, floatGraylevels);
    std::cerr<<"pca-output: floatGraylevels ["<<floatGraylevels.cols<<","<<floatGraylevels.rows<<"]\n";
    floatGraylevels=floatGraylevels.reshape(1,inputImg.cols)*255;
    std::cerr<<"reshape: floatGraylevels ["<<floatGraylevels.cols<<","<<floatGraylevels.rows<<"]\n";
    floatGraylevels.convertTo(uint8Graylevels,CV_8U);
    std::cerr<<"typecast: uint8Graylevels ["<<floatGraylevels.cols<<","<<floatGraylevels.rows<<"]\n";
    cv::namedWindow("input");
    cv::namedWindow("transformed");
    cv::imshow("input",inputImg);
    cv::imshow("transformed",uint8Graylevels);
    cv::waitKey();
    return uint8Graylevels;
}
*/


std::vector<std::string> inputFileNames;
std::string interpolation;
std::string cmpOperation;
std::vector<std::string> radiiList;
std::vector<std::string> windowList;
bool oneRowPerWindow;
int nbSamples;
bool includeOutputHeader;
std::string oFname;
std::string hSeparator;
std::string cmpThreshold;
int whiteBorderSize;

bilde::Buffer<bilde::t_uint8> getSubImage(bilde::Buffer<bilde::t_uint8> img,std::string region){
    std::stringstream ss(region);
    char sep;char mode;
    double l,t,r,b;
    ss>>l>>sep>>t>>sep>>r>>sep>>b;
    if(l>1 || t>1 || r>1 || b>1 ){
        l=int(l);t=int(t);r=int(r);b=int(b);
    }else{
        l*=img.width;r*=img.width;
        t*=img.height;b*=img.height;
        r--;b--;
    }

    if(bilde::util::Args().verboseLevel>5){
        std::cerr<<"Extracting patch at ["<<l<<","<<t<<","<<b<<","<<r<<"]\n";
    }
    if(l<0 || l>=r ||r > img.width|| t<0 || t>=b || b > img.height ){
        if(bilde::util::Args().verboseLevel>3){
            std::cerr<<"Ignoring patch at ["<<l<<","<<t<<","<<b<<","<<r<<"]\n";
        }
        return bilde::Buffer<bilde::t_uint8>(1,1);
    }
    return bilde::Buffer<bilde::t_uint8>(img, l, t, r, b);

}

int main(int argc,char**argv){
    try{
        bilde::util::timer::Timer totalTimer;
        bilde::util::timer::Timer perFileTimer;
        bilde::util::Args("Extracts LBP features and outputs them in csv format.\n","anguelos.nicolaou@gmail.com");
        bilde::util::Args().addStringListParam(false,"i","input-files","A list of image files supported by opencv (tif,png,bmp,jpeg etc.).",-1,&inputFileNames);
        bilde::util::Args().addStringSelectParam(false,"s","sampling","The maner of sampling the points on the periphery",{"nearEuclidean","bilinear","arc-convolution","nearCityblock","nearChessboard"},&interpolation).setDefault("nearEuclidean");
        bilde::util::Args().addStringSelectParam(false,"c","comparisson-operator","The operator that compares the sample on the circle to  the central. For the standard LBP this should be threshold-one-tail.",{"one-tail","two-tails"},&cmpOperation).setDefault("one-tail");
        bilde::util::Args().addStringParam(false,"T","comparisson-threshold","A number describing the threshold at whitch the tails are choped. If you want automatic estimation you can use 'otsu'. For the standart LBP set this to 0.",&cmpThreshold).setDefault("0");
        bilde::util::Args().addStringListParam(false,"r","radii","A list of positive numbers indicating the radii for which the LBP will be computed.",-1,&radiiList).setDefault({"1","2","3"});
        bilde::util::Args().addStringParam (false,"o","output","The path the outputfile. If the file exists it will be appended. In oreder to print to the standart output put 'stdout'",&oFname).setDefault("stdout");
        bilde::util::Args().addStringParam (false,"S","horizontal-separator","The sequence which separates two columns in the csv",&hSeparator).setDefault(", ");
        bilde::util::Args().addIntParam (false,"n","number-of-samples","An integer defining the number of points sampled on the circle and compared to the central pixel.",&nbSamples).setDefault(8);
        bilde::util::Args().addBoolParam(false,"H","output-header","A boolean defining wether the first line of the CSV should contain the name of each feature.",&includeOutputHeader).setDefault(false);
        bilde::util::Args().addStringListParam(false,"w","windows","A list of positive numbers indicating the radii for which the LBP will be computed.",-1,&windowList).setDefault({"0,0,1,1"});
        bilde::util::Args().addBoolParam(false,"d","distict-pattern-per-window","Whether each window should be considered a distinct pattern or not.",&oneRowPerWindow).setDefault(0);
        bilde::util::Args().addIntParam (false,"b","white-border-size","An integer defining the width of the white padding drawn around the image before the patterns are drawn.",&whiteBorderSize).setDefault(0);
        if(!bilde::util::Args().validate()){
            std::cerr<<"Bad arguments:\n"<<argv[0];
            for(int k=1;k<argc;k++){
                std::cerr<<" "<<argv[k];
            }
            std::cerr<<"\n"<<bilde::util::Args().getValidationMsg();
            std::cerr<<"\naborting!\n";
            std::exit(1);
        }
        bilde::util::Args().parseArgv(argc,argv);
        if(argc==1){
            std::cerr<<bilde::util::Args().getHelpMessage()<<"\nAborting!\n";
            std::exit(1);
        }
        std::vector<double> radii=bilde::util::argv::strVector2double(radiiList);

        std::ostream *p_out;
        std::ofstream outFile;
        if(oFname=="stdout"){
            p_out=&std::cout;
        }else{
            outFile.open(oFname.c_str(), std::ofstream::out);// append seems impractical
            p_out=&outFile;
        }

        std::ostream& out=*p_out;
        for(auto iterFname=inputFileNames.begin();iterFname != inputFileNames.end();++iterFname){
            std::vector<std::vector< std::string> > outputHistograms;
            perFileTimer.tic();
            cv::Mat tmpInImg;
            tmpInImg=cv::imread(*iterFname, cv::IMREAD_GRAYSCALE);
            cv::Mat inImg=cv::Mat::ones(tmpInImg.rows+whiteBorderSize*2,tmpInImg.cols+whiteBorderSize*2 , CV_8U)*255;
            tmpInImg.copyTo(inImg.colRange(whiteBorderSize,inImg.cols-whiteBorderSize).rowRange(whiteBorderSize,inImg.rows-whiteBorderSize));


            if(inImg.cols==0 && inImg.rows==0){
                std::cerr<<"Failed to open '"<<*iterFname<<"'\n";
                throw("could not open file");
            }
            if(bilde::util::Args().verboseLevel>1){
                std::cerr<<"Processing "<<*iterFname<<" ["<<inImg.rows<<","<<inImg.cols<<"]\n";
            }

            bilde::Buffer<bilde::t_uint8> fullBuffer(inImg);
            std::vector<int> left;std::vector<int> top;std::vector<int> right;std::vector<int> bottom;
            bilde::Buffer<bilde::t_uint8> b(fullBuffer);
            cv::Mat outImg=cv::Mat::zeros(inImg.rows,inImg.cols, CV_8U);
            std::vector<std::vector<std::string> > perRadiiWindow;
            if(bilde::util::Args().verboseLevel>8){
                cv::namedWindow("InputImage",1);
                cv::imshow("InputImage",inImg);
            }
            for(auto radIter=radii.begin();radIter!=radii.end();++radIter){
                std::vector<std::string> perWindow;
                if(*radIter*2<=b.width && *radIter*2<=b.height){
                    auto lbp=bilde::operations::lbp::__lbp_util__::LbpIterator<bilde::t_uint8>(b,nbSamples,*radIter,interpolation,cmpOperation,cmpThreshold);
                    lbp.applyLBPTransform(outImg);
                    if(bilde::util::Args().verboseLevel>8){
                        cv::namedWindow("LbpImage"+std::to_string(*radIter),1);
                        cv::imshow("LbpImage"+std::to_string(*radIter),outImg);
                    }
                    bilde::Buffer<bilde::t_uint8> choppedImg=bilde::Buffer<bilde::t_uint8>(outImg,*radIter,*radIter,outImg.cols - (*radIter + 1),outImg.rows-(*radIter + 1));
                    if (bilde::util::Args().verboseLevel>6){
                        std::cerr<<"Chopped: radius = "<<*radIter<<", x = "<<choppedImg.width<<". y = "<<choppedImg.height<<"\n";
                    }
                    for(int k =0;k < windowList.size();k++){
                        bilde::Buffer<bilde::t_uint8> imageRegion=getSubImage(choppedImg,windowList[k]);
                        if(bilde::util::Args().verboseLevel>6){
                            std::cerr<<"Extracted: window = "<<windowList[k]<<", x = "<<imageRegion.width<<". y = "<<imageRegion.height<<"\n";
                        }
                        if(imageRegion.width>1){
                            if(bilde::util::Args().verboseLevel>6){
                                std::cerr<<"imageRegion >1, adding\n";
                            }
                            std::vector<int> h=bilde::operations::essential::getHistogram(imageRegion);
                            std::stringstream tmpOut;
                            for(int k=0;k<256;k++){
                                tmpOut<<","<<h[k];
                            }
                            perWindow.push_back(tmpOut.str());
                        }else{
                            if(bilde::util::Args().verboseLevel>6){
                                std::cerr<<"imageRegion <=1, not adding\n";
                            }
                            perWindow.push_back("");
                        }
                    }
                    perRadiiWindow.push_back(perWindow);
                }else{
                    if(bilde::util::Args().verboseLevel>8){
                        std::cerr<<"Skipping radius "<<*radIter<<" for image ["<<b.width<<","<<b.height<<"]\n";
                    }
                    std::stringstream tmpOut;
                    for(int k=0;k<256;k++){
                        tmpOut<<","<<0;
                    }
                    perWindow.push_back(tmpOut.str());
                }
            }
            if(bilde::util::Args().verboseLevel>8){
                cv::waitKey();
                cv::destroyAllWindows();
            }
            for(int winNum=0;winNum<windowList.size();winNum++){
                if(bilde::util::Args().verboseLevel>5){
                    std::cerr<<"Post selection k = "<<winNum<<" of "<<windowList.size()<<" w=["<<windowList[winNum]<<"]"<<"\n";
                }
                if(perRadiiWindow[0][winNum].length()>0){
                    if(bilde::util::Args().verboseLevel>5){
                        std::cerr<<"Adding patch "<<windowList[winNum]<<"\n";
                    }
                    if( oneRowPerWindow==0){
                        if( winNum==0){
                            if(windowList.size()>1){
                                for(int tmpWinNum=0;tmpWinNum<windowList.size();tmpWinNum++){
                                    if(tmpWinNum>0){
                                        out<<"|";
                                    }
                                    out<<boost::algorithm::replace_all_copy(windowList[tmpWinNum],",","-");
                                }
                                out<<"|";
                            }
                            out<<*iterFname;
                        }else{
                            out<<",";
                        }
                    }else if(oneRowPerWindow==1){
                        out<<boost::algorithm::replace_all_copy(windowList[winNum],",","-")<<":"<<*iterFname;
                    }
                    for(int radiusNum=0;radiusNum<perRadiiWindow.size();radiusNum++){
                        out<<perRadiiWindow[radiusNum][winNum];
                    }
                    if(oneRowPerWindow==1 || winNum==windowList.size()-1){
                        out<<"\n";
                    }
                }else{//the patch was outside the image
                    if(bilde::util::Args().verboseLevel>5){
                        std::cerr<<"Omitting patch "<<windowList[winNum]<<"\n";
                    }
                }
            }
            out.flush();
            if(bilde::util::Args().verboseLevel>3){
                std::cerr<<"Completed in "<<perFileTimer.getMsec()<<" msec. Total Time : "<<totalTimer.getMsec()<<" msec.";
            }
        }
    }catch(const char* msg){
        std::cerr<<"Exception : "<<msg<<"\naborting...\n";
    }
}
