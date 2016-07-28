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

#ifndef STRUCTUREDEXTRACTOR_HPP_
#define STRUCTUREDEXTRACTOR_HPP_

#include "DOM.hpp"
#include "Extractor.hpp"

template<typename DataRegionType>
class StructuredExtractor : public Extractor<DataRegionType> {
 public:
  StructuredExtractor() : Extractor<DataRegionType>() {
  }
  virtual ~StructuredExtractor() {
  }
  virtual void extract(pDOM dom) = 0;
};

#endif /* STRUCTUREDEXTRACTOR_HPP_ */
