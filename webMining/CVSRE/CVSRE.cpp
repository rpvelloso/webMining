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

#include "CVSRE.hpp"

#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <string>

#include "../3rdparty/sol.hpp"
#include "../base/Node.hpp"
#include "../base/StructuredDataRegion.hpp"
#include "../base/util.hpp"

CVSRE::CVSRE() {
}

CVSRE::~CVSRE() {
}

void CVSRE::luaBinding(sol::state& lua) {
  lua.new_usertype<CVSRE>("CVSRE",
		  "extract", &CVSRE::extract,
		  "cleanup", &CVSRE::cleanup,
		  "getTps", &CVSRE::getTps,
		  "printTps", &CVSRE::printTps,
		  "regionCount", &CVSRE::regionCount,
		  "getDataRegion", &CVSRE::getDataRegion);
  CVSREDataRegion::luaBinding(lua);
}

void CVSRE::extract(pDOM dom) {
  cleanup();
  buildTagPath("", dom->body(), true);
  segment(0);
}

void CVSRE::segment(size_t offset) {
  std::wstring tps = tagPathSequence.substr(offset);
  std::set<int> symbols(tps.begin(), tps.end());

  for (auto i : symbols) {
    double period = 0;
    auto recpos = checkCV(tps, i, period);
    if (recpos.size() > 0 && period >= 2
        && period < 0.30 * tagPathSequence.size()) {
      auto subTps = tps.substr(recpos[0],
                               recpos[recpos.size() - 1] - recpos[0] + period);
      if (checkFreq(subTps, recpos.size())) {
        auto recSeq = align(subTps, recpos);
        CVSREDataRegion region(offset + recpos[0], period, subTps,
                               nodeSequence.begin() + offset + recpos[0],
                               nodeSequence.begin() + offset + recpos[0] + subTps.size());
        extractRecords(recSeq, recpos, region);
        dataRegions.emplace_back(region);
        segment(region.getEndPos() + 1);
        return;
      }
    }
  }
}

std::vector<std::wstring> CVSRE::align(const std::wstring &tps,
                                       const std::vector<size_t> &recpos) {
  std::vector < std::wstring > m;
  auto offset = recpos[0];

  for (auto r = recpos.begin(); r != recpos.end(); ++r) {
    auto next = r + 1;
    size_t rsize;

    if (next == recpos.end())
      rsize = std::wstring::npos;
    else
      rsize = (*next) - (*r);
    m.push_back(tps.substr(*r - offset, rsize));
  }
  centerStar (m);
  return m;
}

void CVSRE::extractRecords(std::vector<std::wstring> &m,
                           std::vector<size_t> &recpos,
                           CVSREDataRegion &region) {

  int rows = m.size(), cols = m[0].size();

  auto offset = recpos[0];
  auto rp = recpos.begin();
  for (int i = 0; i < rows; i++, ++rp) {
    Record rec;

    std::cerr << std::endl;
    for (int j = 0, k = 0; j < cols; j++) {
      if (m[i][j] != 0) {
        auto node = region.getNodeSequence()[(*rp) - offset + k];
        rec.emplace_back(node);

        auto tagName = node->getTagName();
        std::cerr << "[" << node->toString() << "]; ";

        k++;
      } else
        rec.emplace_back(nullptr);
    }
    region.addRecord(rec);
  }
  region.cleanup();
  std::cerr << std::endl;
}

bool CVSRE::checkFreq(const std::wstring &tps, size_t freq) {
  std::vector<double> signal(tps.begin(), tps.end());
  double m = mean(signal);
  std::for_each(signal.begin(), signal.end(), [m](auto &a)
  {
    a -= m;
  });
  auto spectrum = fft(signal);
  auto sd = stddev(spectrum);
  bool ret = false;
  for (size_t i = freq - 3; i < freq + 3; ++i) {
    if (spectrum[i] > 2.0 * sd) {
      ret = true;
    }
  }
  return ret;
}

std::vector<size_t> CVSRE::checkCV(const std::wstring &tps, int tpc,
                                   double &period) {
  std::vector < size_t > recpos;
  size_t pos = 0;
  while ((pos = tps.find(tpc, pos)) != std::string::npos)
    recpos.emplace_back(pos++);
  if (recpos.size() > 2) {
    auto recsize = difference(recpos);
    period = mean(recsize);
    auto sd = stddev(recsize);
    auto cv = sd / period;
    if (cv > 0.1)
      recpos.clear();
  } else
    recpos.clear();
  return recpos;
}
