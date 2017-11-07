/*
 * iplimage_container.hpp
 *
 *  Created on: Mar 5, 2012
 *      Author: anguelos
 */

#ifndef CVMAT_CONTAINER_HPP_
#define CVMAT_CONTAINER_HPP_

#define __OPENCV_CORE_HPP__
#include <opencv/cv.h>

#ifdef  __OPENCV_CORE_HPP__




namespace bilde {
namespace container {

namespace __cvmat_util__ {

template<typename B> int c2cvDepth() {
	return -1;
}

template<> int c2cvDepth<t_uint8>() {
	return CV_8UC1;
}

template<> int c2cvDepth<t_sint8>() {
	return CV_8SC1;
}

template<> int c2cvDepth<t_uint16>() {
	return CV_16U;
}

template<> int c2cvDepth<t_sint16>() {
	return CV_16S;
}

template<> int c2cvDepth<t_sint32>() {
	return CV_32S;
}

template<> int c2cvDepth<t_real32>() {
	return CV_32F;
}

template<> int c2cvDepth<t_real64>() {
	return CV_64F;
}

}

template<typename PIXELT> struct ContainerWrapper<cv::Mat, PIXELT> {
//Wrapper example.
//This wrapper is used to "wrap" a Buffer in an other Buffer
//AKA used for copy constructors

//BASIC API CLASS METHODS
	static bool isDefined() {
		int depth = __cvmat_util__::c2cvDepth<PIXELT>();
		return depth != -1;
	}

//BASIC API INSTANCE METHODS
	static bool canWrap(const cv::Mat& cont) {
		return (__cvmat_util__::c2cvDepth<PIXELT>() == cont.depth())
				&& (cont.channels() == 1);
	}
	static bool canTakeDataOwnership(const cv::Mat& cont) {
		return false;
	}
	static boost::shared_ptr<t_uint8> takeStorageOwnership(
			const cv::Mat& cont) {
		return boost::shared_ptr<t_uint8>();
	}
	static boost::shared_ptr<t_uint8> deepCopyStorage(const cv::Mat& cont) {
		//its a lot simpler to deep copy a buffer to a buffer than an ipl image to a buffer
		if (!canWrap(cont)) {
			throw "deepCopyStorage for cv::Mat only deepcopies matrices with 1 channel";
		}
		Buffer<PIXELT> tmp(cont);
		//dont forget to set Buffer.data to point to the apropriate place
		t_uint8* outdata = new t_uint8[tmp.height * tmp.__linestride__
				* sizeof(PIXELT)];
		for (int y = 0; y < tmp.height; y++) {
			std::memcpy(outdata + tmp.__linestride__ * y, tmp.getRow(y),
					tmp.width * sizeof(PIXELT));
		}
		return boost::shared_ptr < t_uint8 > (outdata);
	}

	static const PIXELT* getFirstRow(const cv::Mat&cont) {
		return (PIXELT*) (cont.ptr(0));
	}
	static PIXELT* getFirstRow(cv::Mat&cont) {
		return (PIXELT*) (cont.ptr(0));
	}
	static t_sz getWidth(const cv::Mat& cont) {
		return cont.cols;
	}
	static t_sz getHeight(const cv::Mat&cont) {
		return cont.rows;
	}
	static t_sz getByteLinestride(const cv::Mat&cont) {
		return ((t_uint8*) (cont.ptr(1))) - ((t_uint8*) (cont.ptr(0)));
	}
};

}

inline cv::Mat label2CvCollor(const cv::Mat& labeled, bool blackBg) {
	cv::Mat res = cv::Mat(labeled.rows, labeled.cols, CV_8UC(3));
	t_sint32* inRow;
	t_uint8* outRed;
	t_uint8* outGreen;
	t_uint8* outBlue;
	int x, y, curPixel;
	long int count = 0;
	if (blackBg) {
		for (int y = 0; y < labeled.rows; y++) {
			inRow = (t_sint32*) (labeled.ptr(y));
			//native opencv is BGR
			outRed = res.ptr(y) + 2;
			outGreen = res.ptr(y) + 1;
			outBlue = res.ptr(y);
			for (x = 0; x < res.cols * 3; x += 3) {
				curPixel = inRow[x / 3];
				outRed[x] = (128 * ((curPixel % 2)) + 64 * ((curPixel % 16) / 8)
						+ 32 * ((curPixel % 128) / 64) + 31 * (curPixel != 0));
				outGreen[x] = (128 * ((curPixel % 4) / 2)
						+ 64 * ((curPixel % 32) / 16)
						+ 32 * ((curPixel % 256) / 128) + 31 * (curPixel != 0));
				outBlue[x] = (128 * ((curPixel % 8) / 4)
						+ 64 * ((curPixel % 64) / 32)
						+ 32 * ((curPixel % 512) / 256) + 31 * (curPixel != 0));
			}
		}
	} else {
		for (int y = 0; y < labeled.rows; y++) {
			inRow = (t_sint32*) (labeled.ptr(y));
			//native opencv is BGR
			outRed = res.ptr(y) + 2;
			outGreen = res.ptr(y) + 1;
			outBlue = res.ptr(y);
			for (x = 0; x < res.cols * 3; x += 3) {
				curPixel = inRow[x / 3];
				outRed[x] = 255
						- (128 * ((curPixel % 2)) + 64 * ((curPixel % 16) / 8)
								+ 32 * ((curPixel % 128) / 64)
								+ 31 * (curPixel != 0));
				outGreen[x] = 255
						- (128 * ((curPixel % 4) / 2)
								+ 64 * ((curPixel % 32) / 16)
								+ 32 * ((curPixel % 256) / 128)
								+ 31 * (curPixel != 0));
				outBlue[x] = 255
						- (128 * ((curPixel % 8) / 4)
								+ 64 * ((curPixel % 64) / 32)
								+ 32 * ((curPixel % 512) / 256)
								+ 31 * (curPixel != 0));
			}
		}
	}

	return res;
}

}


#endif /* __OPENCV_CORE_HPP__ */


#endif /* IPLIMAGE_CONTAINER_HPP_ */
