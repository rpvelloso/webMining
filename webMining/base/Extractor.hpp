/*
 * Extractor.h
 *
 *  Created on: 15 de jul de 2016
 *      Author: roberto
 */

#ifndef EXTRACTOR_HPP_
#define EXTRACTOR_HPP_

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
