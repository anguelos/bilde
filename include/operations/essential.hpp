/*
 * simple_operations.hpp
 *
 *  Created on: Apr 1, 2012
 *      Author: anguelos
 */

#ifndef ESSENTIAL_OPERATIONS_HPP_
#define ESSENTIAL_OPERATIONS_HPP_

namespace bilde{
namespace operations{
namespace essential{

template <typename T> void __setTo__(Buffer<T> io,T inVal){
	T* row;
	int y,x;
	for(y=0;y<io.height;y++){
		row=io.getRow(y);
		for(x=0;x<io.width;x++){
			row[x]=inVal;
		}
	}
}

template <typename T> T getMaxValue(Buffer<T> in){
	T res=in.__data__[0];
	t_sz x,y;
	T* curRow;
	T* curRowEnd;
	for(y=0;y<in.height;y++){
		curRow=in.getRow(y);
		curRowEnd=curRow+in.width;
		for(;curRow!=curRowEnd;curRow++){
			res=res*(res>=*curRow)+*curRow*(res<*curRow);
		}
	}
	return res;
}

template <typename T> T getMinValue(Buffer<T> in){
	T res=in.__data__[0];
	t_sz x,y;
	T* curRow;
	T* curRowEnd;
	for(y=0;y<in.height;y++){
		curRow=in.getRow(y);
		curRowEnd=curRow+in.width;
		for(;curRow!=curRowEnd;curRow++){
			res=res*(res<=*curRow)+*curRow*(res>*curRow);
		}
	}
	return res;
}

template <typename T> std::pair<T,T> getRange(Buffer<T> in){
	T minFound=in.__data__[0];
	T maxFound=in.__data__[0];
	t_sz x,y;
	T* curRow;
	T* curRowEnd;
	for(y=0;y<in.height;y++){
		curRow=in.getRow(y);
		curRowEnd=curRow+in.width;
		for(;curRow!=curRowEnd;curRow++){
			minFound=minFound*(minFound<=*curRow)+*curRow*(minFound>*curRow);
			maxFound=maxFound*(maxFound>=*curRow)+*curRow*(maxFound<*curRow);
		}
	}
	return std::pair<T,T>(minFound,maxFound);
}


template <typename T> t_real64 getMSE(Buffer<T> in,Buffer<T> reference){
	double diffSum=0;
	t_sz x,y;
	T* inRow;
	T* referenceRow;
	for(y=0;y<in.height;y++){
		inRow=in.getRow(y);
		referenceRow=reference.getRow(y);
		for(x=0;x<in.width;x++){
			diffSum+=(inRow[x]-referenceRow[x]*1.0)*(inRow[x]-referenceRow[x]);
		}
	}
	return diffSum/(in.width*in.height);
}

template <typename T> t_real64 getPSNR(Buffer<T> in,Buffer<T> reference){
	return getMaxValue<T>(in)/getMSE<T>(in,reference);
}

template <typename T> t_real64 pixelDiff(Buffer<T> in,Buffer<T> in2,T maxDiffAllowed){
	t_sz x,y;
	T* inRow1;
	T* inRow2;
	t_sz res=0;//counts similar
	for(y=0;y<in.height;y++){
		inRow1=in.getRow(y);
		inRow2=in2.getRow(y);
		for(x=0;x<in.width;x++){
			res+=((inRow1[x]>inRow2[x])*(inRow1[x]<=inRow2[x]+maxDiffAllowed))+((inRow1[x]<=inRow2[x])*(inRow1[x]+maxDiffAllowed<=inRow2[x]));
		}
	}
	return res*1.0/(in.width*in.height);
}


template <typename T > void __pixelWiseEquals__(Buffer<t_uint8> out,Buffer<T> in1,Buffer<T> in2){
	t_uint8* outRow;
	T* inRow1;
	T* inRow2;
	int x,y;
	for(y=0;y<in1.height;y++){
		outRow=out.getRow(y);
		inRow1=in1.getRow(y);
		inRow2=in2.getRow(y);
		for(x=0;x<in1.width;x++){
			outRow[x]=(inRow1[x]==inRow2[x])*255;
		}
	}
}

template <typename T> void __nornalize__(Buffer<T> out,Buffer<T> in){
	T* outRow;
	T* inRow;
	int x,y;
	t_real64 minFound=out.getRow(0)[0];
	t_real64 maxFound=out.getRow(0)[0];
	for(y=0;y<in.height;y++){
		inRow=in.getRow(y);
		for(x=0;x<in.width;x++){
			minFound=(inRow[x]>minFound)*minFound+(inRow[x]<=minFound)*inRow[x];
			maxFound=(inRow[x]<maxFound)*maxFound+(inRow[x]>=maxFound)*inRow[x];
		}
	}
	t_real64 ratio=maxFound-minFound;
	for(y=0;y<in.height;y++){
		inRow=in.getRow(y);
		outRow=out.getRow(y);
		for(x=0;x<in.width;x++){
			outRow[x]=((maxFound-inRow[x])/ratio)*PixelMetrics<T>::defaultFg;
		}
	}

}


template <typename T> void __bitShiftRight__(Buffer<T> out,Buffer<T> in,int nbBits){
    T* outRow;
    T* inRow;
    int x,y;
    for(y=0;y<in.height;y++){
        inRow=in.getRow(y);
        outRow=out.getRow(y);
        for(x=0;x<in.width;x++){
            outRow[x]=(inRow[x]>>nbBits);
        }
    }
}


template <typename T> void __bitShiftLeft__(Buffer<T> out,Buffer<T> in,int nbBits){
    T* outRow;
    T* inRow;
    int x,y;
    for(y=0;y<in.height;y++){
        inRow=in.getRow(y);
        outRow=out.getRow(y);
        for(x=0;x<in.width;x++){
            outRow[x]=(inRow[x]<<nbBits);
        }
    }
}


template <typename T> void __subtractFrom__(Buffer<T> out,Buffer<T> in,T inVal){
	T* outRow;
	T* inRow;
	int x,y;
	for(y=0;y<in.height;y++){
		inRow=in.getRow(y);
		outRow=out.getRow(y);
		for(x=0;x<in.width;x++){
			outRow[x]=inVal-inRow[x];
		}
	}
}


template <typename T> void __divideBy__(Buffer<T> out,Buffer<T> in,T inVal){
	T* outRow;
	T* inRow;
	int x,y;
	for(y=0;y<in.height;y++){
		inRow=in.getRow(y);
		outRow=out.getRow(y);
		for(x=0;x<in.width;x++){
			outRow[x]=inRow[x]/inVal;
		}
	}
}

template <typename T> void __multiplyBy__(Buffer<T> out,Buffer<T> in,T inVal){
    T* outRow;
    T* inRow;
    int x,y;
    for(y=0;y<in.height;y++){
        inRow=in.getRow(y);
        outRow=out.getRow(y);
        for(x=0;x<in.width;x++){
            outRow[x]=inRow[x]*inVal;
        }
    }
}

template <typename T> void __incrementBy__(Buffer<T> out,Buffer<T> in,T inVal){
	T* outRow;
	T* inRow;
	int x,y;
	for(y=0;y<in.height;y++){
		inRow=in.getRow(y);
		outRow=out.getRow(y);
		for(x=0;x<in.width;x++){
			outRow[x]=inRow[x]+inVal;
		}
	}
}

template <typename T> void __inverce__(Buffer<T> out,Buffer<T> in){
	T* outRow;
	T* inRow;
	int x,y;
	for(y=0;y<in.height;y++){
		inRow=in.getRow(y);
		outRow=out.getRow(y);
		for(x=0;x<in.width;x++){
			outRow[x]=1/inRow[x];
		}
	}
}

template <typename T> void  __threshold__(Buffer<T> out,Buffer<T> in,T inMinimalTrue,T inTrueVal,T inFalseVal){
	T* inRow;
	T* outRow;
	int x,y;
	for(y=0;y<in.height;y++){
		inRow=in.getRow(y);
		outRow=out.getRow(y);
		for(x=0;x<in.width;x++){
			outRow[x]=(inRow[x]>=inMinimalTrue)?inTrueVal:inFalseVal;
		}
	}
}

template <typename T> void  __pixelWiseThreshold__(Buffer<T> outImg,Buffer<T> inImg,Buffer<T> thresholdImg,T inTrueVal,T inFalseVal){
    T* inRow;
    T* outRow;
    T* thresholdRow;
    int x,y;
    for(y=0;y<inImg.height;y++){
        inRow=inImg.getRow(y);
        outRow=outImg.getRow(y);
        thresholdRow=thresholdImg.getRow(y);
        for(x=0;x<inImg.width;x++){
            //outRow[x]=(inRow[x]>=thresholdRow[x])?inTrueVal:inFalseVal;
            outRow[x]=(inRow[x]>=thresholdRow[x])*inTrueVal+(inRow[x]<thresholdRow[x])*inFalseVal;
        }
    }
}


template <typename T > void __pixelWiseMin__(Buffer<T> out,Buffer<T> in1,Buffer<T> in2){
	T* outRow;
	T* inRow1;
	T* inRow2;
	int x,y;
	for(y=0;y<in1.height;y++){
		outRow=out.getRow(y);
		inRow1=in1.getRow(y);
		inRow2=in2.getRow(y);
		for(x=0;x<in1.width;x++){
			outRow[x]=(inRow1[x]<inRow2[x])?inRow1[x]:inRow2[x];
		}
	}
}



template <typename T > void __pixelWiseMax__(Buffer<T> out,Buffer<T> in1,Buffer<T> in2){
	T* outRow;
	T* inRow1;
	T* inRow2;
	int x,y;
	for(y=0;y<in1.height;y++){
		outRow=out.getRow(y);
		inRow1=in1.getRow(y);
		inRow2=in2.getRow(y);
		for(x=0;x<in1.width;x++){
			outRow[x]=(inRow1[x]>inRow2[x])?inRow1[x]:inRow2[x];
		}
	}
}


void __initiateRandomSeed__(){
	static t_uint32 seed=std::time( NULL );
    std::srand(seed);
}


template <typename T> void __randomize__(Buffer<T> out){
    //static int tmp=
    __initiateRandomSeed__();
	if(PixelMetrics<T>::isReal){
		T* outRow;
		int x,y;
		for(y=0;y<out.height;y++){
			outRow=out.getRow(y);
			for(x=0;x<out.width;x++){
				outRow[x]=(std::rand()%1024)/(1024.0);
			}
		}
	}else{
		T* outRow;
		int x,y;
		for(y=0;y<out.height;y++){
			outRow=out.getRow(y);
			for(x=0;x<out.width;x++){
				outRow[x]=std::rand()%(PixelMetrics<T>::defaultFg+1);
			}
		}
	}
}
template <typename T>void __copy__(Buffer<T> dst,Buffer<T>src){
    int x,y;
    T* inRow;
    T*outRow;
    for(y=0;y<dst.height;y++){
        inRow=src.getRow(y);
        outRow=dst.getRow(y);
        for(x=0;x<dst.width;x++){
            outRow[x]=inRow[x];
        }
    }
}

template<typename T> std::vector<int> getHistogram(Buffer<T> img,int nbBins=-1);

template<> std::vector<int> getHistogram<t_uint8>(Buffer<t_uint8> img,int nbBins){
    if( nbBins<2){nbBins=256;}
    std::vector<int> res(nbBins,0);
    t_uint8* curPixel;
    t_uint8* rowEnd;
    if(nbBins!=256){
        const double coef=256/double(nbBins);
        for(int y =0 ;y<img.height;y++){
            curPixel=img.getRow(y);
            rowEnd=curPixel+img.width;
            while(curPixel!=rowEnd){
                res[int(*curPixel/coef)]++;
                curPixel++;
            }
        }
    }else{
        for(int y =0 ;y<img.height;y++){
            curPixel=img.getRow(y);
            rowEnd=curPixel+img.width;
            while(curPixel!=rowEnd){
                res[*curPixel]++;
                curPixel++;
            }
        }
    }
    return res;
}




}
}
}


#endif /* SIMPLE_OPERATIONS_HPP_ */
