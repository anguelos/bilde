/*
 * knn.hpp
 *
 *  Created on: Apr 3, 2013
 *      Author: anguelos
 */

#ifndef KNN_HPP_
#define KNN_HPP_

#include "wrapping_api.hpp"

namespace bilde {
namespace util {
namespace knn {

struct L2Distance {
	double operator()(const std::vector<double>& v1,
			const std::vector<double>& v2) {
		double res = 0;
		for (t_sz k = 0; k < v1.size(); k++) {
			res += ((v1[k] - v2[k]) * (v1[k] - v2[k]));
		}
		return sqrt(res);
	}
};

struct LInfDistance {
	double operator()(const std::vector<double>& v1,
			const std::vector<double>& v2) {
		double res = 0;
		double d;
		for (t_sz k = 0; k < v1.size(); k++) {
			d=((v1[k] > v2[k]) * (v1[k] - v2[k])
					+ (v1[k] <= v2[k]) * (v2[k] - v1[k]));
			res = d>res?d:res;
		}
		return res;
	}
};


struct L1Distance {
	double operator()(const std::vector<double>& v1,
			const std::vector<double>& v2) {
		double res = 0;
		for (t_sz k = 0; k < v1.size(); k++) {
			res += ((v1[k] > v2[k]) * (v1[k] - v2[k])
					+ (v1[k] <= v2[k]) * (v2[k] - v1[k]));
		}
		return res;
	}
};

template<typename DIST> std::vector<std::pair<double, int> > sortByDistance(
		const std::vector<double>& v,
		const std::vector<std::vector<double> >& ps) {
	DIST dist;
	std::vector<std::pair<double, int> > res;
	for (t_sz k = 0; k < ps.size(); k++) {
		res.push_back(std::pair<double, int>(dist(v, ps[k]), int(k)));
	}
	std::sort(res.begin(), res.end());
	return res;
}

template<typename DIST> std::vector<std::vector<std::pair<double, int> > > sortByDistance(
		const std::vector<std::vector<double> >& trainSet,
		const std::vector<std::vector<double> >& testSet) {
	DIST dist;
	std::vector<std::vector<std::pair<double, int> > > res;
	for (t_sz j = 0; j < testSet.size(); j++) {
		std::vector<std::pair<double, int> > tmp;
		for (t_sz k = 0; k < trainSet.size(); k++) {
			tmp.push_back(std::pair<double, int>((dist(testSet[j], trainSet[k])), int(k)));
		}
		std::sort(tmp.begin(), tmp.end());
		res.push_back(tmp);
	}
	return res;
}

template<typename DIST, typename CLASSTYPE> CLASSTYPE NNclassify(
		const std::vector<double> &v1,
		const std::vector<std::vector<double> >& patterns,
		const std::vector<CLASSTYPE>& classes) {
	sortByDistance<DIST>(v1, patterns);
	return classes[sortByDistance<DIST>(v1, patterns)[0].second];
}

}
}
}

#endif /* KNN_HPP_ */
