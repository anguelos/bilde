/*
 * static_vector.hpp
 *
 *  Created on: Feb 19, 2013
 *      Author: anguelos
 */

#ifndef STATIC_VECTOR_HPP_
#define STATIC_VECTOR_HPP_


namespace bilde {
template<typename T, int SZ> struct StaticVector {
	//T __data__[SZ];
	T* const data;
	static const int sz = SZ;
	T&operator[](unsigned int pos) {
		return data[pos];
	}
	const T&operator[](unsigned int pos) const {
		return data[pos];
	}

	StaticVector<T, SZ>& operator=(T val) {
		data[SZ - 1] = val;
		StaticVector<T, SZ - 1>::operator=(val);
		return *this;
	}
	StaticVector<T, SZ>& operator=(T* val) {
		//Deep copy
		data[SZ - 1] = val[SZ - 1];
		StaticVector<T, SZ - 1>::operator=(val);
		return *this;
	}
	StaticVector<T, SZ>& operator=(const StaticVector<T, SZ>& val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] = val.data[k];
		}
		return *this;
	}

	StaticVector<T, SZ>& operator=(const std::vector<T> & vec) {
	 if (vec.size() != SZ) {
	 throw "StaticVector<T, SZ>::operator=(const std::vector<T> & vec) error. vec has a diferent size than SZ!";
	 }
	 for (int k = 0; k < SZ; k++) {
	 data[k] = vec[k];
	 }
	 return *this;
	}
	StaticVector():data(new T[SZ]) {
	}
	StaticVector(T val):data(new T[SZ]) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] = val;
		}
	}
	StaticVector(T* val):data(val) {	}
	StaticVector(const std::vector< double> & vec):data(new T[SZ]) {
		if (vec.size() != SZ) {
			throw "StaticVector<T, SZ>::operator=(const std::vector<T> & vec) error. vec has a diferent size than SZ!";
		}
		for (int k = 0; k < SZ; k++) {
			data[k] = vec[k];
		}
	}
	void setTo(T val){
		for(int k=0;k<SZ;k++){
			data[k]=val;
		}
	}
	std::vector<T> asStdVector(){
		std::vector<T> res(SZ);
		for(int k=0;k<SZ;k++){
			res[k]=data[k];
		}
		return res;
	}
//*****************************************************************************************************

	void operator+=(StaticVector<T, SZ>& val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] += val.data[k];
		}
	}
	void operator+=(T val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] += val;
		}
	}
	void operator+=(T* val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] += val[k];
		}
	}

	void operator*=(StaticVector<T, SZ>& val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] *= val.data[k];
		}
	}
	void operator*=(T val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] *= val;
		}
	}
	void operator*=(T* val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] *= val[k];
		}
	}
	void operator-=(StaticVector<T, SZ>& val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] -= val.data[k];
		}
	}
	void operator-=(T val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] -= val;
		}
	}
	void operator-=(T* val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] -= val[k];
		}
	}
	void operator/=(StaticVector<T, SZ>& val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] /= val.data[k];
		}
	}
	void operator/=(T val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] /= val;
		}
	}
	void operator/=(T* val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] /= val[k];
		}
	}

	void operator^=(StaticVector<T, SZ>& val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] = std::pow(this->data[k], val.data[k]);
		}
	}
	void operator^=(T val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] = std::pow(this->data[k], val);
		}
	}
	void operator^=(T* val) {
		for (int k = 0; k < SZ; k++) {
			this->data[k] = std::pow(this->data[k], val[k]);
		}
	}

