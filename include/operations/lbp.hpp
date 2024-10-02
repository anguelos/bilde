/*
 * lbp.hpp
 *
 *  Created on: Jul 12, 2012
 *      Author: anguelos
 */

#ifndef LBP_HPP_
#define LBP_HPP_

#include <map>
#include <cmath>
#include <vector>
#include <algorithm>

#define MYROUND(x) ((int((x)-.5)*((x)<0))+(int((x)+.5)*((x)>=0)))
#define MYFLOOR(x) ((int((x)-((x)!=int(x)))*((x)<0))+(int((x))*((x)>=0)))
#define MYCEIL(x) ((int((x))*((x)<0))+(int((x)+((x)!=int(x)))*((x)>=0)))



namespace bilde {

namespace operations {
namespace lbp {

namespace __lbp_util__{




template <typename LBPTYPE=t_uint8> struct LbpIterator{
    static int calculateOtsuThreshold(const std::vector<int>& hist){
        //adapted from http://zerocool.is-a-geek.net/java-image-binarization/
        long long int total = 0; //nbPixels
        double varBetween;
        long long int sum = 0; //intencity*occurences
        for (size_t k = 0; k < hist.size(); k++) {
            total += hist[k];
            sum += int(k) * hist[k];
        }
        double sumB = 0;
        int wB = 0;
        int wF = 0;
        double varMax = 0;
        int threshold = 0;
        for (size_t t = 0; t < hist.size(); t++) {
            wB += hist[t]; // Weight Background
            if (wB == 0)continue;
            wF = total - wB; // Weight Foreground
            if (wF == 0)break;
            sumB += (double) (int(t) * hist[t]);
            double mB = sumB / wB; // Mean Background
            double mF = (sum - sumB) / wF; // Mean Foreground
            // Calculate Between Class Variance
            varBetween = (double) wB * (double) wF * (mB - mF) * (mB - mF);
            // Check if new maximum found
            if (varBetween >= varMax) {
                varMax = varBetween;
                threshold = int(t);
            }
        }
        sum=0;
        int t;
        for(t=0;t<threshold;t++){
            sum+=hist[t];
        }
        if(sum>=total*.99){
            if(bilde::util::Args().verboseLevel>=4){
                std::cerr<<"Otsu: copping at "<<t<<" with 99% limit. Normaly "<<threshold<<" returning 0\n";
            }
            return 0;
        }else{
            return threshold;
        }
    }


    typedef double t_real;
    constexpr const static t_real PI=3.14159265358979323846;


    struct OneTailFunctor{
        int threshold;
        OneTailFunctor(unsigned int thr=0):threshold(thr){}
        bool operator()(int v1,int v2){
            return v1>v2+threshold;
        }
        friend std::ostream & operator<<(std::ostream & out, const OneTailFunctor& f){
            out<<"v1+"<<f.threshold<<"<v2";return out;
        }
    };
    struct TwoTailFunctor{
        int threshold;
        TwoTailFunctor(unsigned int thr=0):threshold(thr){}
        bool operator()(int v1,int v2){
            return (v1+threshold>v2) && (v1-threshold<v2);
        }
        friend std::ostream & operator<<(std::ostream & out, const TwoTailFunctor& f){
            out<<"|v1+"<<f.threshold<<"<v2|";return out;
        }
    };

    struct SamplingFunctor{
        virtual t_uint8 getValue(t_uint8*)=0;//Inherited version of operator()
        virtual void __sampleImage__(Buffer<t_uint8>inImg,Buffer<t_uint8>outImg,int xFrom,int yFrom,int xTo,int yTo){
            std::cerr<<"SamplingFunctor::__sampleImage__ BEGIN\n";
            for(int y=yFrom;y<=yTo;y++){
                t_uint8* curInRow=inImg.getRow(y);
                t_uint8* curOutRow=outImg.getRow(y);
                for(int x=xFrom;x<=xTo;x++){
                    curOutRow[x]=this->getValue(curInRow+x);
                }
            }
            std::cerr<<"SamplingFunctor::__sampleImage__ END\n";
        }
        template <typename TCMP> void __sampleImageCmp__(TCMP cmp,Buffer<t_uint8>inImg,Buffer<t_uint8>outImg,int xFrom,int yFrom,int xTo,int yTo){
            std::cerr<<"SamplingFunctor::__sampleImage__ BEGIN\n";
            for(int y=yFrom;y<=yTo;y++){
                t_uint8* curInRow=inImg.getRow(y);
                t_uint8* curOutRow=outImg.getRow(y);
                for(int x=xFrom;x<=xTo;x++){
                    curOutRow[x]=cmp(this->getValue(curInRow+x),curInRow[x]);
                }
            }
            std::cerr<<"SamplingFunctor::__sampleImage__ END\n";
        }

    };
    struct NearestFunctor:public SamplingFunctor{//AKA nearEuclidean
        virtual t_uint8 getValue(t_uint8* p){return (*this)(p);}
        int __offset__;
        NearestFunctor(Buffer<t_uint8> img,double radius,int nbSamples,int specificSample,double phase=0){
            double angle=phase+specificSample*2*PI/nbSamples;
            __offset__=std::round(cos(angle)*radius)+std::round(sin(angle)*radius)*img.getPixelLinestride();
            if(bilde::util::Args().verboseLevel>=10){
                std::cerr<<"\nNearest Functor\nimg=["<<img.width<<","<<img.height<<"], ls="<<img.getPixelLinestride()<<"\nspecificSample: "<<specificSample<<",  __offset__:" <<__offset__<<"\n\n";
            }
        }
        bilde::t_uint8 operator()(t_uint8* pixel){
            return pixel[this->__offset__];
        }
        friend std::ostream & operator<<(std::ostream & out, const NearestFunctor& f){
            out<<f.__offset__;return out;
        }
    };

    struct ChessboardFunctor:public SamplingFunctor{//AKA nearChessboard
        virtual t_uint8 getValue(t_uint8* p){return (*this)(p);}
        int __offset__;
        ChessboardFunctor(Buffer<t_uint8> img,double radius,int nbSamples,int specificSample,double phase=0){
            double angle=phase+specificSample*2*PI/nbSamples;
            double newRadius=radius/(std::abs(sin(angle))+std::abs(cos(angle)));
            double x=(newRadius)*std::cos(angle);
            double y=(newRadius)*std::sin(angle);
            __offset__=std::round(x)+std::round(y)*img.getPixelLinestride();
            if(bilde::util::Args().verboseLevel>=10){
                std::cerr<<"\nChessboard Functor\nimg=["<<img.width<<","<<img.height<<"], ls="<<img.getPixelLinestride()<<"\nspecificSample: "<<specificSample<<",  __offset__:" <<__offset__<<"\n\n";
            }
        }
        bilde::t_uint8 operator()(t_uint8* pixel){
            return pixel[this->__offset__];
        }
        friend std::ostream & operator<<(std::ostream & out, const ChessboardFunctor& f){
            out<<f.__offset__;return out;
        }
    };

    struct CityblockFunctor:public SamplingFunctor{//AKA nearCityblock
        virtual t_uint8 getValue(t_uint8* p){return (*this)(p);}
        int __offset__;
        CityblockFunctor(Buffer<t_uint8> img,double radius,int nbSamples,int specificSample,double phase=0){
            double angle=phase+specificSample*2*PI/nbSamples;
            double x=cos(angle)*radius;double y=sin(angle)*radius;
            if(abs(x)>abs(y)){
                x=radius*(x>0)-radius*(x<=0);
            }else{
                y=radius*(y>0)-radius*(y<=0);
            }
            __offset__=std::round(x)+std::round(y)*img.getPixelLinestride();
            if(bilde::util::Args().verboseLevel>=10){
                std::cerr<<"\nCityblock Functor\nimg=["<<img.width<<","<<img.height<<"], ls="<<img.getPixelLinestride()<<"\nspecificSample: "<<specificSample<<",  __offset__:" <<__offset__<<"\n\n";
            }
        }
        bilde::t_uint8 operator()(t_uint8* pixel){
            return pixel[this->__offset__];
        }
        friend std::ostream & operator<<(std::ostream & out, const CityblockFunctor& f){
            out<<f.__offset__;return out;
        }
    };


    struct BilinearFunctor:public SamplingFunctor{
        virtual t_uint8 getValue(t_uint8* p){return (*this)(p);}
        int __offsetTopLeft__;
        int __offsetTopRight__;
        int __offsetBottomLeft__;
        int __offsetBottomRight__;
        t_real __coefficientTopLeft__;
        t_real __coefficientTopRight__;
        t_real __coefficientBottomLeft__;
        t_real __coefficientBottomRight__;
        BilinearFunctor(Buffer<t_uint8> img,double radius,int nbSamples,int specificSample,double phase=0){
            double angle=phase+specificSample*2*PI/nbSamples;
            double positionX=cos(angle)*radius;
            double positionY=sin(angle)*radius;
            __offsetTopLeft__=std::floor(positionX)+std::floor(positionY)*img.getPixelLinestride();
            __offsetTopRight__=std::ceil(positionX)+std::floor(positionY)*img.getPixelLinestride();
            __offsetBottomLeft__=std::floor(positionX)+std::ceil(positionY)*img.getPixelLinestride();
            __offsetBottomRight__=std::ceil(positionX)+std::ceil(positionY)*img.getPixelLinestride();

            __coefficientTopLeft__=(1-(positionX-std::floor(positionX)))*(1-(positionY-std::floor(positionY)));
            __coefficientTopRight__=(1-(std::ceil(positionX)-positionX))*(1-(positionY-std::floor(positionY)));
            __coefficientBottomLeft__=(1-(positionX-std::floor(positionX)))*(1-(std::ceil(positionY)-positionY));
            __coefficientBottomRight__=(1-(std::ceil(positionX)-positionX))*(1-(std::ceil(positionY)-positionY));
            //when the offset is an perfect integer, there are 4 coefficients of 1
            t_real coefSum=__coefficientTopLeft__+__coefficientTopRight__+__coefficientBottomLeft__+__coefficientBottomRight__;
            __coefficientTopLeft__/=coefSum;
            __coefficientTopRight__/=coefSum;
            __coefficientBottomLeft__/=coefSum;
            __coefficientBottomRight__/=coefSum;
        }
        bilde::t_uint8 operator()(t_uint8* pixel){
            return std::round(pixel[__offsetTopLeft__]*__coefficientTopLeft__+
                    pixel[__offsetTopRight__]*__coefficientTopRight__+
                    pixel[__offsetBottomLeft__]*__coefficientBottomLeft__+
                    pixel[__offsetBottomRight__]*__coefficientBottomRight__);
        }
        friend std::ostream & operator<<(std::ostream & out, const BilinearFunctor& f){
            out<<"[("<<f.__offsetTopLeft__<<"*"<<f.__coefficientTopLeft__<<"),";
            out<<"("<<f.__offsetTopRight__<<"*"<<f.__coefficientTopRight__<<"),";
            out<<"("<<f.__offsetBottomLeft__<<"*"<<f.__coefficientBottomLeft__<<"),";
            out<<"("<<f.__offsetBottomRight__<<"*"<<f.__coefficientBottomRight__<<")]";
            return out;
        }
    };

    struct ConvolutionalFunctor:public SamplingFunctor{
        virtual t_uint8 getValue(t_uint8* p){return (*this)(p);}
        static int removeDuplicates(std::vector<int>& offsets,std::vector<t_real>& coefficients){
            //removes duplicates created from bilinear interpolation of consecutive pixels
            std::map<int,t_real> coefficientMap;
            for(size_t k=0;k<offsets.size();k++){
                coefficientMap.insert(std::pair<int,t_real>(offsets[k],0));
            }
            for(size_t k=0;k<offsets.size();k++){
                coefficientMap[offsets[k]]+=coefficients[k];
            }
            offsets.resize(coefficientMap.size());
            coefficients.resize(coefficientMap.size());
            int pos=0;
            for(auto iter=coefficientMap.begin();iter!=coefficientMap.end();++iter){
                offsets[pos]=iter->first;
                coefficients[pos]=iter->second;
                pos++;
            }
            return coefficientMap.size();
        }
        std::vector<int> __offsets__;
        std::vector<t_real> __coefficients__;
        int __nbOffsets__;
        ConvolutionalFunctor(Buffer<t_uint8> img,double radius,int nbSamples,int specificSample,double phase=0){
            double angle=phase+specificSample*2*PI/nbSamples;
            int nbPoints=std::ceil(2*PI*radius)*2+1;
            __nbOffsets__=nbPoints*4;
            __offsets__.resize(__nbOffsets__);
            __coefficients__.resize(__nbOffsets__);
            std::vector<double> xPoints(nbPoints);
            std::vector<double> yPoints(nbPoints);
            std::vector<double> convolutionValues(nbPoints);
            double sigma=1;
            double cSum=0;
            //Calculation of the 1D signal on the arch
            for(int k=0;k<nbPoints;k++){
                double arcx=2*((k-nbPoints/2)/double(nbPoints));
                convolutionValues[k]=exp((-arcx*arcx)/(2*sigma*sigma));
                cSum+=convolutionValues[k];
                xPoints[k]=radius*cos(angle+arcx*(2*PI/nbSamples));
                yPoints[k]=radius*sin(angle+arcx*(2*PI/nbSamples));
            }
            //Calculation of the bilinear interpolation of the 1D signal on the arch
            for(int k=0;k<nbPoints;k++){
                convolutionValues[k]/=cSum;//Normalising the convolution to 1;
                double arcx=angle+2*((k-nbPoints/2)/double(nbPoints));
                double positionX=cos(arcx)*radius;
                double positionY=sin(arcx)*radius;

                __offsets__[k*4+0]=std::floor(positionX)+std::floor(positionY)*img.getPixelLinestride();
                __offsets__[k*4+1]=std::ceil(positionX)+std::floor(positionY)*img.getPixelLinestride();
                __offsets__[k*4+2]=std::floor(positionX)+std::ceil(positionY)*img.getPixelLinestride();
                __offsets__[k*4+3]=std::ceil(positionX)+std::ceil(positionY)*img.getPixelLinestride();

                __coefficients__[k*4+0]=(1-(positionX-std::floor(positionX)))+(1-(positionY-std::floor(positionY)));
                __coefficients__[k*4+1]=(1-(std::ceil(positionX)-positionX))+(1-(positionY-std::floor(positionY)));
                __coefficients__[k*4+2]=(1-(positionX-std::floor(positionX)))+(1-(std::ceil(positionY)-positionY));
                __coefficients__[k*4+3]=(1-(std::ceil(positionX)-positionX))+(1-(std::ceil(positionY)-positionY));
                t_real coefSum=1/(__coefficients__[k*4+0]+__coefficients__[k*4+1]+__coefficients__[k*4+2]+__coefficients__[k*4+3]);
                coefSum*=convolutionValues[k];
                __coefficients__[k*4+0]*=coefSum;
                __coefficients__[k*4+1]*=coefSum;
                __coefficients__[k*4+2]*=coefSum;
                __coefficients__[k*4+3]*=coefSum;
            }
            this->__nbOffsets__=ConvolutionalFunctor::removeDuplicates(this->__offsets__,this->__coefficients__);//compressing the dictionary
        }
        t_uint8 operator()(t_uint8* pixel){
            t_real sum=0;
            for(int k=0;k<__nbOffsets__;k++){
                sum+=pixel[__offsets__[k]]*__coefficients__[k];
            }
            return std::round(sum);
        }
        friend std::ostream & operator<<(std::ostream & out, const ConvolutionalFunctor& f){
            out<<"["<<f.__nbOffsets__<<"("<<f.__offsets__[0]<<"*"<<f.__coefficients__[0]<<")";
            double coefSum=f.__coefficients__[0];
            for(int k=1;k<f.__nbOffsets__;k++){
                out<<",("<<f.__offsets__[k]<<"*"<<f.__coefficients__[k]<<")";
                coefSum+=f.__coefficients__[k];
            }
            out<<"="<<coefSum<<"]";
            return out;
        }
    };

	Buffer<t_uint8> img;
	const int nbSamples;
	const double radius;
	double phase;
    const int xFrom;//the left most position in the image that wont cause an overflow
	const int xTo;
	const int yFrom;
    const int yTo;
    std::string sampling;
    std::string cmp;
    int threshold;

    template <int NBSAMPLES,int CURRENT_SAMPLE, typename INTERPOLATE_TYPE, typename CMP_TYPE> struct LbpComputer{
        //Metaprogram for fast computation of LBP and Deltas N->N-1
        INTERPOLATE_TYPE f;
        CMP_TYPE cmp;
        LbpComputer<NBSAMPLES,CURRENT_SAMPLE-1,INTERPOLATE_TYPE,CMP_TYPE> next;
        const int LBP_COEFFICIENT;
        LbpComputer(Buffer<t_uint8> img,double radius):f(img,radius,NBSAMPLES,CURRENT_SAMPLE),cmp(0),next(img,radius),LBP_COEFFICIENT(1<<CURRENT_SAMPLE){}
        void updateDeltaHistogram(t_uint8* pixel,int* histogram){            
            histogram[std::abs(*pixel-f(pixel))]++;
            next.updateDeltaHistogram(pixel, histogram);
        }
        void setThreshold(int thr){
            cmp.threshold=thr;
            next.setThreshold(thr);
        }
        void __debugSampling__(Buffer<t_uint8> in,Buffer<t_uint8> out,int xFrom,int yFrom,int xTo,int yTo,int slice){            
            if(CURRENT_SAMPLE==slice){
                  std::cerr<<"__debugSampling__: asked slice:"<<slice<<" our slice:"<<CURRENT_SAMPLE<<", processing\n";
                  f.__sampleImage__(in,out,xFrom,yFrom,xTo,yTo);
            }else if(slice<NBSAMPLES && slice>=0){
                std::cerr<<"__debugSampling__: asked slice:"<<slice<<" our slice:"<<CURRENT_SAMPLE<<", moving to next\n";
                next.__debugSampling__(in,out,xFrom,yFrom,xTo,yTo,slice);
            }else{
                std::cerr<<"RECEIVED SLICE:"<<slice<<" I AM "<<CURRENT_SAMPLE<<" AND NBSAMPLES IS "<<NBSAMPLES<<" throwing!\n";
                throw "DEBUG SLICE N, unexpeted value";
            }
        }
        void __debugCmp__(Buffer<t_uint8> in,Buffer<LBPTYPE> out,int xFrom,int yFrom,int xTo,int yTo,int slice){
            if(CURRENT_SAMPLE==slice){
                  std::cerr<<"__debugCmp__: asked slice:"<<slice<<" our slice:"<<CURRENT_SAMPLE<<", processing\n";
                  f.__sampleImageCmp__(cmp,in,out,xFrom,yFrom,xTo,yTo);
            }else if(slice<NBSAMPLES && slice>=0){
                std::cerr<<"__debugCmp__: asked slice:"<<slice<<" our slice:"<<CURRENT_SAMPLE<<", moving to next\n";
                next.__debugCmp__(in,out,xFrom,yFrom,xTo,yTo,slice);
            }else{
                std::cerr<<"RECEIVED SLICE:"<<slice<<" I AM "<<CURRENT_SAMPLE<<" AND NBSAMPLES IS "<<NBSAMPLES<<" throwing!\n";
                throw "DEBUG SLICE N, unexpeted value";
            }
        }


        int getLBP(t_uint8* pixel){
            return LBP_COEFFICIENT*cmp(f(pixel),*pixel)+next.getLBP(pixel);
        }
        friend std::ostream & operator<<(std::ostream & out, const LbpComputer<NBSAMPLES,CURRENT_SAMPLE,INTERPOLATE_TYPE,CMP_TYPE>& lbpComp){
            if(CURRENT_SAMPLE==NBSAMPLES-1){
                //out<<"Functors:\nImg : ["<<lbpComp.f.img.width<<","<<lbpComp.f.img.height<<"], ls="<<lbpComp.f.img.getPixelLinestride()<<"\n";
                out<<"Functors:\n";
            }
            out<<lbpComp.next;            
            out<<"\t"<<CURRENT_SAMPLE<<": SAMPLING= "<<lbpComp.f<<", CMP= "<<lbpComp.cmp<<" Coefficient="<<lbpComp.LBP_COEFFICIENT<<"\n";
            return out;
        }
    };

    template <int NBSAMPLES, typename INTERPOLATE_TYPE, typename CMP_TYPE> struct LbpComputer<NBSAMPLES,0,INTERPOLATE_TYPE,CMP_TYPE>{
        //Metaprogram for fast computation of LBP and Deltas N=0
        INTERPOLATE_TYPE f;
        CMP_TYPE cmp;
        const int LBP_COEFFICIENT;
        LbpComputer(Buffer<t_uint8> img,double radius):f(img,radius,NBSAMPLES,0),cmp(0),LBP_COEFFICIENT(1<<0){}
        void updateDeltaHistogram(t_uint8* pixel,int* histogram){
            //if(int(*pixel)!=255 && int(f(pixel)!=255)){std::cerr<<int(*pixel)<<","<<int(f(pixel))<<","<<abs(*pixel-f(pixel))<<"\n";}else{std::cerr<<".";}
            histogram[std::abs(*pixel-f(pixel))]++;
        }

        void setThreshold(int thr){
            cmp.threshold=thr;
        }
        void __debugSampling__(Buffer<t_uint8> in,Buffer<t_uint8> out,int xFrom,int yFrom,int xTo,int yTo,int slice){
            if(0==slice){
                std::cerr<<"__debugSampling__: asked slice:"<<slice<<" our slice:"<<0<<", processing\n";
                f.__sampleImage__(in,out,xFrom,yFrom,xTo,yTo);
            }else{
                throw "DEBUG SLICE 0, unexpeted value";
            }
        }
        void __debugCmp__(Buffer<t_uint8> in,Buffer<LBPTYPE> out,int xFrom,int yFrom,int xTo,int yTo,int slice){
            if(0==slice){
                  std::cerr<<"__debugSamplingCmp__: asked slice:"<<slice<<" our slice:"<<0<<", processing\n";
                  f.__sampleImageCmp__(cmp,in,out,xFrom,yFrom,xTo,yTo);
            }else{
                std::cerr<<"RECEIVED SLICE:"<<slice<<" I AM "<<0<<" AND NBSAMPLES IS "<<NBSAMPLES<<" throwing!\n";
                throw "DEBUG SLICE CMP 0, unexpeted value";
            }
        }


        int getLBP(t_uint8* pixel){
            return cmp(f(pixel),*pixel)*LBP_COEFFICIENT;
        }
        friend std::ostream & operator<<(std::ostream & out, const LbpComputer<NBSAMPLES,0,INTERPOLATE_TYPE,CMP_TYPE>& lbpComp){
            out<<"\t0: SAMPLING= "<<lbpComp.f<<", CMP= "<<lbpComp.cmp<<" Coefficient="<<lbpComp.LBP_COEFFICIENT<<"\n";
            return out;
        }
    };


    template<int NBSAMPLES,typename SAMPLING_FUNCTOR,typename CMP_FUNCTOR> std::vector<int> __getDeltaHistogram__(){
        std::vector<int> hist((1<<nbSamples),0);
        int* p2Hist=&(hist[0]);
        LbpComputer<NBSAMPLES,NBSAMPLES-1,SAMPLING_FUNCTOR,CMP_FUNCTOR> allFunctors(img,this->radius);
        for(int y=yFrom;y<=yTo;y++){t_uint8* rowEnd=img.getRow(y)+xTo+1;
            for(t_uint8* curPixel=img.getRow(y)+xFrom;curPixel!=rowEnd;curPixel++){
                allFunctors.updateDeltaHistogram(curPixel,p2Hist);
            }
        }
        return hist;
    }

    void __debugSampling__(Buffer<LBPTYPE> out,int slice=-1){
        const int NBSAMPLES=8;
        if(slice<0 ||slice>=NBSAMPLES){
            slice=0;
        }
        typedef BilinearFunctor SAMPLING_FUNCTOR;
        std::cerr<<"DEBUGGING SLICE:"<<slice<<"\n";
        LbpComputer<NBSAMPLES,NBSAMPLES-1,SAMPLING_FUNCTOR,OneTailFunctor>(img,this->radius).__debugSampling__(this->img,out,this->xFrom,this->yFrom,this->xTo,this->yTo,slice);
        std::cerr<<"DEBUGGING SLICE:"<<slice<<" FINISEHD\n";
    }
    void __debugCmp__(Buffer<LBPTYPE> out,int slice=-1){
        const int NBSAMPLES=8;
        if(slice<0 ||slice>=NBSAMPLES){
            slice=0;
        }
        typedef BilinearFunctor SAMPLING_FUNCTOR;
        std::cerr<<"DEBUGGING SLICE:"<<slice<<"\n";
        LbpComputer<NBSAMPLES,NBSAMPLES-1,SAMPLING_FUNCTOR,OneTailFunctor>(img,this->radius).__debugCmp__(this->img,out,this->xFrom,this->yFrom,this->xTo,this->yTo,slice);
        std::cerr<<"DEBUGGING SLICE:"<<slice<<" FINISEHD\n";
    }

    template<int NBSAMPLES,typename SAMPLING_FUNCTOR,typename CMP_FUNCTOR> void __performLBPTransform__(Buffer<LBPTYPE> out){
        LbpComputer<NBSAMPLES,NBSAMPLES-1,SAMPLING_FUNCTOR,CMP_FUNCTOR> allFunctors(img,this->radius);
        //std::cerr<<"DEBUG THRESHOLD BEFORE OTSU:"<<this->threshold<<"\n";
        if(this->threshold<0){
            std::vector<int> h=__getDeltaHistogram__<NBSAMPLES,SAMPLING_FUNCTOR,CMP_FUNCTOR>();
            this->threshold=calculateOtsuThreshold(h);
            if(bilde::util::Args().verboseLevel>9){
                std::cerr<<"DeltaHistograms:["<<h[0];
                for (int k=1;k<h.size();k++)std::cerr<<","<<h[k];
                std::cerr<<"]\nOtsu threshold="<<this->threshold<<"\n";
            }
        }
        //std::cerr<<"DEBUG THRESHOLD AFTER OTSU:"<<this->threshold<<"\n";
        allFunctors.setThreshold(this->threshold);
        if(bilde::util::Args().verboseLevel>5){
            std::cerr<<allFunctors;
        }
        for(int y=yFrom;y<=yTo;y++){t_uint8* rowEnd=img.getRow(y)+xTo+1;
            LBPTYPE* outPixel=out.getRow(y)+xFrom;
            for(t_uint8* curPixel=img.getRow(y)+xFrom ; curPixel!=rowEnd ;curPixel++,outPixel++){
                *outPixel=allFunctors.getLBP(curPixel);
            }
        }
    }


    template<typename T> void __fillHistogram__(Buffer<T> img,std::vector<int> &h){
        int y;
        T* curPixel;
        T* curRowEnd;
        for(y=yFrom;y<=yTo;y++){
            curPixel=img.getRow(y)+xFrom;
            curRowEnd=img.getRow(y)+xTo+1;
            while(curPixel!=curRowEnd){
                h[*curPixel]++;
                curPixel++;
            }
        }        
    }

    LbpIterator(Buffer<t_uint8> i,int n,double r,std::string samplingStr,std::string cmpStr,std::string thresholdStr):img(i),
        nbSamples(n),radius(r),
        xFrom(std::ceil(r)),xTo(i.width-(1+std::ceil(r))),
        yFrom(std::ceil(r)),yTo(i.height-(1+std::ceil(r))),
        phase(0){
            this->sampling=samplingStr;
            this->cmp=cmpStr;
            if(thresholdStr=="otsu"){
                this->threshold=-1;                
            }else{
                this->threshold=std::stoi(thresholdStr);
            }
            //std::cerr<<"DEBUG:\n\tnbsamples : "<<n<<"\n\trarius : "<<r<<"\n\tsampling : "<<samplingStr<<"\n\tcmp : "<<cmpStr<<"\n\threshold : "<<thresholdStr<<"\n\n";
//            int h1[]={0,0,0,0};
//            for(int y=0;y<img.height;y++){
//                t_uint8 *row=img.getRow(y);
//                for(int x =0 ;x<img.width;x++){
//                    h1[row[x]/64]++;
//                }
//            }
//            std::cout<<"DEBUG:\n\tnbsamples : "<<n<<"\n\trarius : "<<r<<"\n\tsampling : "<<samplingStr<<"\n\tcmp : "<<cmpStr<<"\n\threshold : "<<thresholdStr<<"\n\tinput hist : {"<<h1[0]<<","<<h1[1]<<","<<h1[2]<<","<<h1[3]<<"}\n\n";
    }

    void applyLBPTransform(Buffer<LBPTYPE> out){
        if(this->cmp=="one-tail"){
            typedef OneTailFunctor CMP;
            if(this->sampling=="nearEuclidean"){
                typedef NearestFunctor SAMPLING;
                switch(this->nbSamples){
                case 4:__performLBPTransform__<4,SAMPLING,CMP>(out);break;
                case 6:__performLBPTransform__<6,SAMPLING,CMP>(out);break;
                case 8:__performLBPTransform__<8,SAMPLING,CMP>(out);break;
                case 10:__performLBPTransform__<10,SAMPLING,CMP>(out);break;
                case 12:__performLBPTransform__<12,SAMPLING,CMP>(out);break;
                case 16:__performLBPTransform__<16,SAMPLING,CMP>(out);break;
                case 24:__performLBPTransform__<24,SAMPLING,CMP>(out);break;
                default:throw "Unexpected number of samples";
                }
            }else if(this->sampling=="bilinear"){
                typedef BilinearFunctor SAMPLING;
                switch(this->nbSamples){
                case 4:__performLBPTransform__<4,SAMPLING,CMP>(out);break;
                case 6:__performLBPTransform__<6,SAMPLING,CMP>(out);break;
                case 8:__performLBPTransform__<8,SAMPLING,CMP>(out);break;
                case 10:__performLBPTransform__<10,SAMPLING,CMP>(out);break;
                case 12:__performLBPTransform__<12,SAMPLING,CMP>(out);break;
                case 16:__performLBPTransform__<16,SAMPLING,CMP>(out);break;
                case 24:__performLBPTransform__<24,SAMPLING,CMP>(out);break;
                default:throw "Unexpected number of samples";
                }
            }
            else if(this->sampling=="nearCityblock"){
                typedef CityblockFunctor SAMPLING;
                switch(this->nbSamples){
                case 4:__performLBPTransform__<4,SAMPLING,CMP>(out);break;
                case 6:__performLBPTransform__<6,SAMPLING,CMP>(out);break;
                case 8:__performLBPTransform__<8,SAMPLING,CMP>(out);break;
                case 10:__performLBPTransform__<10,SAMPLING,CMP>(out);break;
                case 12:__performLBPTransform__<12,SAMPLING,CMP>(out);break;
                case 16:__performLBPTransform__<16,SAMPLING,CMP>(out);break;
                case 24:__performLBPTransform__<24,SAMPLING,CMP>(out);break;
                default:throw "Unexpected number of samples";
                }
            }
            else if(this->sampling=="nearChessboard"){
                typedef ChessboardFunctor SAMPLING;
                switch(this->nbSamples){
                case 4:__performLBPTransform__<4,SAMPLING,CMP>(out);break;
                case 6:__performLBPTransform__<6,SAMPLING,CMP>(out);break;
                case 8:__performLBPTransform__<8,SAMPLING,CMP>(out);break;
                case 10:__performLBPTransform__<10,SAMPLING,CMP>(out);break;
                case 12:__performLBPTransform__<12,SAMPLING,CMP>(out);break;
                case 16:__performLBPTransform__<16,SAMPLING,CMP>(out);break;
                case 24:__performLBPTransform__<24,SAMPLING,CMP>(out);break;
                default:throw "Unexpected number of samples";
                }
            }
            else if(this->sampling=="arc-convolution"){
                typedef ConvolutionalFunctor SAMPLING;
                switch(this->nbSamples){
                case 4:__performLBPTransform__<4,SAMPLING,CMP>(out);break;
                case 6:__performLBPTransform__<6,SAMPLING,CMP>(out);break;
                case 8:__performLBPTransform__<8,SAMPLING,CMP>(out);break;
                case 10:__performLBPTransform__<10,SAMPLING,CMP>(out);break;
                case 12:__performLBPTransform__<12,SAMPLING,CMP>(out);break;
                case 16:__performLBPTransform__<16,SAMPLING,CMP>(out);break;
                case 24:__performLBPTransform__<24,SAMPLING,CMP>(out);break;
                default:throw "Unexpected number of samples";
                }
            }else{
                throw "Unexpected sampling type";
            }
        }else if(this->cmp=="two-tail"){
            typedef TwoTailFunctor CMP;
            if(this->sampling=="nearEuclidean"){
                typedef NearestFunctor SAMPLING;
                switch(this->nbSamples){
                case 4:__performLBPTransform__<4,SAMPLING,CMP>(out);break;
                case 6:__performLBPTransform__<6,SAMPLING,CMP>(out);break;
                case 8:__performLBPTransform__<8,SAMPLING,CMP>(out);break;
                case 10:__performLBPTransform__<10,SAMPLING,CMP>(out);break;
                case 12:__performLBPTransform__<12,SAMPLING,CMP>(out);break;
                case 16:__performLBPTransform__<16,SAMPLING,CMP>(out);break;
                case 24:__performLBPTransform__<24,SAMPLING,CMP>(out);break;
                default:throw "Unexpected number of samples";
                }
            }else if(this->sampling=="bilinear"){
                typedef BilinearFunctor SAMPLING;
                switch(this->nbSamples){
                case 4:__performLBPTransform__<4,SAMPLING,CMP>(out);break;
                case 6:__performLBPTransform__<6,SAMPLING,CMP>(out);break;
                case 8:__performLBPTransform__<8,SAMPLING,CMP>(out);break;
                case 10:__performLBPTransform__<10,SAMPLING,CMP>(out);break;
                case 12:__performLBPTransform__<12,SAMPLING,CMP>(out);break;
                case 16:__performLBPTransform__<16,SAMPLING,CMP>(out);break;
                case 24:__performLBPTransform__<24,SAMPLING,CMP>(out);break;
                default:throw "Unexpected number of samples";
                }
            }
            else if(this->sampling=="nearCityblock"){
                typedef CityblockFunctor SAMPLING;
                switch(this->nbSamples){
                case 4:__performLBPTransform__<4,SAMPLING,CMP>(out);break;
                case 6:__performLBPTransform__<6,SAMPLING,CMP>(out);break;
                case 8:__performLBPTransform__<8,SAMPLING,CMP>(out);break;
                case 10:__performLBPTransform__<10,SAMPLING,CMP>(out);break;
                case 12:__performLBPTransform__<12,SAMPLING,CMP>(out);break;
                case 16:__performLBPTransform__<16,SAMPLING,CMP>(out);break;
                case 24:__performLBPTransform__<24,SAMPLING,CMP>(out);break;
                default:throw "Unexpected number of samples";
                }
            }
            else if(this->sampling=="nearChessboard"){
                typedef ChessboardFunctor SAMPLING;
                switch(this->nbSamples){
                case 4:__performLBPTransform__<4,SAMPLING,CMP>(out);break;
                case 6:__performLBPTransform__<6,SAMPLING,CMP>(out);break;
                case 8:__performLBPTransform__<8,SAMPLING,CMP>(out);break;
                case 10:__performLBPTransform__<10,SAMPLING,CMP>(out);break;
                case 12:__performLBPTransform__<12,SAMPLING,CMP>(out);break;
                case 16:__performLBPTransform__<16,SAMPLING,CMP>(out);break;
                case 24:__performLBPTransform__<24,SAMPLING,CMP>(out);break;
                default:throw "Unexpected number of samples";
                }
            }
            else if(this->sampling=="arc-convolution"){
                typedef ConvolutionalFunctor SAMPLING;
                switch(this->nbSamples){
                case 4:__performLBPTransform__<4,SAMPLING,CMP>(out);break;
                case 6:__performLBPTransform__<6,SAMPLING,CMP>(out);break;
                case 8:__performLBPTransform__<8,SAMPLING,CMP>(out);break;
                case 10:__performLBPTransform__<10,SAMPLING,CMP>(out);break;
                case 12:__performLBPTransform__<12,SAMPLING,CMP>(out);break;
                case 16:__performLBPTransform__<16,SAMPLING,CMP>(out);break;
                case 24:__performLBPTransform__<24,SAMPLING,CMP>(out);break;
                default:throw "Unexpected number of samples";
                }
            }else{
                throw "Unexpected sampling type";
            }
        }else{
            throw "Unexpected comparisson type";
        }
    }

    std::vector<int> getLBPHistogram(){
        Buffer<LBPTYPE> out(img.width,img.height);
        this->applyLBPTransform(out);
        std::vector<int> h(1<<nbSamples,0);
        this->__fillHistogram__(out,h);
        return h;
    }

};





struct DictionaryCompressor{

