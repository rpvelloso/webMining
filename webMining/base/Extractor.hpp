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

#ifndef EXTRACTOR_HPP_
#define EXTRACTOR_HPP_

//#include <crtdefs.h>
#include <stdexcept>
#include <vector>

#include "DOM.hpp"

template<typename DataRegionType>
class Extractor {
 public:
  Extractor() {
  }
  virtual ~Extractor() {
  }
  virtual void extract(pDOM dom) = 0;
  virtual void cleanup() {
    dataRegions.clear();
  }
  size_t regionCount() const noexcept {
    return dataRegions.size();
  }
  DataRegionType getDataRegion(size_t pos) const {
    if (pos < dataRegions.size())
      return dataRegions[pos];

    throw new std::out_of_range("data region not found");
  }
  void addDataRegion(const DataRegionType &dr) {
    dataRegions.emplace_back(dr);
  }
 protected:
  std::vector<DataRegionType> dataRegions;
};

#endif /* EXTRACTOR_HPP_ */
