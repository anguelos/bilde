#include <png.h>
#include <tiff.h>
#include "__bilde__.hpp"
#include <fstream>

int main(int argc,char** argv){
	bilde::io::__initIO__();
	try{
		//std::vector<bilde::Buffer<bilde::t_uint8> > vals;
		//bilde::Buffer<bilde::t_uint8>vals=bilde::io::png::loadPngGray<bilde::t_uint8>(argv[1]);
		//bilde::io::tiff::saveTiffGray<bilde::t_uint8>(vals,"outGray.tiff");
		//bilde::io::tiff::saveTiffColor<bilde::t_uint8>({vals,vals,vals},"outColor.tiff");

		std::vector<bilde::Buffer<bilde::t_uint8> > vals;
		vals=bilde::io::loadColorImage<bilde::t_uint8>(argv[1]);
		bilde::io::saveGrayImage<bilde::t_uint8>(vals[0],"outGray.tiff");
		bilde::io::saveColorImage<bilde::t_uint8>({vals[0],vals[1],vals[2]},"outColor.tiff");

	}catch(const char* msg){
		std::cerr<<"Exception thrown:\n"<<msg<<"\nAborting.\n";
	}
}
