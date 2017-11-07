#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "bilde.hpp"

#include <cstdlib>

std::vector<std::string> inputFileNames;
std::string interpolation;
std::string cmpOperation;
std::vector<std::string> radiiList;
int nbSamples;
bool includeOutputHeader;
std::string oFname;
std::string hSeparator;
std::string cmpThreshold;
/*
std::vector<int> multiplexTop;
std::vector<int> multiplexBottom;
std::vector<int> multiplexLeft;
std::vector<int> multiplexRight;
std::vector<std::string> multiplexOutname;


void setMultiplex(bilde::Buffer<bilde::t_uint8> b,std::string fname,std::string cmd){
    multiplexTop={0}; multiplexBottom={b.height-1}; multiplexLeft={0}; multiplexRight={b.width-1};
    if(cmd==""){
        return;
    }else if(cmd.)

}

*/
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
//        bilde::util::Args().addBoolParam(false,"R","rotation-invariance","A boolean defining wether the dictionary should be compressed to the rotation invariant patterrns.",&rotationInvariance).setDefault(false);
//        bilde::util::Args().addBoolParam(false,"u","uniformity","A boolean defining wether the dictionary should be compressed to conatain Uniformity 2 patterrns.",&uniformity).setDefault(false);
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

        std::stringstream out;
        for(auto iterFname=inputFileNames.begin();iterFname != inputFileNames.end();++iterFname){
            perFileTimer.tic();
            cv::Mat inImg;
            inImg = cv::imread(*iterFname, CV_LOAD_IMAGE_GRAYSCALE);
            if(inImg.cols==0 && inImg.rows==0){
                std::cerr<<"Failed to open '"<<*iterFname<<"'\n";
                throw("could not open file");
            }
            if(bilde::util::Args().verboseLevel>1){
                std::cerr<<"Processing "<<*iterFname<<" ["<<inImg.rows<<","<<inImg.cols<<"]\n";
            }
            bilde::Buffer<bilde::t_uint8> fullBuffer(inImg);
            std::vector<int> left;std::vector<int> top;std::vector<int> right;std::vector<int> bottom;
            //setSliceSubRegions(inImg,left,top,right,bottom);
            //for(int sliceNum=0;sliceNum<nbTotalSlices;sliceNum++){
            bilde::Buffer<bilde::t_uint8> b(fullBuffer);//,left[sliceNum],top[sliceNum],right[sliceNum],bottom[sliceNum]);
            out<<*iterFname;
            for(auto radIter=radii.begin();radIter!=radii.end();++radIter){
                auto lbp=bilde::operations::lbp::__lbp_util__::LbpIterator<bilde::t_uint8>(b,nbSamples,*radIter,interpolation,cmpOperation,cmpThreshold);
                std::vector<int> h=lbp.getLBPHistogram();
                for(int k=0;k<h.size();k++){
                    out<<","<<h[k];
                }
            }
            out<<"\n";
            if(bilde::util::Args().verboseLevel>3){
                std::cerr<<"Completed in "<<perFileTimer.getMsec()<<" msec. Total Time : "<<totalTimer.getMsec()<<" msec.";
            }
        }
        if(oFname=="stdout"){
            std::cout<<out.str();
        }else{
            std::ofstream outFile;
            outFile.open(oFname.c_str(), std::ofstream::out);// append seems impractical
            outFile<<out.str();
        }
    }catch(const char* msg){
        std::cerr<<"Exception : "<<msg<<"\naborting...\n";
    }
}
