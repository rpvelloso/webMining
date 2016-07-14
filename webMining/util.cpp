/*
 * util.cpp
 *
 *  Created on: 13 de jul de 2016
 *      Author: rvelloso
 */

#include <string>
#include <vector>
#include "hsfft.h"

std::string stringTok(std::string &inp, const std::string &delim) {
	size_t startPos = inp.find_first_not_of(delim);
	size_t endPos = inp.find_first_of(delim, startPos);

	std::string token;

	if (startPos != std::string::npos) {
		if (endPos != std::string::npos)
			token = inp.substr(startPos, endPos - startPos);
		else
			token = inp.substr(startPos);
	}

	if (endPos != std::string::npos) {
		startPos = inp.find_first_not_of(delim, endPos);
		if (startPos != std::string::npos)
			inp = inp.substr(startPos);
		else
			inp = "";
	} else
		inp = "";

	return token;
}

std::vector<double> _fft(std::vector<double> signal, int dir, bool psd = true) {
	size_t N = (signal.size() + (signal.size()%2));
	std::vector<double> ret;

	fft_object obj = fft_init(N,dir); // Initialize FFT object obj . N - FFT Length. 1 - Forward FFT and -1 for Inverse FFT

	fft_data* inp = (fft_data*) malloc (sizeof(fft_data) * N);
	fft_data* oup = (fft_data*) malloc (sizeof(fft_data) * N);

	for (size_t i=0;i<N;i++) {
		inp[i].re = signal[i];
		inp[i].im = 0;
	}

	if (signal.size() != N) { // repeat last sample when signal size is odd
		inp[N-1].re = signal[N-2];
		inp[N-1].im = 0;
	}

	fft_exec(obj,inp,oup);

	for (size_t i=0;i<N;i++) {
		if (psd)
			ret.push_back((oup[i].re*oup[i].re) + (oup[i].im*oup[i].im));
		else
			ret.push_back(oup[i].re);
	}

	free(inp);
	free(oup);
	free_fft(obj);

	return ret;
}

std::vector<double> fct(std::vector<double> signal) {
	std::vector<double> sig4n(signal.size()*4,0);

	for (size_t i = 0; i < signal.size(); ++i) {
		sig4n[(i*2)+1] = signal[i];
		sig4n[(signal.size()*4)-(i*2)-1] = signal[i];
	}
	auto ft = _fft(sig4n, 1, false);
	ft.resize(signal.size());

	for (size_t i = 0; i < ft.size()-3; ++i)
		ft[i] = abs(ft[i] - ft[i+2]);

	ft.resize(signal.size() - 2);
	return ft;
}