//********************************************************************

	StaticVector<T, SZ> operator+(StaticVector<T, SZ>& val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = this->data[k] + val.data[k];
		}
		return res;
	}
	StaticVector<T, SZ> operator+(T val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = this->data[k] + val;
		}
		return res;
	}
	StaticVector<T, SZ> operator+(T* val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = this->data[k] + val[k];
		}
		return res;
	}

	StaticVector<T, SZ> operator*(StaticVector<T, SZ>& val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = this->data[k] * val.data[k];
		}
		return res;
	}
	StaticVector<T, SZ> operator*(T val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = this->data[k] * val;
		}
		return res;
	}
	StaticVector<T, SZ> operator*(T* val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = this->data[k] * val[k];
		}
		return res;
	}
	StaticVector<T, SZ> operator-(StaticVector<T, SZ>& val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = this->data[k] - val.data[k];
		}
		return res;
	}
	StaticVector<T, SZ> operator-(T val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = this->data[k] - val;
		}
		return res;
	}
	StaticVector<T, SZ> operator-(T* val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = this->data[k] - val[k];
		}
		return res;
	}
	StaticVector<T, SZ> operator/(StaticVector<T, SZ>& val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = this->data[k] / val.data[k];
		}
		return res;
	}
	StaticVector<T, SZ> operator/(T val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = this->data[k] / val;
		}
		return res;
	}
	StaticVector<T, SZ> operator/(T* val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = this->data[k] / val[k];
		}
		return res;
	}

	StaticVector<T, SZ> operator^(StaticVector<T, SZ>& val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = std::pow(this->data[k], val.data[k]);
		}
		return res;
	}
	StaticVector<T, SZ> operator^(T val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = std::pow(this->data[k], val);
		}
		return res;
	}
	StaticVector<T, SZ> operator^(T* val) {
		StaticVector<T, SZ> res;
		for (int k = 0; k < SZ; k++) {
			res.data[k] = std::pow(this->data[k], val[k]);
		}
		return res;
	}
	static double L2(const StaticVector<T, SZ>& v1,
			const StaticVector<T, SZ>& v2) {
		double res = 0;
		for (int k = 0; k < SZ; k++) {
			res += ((v1.data[k] - v2.data[k]) * (v1.data[k] - v2.data[k]));
		}
		return std::sqrt(res);
	}
	static double L1(const StaticVector<T, SZ>& v1,
			const StaticVector<T, SZ>& v2) {
		double res = 0;
		for (int k = 0; k < SZ; k++) {
			res += abs((v1.data[k] - v2.data[k]));
		}
		return res;
	}
	static double Linf(const StaticVector<T, SZ>& v1,
			const StaticVector<T, SZ>& v2) {
		double res = 0;
		for (int k = 0; k < SZ; k++) {
			res = (res >= abs((v1.data[k] - v2.data[k]))) ?
					res : abs((v1.data[k] - v2.data[k]));
		}
		return res;
	}
	//********************************************************************************
	friend std::ostream& operator<<(std::ostream& stream,
			const StaticVector<T, SZ>& vec) {
		stream << vec[0];
		for (int k = 1; k < SZ; k++) {
			stream << "," << vec[k];
		}
	}
	friend std::istream& operator>>(std::istream& stream,
			const StaticVector<T, SZ>& vec) {
		stream >> std::skipws >> vec[0];
		char c = ',';
		for (int k = 1; k < SZ && c == ','; k++) {
			stream >> std::skipws >> c >> vec[k];
		}
		if (c != ',') {
			throw "std::istream& operator>>(std::ostream& stream, const StaticVector<T,SZ>& vec) expected comma";
		}
	}
	template<int OSZ> friend StaticVector<T,OSZ+SZ> concat(const StaticVector<T,OSZ>& v1,const StaticVector<T,SZ>& v2) {
		StaticVector<T,OSZ+SZ> res;
		for(int k=0;k<OSZ;k++) {
			res[k]=v1[k];
		}
		for(int k=0;k<SZ;k++) {
			res[k+OSZ]=v2[k];
		}
		return res;
	}
	//**********************************************************************************
	typename PixelMetrics<T>::t_accumulator getSum(){
		typename PixelMetrics<T>::t_accumulator res=0;
		for(int k=0;k<SZ;k++){
			res+=data[k];
		}
		return res;
	}
	T getMaxPos(){
		T max=data[0];
		int maxPos=0;
		for(int k=1;k<SZ;k++){
			(max<data[k])&&((maxPos=k)&&(max=data[k]));
		}
		return maxPos;
	}

};

}

#endif /* STATIC_VECTOR_HPP_ */
