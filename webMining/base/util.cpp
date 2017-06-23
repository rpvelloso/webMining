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

//#include <crtdefs.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <cmath>
#include <memory>
#include <unordered_map>

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

std::vector<double> _fft(std::vector<double>::iterator beginIt, std::vector<double>::iterator endIt, int dir, bool psd = true) {
  size_t signalSize = std::distance(beginIt, endIt);
  size_t N = (signalSize + (signalSize % 2));
  std::vector<double> ret(N);

  auto fftDeleter = [](fft_set *ptr){free(ptr);};
  std::unique_ptr<fft_set, decltype(fftDeleter)> obj(fft_init(N, dir),fftDeleter);  // Initialize FFT object obj . N - FFT Length. 1 - Forward FFT and -1 for Inverse FFT

  std::unique_ptr<fft_data[]> inp(new fft_data[N]);
  std::unique_ptr<fft_data[]> oup(new fft_data[N]);

  auto it = beginIt;
  for (size_t i = 0; it != endIt ;++it, ++i) {
    inp[i].re = *it;
    inp[i].im = 0;
  }

  if (signalSize != N) {  // repeat last sample when signal size is odd
    inp[N - 1].re = *it;
    inp[N - 1].im = 0;
  }

  fft_exec(obj.get(), inp.get(), oup.get());

  for (size_t i = 0; i < N; i++) {
    if (psd)
      ret[i] = (oup[i].re * oup[i].re) + (oup[i].im * oup[i].im);
    else
      ret[i] = oup[i].re;
  }

  return ret;
}

std::vector<double> fct(const std::vector<double> &signal) {
  std::vector<double> sig4n(signal.size() * 4, 0);

  for (size_t i = 0; i < signal.size(); ++i) {
    sig4n[(i * 2) + 1] = signal[i];
    sig4n[(signal.size() * 4) - (i * 2) - 1] = signal[i];
  }
  auto ft = _fft(sig4n.begin(), sig4n.end(), 1, false);
  ft.resize(signal.size());

  return ft;
}

void hannWindow(std::vector<double> &inp) {
	for (size_t i = 0; i < inp.size(); i++) {
	    double multiplier = 0.5 * (1.0 - std::cos(2.0*M_PI*(double)i/(double)(inp.size()-1)));
	    inp[i] = multiplier * inp[i];
	}
}

void decode64(const std::string &inp, std::vector<unsigned char> &outp) {
	static std::unordered_map<char, unsigned char> decodeTable({
		{'A',  0}, {'B',  1}, {'C',  2}, {'D',  3}, {'E',  4}, {'F',  5}, {'G',  6}, {'H',  7},
		{'I',  8}, {'J',  9}, {'K', 10}, {'L', 11}, {'M', 12}, {'N', 13}, {'O', 14}, {'P', 15},
		{'Q', 16}, {'R', 17}, {'S', 18}, {'T', 19}, {'U', 20}, {'V', 21}, {'W', 22}, {'X', 23},
		{'Y', 24}, {'Z', 25}, {'a', 26}, {'b', 27}, {'c', 28}, {'d', 29}, {'e', 30}, {'f', 31},
		{'g', 32}, {'h', 33}, {'i', 34}, {'j', 35}, {'k', 36}, {'l', 37}, {'m', 38}, {'n', 39},
		{'o', 40}, {'p', 41}, {'q', 42}, {'r', 43}, {'s', 44}, {'t', 45}, {'u', 46}, {'v', 47},
		{'w', 48}, {'x', 49}, {'y', 50}, {'z', 51}, {'0', 52}, {'1', 53}, {'2', 54}, {'3', 55},
		{'4', 56}, {'5', 57}, {'6', 58}, {'7', 59}, {'8', 60}, {'9', 61}, {'+', 62}, {'/', 63},
		{'=',0}
	});

	if (inp.size()%4 != 0 || inp.empty())
	  throw std::length_error("invalid base64 input length");

	outp.clear();
	outp.reserve(inp.size()*0.75);

  std::vector<unsigned char> u({0,0,0});
  std::vector<unsigned char> v({0,0,0,0});

	for (size_t i = 0; i < inp.size(); ++i) {
		auto it = decodeTable.find(inp[i]);
		if (it == decodeTable.end()) // invalid symbol
			throw std::out_of_range("invalid base64 symbol");

		size_t pos = i%4;
		v[pos] = it->second;
		if (pos == 3) { // decode block
      u[0] = ((v[0] << 2) & 0b11111100) | ((v[1] >> 4) & 0b00000011);
      u[1] = ((v[1] << 4) & 0b11110000) | ((v[2] >> 2) & 0b00001111);
      u[2] = ((v[2] << 6) & 0b11000000) |  (v[3]       & 0b00111111);
      if (i != inp.size()-1) {
        outp.push_back(u[0]);
        outp.push_back(u[1]);
        outp.push_back(u[2]);
      }
		}
	}

	outp.push_back(u[0]);
	if (inp[inp.size() - 2] != '=')
	outp.push_back(u[1]);
	if (inp[inp.size() - 1] != '=')
	outp.push_back(u[2]);
}
