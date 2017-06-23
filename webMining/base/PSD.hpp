/*
 * psd.h
 *
 *  Created on: 23 de jun de 2017
 *      Author: rvelloso
 */

#ifndef BASE_PSD_HPP_
#define BASE_PSD_HPP_

#include <vector>

class PSD {
public:
	PSD() {};
	virtual ~PSD() {};
	virtual double getPSD(int targetFrequency) = 0;
	virtual std::vector<double> &getFullPSD() = 0;
};


#endif /* BASE_PSD_HPP_ */
