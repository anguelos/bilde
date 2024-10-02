/*
 * integral_histograms.hpp
 *
 *  Created on: Mar 31, 2012
 *      Author: anguelos
 */

#ifndef INTEGRAL_HISTOGRAMS_HPP_
#define INTEGRAL_HISTOGRAMS_HPP_

namespace bilde {
namespace operations {
namespace __histogram__{


template <typename BINT,int NBBINS> void __getCountHistogram__(Buffer<t_uint8> in,BINT* hist){
    t_uint8* inRow;
    int x,y;
    memset(hist,0,NBBINS*sizeof(BINT));
    for(y=0;y<in.height;y++){
        inRow=in.getRow(x);
        for(x=0;x<in.width;x++){
            hist[inRow[x]]++;
        }
    }
}


template <typename BINT,int NBBINS> void __getCummulativeHistogram__(Buffer<t_uint8> in,BINT* hist){
    t_uint8* inRow;
    int x,y;
    memset(hist,0,NBBINS*sizeof(BINT));
    for(y=0;y<in.height;y++){
        inRow=in.getRow(y);
        for(x=0;x<in.width;x++){
            hist[inRow[x]]++;
        }
    }
    for(int b=1;b<NBBINS;b++){
        hist[b]=hist[b]+hist[b-1];
    }
}

template <typename BINT,int NBBINS> void __cum2count__(BINT* data){
    for(int k=NBBINS-1;k>0;k--){
        data[k]=data[k]-data[k-1];
    }
}

template <typename BINT,int NBBINS> void __cum2count__(BINT* dst,const BINT* src){
    for(int k=NBBINS-1;k>0;k--){
        dst[k]=src[k]-src[k-1];
    }
    dst[0]=src[0];
}

template <typename BINT,int NBBINS> void __count2cum__(BINT* data){
    for(int k=1;k<NBBINS;k++){
        data[k]=data[k]+data[k-1];
    }
    //std::cerr<<"L:"<<data[NBBINS-1]<<"\n";
}

template <typename BINT,int NBBINS> void __count2cum__(BINT* dst,const BINT* src){
    dst[0]=src[0];
    for(int k=1;k<NBBINS;k++){
        dst[k]=src[k]+src[k-1];
    }
}

template <typename BINT,int NBBINS,bool ISCUMMULATIVE> struct __HistogramOperations__{};

template <typename BINT,int NBBINS> struct __HistogramOperations__<BINT,NBBINS,true>{
    //CUMMULATIVE
    static int __getMedian__(BINT* data){
        BINT half=data[NBBINS-1]/2;
        int pos=0;
        while(pos<NBBINS && data[pos]<half)pos++;
        pos-=((data[pos]*2-data[NBBINS-1])>(data[NBBINS-1]-2*data[pos-1]) );
        return pos;
    }
    static int __getMode__(const BINT* data){
        int res=0;
        BINT maxFound=data[0];
        const BINT* dataMinusOne=data-1;
        for(int k=1;k<NBBINS;k++){
            if(data[k]-dataMinusOne[k]>maxFound){
                res=k;
                maxFound=data[k]-dataMinusOne[k];
            }
        }
        return res;
    }
    static int __getOtsu__(const BINT* data){
        BINT cum[NBBINS];
        __count2cum__<BINT,NBBINS>(cum,data);
        return __HistogramOperations__<BINT,NBBINS,false>::__getOtsu__(cum);
    }
    static float __getRank__(int pos,const BINT* data ){
        return data[pos]/double(data[pos]);
    }
    static int __getCentilePos__(float centile,const BINT* data){
        BINT nbPixelsAtCentile=data[NBBINS-1]*centile;
        int res=0;
        while(data[res]<nbPixelsAtCentile){
            res++;
        }
        res-=((res>0)*(nbPixelsAtCentile-data[res-1]<data[res]-nbPixelsAtCentile));
        return res;
    }
};


template <typename BINT,int NBBINS> struct __HistogramOperations__<BINT,NBBINS,false>{
    //NON CUMMULATIVE
    static  int __getMedian__(BINT* data){
        BINT cum[NBBINS];
        __count2cum__(cum,data);
        return __HistogramOperations__<BINT,NBBINS,false>::__getMedian__(data);
    }
    static int __getMode__(const BINT* data){
        int res=0;
        BINT maxFound=data[0];
        const BINT* dataMinusOne=data-1;
        for(int k=1;k<NBBINS;k++){
            if(data[k]-dataMinusOne[k]>maxFound){
                res=k;
                maxFound=data[k]-dataMinusOne[k];
            }
        }
        return res;
    }
    static int __getOtsu__(const BINT* data){
        int k;
        long long int total = 0; //nbPixels
        double varBetween;
        long long int sum = 0; //intencity*occurences
        for (k = 0; k < NBBINS; k++) {
            total += data[k];
            sum += k * data[k];
        }
        t_real64 sumB = 0;
        int wB = 0;
        int wF = 0;
        t_real64 varMax = 0;
        int threshold = 0;
        for (int t = 0; t < NBBINS; t++) {
            wB += data[t]; // Weight Background
            if (wB == 0)continue;
            wF = total - wB; // Weight Foreground
            if (wF == 0)break;
            sumB += (t_real64) (t * data[t]);
            t_real64 mB = sumB / wB; // Mean Background
            t_real64 mF = (sum - sumB) / wF; // Mean Foreground
            // Calculate Between Class Variance
            varBetween = (t_real64) wB * (t_real64) wF * (mB - mF) * (mB - mF);
            // Check if new maximum found
            if (varBetween >= varMax) {
                varMax = varBetween;
                threshold = t;
            }
        }
        return threshold;
    }
    static float __getRank__(int pos,const BINT* data ){
        BINT sumAtPos=data[0];
        int k=1;
        while(k<pos){
            sumAtPos+=data[k];
            k++;
        }
        BINT totalSum=sumAtPos;
        while(k<NBBINS){
            totalSum+data[pos];
        }
        return sumAtPos/double(totalSum);
    }
    static int __getCentilePos__(float centile,const BINT* data){
        BINT cum[NBBINS];
        return __HistogramOperations__<BINT,NBBINS,true>::__getCentilePos__(centile,data);
    }


};

}

namespace histogram{
template<typename T,int NBBINS,bool ISCUMMULATIVE> struct Histogram: public bilde::StaticVector<T,NBBINS>{
    bool isCummulative(){return ISCUMMULATIVE;}
    template <typename K>
    Histogram(Buffer<K> in){
        double min=PixelMetrics<K>::defaultBg;
        double max=PixelMetrics<K>::defaultFg;
        double ratio=(max-min)*(NBBINS-1);
        int x,y;
        K* inRow;
        this->setTo(0);
        for(y=0;y<in.height;y++){
            inRow=in.getRow(y);
            for(x=0;x<in.width;x++){
                this->data[int(round(inRow[x]-min))]++;
            }
        }
    }
    t_uint32 getOtsu(){
        return __histogram__::__HistogramOperations__<t_uint32,NBBINS,ISCUMMULATIVE>::__getOtsu__(this->data);
    }
    t_uint32 getMode(){
        return __histogram__::__HistogramOperations__<t_uint32,NBBINS,ISCUMMULATIVE>::__getMode__(this->data);
    }
    friend std::ostream& operator<<(std::ostream& stream, const Histogram<T,NBBINS,ISCUMMULATIVE>& hist){
        stream<<"Histogram of "<<NBBINS<<" bins {0 => "<<hist.data;
        for(int k=1;k<NBBINS;k++){
            stream<<" , "<<k<<" => "<<hist.data[k];
        }
        stream<<"}\n";
        return stream;
    }
};
}

namespace integral_histograms{

template <typename BINT,int NBBINS,bool ISCUMMULATIVE> struct IntegralHistogram{
    //boost::shared_ptr<IntegralHistogram<BINT,NBBINS,ISCUMMULATIVE> > sharedThis;
    boost::shared_ptr<BINT> sharedData;
    static const bool IS_CUMMULATIVE=ISCUMMULATIVE;
    static const int BIN_COUNT=NBBINS;
    typedef BINT BIN_TYPE;
    typedef __histogram__::__HistogramOperations__<BINT,NBBINS,ISCUMMULATIVE> HistogramOperations;
    BINT* const __data__;
    const int width;
    const int height;
    const t_sint64 binLinestride;
    const t_sint64 byteLinestride;
    BINT* const __getHistogramAt__(int x,int y){
        x=x*(x>0);
        if(x>=width){x=width-1;}
        y=y*(y>0);
        if(y>=height){y=height-1;}

        //x=x*(x>0)+(width-(x+1))*(x>width);
        //y=y*(y>=0)+(height-(y+1))*(y>=height);
        //std::cerr<<"y="<<y<<" x="<<x<<"\n";
        return &(__data__[x*NBBINS+y*binLinestride]);
    }
    IntegralHistogram(Buffer<t_uint8> img):width(img.width),height(img.height),
        sharedData(new BINT[img.width*img.height*NBBINS]),
        __data__((BINT*)(sharedData.get())),binLinestride(img.width*NBBINS),
        byteLinestride(img.width*NBBINS*sizeof(BINT)){
        BINT data[NBBINS];
        int x,y,bin;
        t_uint8* inRow;
        BINT* topHist;
        //BINT* leftHist;
        BINT* curHist;
        inRow=img.getRow(0);
        memset(data,0,NBBINS*sizeof(BINT));
        for(x=0;x<width;x++){
            data[inRow[x]]++;
            curHist=__data__+x*NBBINS;
            memcpy(curHist,data,sizeof(BINT)*NBBINS);
        }
        if(IS_CUMMULATIVE ){
            for(y=1;y<height;y++){
                inRow=img.getRow(y);
                memset(data,0,NBBINS*sizeof(BINT));
                for(x=0;x<width;x++){
                    data[inRow[x]]++;
                    curHist=__data__+y*binLinestride+x*NBBINS;
                    topHist=__data__+(y-1)*binLinestride+x*NBBINS;
                    curHist[0]=topHist[0]+data[0];
                    for(bin=1;bin<NBBINS;bin++){
                        curHist[bin]=topHist[bin]+data[bin];//+data[bin-1];
                    }
                    //std::cerr<<"x:"<<x<<" sum:"<<sum<<"\n";
                }
            }
        }else{
            for(y=1;y<height;y++){
                inRow=img.getRow(y);
                memset(data,0,NBBINS*sizeof(BINT));
                for(x=0;x<width;x++){
                    data[inRow[x]]++;
                    curHist=__data__+y*binLinestride+x*NBBINS;
                    topHist=__data__+(y-1)*binLinestride+x*NBBINS;
                    for(bin=0;bin<NBBINS;bin++){
                        curHist[bin]=topHist[bin]+data[bin];
                    }
                    //std::cerr<<"x:"<<x<<" sum:"<<sum<<"\n";
                }
            }
        }
        if(IS_CUMMULATIVE){
            int x,y;
            for(y=0;y<height;y++){
                for(x=0;x<width;x++){
                    __histogram__::__count2cum__<BINT,NBBINS>(this->__getHistogramAt__(x,y));
                    //this->__getHistogramAt__(x,y)[NBBINS-1];
                    //std::cerr<<"y"<<y<<" x"<<x<<this->__getHistogramAt__(x,y)[255];
                }
            }
        }
    }
    struct Iterator{
        IntegralHistogram<BINT,NBBINS,ISCUMMULATIVE> *const integralHistogram;
        const int width;
        const int height;
        const t_sint64 binLinestride;
        const int left;
        const int right;
        const int top;
        const int bottom;
        BINT __curHist__[NBBINS];
        int  __curCol__;
        int  __curRow__;
        void __updateCurentHistogram__(){
            BINT* topLeftH=integralHistogram->__getHistogramAt__(__curCol__+left,__curRow__+top);
            BINT* topRightH=integralHistogram->__getHistogramAt__(__curCol__+right,__curRow__+top);

            BINT* bottomLeftH=integralHistogram->__getHistogramAt__(__curCol__+left,__curRow__+bottom);
            BINT* bottomRightH=integralHistogram->__getHistogramAt__(__curCol__+right,__curRow__+bottom);

            for(int bin=0;bin<NBBINS;bin++){
                __curHist__[bin]=bottomRightH[bin]+topLeftH[bin]-(bottomLeftH[bin]+topRightH[bin]);
            }
        }

        Iterator(IntegralHistogram<BINT,NBBINS,ISCUMMULATIVE>& ih,int l,int t,int r,int b):
            integralHistogram(&ih),width(ih.width),height(ih.height),
            binLinestride(ih.binLinestride),left(l),right(r),top(t),bottom(b){
            __curCol__=0;
            __curRow__=0;
            __updateCurentHistogram__();
        }
        Iterator(const Iterator& i):integralHistogram(i.integralHistogram),binLinestride(i.binLinestride)
          ,width(i.width),height(i.height),left(i.left),top(i.top),right(i.right),bottom(i.bottom)
          ,__curCol__(i.__curCol__),__curRow__(i.__curRow__){
            __updateCurentHistogram__();
        }

        typedef double (*t_HistogramFeatureIIntOReal)(int val,const BINT* data);
        typedef double (*t_HistogramFeatureIVoidOReal)(const BINT* data);
        typedef double (*t_HistogramFeatureIRealOReal)(double val,const BINT* data);

        typedef int (*t_HistogramFeatureIIntOInt)(int val,const BINT* data);
        typedef int (*t_HistogramFeatureIVoidOInt)(const BINT* data);
        typedef int (*t_HistogramFeatureIRealOInt)(double val,const BINT* data);

        void applyFilter(Buffer<t_real64> outImg,Buffer<t_uint8> inImg,t_HistogramFeatureIIntOReal func){
            t_real64* outRow;
            t_uint8* inRow;
            const BINT* curHist;
            int x,y;
            __curRow__=0;
            for(y=0;y<height;y++){
                __curRow__++;
                outRow=outImg.getRow(y);
                inRow=inImg.getRow(y);
                __curCol__=0;
                for(x=0;x<width;x++){
                    __curCol__++;
                    this->__updateCurentHistogram__();
                    outRow[x]=func(inRow[x],this->curHist);
                }
            }
        }

        void applyFilter(Buffer<t_real64> outImg,Buffer<t_real64> inImg,t_HistogramFeatureIRealOReal func){
            t_real64* outRow;
            t_real64* inRow;
            const BINT* curHist;
            int x,y;
            __curRow__=0;
            for(y=0;y<height;y++){
                __curRow__++;
                outRow=outImg.getRow(y);
                inRow=inImg.getRow(y);
                __curCol__=0;
                for(x=0;x<width;x++){
                    __curCol__++;
                    this->__updateCurentHistogram__();
                    outRow[x]=func(inRow[x],this->curHist);
                }
            }
        }

        void applyFilter(Buffer<t_real64> outImg,t_HistogramFeatureIVoidOReal func){
            t_real64* outRow;
            const BINT* curHist;
            int x,y;
            __curRow__=0;
            for(y=0;y<height;y++){
                __curRow__++;
                outRow=outImg.getRow(y);
                __curCol__=0;
                for(x=0;x<width;x++){
                    __curCol__++;
                    this->__updateCurentHistogram__();
                    outRow[x]=func(this->curHist);
                }
            }
        }

        void applyFilter(Buffer<t_uint8> outImg,Buffer<t_uint8> inImg,t_HistogramFeatureIIntOInt func){
            //std::cerr<<"Apply Filter: start\n";
            t_uint8* outRow;
            t_uint8* inRow;
            const BINT* curHist;
            int x,y;
            __curRow__=0;
            for(y=0;y<height;y++){
                __curRow__++;
                outRow=outImg.getRow(y);
                inRow=inImg.getRow(y);
                __curCol__=0;

                for(x=0;x<width;x++){
                    __curCol__++;
                    this->__updateCurentHistogram__();
                    outRow[x]=func(inRow[x],this->__curHist__);
                    //std::cerr<<"Apply Filter y:"<<y<<" x:"<<x<<" s:"<<double(this->__curHist__[NBBINS-1]+7)<<"\n";
                }

            }
            //std::cerr<<"Apply Filter: end\n";
        }

        void applyFilter(Buffer<t_uint8> outImg,Buffer<t_real64> inImg,t_HistogramFeatureIRealOInt func){
            t_uint8* outRow;
            t_real64* inRow;
            const BINT* curHist;
            int x,y;
            __curRow__=0;
            for(y=0;y<height;y++){
                __curRow__++;
                outRow=outImg.getRow(y);
                inRow=inImg.getRow(y);
                __curCol__=0;
                for(x=0;x<width;x++){
                    __curCol__++;
                    this->__updateCurentHistogram__();
                    outRow[x]=func(inRow[x],this->curHist);
                }
            }
        }
        void applyFilter(Buffer<t_uint8> outImg,t_HistogramFeatureIVoidOInt func){
            t_uint8* outRow;
            //const BINT* curHist;
            int x,y;
            __curRow__=0;
            for(y=0;y<height;y++){
                __curRow__++;
                outRow=outImg.getRow(y);
                __curCol__=0;
                for(x=0;x<width;x++){
                    __curCol__++;
                    this->__updateCurentHistogram__();
                    outRow[x]=func(this->__curHist__);
                }
            }
        }
    };
    Iterator getIterator(int radius){
        return Iterator(*this,-radius,-radius,radius,radius);
    }
    Iterator getIterator(int width,int height){
        return Iterator(*this,-width/2,-height/2,width/2,height/2);
    }
};



}

}
}
#endif /* INTEGRAL_HISTOGRAMS_HPP_ */
