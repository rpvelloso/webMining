/*
    Copyright 2011 Roberto Panerai Velloso.

    This file is part of libsockets.

    libsockets is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libsockets is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libsockets.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <cmath>
#include <string>
#include <vector>
#include <tidy.h>
#include <sstream>
#include "hsfft.h"
#include "Ckmeans.1d.dp.h"

using namespace std;

string &trim(string &s) {
	s.erase(0, s.find_first_not_of(" \t\r\n"));
	s.erase(s.find_last_not_of(" \t\r\n")+1);
	return s;
}

string stringTok(string &s, string d) {
	string tok="";
	unsigned int i=0;

	while ((d.find(s[i++])!=string::npos) && (i<s.size()));
	for (--i;i<s.size();i++) {
		if (d.find(s[i])==string::npos) tok = tok + s[i];
		else break;
	}
	while ((d.find(s[i++])!=string::npos) && (i<s.size()));
	s.erase(0,--i);
	return tok;
}

// returns power spectrum of signal
vector<double> _fft(vector<double> signal, int dir, bool psd = true) {
	size_t N = (signal.size() + (signal.size()%2));
	vector<double> ret;

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

vector<double> autoCorrelation(vector<double> signal) {
	size_t N = (signal.size() + (signal.size()%2));

	if (signal.size() != N) { // repeat last sample when signal size is odd
		signal.push_back(signal[N-1]);
	}
	signal.resize(2*N); // zero pad
	return _fft(_fft(signal,1),-1);
}

vector<double> fct(vector<double> signal) {
	vector<double> sig4n(signal.size()*4,0);

	for (size_t i = 0; i < signal.size(); ++i) {
		sig4n[(i*2)+1] = signal[i];
		sig4n[(signal.size()*4)-(i*2)-1] = signal[i];
	}
	auto ft = _fft(sig4n, 1, false);
	ft.resize(signal.size());

	for (size_t i = 0; i < ft.size()-3; ++i)
		ft[i] = abs(ft[i]) + abs(ft[i+2]);

	ft.resize(signal.size() - 2);
	return ft;
}
