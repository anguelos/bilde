/*
 * t_accumulator.hpp
 *
 *  Created on: Jan 17, 2013
 *      Author: anguelos
 */

#ifndef T_ACCUMULATOR_HPP_
#define T_ACCUMULATOR_HPP_



namespace bilde {

/*
template <int bitdepth> struct Depth2Type{
	typedef void type;
};

template<> struct Depth2Type<1>{typedef t_uint8 type;};
template<> struct Depth2Type<2>{typedef t_uint8 type;};
template<> struct Depth2Type<3>{typedef t_uint8 type;};
template<> struct Depth2Type<4>{typedef t_uint8 type;};
template<> struct Depth2Type<5>{typedef t_uint8 type;};
template<> struct Depth2Type<6>{typedef t_uint8 type;};
template<> struct Depth2Type<7>{typedef t_uint8 type;};
template<> struct Depth2Type<8>{typedef t_uint8 type;};

template<> struct Depth2Type<9>{typedef t_uint16 type;};
template<> struct Depth2Type<10>{typedef t_uint16 type;};
template<> struct Depth2Type<11>{typedef t_uint16 type;};
template<> struct Depth2Type<12>{typedef t_uint16 type;};
template<> struct Depth2Type<13>{typedef t_uint16 type;};
template<> struct Depth2Type<14>{typedef t_uint16 type;};
template<> struct Depth2Type<15>{typedef t_uint16 type;};
template<> struct Depth2Type<16>{typedef t_uint16 type;};

template<> struct Depth2Type<17>{typedef t_uint32 type;};
template<> struct Depth2Type<18>{typedef t_uint32 type;};
template<> struct Depth2Type<19>{typedef t_uint32 type;};
template<> struct Depth2Type<20>{typedef t_uint32 type;};
template<> struct Depth2Type<21>{typedef t_uint32 type;};
template<> struct Depth2Type<22>{typedef t_uint32 type;};
template<> struct Depth2Type<23>{typedef t_uint32 type;};
template<> struct Depth2Type<24>{typedef t_uint32 type;};
template<> struct Depth2Type<25>{typedef t_uint32 type;};
template<> struct Depth2Type<26>{typedef t_uint32 type;};
template<> struct Depth2Type<27>{typedef t_uint32 type;};
template<> struct Depth2Type<28>{typedef t_uint32 type;};
template<> struct Depth2Type<29>{typedef t_uint32 type;};
template<> struct Depth2Type<30>{typedef t_uint32 type;};
template<> struct Depth2Type<31>{typedef t_uint32 type;};
template<> struct Depth2Type<32>{typedef t_uint32 type;};
*/

template <typename T>struct PixelMetrics{

};

template <>struct PixelMetrics<t_uint8>{
	typedef t_uint8 t;
	typedef t_sint8 t_signed;
	typedef t_uint32 t_accumulator;
	static const t_uint8 defaultFg=255;
	static const t_uint8 defaultBg=0;
	static const bool isDiscrete=1;
	static const bool isContinius=0;
	static const bool hasNegative=0;
	static const int bitCount=8;
	t roundIfNeeded(double v){return boost::math::round(v);}
	t_real64 normaliseTo01(t val){
		return t_real64(val)/defaultFg;
	}
	t normaliseFrom01(t_real64 v){
		return roundIfNeeded(defaultFg* v);
	}
};

template <>struct PixelMetrics<t_uint16>{
	typedef t_uint32 t;
	typedef t_sint16 t_signed;
	typedef t_uint32 t_accumulator;
	static const t_uint16 defaultFg=65535;
	static const t_uint16 defaultBg=0;
	static const bool isDiscrete=1;
	static const bool isContinius=0;
	static const bool hasNegative=0;
	static const int bitCount=16;
	t roundIfNeeded(double v){return boost::math::round(v);}
	t_real64 normaliseTo01(t val){
		return t_real64(val)/defaultFg;
	}
	t normaliseFrom01(t_real64 v){
		return roundIfNeeded(defaultFg* v);
	}
};

template <>struct PixelMetrics<t_uint32>{
	typedef t_uint32 t;
	typedef t_sint32 t_signed;
	typedef t_uint64 t_accumulator;
	static const t_uint32 defaultFg=(255<<24)+(255<<16)+(255<<8)+255;
	static const t_uint32 defaultBg=0;
	static const bool isDiscrete=1;
	static const bool isContinius=0;
	static const bool hasNegative=0;
	static const int bitCount=32;
	t roundIfNeeded(double v){return boost::math::round(v);}
	t_real64 normaliseTo01(t val){
		return t_real64(val)/defaultFg;
	}
	t normaliseFrom01(t_real64 v){
		return roundIfNeeded(defaultFg* v);
	}
};

template <>struct PixelMetrics<t_uint64>{
	typedef t_uint64 t;
	typedef t_sint64 t_signed;
	typedef t_uint64 t_accumulator;
	static const t_uint64 defaultFg=ULLONG_MAX;
	static const t_uint64 defaultBg=0;
	static const bool isDiscrete=1;
	static const bool isContinius=0;
	static const bool hasNegative=0;
	static const int bitCount=64;
	t roundIfNeeded(double v){return boost::math::round(v);}
	t_real64 normaliseTo01(t val){
		return t_real64(val)/defaultFg;
	}
	t normaliseFrom01(t_real64 v){
		return roundIfNeeded(defaultFg* v);
	}
};

template <>struct PixelMetrics<t_sint8>{
	typedef t_sint8 t;
	typedef t_sint8 t_signed;
	typedef t_sint32 t_accumulator;
	static const t_sint8 defaultFg=127;
	static const t_sint8 defaultBg=0;
	static const bool isDiscrete=1;
	static const bool isContinius=0;
	static const bool hasNegative=1;
	static const int bitCount=8;
	t roundIfNeeded(double v){return boost::math::round(v);}
	t_real64 normaliseTo01(t val){
		return t_real64(val)/defaultFg;
	}
	t normaliseFrom01(t_real64 v){
		return roundIfNeeded(defaultFg* v);
	}
};

template <>struct PixelMetrics<t_sint16>{
	typedef t_sint16 t;
	typedef t_sint16 t_signed;
	typedef t_sint32 t_accumulator;
	static const t_sint16 defaultFg=32767;
	static const t_sint16 defaultBg=0;
	static const bool isDiscrete=1;
	static const bool isContinius=0;
	static const bool hasNegative=1;
	static const int bitCount=16;
	t roundIfNeeded(double v){return boost::math::round(v);}
	t_real64 normaliseTo01(t val){
		return t_real64(val)/defaultFg;
	}
	t normaliseFrom01(t_real64 v){
		return roundIfNeeded(defaultFg* v);
	}
};

template <>struct PixelMetrics<t_sint32>{
	typedef t_sint32 t;
	typedef t_sint32 t_signed;
	typedef t_sint64 t_accumulator;
	static const t_sint32 defaultFg=(127<<24)+(255<<16)+(255<<8)+255;
	static const t_sint32 defaultBg=0;
	static const bool isDiscrete=1;
	static const bool isContinius=0;
	static const bool hasNegative=1;
	static const int bitCount=32;

	static const t_uint32 nbValidLabels=(1<<24);
	static const t_uint32 maxValidLabel=(1<<24)-1;


	t roundIfNeeded(double v){return boost::math::round(v);}

	t_real64 normaliseTo01(t val){
		return t_real64(val)/defaultFg;
	}

	t normaliseFrom01(t_real64 v){
		return roundIfNeeded(defaultFg* v);
	}

	static std::vector< t_uint32 > __getLabel2ARGBMapping__(bool colorNullWhite) {
		static const int nbBits = 24;
		static const int maxVal = 1 << nbBits;
		t_uint32 currentVal;
		unsigned char * argb = (unsigned char *) (&currentVal);
		std::vector<t_uint32> res(maxVal);
		if (colorNullWhite) {
			res[0] = (255 << 24) + (255 << 16) + (255 << 8) + 255;
		} else {
			res[0] = (255 << 24);
		}
		int b, k;
		for (k = 1; k < (maxVal); k++) {
			currentVal = 0;
			for (b = 0; b < nbBits; b++) {
				argb[b % 3] += (((k & (1 << b)) != 0)
						* (1 << (((nbBits - 1) - b)) / 3));
			}
			currentVal+=(255<<24);
			res[k] = currentVal;
		}
		return res;
	}

	static std::vector<t_sint32> __getARGB2LabelMapping__() {
		static const int nbBits = 24;
		static const int maxVal = 1 << nbBits;
		t_uint8 currentVal;
		unsigned char * rgba = (unsigned char *) (&currentVal);
		std::vector<t_sint32> res(maxVal);
		res[(255 << 16) + (255 << 8) + (255)] = 0;
		res[0] = 0;
		int k, b;
		for (k = 1; k < (maxVal); k++) {
			currentVal = 0;
			for (b = 0; b < nbBits; b++) {
				rgba[b % 3] += (((k & (1 << b)) != 0)
						* (1 << (((nbBits - 1) - b)) / 3));
			}
			res[currentVal] = k;
		}
		res[res.size() - 1] = 0;
		return res;
	}
	static t_uint32 label2ARGB(t_sint32 lab){
		static std::vector<t_uint32> map=__getLabel2ARGBMapping__(false);
		return map[lab%nbValidLabels];
	}
	static t_sint32 ARGB2Label(t_uint32 argb){
		static std::vector<t_sint32> map=__getARGB2LabelMapping__();
		return map[argb%(1<<24)];
	}
	static t_sint32 RGB2Label(t_uint8 *val){
		return ARGB2Label(val[0]*65536+val[1]*256+val[2]);
	}
	static void label2RGB(t_uint8* dest,t_sint32 val){
		t_uint32 tmp=label2ARGB(val);
		dest[0]=(tmp/65536)%256;
		dest[1]=(tmp/256)%256;
		dest[2]=tmp%256;
	}

};

template <>struct PixelMetrics<t_sint64>{
	typedef t_sint64 t;
	typedef t_sint64 t_signed;
	typedef t_sint64 t_accumulator;
	static const t_sint64 defaultFg=LLONG_MAX;
	static const t_sint64 defaultBg=0;
	static const bool isDiscrete=1;
	static const bool isContinius=0;
	static const bool hasNegative=1;
	static const int bitCount=64;
	t roundIfNeeded(double v){return boost::math::round(v);}
	t_real64 normaliseTo01(t val){
		return t_real64(val)/defaultFg;
	}
	t normaliseFrom01(t_real64 v){
		return roundIfNeeded(defaultFg* v);
	}
};


template <>struct PixelMetrics<t_real32>{
	typedef t_real32 t;
	typedef t_real32 t_signed;
	typedef t_real32 t_accumulator;
	constexpr static t_real32 defaultFg=1.0;
	constexpr static t_real32 defaultBg=.0;
	static const bool isDiscrete=0;
	static const bool isContinius=1;
	static const bool hasNegative=1;
	static const int bitCount=32;
	t roundIfNeeded(double v){return v;}
	t_real32 normaliseTo01(t val){
		return val;
	}
	t normaliseFrom01(t v){
		return v;
	}
};


template <>struct PixelMetrics<t_real64>{
	typedef t_real64 t;
	typedef t_real64 t_signed;
	typedef t_real64 t_accumulator;
	constexpr static t_real64 defaultFg=1.0;
	constexpr static t_real64 defaultBg=.0;
	static const bool isDiscrete=0;
	static const bool isContinius=1;
	static const bool hasNegative=1;
	static const int bitCount=64;
	t roundIfNeeded(double v){return v;}
	t_real64 normaliseTo01(t val){
		return val;
	}
	t normaliseFrom01(t v){
		return v;
	}
};

template <>struct PixelMetrics<t_real128>{
	typedef t_real128 t;
	typedef t_real128 t_signed;
	typedef t_real128 t_accumulator;
	constexpr static t_real128 defaultFg=1.0;
	constexpr static t_real128 defaultBg=.0;
	static const bool isDiscrete=0;
	static const bool isContinius=1;
	static const bool hasNegative=1;
	static const int bitCount=128;
	t roundIfNeeded(double v){return v;}
	t_real128 normaliseTo01(t val){
		return val;
	}
	t normaliseFrom01(t v){
		return v;
	}
};


}



#endif /* T_ACCUMULATOR_HPP_ */
