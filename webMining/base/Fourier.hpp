/*
 * fourier.h
 *
 *  Created on: 23 de jun de 2017
 *      Author: rvelloso
 */

#ifndef BASE_FOURIER_HPP_
#define BASE_FOURIER_HPP_

#include "PSD.hpp"
#include "util.hpp"

template<typename Iterator>
class Fourier : public PSD {
public:
	Fourier(Iterator beginIt, Iterator endIt) : PSD(), beginIt(beginIt), endIt(endIt) {
		len = std::distance(beginIt, endIt);
		psd = fft(beginIt, endIt);
        psdSD = stddev(psd);
        psdMean = mean(psd);
        for (auto &i : psd)  // convert to Z Score
          i = (i - psdMean) / psdSD;
	};
	virtual ~Fourier() {};

	double getPSD(int targetFrequency) {
		if (targetFrequency >= 0 && targetFrequency < len)
			return psd[targetFrequency];
		else
			return 0;
	};

	std::vector<double> &getFullPSD() {
		return psd;
	}
private:
	Iterator beginIt, endIt;
	std::vector<double> psd;
	size_t len = 0;
	double psdMean, psdSD;
};

template<class Iterator>
PSD *make_fourier(Iterator itBegin, Iterator itEnd) {
	return new Fourier<Iterator>(itBegin, itEnd);
}

#endif /* BASE_FOURIER_HPP_ */
