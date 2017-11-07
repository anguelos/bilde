#include <tiff.h>
#include <png.h>
#include "__bilde__.hpp"

std::vector<std::string> inFileNames;
std::vector<std::string> outFileNames;
int neiborhood;
bool whiteIsBg;
bool visibleColors;
bool outputBgWhite;

void checkArgs(){
	if(inFileNames.size()!=outFileNames.size()){
		std::cerr<<"Input and Output sizes must be equal.\nAborting.\n";
		exit(1);
	}
	if(neiborhood!= 4 && neiborhood!=8 ){
		std::cerr<<"Neighborhood must be either 4 or 8.\nAborting.\n";
		exit(1);
	}
}

int main(int argc,char** argv){
try{
	bilde::util::Args("Converts binary tiff images to colorencoded connected component labeled ");
	bilde::util::Args().addStringListParam(false,"i","input","the input tiff files, assumed to be binarized images",-1,&inFileNames);
	bilde::util::Args().addStringListParam(false,"o","output","the output tiff files, must be as many and in the same order as the inputs as the input files",-1,&outFileNames);
	bilde::util::Args().addIntParam(false,"n","neighborhood","The type of neighborhood connectivity 8-pixel or 4 pixel",&neiborhood).setDefault(8);
	bilde::util::Args().addBoolParam(false,"w","white-bg"," Should the componnets be the white or the black objects",&whiteIsBg).setDefault(true);
	bilde::util::Args().addBoolParam(false,"V","visible-colorspace"," Should the colorspace be optimized for visualisation or for compact values ",&visibleColors).setDefault(true);
	bilde::util::Args().addBoolParam(false,"p","printer-friendly","If outputing to a visual colorspace, should the background be white. If -V is set to false, this is ignored",&outputBgWhite).setDefault(false);
	bilde::util::Args().parseArgv(argc,argv);
	if(!bilde::util::Args().validate()){
		std::cerr<<bilde::util::Args().getValidationMsg()<<"\nAborting.\n";
	}
	checkArgs();
	for(int fnum=0;fnum<inFileNames.size();fnum++){
		if(bilde::util::Args().verboseLevel>1){
			std::cerr<<"Computing cc for "<<inFileNames[fnum]<<" and saving in to "<<outFileNames[fnum]<<" ... ";
		}
		bilde::Buffer<bilde::t_uint8> in=bilde::io::png::loadPngGrayscale<bilde::t_uint8>(inFileNames[fnum]);
		bilde::Buffer<bilde::t_sint32> outLabels(in.width,in.height);
		if(whiteIsBg){
			bilde::operations::essential::__subtractFrom__<bilde::t_uint8>(in,in,255);
		}
		bilde::operations::components::__labelConnectedComponents__<bilde::t_uint8>(outLabels,in,neiborhood);
		std::vector<bilde::Buffer<bilde::t_uint8> > outCol={bilde::Buffer<bilde::t_uint8>(in.width,in.height)
			,bilde::Buffer<bilde::t_uint8>(in.width,in.height)
			,bilde::Buffer<bilde::t_uint8>(in.width,in.height)
			,bilde::Buffer<bilde::t_uint8>(in.width,in.height)
		};
		if(visibleColors){
			bilde::operations::colorspace::__labels2RGB__<bilde::t_uint8>(outCol,outLabels,outputBgWhite);
		}else{
			bilde::operations::colorspace::__sint32ToRGBA__(outCol,outLabels);
		}
		bilde::io::png::savePngColor(outCol,outFileNames[fnum]+".png");
		bilde::io::tiff::saveColorTiff(outCol,outFileNames[fnum]);
		if(bilde::util::Args().verboseLevel>1){
			std::cerr<<" done\n";
		}
	}
}catch(const char* e){
	std::cerr<<e<<"\nAborting!\n";
	exit(1);
}
}
