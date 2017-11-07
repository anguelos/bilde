/*
 * connected_component_labeling.hpp
 *
 *  Created on: Mar 30, 2012
 *      Author: anguelos
 */

#ifndef CONNECTED_COMPONENT_LABELING_HPP_
#define CONNECTED_COMPONENT_LABELING_HPP_

namespace bilde {
namespace operations {
namespace components {

namespace __bwlabel__ {

struct __FastEquivalenceSet__ {
	static const t_sint32 DEFAULT_MAX_LABELS =
			PixelMetrics<t_sint32>::maxValidLabel; //(2^24)*2 ~= 32 MPIXELS for 4 neibors, 64 MPIXELS for 8 neibors
	const t_sint32 max_tmp_labels;
	t_sint32* const labelMap;
	void** const labelMapRef;
	t_sint32 nbLabels;
	t_sint32 nbFinalLabels;
	__FastEquivalenceSet__(t_sint32 maxlab = 0) :
			max_tmp_labels((maxlab > 0) ? maxlab : DEFAULT_MAX_LABELS), labelMap(
					new t_sint32[maxlab]), labelMapRef(new void*[maxlab]), nbLabels(
					1) {
		labelMap[0] = 0;
		labelMapRef[0] = &(labelMapRef[0]);
		nbFinalLabels=0;//just so that there will be no warning;
	}
	t_sint32 getNewCode() {
		labelMapRef[nbLabels] = &labelMapRef[nbLabels];
		nbLabels++;
		return nbLabels - 1;
	}
	t_sint32 operator[](t_sint32 lab) {
		return labelMap[lab];
	}
	void setEquivalent(t_sint32 lab1, t_sint32 lab2) {
		if (labelMapRef[lab1] != labelMapRef[lab2]) {
			void* iter1 = &labelMapRef[lab1];
			void* iter2 = &labelMapRef[lab2];
			while (iter1 != (*((void**) iter1))) {
				iter1 = (*((void**) (iter1)));
			}
			while (iter2 != (*((void**) iter2))) {
				iter2 = (*((void**) (iter2)));
			};
			*((void**) iter2) = iter1;
		}
	}
	void setEquivalent(t_sint32 lab1, t_sint32 lab2, t_sint32 lab3) {
		setEquivalent(lab1, lab2);
		setEquivalent(lab1, lab3);
	}
	void setEquivalent(t_sint32 lab1, t_sint32 lab2, t_sint32 lab3,
			t_sint32 lab4) {
		setEquivalent(lab1, lab2);
		setEquivalent(lab3, lab4);
		setEquivalent(lab1, lab3);
	}
	void finalize() {
		t_sint32 countFound = 0;
		t_sint32 pos, final_label;
		void* next;
		for (pos = 1; pos < nbLabels; pos++) {
			next = &labelMapRef[pos];
			final_label = (next == (*((void**) next)));
			while (next != (*((void**) next))) {
				next = (*((void**) (next)));
			}
			if (final_label) {
				labelMap[pos] = 0;
			} else {
				labelMap[pos] = ((((void**) (next))
						- ((void**) (labelMapRef/*[0]*/))));
			}
		}
		for (pos = 1; pos < nbLabels; pos++) {
			if (labelMap[pos] == 0) {
				countFound++;
				labelMap[pos] = -countFound;
			}
		}
		for (pos = 1; pos < nbLabels; pos++) {
			if (labelMap[pos] < 0) {
				labelMap[pos] = -labelMap[pos];
			} else {
				labelMap[pos] = labelMap[labelMap[pos]];
				if (labelMap[pos] < 0) {
					labelMap[pos] = -labelMap[pos];
				}
			}
		}
		labelMap[0] = 0;
		nbFinalLabels = countFound + 1;
	}
	~__FastEquivalenceSet__() {
		delete[] labelMap;
		delete[] labelMapRef;
	}
};

}

template<typename T> int __labelConnectedComponents__(Buffer<t_label> out,
		Buffer<T> in, const int neighbors_type) {
	if (neighbors_type != 8 && neighbors_type != 4) {
		throw "__labelConnectedComponents__ : neighborhood must be either 8 or 4";
	}
#define as_bool(val) (val!=0)
	t_label *out_CC = out[0]; //pointer_to_the_current_labeled_pixel
	const int out_linestride = out[1] - out[0];

	const t_label *out_NW = out[-1] - 1;
	const t_label *out_NC = out[-1];
	const t_label *out_NE = out[-1] + 1;
	const t_label *out_CW = out[0] - 1;
	const T *in_CC = in[0];
	const T *in_NW = in[-1] - 1;
	const T *in_NC = in[-1];
	const T *in_NE = in[-1] + 1;
	const T *in_CW = in[0] - 1;
	__bwlabel__:: __FastEquivalenceSet__ equiv((out.width * out.height) / 2);
	//int out_pixel_pos = 0;
	int x;
	int line_last_pixel;
	int width = out.width;
	int height = out.height;
	int y = 0;
	if (neighbors_type == 8) {
		if (as_bool(in_CC[0])) {
			out_CC[0] = equiv.getNewCode();
		} else {
			out_CC[0] = 0;
		}
		for (x = 1; x < width; x++) {
			if (as_bool(in_CC[x])) {
				if (as_bool(in_CW[x])) {
					out_CC[x] = out_CW[x];
				} else {
					out_CC[x] = equiv.getNewCode();
				}
			} else {
				out_CC[x] = 0;
			}
		}
		for (y = 1; y < height; y++) {
			out_NW = out[y - 1] - 1;
			out_NC = out[y - 1];
			out_NE = out[y - 1] + 1;
			out_CW = out[y] - 1;
			out_CC = out[y];
			in_CC = in[y];
			in_NW = in[y - 1] - 1;
			in_NC = in[y - 1];
			in_NE = in[y - 1] + 1;
			in_CW = in[y] - 1;
			x = 0;
			line_last_pixel = width - 1;
			if (as_bool(in_CC[x])) { //first pixel off row
				if (as_bool(in_NC[x])) {
					out_CC[x] = out_NC[x];
				} else {
					out_CC[x] = equiv.getNewCode();
				}
			} else {
				out_CC[x] = 0;
			}
			x++;
			while (x < line_last_pixel) {
				if (in_CC[x]) {
					switch (as_bool(in_NW[x]) * 8 + as_bool(in_NC[x]) * 4
							+ as_bool(in_NE[x]) * 2 + as_bool(in_CW[x])) {
					case 15:
						out_CC[x] = out_CW[x];
						break;
					case 0:
						out_CC[x] = equiv.getNewCode();
						break;
					case 1:
						out_CC[x] = out_CW[x];
						break;
					case 2:
						out_CC[x] = out_NE[x];
						break;
					case 4:
						out_CC[x] = out_NC[x];
						break;
					case 8:
						out_CC[x] = out_NW[x];
						break;
					case 3: //CW+NE
						out_CC[x] = out_CW[x];
						equiv.setEquivalent(out_CW[x], out_NE[x]);
						break;
					case 5: //CW+NC
						out_CC[x] = out_CW[x];
						equiv.setEquivalent(out_CW[x], out_NC[x]);
						break;
					case 9: //CW+NW
						out_CC[x] = out_CW[x];
						equiv.setEquivalent(out_CW[x], out_NW[x]);
						break;
					case 6:
						out_CC[x] = out_NC[x];
						equiv.setEquivalent(out_NE[x], out_NC[x]);
						break;
					case 10:
						out_CC[x] = out_NW[x];
						equiv.setEquivalent(out_NW[x], out_NE[x]);
						break;
					case 12:
						out_CC[x] = out_NW[x];
						equiv.setEquivalent(out_NW[x], out_NC[x]);
						break;
						//three neighbors found
					case 7:
						out_CC[x] = out_CW[x];
						equiv.setEquivalent(out_CW[x], out_NE[x], out_NC[x]);
						break;
					case 11:
						out_CC[x] = out_CW[x];
						equiv.setEquivalent(out_CW[x], out_NE[x], out_NW[x]);
						break;
					case 13:
						out_CC[x] = out_CW[x];
						equiv.setEquivalent(out_NW[x], out_CW[x], out_NC[x]);
						break;
					case 14:
						out_CC[x] = out_NW[x];
						equiv.setEquivalent(out_NC[x], out_NE[x], out_NW[x]);
						break;
					}
				} else {
					out_CC[x] = 0;
				}
				x++;
			}
			if (in_CC[x]) {
				switch (as_bool(in_NW[x]) * 8 + as_bool(in_NC[x]) * 4
						+ as_bool(in_CW[x])) {
				case 0:
					out_CC[x] = equiv.getNewCode();
					break;
					//all four neighbors
				case 1:
					out_CC[x] = out_CW[x];
					break;
				case 4:
					out_CC[x] = out_NC[x];
					break;
				case 8:
					out_CC[x] = out_NW[x];
					break;
					//two neighbors found
				case 5:
					out_CC[x] = out_CW[x];
					equiv.setEquivalent(out_CW[x], out_NC[x]);
					break;
				case 9:
					out_CC[x] = out_CW[x];
					equiv.setEquivalent(out_CW[x], out_NW[x]);
					break;
				case 12:
					out_CC[x] = out_NW[x];
					equiv.setEquivalent(out_NW[x], out_NC[x]);
					break;
					//three neighbors found
				case 13:
					out_CC[x] = out_CW[x];
					equiv.setEquivalent(out_NW[x], out_CW[x], out_NC[x]);
					break;
				}
			} else {
				out_CC[x] = 0;
			}
		}
	}
	if (neighbors_type == 4) {
		x = 0;
		if (as_bool(in_CC[x])) {
			out_CC[x] = equiv.getNewCode();
		} else {
			out_CC[x] = 0;
		}
		for (x = 1; x < width; x++) {
			if (as_bool(in_CC[x])) {
				if (as_bool(in_CW[x])) {
					out_CC[x] = out_CW[x];
				} else {

					out_CC[x] = equiv.getNewCode();
				}
			} else {
				out_CC[x] = 0;
			}
		}
		for (y = 1; y < height; y++) {

			out_NC = out[y - 1];
			out_CW = out[y] - 1;
			out_CC = out[y];
			in_CC = in[y];
			in_NC = in[y - 1];
			in_CW = in[y] - 1;
			x = 0;
			line_last_pixel = x + width - 1;
			if (as_bool(in_CC[x])) { //first pixel off row

				if (as_bool(in_NC[x])) {
					out_CC[x] = out_NC[x];
				} else {
					out_CC[x] = equiv.getNewCode();
				}
			} else {
				out_CC[x] = 0;
			}
			x++;
			while (x <= line_last_pixel) {
				if (in_CC[x]) {
					switch (+as_bool(in_NC[x]) * 2 + as_bool(in_CW[x])) {
					case 3:
						out_CC[x] = out_CW[x];
						equiv.setEquivalent(out_CW[x], out_NC[x]);
						break;
						//no neighbors found preveusly
					case 0:
						out_CC[x] = equiv.getNewCode();
						break;
					case 1:
						out_CC[x] = out_CW[x];
						break;
					case 2:
						out_CC[x] = out_NC[x];
						break;
					}
				} else {
					out_CC[x] = 0;
				}
				x++;
			}
		}
	}
	equiv.finalize();
	int it, it_end;
	t_label* out_fp = out[0];
	for (int y = 0; y < height; y++) {
		it = out_linestride * y;
		it_end = it + out.width;
		while (it < it_end) {
			out_fp[it] = equiv[out_fp[it]];
			it++;
		}
	}
	return equiv.nbFinalLabels;
#undef as_bool
}

template<typename T> T __getMaxLabel__(Buffer<T> labImg) {
	T maxFound = 0;
	T* curRow;
	T* curRowEnd;
	t_sz y;
	for (y = 0; y < labImg.height; y++) {
		curRow = labImg.getRow(y);
		curRowEnd = curRow + labImg.width;
		for (; curRow != curRowEnd; curRow++) {
			maxFound = maxFound > *curRow ? maxFound : *curRow;
		}
	}
	return maxFound;
}

template<typename T> T __remapLabels__(Buffer<T> out, Buffer<T> in) {
	typedef typename std::map<T, T> t_lmap;
	t_lmap labelMap;
	T* curRow;
	T* curRowIn;
	T* curRowOut;
	T* curRowEnd;
	t_sz y, x;
	for (y = 0; y < in.height; y++) {
		curRow = in.getRow(y);
		curRowEnd = curRow + in.width;
		for (; curRow != curRowEnd; curRow++) {
			labelMap[*curRow]++;
		}
	}
	T counter = 0;
	typedef typename t_lmap::iterator liter;
	liter iter;
	for (iter = labelMap.begin(); iter != labelMap.end(); ++iter) {
		iter.second = counter;
		counter++;
	}
	for (y = 0; y < in.height; y++) {
		curRowOut = out.getRow(y);
		curRowIn = in.getRow(y);
		for (x = 0; x < in.width; x++) {
			curRowOut[x] = labelMap[curRowIn[x]];
		}
	}
	return counter;
}

///Struct that hosts the column positions of labeled
///component features
struct LabeledComponentsFeatures {
	static const int LC_LABEL_POS = 0;
	static const int LC_NBPIXELS_POS = 1;
	static const int LC_LEFT_POS = 2;
	static const int LC_RIGHT_POS = 3;
	static const int LC_TOP_POS = 4;
	static const int LC_BOTTOM_POS = 5;
	static const int LC_SUMX_POS = 6;
	static const int LC_SUMY_POS = 7;
	static const int LC_LASTX_POS = 8;
	static const int LC_LASTY_POS = 9;
	static std::vector<std::string> getLabelComponentsFeatureNames() {
		return {"Label","NbPixels","Left","Right","Top","Bottom","SumX","SumY","LastX","LastY"};
	}
	const t_real32 label;
	const t_real32 nbPixels;
	const t_real32 left;
	const t_real32 right;
	const t_real32 top;
	const t_real32 bottom;
	const t_real32 sumX;
	const t_real32 sumY;
	const t_real32 lastX;
	const t_real32 lastY;
	LabeledComponentsFeatures(t_real32*v) :
			label(v[0]), nbPixels(v[1]), left(v[2]), right(v[3]), top(v[4]), bottom(
					v[5]), sumX(v[6]), sumY(v[7]), lastX(v[8]), lastY(v[9]) {
	}
	t_real32 getCentroidX() {
		return sumX / nbPixels;
	}

