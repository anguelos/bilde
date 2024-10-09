/*
 * colorspace.hpp
 *
 *  Created on: Mar 14, 2013
 *      Author: anguelos
 */

#ifndef COLORSPACE_HPP_
#define COLORSPACE_HPP_

#include <cmath>
#include <vector>

namespace bilde {
namespace operations {
namespace colorspace {

template<typename T>void __rightBitShift__(Buffer<T> out,Buffer<T> in,int nbBits){
	//Upsample
	T* inRow;
	T* outRow;
	int x,y;
	for(y=0;y<in.height;y++){
		inRow=in.getRow(y);
		outRow=out.getRow(y);
		for(x=0;x<in.width;x++){
			outRow[x]=(inRow[x]>>nbBits);
		}
	}
}

template<typename T>void __leftBitShift__(Buffer<T> out,Buffer<T> in,int nbBits){
	//Downsample
	T* inRow;
	T* outRow;
	int x,y;
	for(y=0;y<in.height;y++){
		inRow=in.getRow(y);
		outRow=out.getRow(y);
		for(x=0;x<in.width;x++){
			outRow[x]=(inRow[x]<<nbBits);
		}
	}
}

template <typename T>Buffer<T> getLeftBitShift(Buffer<T> in,int nbBits){
	Buffer<T> res(in.width,in.height);
	__leftBitShift__<T>(res,in,nbBits);
	return res;
}

template <typename T>Buffer<T> getRightBitShift(Buffer<T> in,int nbBits){
	Buffer<T> res(in.width,in.height);
	__rightBitShift__<T>(res,in,nbBits);
	return res;
}

template<typename T>void  __rgb2grayscale__(Buffer<T> out,
		std::vector<Buffer<T> > rgb) {
	Buffer<T> R(rgb[0]);
	Buffer<T> G(rgb[1]);
	Buffer<T> B(rgb[2]);
	T* rRow;
	T* gRow;
	T* bRow;
	T* outRow;
	int x, y;
	for (y = 0; y < out.height; y++) {
		rRow = R.getRow(y);
		gRow = G.getRow(y);
		bRow = B.getRow(y);
		outRow = out.getRow(y);
		for (x = 0; x < out.width; x++) {
			outRow[x] = PixelMetrics<T>::roundIfNeeded(
					.3 * rRow[x] + .59 * gRow[x] + .11 * bRow[x]);
		}
	}
}


namespace __hsv__{
template<typename T> T __min3__(T v1, T v2, T v3) {
	if (v1 > v2)
		v1 = v2;
	if (v1 > v3)
		return v3;
	else
		return v1;
}

template<typename T> T __max3__(T v1, T v2, T v3) {
	if (v1 > v2)
		v1 = v2;
	if (v1 > v3)
		return v3;
	else
		return v1;
}


}
template<typename T> void __rgb2hsv__(std::vector<Buffer<T> > out,
		std::vector<Buffer<T> > rgb) {
	//taken from http://www.cs.rit.edu/~ncs/color/t_convert.html
	Buffer<T> R(rgb[0]);
	Buffer<T> G(rgb[1]);
	Buffer<T> B(rgb[2]);
	Buffer<T> H(out[0]);
	Buffer<T> S(out[1]);
	Buffer<T> V(out[2]);
	T* rRow;
	T* gRow;
	T* bRow;
	T* hRow;
	T* sRow;
	T* vRow;
	int x, y;
	double min, max, delta, h, r, g, b;
	for (y = 0; y < out.height; y++) {
		rRow = R.getRow(y);
		gRow = G.getRow(y);
		bRow = B.getRow(y);
		hRow = H.getRow(y);
		sRow = S.getRow(y);
		vRow = V.getRow(y);
		for (x = 0; x < out.width; x++) {
			r = PixelMetrics<T>::normaliseTo01(rRow[x]);
			g = PixelMetrics<T>::normaliseTo01(gRow[x]);
			b = PixelMetrics<T>::normaliseTo01(bRow[x]);
			min = __hsv__::__min3__(r, g, b);
			max = __hsv__::__max3__(r, g, b);
			delta = max - min;
			vRow[x] = max;
			if (max == 0) {
				sRow[x] = 0;
				hRow[x] = 0;
			} else {
				sRow[x] = PixelMetrics<T>::roundIfNeeded(
						PixelMetrics<T>::defaultFg * delta / max);
			}

			if (max == r) {
				//between yellow and magenda
				h = (g - b) / delta;
			} else if (g == max) {
				h = 2.0 + (b - r) / delta;
			} else {
				h = 4.0 + (r - g) / delta;
			}
			if (h < 0) {
				h += 6;
			}
			hRow[x] = PixelMetrics<T>::roundIfNeeded(
					PixelMetrics<T>::defaultFg * h / 6);
		}
	}
}

template<typename T> void __hsv2rgb__(std::vector<Buffer<T> >out,
		const std::vector<Buffer<T> > hsv) {
	//taken from http://www.cs.rit.edu/~ncs/color/t_convert.html
	Buffer<T> R(out[0]);
	Buffer<T> G(out[1]);
	Buffer<T> B(out[2]);
	Buffer<T> H(hsv[0]);
	Buffer<T> S(hsv[1]);
	Buffer<T> V(hsv[2]);
	T* rRow;
	T* gRow;
	T* bRow;
	T* hRow;
	T* sRow;
	T* vRow;
	int x, y;
	double f, p, q, t, h, s, v;
	for (y = 0; y < out.height; y++) {
		rRow = R.getRow(y);
		gRow = G.getRow(y);
		bRow = B.getRow(y);
		hRow = H.getRow(y);
		sRow = S.getRow(y);
		vRow = V.getRow(y);
		for (x = 0; x < out.width; x++) {
			h = PixelMetrics<T>::normaliseTo01(hRow[x]) * 6;
			s = PixelMetrics<T>::normaliseTo01(sRow[x]);
			v = PixelMetrics<T>::normaliseTo01(vRow[x]);
			if (s == 0) {
				rRow[x] = v;
				gRow[x] = v;
				bRow[x] = v;
			} else {
				f = h - std::floor(h);
				p = v * (1 - s);
				q = v * (1 - s * f);
				t = v * (1 - s * (1 - f));
				switch (int(std::floor(h))) {
				case 0:
					rRow[x] = v;
					gRow[x] = t;
					bRow[x] = p;
					break;
				case 1:
					rRow[x] = q;
					gRow[x] = v;
					bRow[x] = p;
					break;
				case 2:
					rRow[x] = p;
					gRow[x] = v;
					bRow[x] = t;
					break;
				case 4:
					rRow[x] = t;
					gRow[x] = p;
					bRow[x] = v;
					break;
				default: //case 5
					rRow[x] = v;
					gRow[x] = p;
					bRow[x] = q;
					break;
				}
			}
		}
	}
}

template <typename T> void __labels2RGB__(std::vector<Buffer<T> >RGBA,Buffer<t_sint32>labels,bool bgColorWhite){

	//bool bgColorWhite = false;
	static std::vector<t_uint32> mappingLabel2ARGB (
			PixelMetrics<t_sint32>::__getLabel2ARGBMapping__(bgColorWhite));
	if(bgColorWhite){
		mappingLabel2ARGB[0]=t_uint32(t_uint32(255)*256*256*256+255*256*256+255*256+255);
	}else{
		mappingLabel2ARGB[0]=0;
	}
	int x,y;
	T* rRow;
	T* gRow;
	T* bRow;
	T* aRow;
	t_sint32* inRow;
	t_uint32  curPixel;
	//struct t_argb{t_uint8 alpha;t_uint8 red;t_uint8 green;t_uint8 blue;};
	struct t_argb{t_uint8 alpha;t_uint8 blue;t_uint8 green;t_uint8 red;};
	t_argb & curPixelRgb=(*((t_argb*)(&curPixel)));
	Buffer<T>R(RGBA[0]);
	Buffer<T>G(RGBA[1]);
	Buffer<T>B(RGBA[2]);
	Buffer<T>A(RGBA[3]);
	for(y=0;y<labels.height;y++){
		aRow=A.getRow(y);
		rRow=R.getRow(y);
		gRow=G.getRow(y);
		bRow=B.getRow(y);
		inRow=labels.getRow(y);
		for(x=0;x<labels.width;x++){
			curPixel=mappingLabel2ARGB[inRow[x]%(1<<24)];
			aRow[x]=curPixelRgb.alpha;
			rRow[x]=curPixelRgb.red;
			gRow[x]=curPixelRgb.green;
			bRow[x]=curPixelRgb.blue;
		}
	}
}

inline void __sint32ToRGBA__(std::vector<Buffer<t_uint8> >out,Buffer<t_sint32> inLabels){
	if(out.size()!=4){
		throw "__sint32ToRGBA__ expecting a vector of 4 images";
	}
	Buffer<t_uint8> R(out[0]);
	Buffer<t_uint8> G(out[1]);
	Buffer<t_uint8> B(out[2]);
	Buffer<t_uint8> A(out[3]);
	t_sz x,y;
	t_sint32* inRow;
	t_uint8* rRow;
	t_uint8* gRow;
	t_uint8* bRow;
	t_uint8* aRow;
	t_uint32 curPixel;
	for(y=0;y<inLabels.height;y++){
		inRow=inLabels.getRow(y);
		rRow=R.getRow(y);
		gRow=G.getRow(y);
		bRow=B.getRow(y);
		aRow=A.getRow(y);
		for(x=0;x<inLabels.width;x++){
			curPixel=inRow[x];
			bRow[x]=curPixel%256;
			gRow[x]=(curPixel/256)%256;
			rRow[x]=(curPixel/65536)%256;
			aRow[x]=(curPixel/16777216)%256;
		}
	}
}

template <typename T> void __RGB2Labels__(Buffer<t_sint32>out,std::vector<Buffer<T> >rgb){
	bool bgColorWhite = false;
	static std::vector<t_sint32> mappingARGB2Label = PixelMetrics<t_sint32>::__getARGB2LabelMapping__();
	int x,y;
	T* rRow;
	T* gRow;
	T* bRow;
	t_sint32* outRow;
	t_sint32  curPixel;
	struct t_argb{t_uint8 alpha;t_uint8 red;t_uint8 green;t_uint8 blue;};
	t_argb & curPixelArgb=(*((t_argb*)(&curPixel)));
	Buffer<T>R(rgb[0]);
	Buffer<T>G(rgb[1]);
	Buffer<T>B(rgb[2]);
	for(y=0;y<out.height;y++){
		rRow=R.getRow(y);
		gRow=G.getRow(y);
		bRow=R.getRow(y);
		outRow=out.getRow(y);
		for(x=0;x<out.width;x++){
			curPixelArgb.alpha=255;
			curPixelArgb.red=rRow[x];
			curPixelArgb.green=gRow[x];
			curPixelArgb.blue=bRow[x];
			outRow[x]=curPixel;
		}
	}
}

template <typename T> Buffer<T> getRgb2Grayscale(std::vector<Buffer<T> > rgb){
	Buffer<T> res(rgb[0].width,rgb[0].height);
	__rgb2grayscale__(res,rgb);
	return res;
}

template <typename T> std::vector<Buffer<T> > getHSV2RGB(std::vector<Buffer<T> > hsv){
	Buffer<T> R(hsv[0].width,hsv[0].height);
	Buffer<T> G(hsv[0].width,hsv[0].height);
	Buffer<T> B(hsv[0].width,hsv[0].height);
	__rgb2hsv__({R,G,B},hsv);
	return {R,G,B};
}


template <typename T> std::vector<Buffer<T> > getRGB2HSV(std::vector<Buffer<T> > rgb){
	Buffer<T> H(rgb[0].width,rgb[0].height);
	Buffer<T> S(rgb[0].width,rgb[0].height);
	Buffer<T> V(rgb[0].width,rgb[0].height);
	__rgb2hsv__({H,S,V},rgb);
	return {H,S,V};
}



template <typename T> Buffer<t_sint32> getRgb2Labels(std::vector<Buffer<T> >rgb){
	Buffer<t_sint32>out(rgb[0].width,rgb[0].height);
	__RGB2Labels__(out,rgb);
	return out;
}

template <typename T> std::vector<Buffer<T> > getLabels2RGB(Buffer<t_sint32> labels ){
	Buffer<T> R(labels.width,labels.height);
	Buffer<T> G(labels.width,labels.height);
	Buffer<T> B(labels.width,labels.height);
	__labels2RGB__<T>({R,G,B},labels);
	return {R,G,B};
}


}
}
}

#endif /* COLORSPACE_HPP_ */
