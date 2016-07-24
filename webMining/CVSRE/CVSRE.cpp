/*
 * CVSRE.cpp
 *
 *  Created on: 23 de jul de 2016
 *      Author: roberto
 */

#include <vector>
#include <set>
#include "CVSRE.hpp"

CVSRE::CVSRE() {
}

CVSRE::~CVSRE() {
}

void CVSRE::luaBinding(sol::state& lua) {
  lua.new_usertype < CVSRE
      > ("CVSRE", "extract", &CVSRE::extract, "cleanup", &CVSRE::cleanup, "getTps", &CVSRE::getTps, "printTps", &CVSRE::printTps, "regionCount", &CVSRE::regionCount, "getDataRegion", &CVSRE::getDataRegion);
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
      offset = recpos[0];
      auto subTps = tps.substr(offset,
                               recpos[recpos.size() - 1] - recpos[0] + period);
      if (checkFreq(subTps, recpos.size())) {
        auto recSeq = align(subTps, recpos);
        CVSREDataRegion region(offset, period, subTps,
                               nodeSequence.begin() + offset,
                               nodeSequence.begin() + offset + subTps.size());
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
  auto m = mean(signal);
  std::for_each(signal.begin(), signal.end(), [m](auto a)
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
