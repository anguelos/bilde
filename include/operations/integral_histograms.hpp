/*
 * integral_histograms.hpp
 *
 *  Created on: Mar 31, 2012
 *      Author: anguelos
 */

#ifndef INTEGRAL_HISTOGRAMS_HPP_
#define INTEGRAL_HISTOGRAMS_HPP_

#include <functional>

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
    t_sz x,y;
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

template <typename PIXELT, typename BINT,int NBBINS,bool ISCUMMULATIVE> struct __HistogramOperations__{};

template <typename PIXELT, typename BINT,int NBBINS> struct __HistogramOperations__<PIXELT, BINT,NBBINS,true>{
    //CUMMULATIVE
    static PIXELT __getMedian__(BINT* data){
        BINT half=data[NBBINS-1]/2;
        int pos=0;
        while(pos<NBBINS && data[pos]<half)pos++;
        pos-=((data[pos]*2-data[NBBINS-1])>(data[NBBINS-1]-2*data[pos-1]) );
        return pos;
    }
    static PIXELT __getMode__(const BINT* data){
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
    static PIXELT __getOtsu__(const BINT* data){
        BINT cum[NBBINS];
        __count2cum__<BINT,NBBINS>(cum,data);
        return __HistogramOperations__<PIXELT,BINT,NBBINS,false>::__getOtsu__(cum);
    }
    static float __getRank__(int pos,const BINT* data ){
        return data[pos]/double(data[pos]);
    }
    static PIXELT __getCentilePos__(float centile,const BINT* data){
        BINT nbPixelsAtCentile=data[NBBINS-1]*centile;
        int res=0;
        while(data[res]<nbPixelsAtCentile){
            res++;
        }
        res-=((res>0)*(nbPixelsAtCentile-data[res-1]<data[res]-nbPixelsAtCentile));
        return res;
    }
};


template <typename PIXELT, typename BINT,int NBBINS> struct __HistogramOperations__<PIXELT, BINT,NBBINS,false>{
    //NON CUMMULATIVE
    static  PIXELT __getMedian__(BINT* data){
        BINT cum[NBBINS];
        __count2cum__(cum,data);
        return __HistogramOperations__<PIXELT,BINT,NBBINS,false>::__getMedian__(data);
    }
    static PIXELT __getMode__(const BINT* data){
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
    static PIXELT __getOtsu__(const BINT* data){
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
        throw std::runtime_error("Not Implemeted");
        return data[pos]/double(data[pos]);
    }
    static PIXELT __getCentilePos__(float centile,const BINT* data){
        throw std::runtime_error("Not Implemeted");
        BINT nbPixelsAtCentile=data[NBBINS-1]*centile;
        int res=0;
        while(data[res]<nbPixelsAtCentile){
            res++;
        }
        res-=((res>0)*(nbPixelsAtCentile-data[res-1]<data[res]-nbPixelsAtCentile));
        return res;
    }
};

}

namespace histogram{
template<typename T,int NBBINS,bool ISCUMMULATIVE> struct Histogram: public bilde::StaticVector<T,NBBINS>{
    bool isCummulative(){return ISCUMMULATIVE;}
    template <typename K>
    Histogram(Buffer<K> in){
        double min=PixelMetrics<K>::defaultBg;
        //double max=PixelMetrics<K>::defaultFg;
        //double ratio=(max-min)*(NBBINS-1);
        K* inRow;
        this->setTo(0);
        for(t_sz y=0;y<in.height;y++){
            inRow=in.getRow(y);
            for(t_sz x=0;x<in.width;x++){
                this->data[int(round(inRow[x]-min))]++;
            }
        }
    }
    T getOtsu(){
        return __histogram__::__HistogramOperations__<T, t_uint32,NBBINS,ISCUMMULATIVE>::__getOtsu__(this->data);
    }
    T getMode(){
        return __histogram__::__HistogramOperations__<T, t_uint32,NBBINS,ISCUMMULATIVE>::__getMode__(this->data);
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



template <typename PIXELT, typename BINT,int NBBINS,bool ISCUMMULATIVE> struct IntegralHistogram{
    void fill_onehot_image(Buffer<PIXELT> img){
        for(t_sz y=0; y<width; y++){
            const PIXELT* inRow=img.getConstRow(y);
            for(t_sz x=0; x<width; x++){
                BINT* cur_hist=this->__getHistogramAt__(x,y);
                std::memset(cur_hist, 0, sizeof(BINT)*NBBINS);
                cur_hist[inRow[x]]=1;
            }
        }
    }
    void fill_integral_himage(Buffer<PIXELT> img){
        BINT nullhistogram[NBBINS];
        for(t_sz y=0; y<width; y++){
            const PIXELT* inRow=img.getConstRow(y);
            for(t_sz x=0; x<width; x++){
                BINT* cur_hist=this->__getHistogramAt__(x,y);
                const BINT* left_histogram;
                const BINT* top_histogram;
                const BINT* topleft_histogram;

                if(y>0){
                    top_histogram = this->__getHistogramAt__(x,y-1);
                    topleft_histogram = this->__getHistogramAt__(x-1,y-1);
                }else{
                    top_histogram = nullhistogram;
                    topleft_histogram = nullhistogram;
                }

                if(x>0){
                    left_histogram = this->__getHistogramAt__(x-1,y); // top_left already assigned from the y>0 check
                }else{
                    left_histogram = nullhistogram;
                    topleft_histogram = nullhistogram; // Essentially topleft_histogram (x>0 && y>0)
                }

                for(int n =0;n<NBBINS;n++){
                    cur_hist[n] = top_histogram[n] + left_histogram[n] - topleft_histogram[n];
                }
                cur_hist[inRow[x]]+=1;
            }
        }
    }
    void fill_cumulative_integral_himage(Buffer<PIXELT> img){
        BINT nullhistogram[NBBINS];
        std::memset(nullhistogram, 0, sizeof(BINT)*NBBINS);
        for(t_sz y=0; y<height; y++){
            const PIXELT* inRow=img.getConstRow(y);
            for(t_sz x=0; x<width; x++){
                BINT* cur_hist=this->__getHistogramAt__(x,y);
                const BINT* left_histogram;
                const BINT* top_histogram;
                const BINT* topleft_histogram;
                std::memset(cur_hist, 0, sizeof(BINT)*NBBINS);
                if(y>0){
                    top_histogram = this->__getHistogramAt__(x, y-1);
                }else{
                    top_histogram = nullhistogram;
                }

                if(x>0){
                    left_histogram = this->__getHistogramAt__(x-1, y);
                }else{
                    left_histogram = nullhistogram;
                }

                if(x>0 && y>0){
                    topleft_histogram = this->__getHistogramAt__(x-1, y-1);
                }else{
                    topleft_histogram = nullhistogram;
                }
                for(int n =0; n < NBBINS; n++){
                    cur_hist[n] = ((n>=inRow[x]) + top_histogram[n] + left_histogram[n]) - topleft_histogram[n];
                }
            }
        }
    }    
    //boost::shared_ptr<IntegralHistogram<BINT,NBBINS,ISCUMMULATIVE> > sharedThis;
    boost::shared_ptr<BINT> sharedData;
    //static const bool IS_CUMMULATIVE=ISCUMMULATIVE;
    static const int BIN_COUNT=NBBINS;
    typedef __histogram__::__HistogramOperations__<PIXELT,BINT,NBBINS,ISCUMMULATIVE> HistogramOperations;
    BINT* const __data__;
    const t_sz width;
    const t_sz height;
    const t_sint64 binLinestride;
    
    IntegralHistogram(Buffer<t_uint8> img):
        sharedData(new BINT[img.width*img.height*NBBINS]),
            __data__((BINT*)(sharedData.get())),
            width(img.width),
            height(img.height),
            binLinestride(img.width*NBBINS){
        if(ISCUMMULATIVE){
            this->fill_cumulative_integral_himage(img);
        }else{
            this->fill_integral_himage(img);
        }
    }
    BINT* const __getHistogramAt__(int x, int y){
        x=x*(x>0);
        if(x>=width){x=width-1;}
        y=y*(y>0);
        if(y>=height){y=height-1;}
        return &(__data__[x*NBBINS + y*binLinestride]);
    }
    
    struct Iterator{
        IntegralHistogram<PIXELT,BINT,NBBINS,ISCUMMULATIVE> *const integralHistogram;
        const t_sz width;
        const t_sz height;
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

        Iterator(IntegralHistogram<PIXELT, BINT,NBBINS,ISCUMMULATIVE>& ih,int l,int t,int r,int b):
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


        template <typename FilterFunc> void applyFilterFunctor(Buffer<PIXELT> outImg, FilterFunc func){
            static_assert(std::is_invocable_r<PIXELT, FilterFunc, const BINT*>::value, "FilterFunc must be callable with (PIXELT, BINT[]) and return PIXELT.");
            PIXELT* outRow;
            const BINT* curHist;
            int x,y;
            __curRow__=0;
            for(y=0; y < height;y++){
                __curRow__++;
                outRow=outImg.getRow(y);
                __curCol__=0;
                for(x=0; x < width;x++){
                    __curCol__++;
                    this->__updateCurentHistogram__();
                    outRow[x]=func(this->__curHist__);
                }
            }
        }

        template <typename FilterFunc> void applyFilterContextualFunctor(Buffer<PIXELT> outImg, Buffer<PIXELT> inImg,FilterFunc func){
            static_assert(std::is_invocable_r<PIXELT, FilterFunc, PIXELT, const BINT*>::value, "FilterFunc must be callable with (PIXELT, BINT[]) and return PIXELT.");
            PIXELT* outRow;
            PIXELT* inRow;
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
