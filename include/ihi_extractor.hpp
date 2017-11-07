/*
 * ihi_extractor.hpp
 *
 *  Created on: Sep 11, 2014
 *      Author: anguelos
 */

#ifndef IHI_EXTRACTOR_HPP_
#define IHI_EXTRACTOR_HPP_

#include <cstring>

namespace bilde{
namespace util{

template<int NBBINS> struct IHI{
	const int width;
	const int height;
	const int linestride;//measured in integers
	const int columnstride;
	int const * __data__;
	int const * fp;

	IHI(Buffer<t_uint8> img):width(img.width),height(img.height),linestride((img.width+1)*NBBINS),columnstride(NBBINS),__data__(new int[NBBINS*(img.width+1)*(img.height+1)]),fp(__data__+columnstride+linestride){
		int tmpHist[NBBINS];
		for(int y=0;y<height;y++){
			int* ihiIter=this->fp+y*this->linestride;
			memset(tmpHist,0,sizeof(int)*NBBINS);
			t_uint8* inRow=img.getRow(y);
			for(int x=0;x<width;x++){
				tmpHist[imRow[x]]++;
				memcpy(ihiIter,tmpHist,sizeof(int)*NBBINS);
				ihiIter+=NBBINS;
			}
			memset(ihiIter,0,sizeof(int)*NBBINS);
		}
	}
	void __getAreaHistogram__(int*res,unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2){
		//assuming 0<=x1<=x2<img.width and  0<=y1<=y2<img.height
		x1--;
		y1--;
		int* tl=this->data+this->linestride*y1+this->columnstride*x1;
		int* tr=this->data+this->linestride*y1+this->columnstride*x2;
		int* bl=this->data+this->linestride*y2+this->columnstride*x1;
		int* br=this->data+this->linestride*y2+this->columnstride*x2;
		for(int k=0;k<NBBINS;k++){
			res[k]=(br[k]+tl[k])-(tr[k]+bl[k]);
		}
	}
	~IHI(){
		delete[] data;
	}

};

}
}



#endif /* IHI_EXTRACTOR_HPP_ */