    struct UniformityFunctor{
        int nbSamples;
        UniformityFunctor(int ns){
            nbSamples=ns;
        }
        int operator()(int pattern){
            int res=0;
            pattern+=((1<<nbSamples)*(pattern&1)+(2<<nbSamples));
            while(pattern>2){
                res+=(pattern%4 ==1);
                pattern/=2;
            }
            return res;
        }
    };

    struct RotationInvarianceFunctor{
        int nbSamples;
        int mask;
        std::vector<int> equivalents;
        RotationInvarianceFunctor(int ns){
            nbSamples=ns;
            equivalents.resize(nbSamples);
            mask=(2<<nbSamples)-1;
        }
        void operator()(int pattern,std::vector<int> &res){
            if( res[pattern]>=0){//if not already computed
                for(int k=0;k<nbSamples;k++){
                    equivalents[k]=((pattern<<k)&mask)+(pattern>>(nbSamples-k));
                }
                int ri= *std::min(equivalents.begin(),equivalents.end());
                for(int k=0;k<nbSamples;k++){
                    res[equivalents[k]]=ri;
                }
            }
        }
        int operator()(int pattern){
            for(int k=0;k<nbSamples;k++){
                equivalents[k]=((pattern<<k)&mask)+(pattern>>(nbSamples-k));
            }
            return *std::min(equivalents.begin(),equivalents.end());
        }
    };
    bool rotationInvariance;
    bool uniformity;
    int nbSamples;
    int hSize;
    int compressedSize;
    std::vector<int> finalMap;

