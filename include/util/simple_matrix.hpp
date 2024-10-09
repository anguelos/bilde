/*
 * simple_matrix.hpp
 *
 *  Created on: May 17, 2013
 *      Author: anguelos
 */

#ifndef SIMPLE_MATRIX_HPP_
#define SIMPLE_MATRIX_HPP_

namespace bilde {
namespace util {
namespace matrix {

struct Matrix {
	const t_sz width;
	const t_sz height;
	std::vector<double> data;
	Matrix(int w, int h) :
			width(w), height(h), data(w * h) {
	}
	Matrix(const std::vector<std::vector<double> >&in) :
			width(in[0].size()), height(in.size()), data(width * height) {
		for (t_sz y = 0; y < height; y++) {
			if (in[y].size() != width) {
				std::cerr
						<< "Matrix(const std::vector<std::vector<double> >&in)\nin["
						<< y << "].size() = " << in[y].size() << "\nwidth="
						<< width << "\nheight=" << height << "\n";
				throw "Matrix(const std::vector<std::vector<double> >&in)";
			}
			for (t_sz x = 0; x < width; x++) {
				data[x + y * width] = in[y][x];
			}
		}
	}
	std::vector<std::vector<std::string> > getAsVector() {
		std::vector<std::vector<std::string> > res;
		for (t_sz y = 0; y < height; y++) {
			std::vector<std::string> tmp(width);
			for (t_sz x = 0; x < width; x++) {
				tmp[x] = data[y * width + x];
			}
			res.push_back(tmp);
		}
		return res;
	}
	Matrix hslice(t_sz from, t_sz to) {
		Matrix res(1 + to - from, height);
		t_sz x, y;
		for (y = 0; y < height; y++) {
			for (x = from; x <= to; x++) {
				res.data[y * res.width + x - from] = data[y * res.width + x];
			}
		}
		return res;
	}
	Matrix vslice(t_sz from, t_sz to) {
		Matrix res(width, 1 + to - from);
		t_sz x, y;
		for (y = from; y <= to; y++) {
			for (x = 0; x < width; x++) {
				res.data[(y - from) * res.width + x] = data[y * res.width + x];
			}
		}
		return res;
	}
	void scaleRowsByVector(const std::vector<double>& sc) {
		t_sz x, y;
		if (sc.size() != width) {
			std::cerr
					<< "void scaleRowsByVector(const std::vector<double>& sc)\nsc.size = "
					<< sc.size() << " , width=" << width;
			throw "void scaleRowsByVector(const std::vector<double>& sc) wrong scaler size";
		}
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				data[y * width + x] = sc[x];
			}
		}
	}
	void scaleColsByVector(const std::vector<double>& sc) {
		t_sz x, y;
		if (sc.size() != height) {
			std::cerr
					<< "void scaleColsByVector(const std::vector<double>& sc)\nsc.size = "
					<< sc.size() << " , height=" << height;
			throw "void scaleColsByVector(const std::vector<double>& sc) wrong scaler size";
		}
		for (y = 0; y < height; y++) {
			for (x = 0; x < width; x++) {
				data[y * width + x] = sc[y];
			}
		}
	}
	friend Matrix operator*(const Matrix& m1, const Matrix&m2) {
		if (m1.width != m2.height) {
			std::cerr
					<< "friend Matrix operator*(const Matrix& m1,const Matrix&m2)\nsize missmatch\n";
			throw "Matrix operator*(const Matrix& m1,const Matrix&m2)";
		}
		Matrix res(m1.height, m2.width);
		t_sz x, y, k;
		double s;
		for (y = 0; y < res.height; y++) {
			for (x = 0; x < res.width; x++) {
				s = 0;
				for (k = 0; k < m1.width; k++) {
					s +=
							(m1.data[y * m1.width + k]
									* m2.data[k * m2.width + x]);
				}
				res.data[y * res.width + x] = s;
			}
		}
		return res;
	}
	friend std::ostream& operator<<(std::ostream & out, const Matrix& m) {
		t_sz x, y;
		out << "[ " << m.width << " x " << m.height;
		for (y = 0; y < m.height; y++) {
			out << "\n\t[" << m.data[y * m.width];
			for (x = 1; x < m.width; x++) {
				out << "," << m.data[y * m.width];
			}
			out << "]";
		}
		out << "\n]";
		return out;
	}
};

inline t_sz __geMartixWidthValidate__(
		const std::vector<std::vector<double> >& v) {
	t_sz r = 0;
	if (v.size() == 0) {
		return 0;
	} else {
		r = v[0].size();
		for(t_sz k=1;k<v.size();k++){
			if(v[k].size()!=r){
				std::cerr << "__geMartixWidthValidate__ wrong row size\n" << k
						<< "th row has a length of " << v[k].size()
						<< " while it should be " << r << "\n";
				throw "__geMartixWidthValidate__ wrong row size";
			}
		}
		return r;
	}
}
inline std::vector<std::vector<double> > multiply(
	const std::vector<std::vector<double> >&m1,
	const std::vector<std::vector<double> >& m2) {
	t_sz m1height=m1.size();
	t_sz m1width=__geMartixWidthValidate__(m1);
	t_sz m2height=m2.size();
	t_sz m2width=__geMartixWidthValidate__(m2);
	if(m1width!=m2height){
		std::cerr<<"multiplication of ["<<m1width<<"x"<<m1height<<"] with ["<<m2width<<"x"<<m2height<<"] is imposible";
	}
	std::vector<std::vector<double> > res;
	t_sz x,y,k;
	double s;
	for(y=0;y<m1height;y++){
		res.push_back(std::vector<double>(m2width));
	}
	for(y=0;y<m1height;y++){
		for(x=0;x<m2width;x++){
			s=0;
			for(k=0;k<m1width;k++){
				s+=((m1[y][k])*(m2[k][x]));
			}
			res[y][x]=s;
		}
	}
	return res;
}

inline std::vector<std::vector<double> > scaleRows(const std::vector<std::vector<double> >& m,std::vector<double> & v){
	std::vector<std::vector<double> >res;
	for(t_sz y=0;y<m.size();y++){
		if(v.size()!=m[y].size()){
			std::cerr<<"row "<<y<<" of "<<m.size() <<" Size missmatch:  vector size= "<<v.size()<<"  row size= "<<m[y].size()<<"\n";
			throw "inline std::vector<std::vector<double> > scaleRows(const std::vector<std::vector<double> >& m,std::vector<double> & v) scale vector missmatch";
		}
		res.push_back(std::vector<double>(m[y].size()));
		for(t_sz x=0;x<m[y].size();x++){
			res[y][x]=m[y][x]*v[x];
		}
	}
	return res;
}

}
}
}

#endif /* SIMPLE_MATRIX_HPP_ */
