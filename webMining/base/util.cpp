/*
 Copyright 2011 Roberto Panerai Velloso.
 This file is part of webMining.
 webMining is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 webMining is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with webMining.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <crtdefs.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <cmath>
#include <memory>

#include "../3rdparty/hsfft.h"

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

std::vector<double> _fft(std::vector<double> &signal, int dir, bool psd = true) {
  size_t N = (signal.size() + (signal.size() % 2));
  std::vector<double> ret(N);

  //fft_object obj = fft_init(N, dir);  // Initialize FFT object obj . N - FFT Length. 1 - Forward FFT and -1 for Inverse FFT
  auto fftDeleter = [](fft_set *ptr){free(ptr);};
  std::unique_ptr<fft_set, decltype(fftDeleter)> obj(fft_init(N, dir),fftDeleter);  // Initialize FFT object obj . N - FFT Length. 1 - Forward FFT and -1 for Inverse FFT

  std::unique_ptr<fft_data[]> inp(new fft_data[N]);
  std::unique_ptr<fft_data[]> oup(new fft_data[N]);
  //fft_data* inp = (fft_data*) malloc(sizeof(fft_data) * N);
  //fft_data* oup = (fft_data*) malloc(sizeof(fft_data) * N);

  for (size_t i = 0; i < N; i++) {
    inp[i].re = signal[i];
    inp[i].im = 0;
  }

  if (signal.size() != N) {  // repeat last sample when signal size is odd
    inp[N - 1].re = signal[N - 2];
    inp[N - 1].im = 0;
  }

  fft_exec(obj.get(), inp.get(), oup.get());

  for (size_t i = 0; i < N; i++) {
    if (psd)
      ret[i] = (oup[i].re * oup[i].re) + (oup[i].im * oup[i].im);
    else
      ret[i] = oup[i].re;
  }

  //free(inp);
  //free(oup);
  //free_fft(obj);

  return ret;
}

std::vector<double> fct(std::vector<double> signal) {
  std::vector<double> sig4n(signal.size() * 4, 0);

  for (size_t i = 0; i < signal.size(); ++i) {
    sig4n[(i * 2) + 1] = signal[i];
    sig4n[(signal.size() * 4) - (i * 2) - 1] = signal[i];
  }
  auto ft = _fft(sig4n, 1, false);
  ft.resize(signal.size());

  return ft;
}

#ifndef M_PI
const double M_PI = 3.14159265359;
#endif

void hannWindow(std::vector<double> &inp) {
	for (size_t i = 0; i < inp.size(); i++) {
	    double multiplier = 0.5 * (1.0 - std::cos(2.0*M_PI*(double)i/(double)(inp.size()-1)));
	    inp[i] = multiplier * inp[i];
	}
}
