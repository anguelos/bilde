/*
 * buffer_container_libtiff.hpp
 *
 *  Created on: Feb 12, 2013
 *      Author: anguelos
 */

#ifndef BUFFER_CONTAINER_LIBTIFF_HPP_
#define BUFFER_CONTAINER_LIBTIFF_HPP_

//#include <boost/algorithm/string.hpp>

//Derived from a tutorial  on 41J blog titled "libtiff RGB image to greyscale + loading and saving"
//http://41j.com/blog/2011/10/libtiff-rgb-image-to-greyscale-loading-and-saving/


#include <string>
#include <list>
#include <cmath>
#include <vector>

namespace bilde {
namespace io {
namespace tiff {

template<typename T> std::vector<Buffer<T> > loadTiffColor(std::string filename);

template<> std::vector<Buffer<t_uint8> > loadTiffColor<t_uint8>(
		std::string filename) {
	TIFF* tif = TIFFOpen(filename.c_str(), "r");
	if (tif) {
		t_uint32 photometric;
		t_uint32 w, h, x, y;
		t_uint64 npixels;
		t_uint32* raster;
		t_uint8* outR;
		t_uint8* outG;
		t_uint8* outB;
		t_uint32* inLine;
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);

		Buffer<t_uint8> R(w, h);
		Buffer<t_uint8> G(w, h);
		Buffer<t_uint8> B(w, h);

		std::vector<Buffer<t_uint8> > res = { R, G, B };
		npixels = w * h;
		raster = (uint32*) _TIFFmalloc(npixels * sizeof(uint32));
		if (raster != NULL) {
			if (TIFFReadRGBAImageOriented(tif, w, h, raster,
					ORIENTATION_TOPLEFT, 0)) {
				if (TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric) == 0) {
					throw "loadTiffFile<t_uint8> error :Image has an undefined photometric interpretation";
				}
				if (photometric == PHOTOMETRIC_MINISWHITE) {
					for (y = 0; y < h; y++) {
						outR = R.getRow(y);
						outG = G.getRow(y);
						outB = B.getRow(y);
						inLine = raster + (y * w);
						for (x = 0; x < w; x++) {
							outR[x] = 255 - ((inLine[x] / 65536) % 256);
							outG[x] = 255 - ((inLine[x] / 256) % 256);
							outB[x] = 255 - (inLine[x] % 256);
						}
					}
				} else {
					for (y = 0; y < h; y++) {
						outR = R.getRow(y);
						outG = G.getRow(y);
						outB = B.getRow(y);
						inLine = raster + (y * w);
						for (x = 0; x < w; x++) {
							outR[x] = ((inLine[x] / 65536) % 256);
							outG[x] = ((inLine[x] / 256) % 256);
							outB[x] = (inLine[x] % 256);
						}
					}
				}
			}
			_TIFFfree(raster);
		}
		TIFFClose(tif);
		return res;
	} else {
		throw "loadTiffColor<t_uint8>: could not open TIFF file";
	}
}

template<typename T> bool saveTiffColor(std::vector<Buffer<T> > RGB,
		std::string filename);

template<> bool saveTiffColor<t_uint8>(std::vector<Buffer<t_uint8> > RGB,
		std::string filename) {
	if (RGB.size() != 3) {
		throw "saveTiffColor expect a list of 3 images";
	}
	Buffer<t_uint8> R(RGB[0]);
	Buffer<t_uint8> G(RGB[1]);
	Buffer<t_uint8> B(RGB[2]);
	TIFF *output_image;
	int x, y;
	t_sint32* inRow;
	if ((output_image = TIFFOpen(filename.c_str(), "w")) == NULL) {
		throw "saveTiffFile<t_sint32>: could not open file for wtiting.";
	}
	TIFFSetField(output_image, TIFFTAG_IMAGEWIDTH, R.width);
	TIFFSetField(output_image, TIFFTAG_IMAGELENGTH, R.height);
	TIFFSetField(output_image, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(output_image, TIFFTAG_SAMPLESPERPIXEL, 3);
	TIFFSetField(output_image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(output_image, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	TIFFSetField(output_image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	//TIFFSetField(output_image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	//Buffers are not guarantied to be continus, moving data to a vector
	std::vector<t_uint8> data(R.width * R.height * 3);
	t_uint8 *rRow;
	t_uint8 *gRow;
	t_uint8 *bRow;
	for (y = 0; y < R.height; y++) {
		rRow = R.getRow(y);
		gRow = G.getRow(y);
		bRow = B.getRow(y);
		for (x = 0; x < R.width; x++) {
			data[x * 3 + 3 * y * R.width] = rRow[x];
			data[x * 3 + 3 * y * R.width + 1] = gRow[x];
			data[x * 3 + 3 * y * R.width + 2] = bRow[x];
		}
	}

	// Write the information to the file
	TIFFWriteEncodedStrip(output_image, 0, &data[0], R.width * R.height * 3);

	// Close the file
	TIFFClose(output_image);
	return true;
}

template<typename T> bool saveTiffGrayscale(Buffer<T> img,
		std::string filename);

template<> bool saveTiffGrayscale<t_uint8>(Buffer<t_uint8> img,
		std::string filename) {
	TIFF *output_image;
	int x, y;
	t_uint8* inRow;
	if ((output_image = TIFFOpen(filename.c_str(), "w")) == NULL) {
		throw "saveTiffFile<t_uint8>: could not open file for wtiting.";
	}

	TIFFSetField(output_image, TIFFTAG_IMAGEWIDTH, img.width);
	TIFFSetField(output_image, TIFFTAG_IMAGELENGTH, img.height);
	TIFFSetField(output_image, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(output_image, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(output_image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(output_image, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	TIFFSetField(output_image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

	//Buffers are not guarantied to be continus, moving data to a vector
	std::vector<t_uint8> data(img.width * img.height);

	for (y = 0; y < img.height; y++) {
		inRow = img.getRow(y);
		for (x = 0; x < img.width; x++) {
			data[y * img.width + x] = inRow[x];
		}
	}

	// Write the information to the file
	TIFFWriteEncodedStrip(output_image, 0, &data[0], img.width * img.height);

	// Close the file
	TIFFClose(output_image);
	return true;
}


template<> bool saveTiffGrayscale<t_real64>(Buffer<t_real64> img,
		std::string filename) {
//Bad implementation should use TIFFs native image;
	TIFF *output_image;
	int x, y;
	t_real64* inRow;
	if ((output_image = TIFFOpen(filename.c_str(), "w")) == NULL) {
		throw "saveTiffFile<t_uint8>: could not open file for wtiting.";
	}

	TIFFSetField(output_image, TIFFTAG_IMAGEWIDTH, img.width);
	TIFFSetField(output_image, TIFFTAG_IMAGELENGTH, img.height);
	TIFFSetField(output_image, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(output_image, TIFFTAG_SAMPLESPERPIXEL, 1);
	TIFFSetField(output_image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(output_image, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	TIFFSetField(output_image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

	//Buffers are not guarantied to be continus, moving data to a vector
	std::vector<t_uint8> data(img.width * img.height);

	for (y = 0; y < img.height; y++) {
		inRow = img.getRow(y);
		for (x = 0; x < img.width; x++) {
			data[y * img.width + x] = (inRow[x]>0)?(inRow[x]<1?round(inRow[x]*255):255):0;
		}
	}

	// Write the information to the file
	TIFFWriteEncodedStrip(output_image, 0, &data[0], img.width * img.height);

	// Close the file
	TIFFClose(output_image);
	return true;
}


/*
inline bool saveTiffLabelsVisible(Buffer<t_sint32> img, std::string filename) {
	bool bgColorWhite = false;
	static std::vector<t_uint32> mappingLabel2ARGB =
			PixelMetrics<t_sint32>::__getLabel2ARGBMapping__(bgColorWhite);
	TIFF *output_image;
	int x, y;
	t_sint32* inRow;
	if ((output_image = TIFFOpen(filename.c_str(), "w")) == NULL) {
		throw "saveTiffFile<t_sint32>: could not open file for wtiting.";
	}
	TIFFSetField(output_image, TIFFTAG_IMAGEWIDTH, img.width);
	TIFFSetField(output_image, TIFFTAG_IMAGELENGTH, img.height);
	TIFFSetField(output_image, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(output_image, TIFFTAG_SAMPLESPERPIXEL, 3);
	TIFFSetField(output_image, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(output_image, TIFFTAG_COMPRESSION, COMPRESSION_DEFLATE);
	TIFFSetField(output_image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	//TIFFSetField(output_image, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
	//Buffers are not guarantied to be continus, moving data to a vector
	std::vector<t_uint8> data(img.width * img.height * 3);
	t_uint32 curVal;
	t_uint8* curARGB = ((t_uint8*) &curVal);
	t_uint8* curRGB = curARGB + 1;
	for (y = 0; y < img.height; y++) {
		inRow = img.getRow(y);
		for (x = 0; x < img.width; x++) {
			curVal = mappingLabel2ARGB[inRow[x]];
			data[x * 3 + 3 * y * img.width] = curARGB[0];
			data[x * 3 + 3 * y * img.width + 1] = curARGB[1];
			data[x * 3 + 3 * y * img.width + 2] = curARGB[2];
		}
	}

	// Write the information to the file
	TIFFWriteEncodedStrip(output_image, 0, &data[0],
			img.width * img.height * 3);

	// Close the file
	TIFFClose(output_image);
	return true;
}
*/

template<typename T> Buffer<T> loadTiffGrayscale(std::string filename);

template<> Buffer<t_uint8> loadTiffGrayscale<t_uint8>(std::string filename) {
	TIFF* tif = TIFFOpen(filename.c_str(), "r");
	if (tif) {
		t_uint32 photometric;
		t_uint32 w, h, x, y;
		t_uint64 npixels;
		t_uint32* raster;
		t_uint8* outLine;
		t_uint32* inLine;
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		Buffer<t_uint8> res(w, h);
		npixels = w * h;
		raster = (uint32*) _TIFFmalloc(npixels * sizeof(uint32));
		if (raster != NULL) {
			if (TIFFReadRGBAImageOriented(tif, w, h, raster,
					ORIENTATION_TOPLEFT, 0)) {
				if (TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric) == 0) {
					throw "loadTiffFile<t_uint8> error :Image has an undefined photometric interpretation";
				}
				if (photometric == PHOTOMETRIC_MINISWHITE) {
					for (y = 0; y < h; y++) {
						outLine = res.getRow(y);
						inLine = raster + (y * w);
						for (x = 0; x < w; x++) {
							outLine[x] =
									255
											- round(
													(inLine[x] % 256) * .11
															+ ((inLine[x] / 256)
																	% 256) * .59
															+ ((inLine[x]
																	/ 65536)
																	% 256)
																	* .3);
						}
					}
				} else {
					for (y = 0; y < h; y++) {
						outLine = res.getRow(y);
						inLine = raster + (y * w);
						for (x = 0; x < w; x++) {
							outLine[x] = round(
									(inLine[x] % 256) * .11
											+ ((inLine[x] / 256) % 256) * .59
											+ ((inLine[x] / 65536) % 256) * .3);
						}
					}
				}
			}
			_TIFFfree(raster);
		}
		TIFFClose(tif);
		return res;
	} else {
		throw "loadTiffFile<t_uint8>: could not open TIFF file";
	}
}


template<typename T> std::list<Buffer<T> > loadRGBATiff(std::string filename);

template<> std::list<Buffer<t_uint8> > loadRGBATiff<t_uint8>(
		std::string filename) {
	std::list<Buffer<t_uint8> > res;
	TIFF* tif = TIFFOpen(filename.c_str(), "r");
	if (tif) {
		t_uint32 photometric;
		t_uint32 w, h, x, y;
		t_uint64 npixels;
		t_uint32* raster;
		t_uint8* outLineA, *outLineR, *outLineG, *outLineB;
		t_uint32* inLine;
		TIFFGetField(tif, TIFFTAG_IMAGEWIDTH, &w);
		TIFFGetField(tif, TIFFTAG_IMAGELENGTH, &h);
		Buffer<t_uint8> A(w, h);
		Buffer<t_uint8> R(w, h);
		Buffer<t_uint8> G(w, h);
		Buffer<t_uint8> B(w, h);
		res.push_back(A);
		res.push_back(R);
		res.push_back(G);
		res.push_back(B);
		npixels = w * h;
		raster = (uint32*) _TIFFmalloc(npixels * sizeof(uint32));
		if (raster != NULL) {
			if (TIFFReadRGBAImageOriented(tif, w, h, raster,
					ORIENTATION_TOPLEFT, 0)) {
				if (TIFFGetField(tif, TIFFTAG_PHOTOMETRIC, &photometric) == 0) {
					throw "loadTiffFile<t_uint8> error :Image has an undefined photometric interpretation";
				}
				if (photometric == PHOTOMETRIC_MINISWHITE) {
					for (y = 0; y < h; y++) {
						outLineA = A.getRow(y);
						outLineR = R.getRow(y);
						outLineG = G.getRow(y);
						outLineB = B.getRow(y);
						inLine = raster + (y * w);
						for (x = 0; x < w; x++) {
							outLineR[x] = 255 - ((inLine[x] / (1 << 24)) % 256);
							outLineG[x] = 255 - ((inLine[x] / (1 << 16)) % 256);
							outLineB[x] = 255 - ((inLine[x] / (1 << 8)) % 256);
							outLineA[x] = ((inLine[x]) % 256);
						}
					}
				} else {
					for (y = 0; y < h; y++) {
						outLineA = A.getRow(y);
						outLineR = R.getRow(y);
						outLineG = G.getRow(y);
						outLineB = B.getRow(y);
						inLine = raster + (y * w);
						for (x = 0; x < w; x++) {
							outLineR[x] = ((inLine[x] / (1 << 24)) % 256);
							outLineG[x] = ((inLine[x] / (1 << 16)) % 256);
							outLineB[x] = ((inLine[x] / (1 << 8)) % 256);
							outLineA[x] = ((inLine[x]) % 256);
						}
					}
				}
			}
			_TIFFfree(raster);
		}
		TIFFClose(tif);
		return res;
	} else {
		throw "loadTiffFile<t_uint8>: could not open TIFF file";
	}
}

}
}
}


#endif /* BUFFER_CONTAINER_LIBTIFF_HPP_ */
