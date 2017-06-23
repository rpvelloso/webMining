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

#ifndef UTIL_HPP_
#define UTIL_HPP_

//#include <crtdefs.h>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "../3rdparty/Ckmeans.1d.dp.h"

#define fft(beginIt, endIt) _fft(beginIt, endIt, 1)
#define ifft(beginIt, endIt) _fft(beginIt, endIt, -1,false)
void hannWindow(std::vector<double> &inp);

struct LinearRegression {
  /* y = a*x + b; e = sum of squared error */
  double a, b, e;
};

std::string stringTok(std::string &inp, const std::string &delim);
std::vector<double> fct(const std::vector<double> &signal);
std::vector<double> _fft(std::vector<double>::iterator, std::vector<double>::iterator, int, bool = true);
void decode64(const std::string &, std::vector<unsigned char> &);

template<typename T>
T contour(T s) {
  auto height = s[0];

  for (auto &c : s) {
    if (c > height)
      height = c;
    c = height;
  }
  return s;
}

template<typename T>
std::vector<double> difference(const T &s) {
  std::vector<double> d(s.size() - 1);

  for (size_t i = 1; i < s.size(); ++i)
    d[i - 1] = s[i] - s[i - 1];

  return d;
}

#define SPACE T(1,'\0')

template<class T>
size_t edit_distance(T &s1, T &s2, bool align,
                     std::vector<unsigned int> *spaces) {
  const size_t len1 = s1.size(), len2 = s2.size();
  std::vector < std::vector<unsigned int>
      > d(len1 + 1, std::vector<unsigned int>(len2 + 1));

  d[0][0] = 0;
  for (unsigned int i = 1; i <= len1; ++i)
    d[i][0] = i;
  for (unsigned int i = 1; i <= len2; ++i)
    d[0][i] = i;

  for (unsigned int i = 1; i <= len1; ++i) {
    for (unsigned int j = 1; j <= len2; ++j) {
      d[i][j] = std::min(std::min(d[i - 1][j] + 1, d[i][j - 1] + 1),
                         d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1));
    }
  }

  if (align) {
    unsigned int i = len1;
    unsigned int j = len2;
    T s11, s22;
    while ((i > 0) && (j > 0)) {
      if ((d[i - 1][j - 1] <= d[i - 1][j])
          && (d[i - 1][j - 1] <= d[i][j - 1])) {
        s11 = s1[i - 1] + s11;
        s22 = s2[j - 1] + s22;
        i--;
        j--;
      } else if (d[i - 1][j] < d[i][j - 1]) {
        s11 = s1[i - 1] + s11;
        s22 = SPACE + s22;
        i--;
      } else {
        spaces->push_back(i - 1);
        s11 = SPACE + s11;
        s22 = s2[j - 1] + s22;
        j--;
      }
    }
    s1 = s11;
    s2 = s22;
  }

  return d[len1][len2];
}

template<class T>
void normalizeSequences(std::vector<T> &M) {
  std::vector<double> ckmeansInput;

  ckmeansInput.push_back(0);
  for (size_t i = 0; i < M.size(); i++) {
    ckmeansInput.push_back(M[i].size());
  }

  ClusterResult result;
  result = kmeans_1d_dp(ckmeansInput, 2, 2);

  if (result.nClusters == 2) {
    size_t maxSize = 0;

    if (result.size[1] > result.size[2]) {
      for (size_t i = 0; i < M.size(); i++) {
        if (result.cluster[i + 1] == 1) {
          if (M[i].size() > maxSize)
            maxSize = M[i].size();
        }
      }

      for (size_t i = 0; i < M.size(); i++) {
        if (M[i].size() > maxSize)
          M[i] = M[i].substr(1, maxSize);
      }
    }
  }
}

template<class T>
double centerStar(std::vector<T> &M) {
  size_t ret = 0;

  normalizeSequences(M);

  size_t len = M.size();
  std::vector < std::vector<unsigned int>
      > d(len, std::vector<unsigned int>(len));
  //unsigned int d[len][len];
  size_t minscore = 0xffffffff, center = 0;

  // find the center string
  size_t score;
  for (size_t i = 0; i < len; i++) {
    d[i][i] = 0;
    score = 0;
    for (size_t j = i + 1; j < len; j++) {
      d[i][j] = edit_distance(M[i], M[j], false, NULL);
      d[j][i] = d[i][j];
      score += d[i][j];
    }
    for (size_t j = 0; j <= i; j++)
      score += d[i][j];
    if (score < minscore) {
      minscore = score;
      center = i;
    }
  }

  // align
  for (size_t i = 0; i < M.size(); i++) {
    if (i != center) {
      std::vector<unsigned int> spaces;

      ret += edit_distance(M[center], M[i], true, &spaces);
      if (spaces.size()) {
        for (size_t j = i; j > 0; j--) {
          if ((j - 1) != center) {
            for (size_t k = 0; k < spaces.size(); k++) {
              M[j - 1].insert(spaces[k], SPACE);
            }
          }
        }
      }
    }
  }

  for (size_t i = 0; i < M.size(); i++) {
    for (size_t j = 0; j < M[i].size(); j++)
      std::cerr << M[i][j] << ";";
    std::cerr << std::endl;
  }
  return ((double) M[0].size() * (double) M.size()
      / (double) std::max(ret, (size_t) 1));
}

template<class T>
LinearRegression computeLinearRegression(const T &s) {
  //trimSequence(s);
  double delta, x, y, xy, x2, sx = 0, sy = 0, sxy = 0, sx2 = 0, n = s.size();
  LinearRegression lc;

  for (long int i = 0; i < n; i++) {
    y = s[i];
    x = i;
    xy = x * y;
    x2 = x * x;

    sx += x;
    sy += y;
    sxy += xy;
    sx2 += x2;
  }

  delta = (n * sx2) - (sx * sx);
  lc.a = (double) ((double) ((n * sxy) - (sy * sx)) / (double) delta);
  lc.b = (double) ((double) ((sx2 * sy) - (sx * sxy)) / (double) delta);

  for (long int i = 0; i < n; i++) {
    double ee;
    ee = std::abs(s[i] - (lc.a * i + lc.b));
    lc.e += ee * ee;
  }
  lc.e /= n;

  return lc;
}

template<typename T>
double mean(const T &s) {

  double total = 0;

  for (auto i : s) {
    total += i;
  }
  return total / (double) s.size();
}

template<typename T>
double variance(const T &s) {
  auto m = mean(s);

  double var = 0;
  for (auto i : s) {
    auto dev = i - m;
    dev *= dev;
    var += dev;
  }
  var /= (double) (std::max(s.size() - 1, (size_t) 1));
  return var;
}

template<typename T>
double stddev(const T &s) {
  return sqrt(variance(s));
}

#endif /* UTIL_HPP_ */
