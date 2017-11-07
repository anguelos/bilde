/*
 * timer.hpp
 *
 *  Created on: Mar 24, 2013
 *      Author: anguelos
 */

#ifndef TIMER_HPP_
#define TIMER_HPP_

#include <ctime>
#include <cstdio>
#include <sstream>
#include <iomanip>

namespace bilde{
namespace util{
namespace timer{
struct Timer{
	std::clock_t t;
	std::clock_t p;
	std::stringstream s;
	Timer(const Timer& timer){
		s<<std::setprecision(3);
		t=timer.t;
		p=timer.p;
	}
	Timer& operator=(const Timer& timer){
		s<<std::setprecision(3);
		t=timer.t;
		p=timer.p;
		return *this;
	}
	Timer(){
		s<<std::setprecision(3);
		t=std::clock();
		p=0;
	}
	double toc(){
		return (p+(p==0)*std::clock()-t)/double(CLOCKS_PER_SEC);
	}
	void pause(){
		p=std::clock();
	}
	void resume(){
		t=std::clock()-(p-t);
	}
	void tic(){
		t=std::clock();
	}
	void reset(){
		t=std::clock();
		p=t+1;
	}
	std::string getSec(){
		s.str("");
		s<<toc()<<" sec.";
		return s.str();
	}
	std::string getMsec(){
		s.str("");
		s<<(toc()*1000)<<" ms";
		return s.str();
	}
	std::string getMin(){
		s.str("");
		s<<(toc()/60)<<" ms";
		return s.str();
	}
	std::string getHour(){
		s.str("");
		s<<(toc()/3600)<<" ms";
		return s.str();
	}
	Timer& operator+=(const Timer&t_){
		t-=(t_.p+(t_.p==0)*std::clock()-t_.t);
		return *this;
	}
	friend std::ostream& operator<<(std::ostream& out,Timer& t){
		out<<t.getMsec();
		return out;
	}
};
}

timer::Timer & globalTimer(){
	static timer::Timer t;
	return t;
}


}
}


#endif /* TIMER_HPP_ */