	t_real32 getCentroidY() {
		return sumY / nbPixels;
	}

	t_real32 getWidth() {
		return 1 + right - left;
	}
	t_real32 getHeight() {
		return 1 + bottom - top;
	}
	t_real32 getSurface() {
		return getWidth() * getHeight();
	}
	t_real32 getDencity() {
		return nbPixels / getSurface();
	}
	t_real32 getLongality() {
		t_real32 res = getWidth() / getHeight();
		return (res > 1) ? 1 / res : res;
	}
	std::string getDescription() {
		std::stringstream tmp;
		tmp << "{ label:" << int(label) << ", NbPixels:" << int(nbPixels)
				<< ", Location:{ left:" << left << " , top:" << top
				<< " , right:" << right << " , bottom:" << bottom << "}, meanX:"
				<< (sumX / nbPixels) << ", meanY:" << (sumY / nbPixels)
				<< ", LastPixel{ X:" << lastX << " , Y:" << lastY << "} }";
		return tmp.str();
	}
};

///Function that extracts the features of all labeled components
template<typename T> void __getLabeledComponentFeatures__(Buffer<t_real32> out,
		Buffer<T> in, T maxLabel = 0) {
//out must be 10x(maxLabel+1)

	t_sz y, x;
	t_real32* outRow;
	T* inRow;
	for (y = 0; y < out.height; y++) {
		outRow = out.getRow(y);
		outRow[LabeledComponentsFeatures::LC_LABEL_POS] = y;
		outRow[LabeledComponentsFeatures::LC_NBPIXELS_POS] = 0;
		outRow[LabeledComponentsFeatures::LC_LEFT_POS] = in.width;
		outRow[LabeledComponentsFeatures::LC_RIGHT_POS] = 0;
		outRow[LabeledComponentsFeatures::LC_TOP_POS] = in.height;
		outRow[LabeledComponentsFeatures::LC_BOTTOM_POS] = 0;
		outRow[LabeledComponentsFeatures::LC_SUMX_POS] = 0;
		outRow[LabeledComponentsFeatures::LC_SUMY_POS] = 0;
		outRow[LabeledComponentsFeatures::LC_LASTX_POS] = 0;
		outRow[LabeledComponentsFeatures::LC_LASTY_POS] = 0;
	}
	t_real32* nullComponent = out.getRow(0);
	nullComponent[LabeledComponentsFeatures::LC_NBPIXELS_POS] = in.width
			* in.height;
	for (y = 0; y < in.height; y++) {
		inRow = in.getRow(y);
		for (x = 0; x < in.width; x++) {
			if (inRow[x]) {
				outRow = out.getRow(inRow[x]);
				outRow[LabeledComponentsFeatures::LC_NBPIXELS_POS]++;
				outRow[LabeledComponentsFeatures::LC_LEFT_POS] =
						(outRow[LabeledComponentsFeatures::LC_LEFT_POS] < x) ?
								outRow[LabeledComponentsFeatures::LC_LEFT_POS] :
								x;
				outRow[LabeledComponentsFeatures::LC_RIGHT_POS] =
						(outRow[LabeledComponentsFeatures::LC_RIGHT_POS] > x) ?
								outRow[LabeledComponentsFeatures::LC_RIGHT_POS] :
								x;
				outRow[LabeledComponentsFeatures::LC_TOP_POS] =
						(outRow[LabeledComponentsFeatures::LC_TOP_POS] < y) ?
								outRow[LabeledComponentsFeatures::LC_TOP_POS] :
								y;

				outRow[LabeledComponentsFeatures::LC_BOTTOM_POS] = y;

				outRow[LabeledComponentsFeatures::LC_SUMX_POS] += x;
				outRow[LabeledComponentsFeatures::LC_SUMY_POS] += y;
				outRow[LabeledComponentsFeatures::LC_LASTX_POS] = x;
				outRow[LabeledComponentsFeatures::LC_LASTY_POS] = y;
				nullComponent[LabeledComponentsFeatures::LC_NBPIXELS_POS]--;
			}
		}
	}
}

///Function that counts the intersection
template<typename T> void __countComponentIntersectionUnion__(
		Buffer<t_label> out, Buffer<T> in1, Buffer<t_real32> cFeats1,
		Buffer<T> in2, Buffer<t_real32> cFeats2) {
//out is a 4 column buffer with nbFeats1*nbFeats2 rows.
//the first column is the labels from the first image
//the second column is the labels from the second image
//the third column is the number of pixels in the instersection
//of each component pair
//the fourth column is the number of pixels in the intersection
//of each component pair
	t_label curLab1, curLab2;
	t_sz left, top, right, bottom, x, y;
	t_real32 *curFeats1;
	t_real32 *curFeats2;
	t_label *outRow;
	t_sz counter = 0;
	T* imageRow1;
	T* imageRow2;
	for (curLab1 = 1; curLab1 < cFeats1.height; curLab1++) {
		curFeats1 = cFeats1.getRow(curLab1);
		for (curLab2 = 1; curLab2 < cFeats1.height; curLab2++) {
			curFeats2 = cFeats2.getRow(y);
			outRow = out.getRow(counter);
			left = curFeats1[LabeledComponentsFeatures::LC_LEFT_POS]
					> curFeats2[LabeledComponentsFeatures::LC_LEFT_POS] ?
					curFeats1[LabeledComponentsFeatures::LC_LEFT_POS] :
					curFeats2[LabeledComponentsFeatures::LC_LEFT_POS];
			top = curFeats1[LabeledComponentsFeatures::LC_TOP_POS]
					> curFeats2[LabeledComponentsFeatures::LC_TOP_POS] ?
					curFeats1[LabeledComponentsFeatures::LC_TOP_POS] :
					curFeats2[LabeledComponentsFeatures::LC_TOP_POS];
			right = curFeats1[LabeledComponentsFeatures::LC_RIGHT_POS]
					< curFeats2[LabeledComponentsFeatures::LC_RIGHT_POS] ?
					curFeats1[LabeledComponentsFeatures::LC_RIGHT_POS] :
					curFeats2[LabeledComponentsFeatures::LC_RIGHT_POS];
			bottom =
					curFeats1[LabeledComponentsFeatures::LC_BOTTOM_POS]
							< curFeats2[LabeledComponentsFeatures::LC_BOTTOM_POS] ?
							curFeats1[LabeledComponentsFeatures::LC_BOTTOM_POS] :
							curFeats2[LabeledComponentsFeatures::LC_BOTTOM_POS];
			outRow[0] = curLab1;
			outRow[1] = curLab2;
			outRow[2] = 0;
			outRow[3] = curFeats1[LabeledComponentsFeatures::LC_NBPIXELS_POS]
					+ curFeats2[LabeledComponentsFeatures::LC_NBPIXELS_POS];
			for (y = top; y <= bottom; y++) {
				imageRow1 = in1.getRow(y);
				imageRow2 = in2.getRow(y);
				for (x = left; x <= right; x++) {
					outRow[2] += ((imageRow1[x] == curLab1)
							&& (imageRow2[x] == curLab2));
				}
			}
			outRow[3] -= outRow[2];
			counter++;
		}
	}
}

template<typename T> void __drawComponents__(Buffer<T> out,
		std::map<t_label, T> &valueMap, Buffer<t_label> cimg,
		Buffer<t_real32> cFeats) {
	typedef typename std::map<t_label, T>::iterator miter;
	t_sz top, bottom, left, right, x, y, cwidth;
	t_label inLabel;
	T outVal;
	t_label *inRow;
	T* outRow;
	for (miter iter = valueMap.begin(); iter != valueMap.end(); ++iter) {
		inLabel = iter.first;
		outVal = iter.second;
		left = cFeats.getRow(inLabel)[LabeledComponentsFeatures::LC_LEFT_POS];
		top = cFeats.getRow(inLabel)[LabeledComponentsFeatures::LC_TOP_POS];
		right = cFeats.getRow(inLabel)[LabeledComponentsFeatures::LC_RIGHT_POS];
		bottom =
				cFeats.getRow(inLabel)[LabeledComponentsFeatures::LC_BOTTOM_POS];
		cwidth = 1 + right - left;
		for (y = top; y <= bottom; y++) {
			inRow = cimg.getRow(y) + left;
			outRow = out.getRow(y) + left;
			for (x = 0; x < cwidth; x++) {
				(inRow[x] == inLabel) && (outRow[x] = outVal);
			}
		}
	}
}

template<typename T> void __drawComponent__(Buffer<T> out, t_label inLabel,
		T outVal, Buffer<t_label> cimg, t_sz left, t_sz top, t_sz right,
		t_sz bottom) {
	t_sz x, y, cwidth;
	t_label *inRow;
	T* outRow;
	cwidth = 1 + right - left;

	for (y = top; y <= bottom; y++) {
		inRow = cimg.getRow(y) + left;
		outRow = out.getRow(y) + left;
		for (x = 0; x < cwidth; x++) {
			(inRow[x] == inLabel) && (outRow[x] = outVal);
		}
	}
}


template <typename T> void __getFeatureImage__(Buffer<T>out,Buffer<t_sint32> cimg,Buffer<t_real32> features,int featurePos,T nullVal){
	int x,y;
	T* outRow;
	t_sint32* inRow;
	for(y=0;y<cimg.height;y++){
		inRow = cimg.getRow(y);
		outRow=out.getRow(y);
		for(x=0;x<cimg.width;x++){
			outRow[x]=(inRow[x]==0)?0:(features.getRow(inRow[x])[featurePos]);
		}
	}
}

template<typename T> void __drawComponent__(Buffer<T> out, Buffer<t_label> cimg, LabeledComponentsFeatures feats,T outVal) {
	t_sz x, y;
	t_label *inRow;
	T* outRow;
	for (y = feats.top; y <= feats.bottom; y++) {
		inRow = cimg.getRow(y) ;
		outRow = out.getRow(y) ;
		for (x = feats.left; x <= feats.right; x++) {
			if(inRow[x] == feats.label) { outRow[x] = outVal;}
		}
	}
}


//template<typename T> Buffer<t_label> getLabeledConnectedComponents(Buffer<T> in,
//		int neighborhood) {
//	Buffer<t_label> out(in.width, in.height);
//	__labelConnectedComponents__<T>(out, in, neighborhood);
//	return out;
//}

}
}
}

#endif /* CONNECTED_COMPONENT_LABELING_HPP_ */