    static std::vector<int> createUniformMap(int nbSamples){
        UniformityFunctor f(nbSamples);
        int hSize=(1<<nbSamples);
        std::vector<int> res(hSize+1,0);
        for(int k=0;k<hSize;k++){
            if(f(k)>1){
                res[k]=hSize;
            }else{
                res[k]=k;
            }
        }
        return res;
    }

    static std::vector<int> createRotationInvariantMap(int nbSamples){
        RotationInvarianceFunctor f(nbSamples);
        int hSize=(1<<nbSamples);
        std::vector<int> res(hSize,-2);
        for(int k=0;k<hSize;k++){
            f(k,res);
        }
        return res;
    }

    DictionaryCompressor(bool ri,bool u,int ns){
        rotationInvariance=ri;
        uniformity=u;
        nbSamples=ns;
        hSize=1<<ns;

        std::vector<int> riMap(hSize,-1);
        std::vector<int> uMap;
        if(uniformity){
            uMap=createUniformMap(nbSamples);
            if(rotationInvariance){
                riMap=createRotationInvariantMap(nbSamples);
                for(int k=0;k<hSize;k++){
                    finalMap[k]=uMap[riMap[k]];
                }
            }else{
                for(int k=0;k<hSize;k++){
                    finalMap[k]=uMap[k];
                }
            }
        }else{
            if(rotationInvariance){
                riMap=createRotationInvariantMap(nbSamples);
                for(int k=0;k<hSize;k++){
                    finalMap[k]=riMap[k];
                }
            }else{
                for(int k=0;k<hSize;k++){
                    finalMap[k]=k;
                }
            }
        }
        std::vector<int> compressorMap(hSize+1,0);
        compressedSize=0;
        for(int k=0;k<finalMap.size();k++){
            if(finalMap[k]==k){
                compressorMap[k]=compressedSize;
                compressedSize++;
            }else{
                finalMap[k]=-1;
            }
        }
        for(int k=0;k<hSize;k++){
            finalMap[k]=compressorMap[finalMap[k]];
        }
    }
    std::vector<int> compress(std::vector<int> & histogram){
        if(uniformity*rotationInvariance==0){
            return histogram;
        }
        std::vector<int> res(compressedSize,0);
        for(int k=0;k<hSize;k++){
            res[finalMap[k]]+=histogram[k];
        }
        return res;
    }
    std::vector<std::string> compress(std::vector<std::string> & featureNames){
        if(uniformity*rotationInvariance==0){
            return featureNames;
        }
        std::vector<std::string> res(compressedSize,"...");
        for(int k=0;k<hSize;k++){
            //res[this->finalMap[featureNames[k]]]=featureNames[k];
        }
        return res;
    }
};



}

template <typename LBPTYPE> void __lbpTransform__(Buffer<LBPTYPE> out,Buffer<t_uint8> in,int nbSamples=8,double radius=4,std::string interpolation="bilinear",std::string cmpOperation="one-tail",std::string cmpThreshold="otsu"){
    __lbp_util__::LbpIterator<LBPTYPE> iter(in,nbSamples,radius,interpolation,cmpOperation,cmpThreshold);
    iter.applyLBPTransform(out);
}

}

}
}

#endif /* LBP_HPP_ */
