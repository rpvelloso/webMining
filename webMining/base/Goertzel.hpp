#include <vector>
#include <algorithm>

#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <cmath>

#include "PSD.hpp"

template<class Iterator>
class Goertzel : public PSD {
public:
	Goertzel(Iterator itBegin, Iterator itEnd) : itBegin(itBegin), itEnd(itEnd) {
		double PSDMean;
		/* Computes PSD mean according to Parseval's relation */
		std::for_each(itBegin, itEnd, [&PSDMean](double v){
			PSDMean += std::pow(v, 2);
		});
		this->PSDMean = PSDMean;
		len = std::distance(itBegin, itEnd);
		computed.resize(len, 0);
	};
	
	virtual ~Goertzel() {
		
	};
	
	double getPSDMean() {
		return PSDMean;
	};
	
	/* Computes 'optimized' Goertzel's and returns frequency's 
	 * PSD value instead of DFT coeff (mag + i*phase)  
	 */
	double getPSD(int targetFrequency) {
		if (targetFrequency < 0 || targetFrequency >= len)
			return 0;

		double omega = (2.0 * M_PI * targetFrequency) / (double)len;
		auto cosine = cos(omega);
		auto coeff = 2.0 * cosine;
		double Q2 = 0;
		double Q1 = 0;
		
		for (auto it = itBegin; it != itEnd; ++it) {
			double Q0;
			Q0 = coeff * Q1 - Q2 + (*it);
			Q2 = Q1;
			Q1 = Q0;
		}
		
		computed[targetFrequency] = (Q1 * Q1 + Q2 * Q2 - Q1 * Q2 * coeff)/PSDMean;
		return computed[targetFrequency];
	}

	std::vector<double> &getFullPSD() {
		return computed;
	};
private:
	Iterator itBegin, itEnd;
	size_t len = 0;
	double PSDMean = 0;
	std::vector<double> computed;
};

template<class Iterator>
PSD *make_goertzel(Iterator itBegin, Iterator itEnd) {
	return new Goertzel<Iterator>(itBegin, itEnd);
}
