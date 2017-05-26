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

#ifndef STRUCTUREDDATAREGION_HPP_
#define STRUCTUREDDATAREGION_HPP_

//#include <crtdefs.h>
#include <vector>

#include "DataRegion.hpp"
#include "Node.hpp"

using Record = std::vector<pNode>;

class StructuredDataRegion : public DataRegion {
 public:
  StructuredDataRegion();
  virtual ~StructuredDataRegion();
  Record getRecord(size_t pos) const;
  void addRecord(const std::vector<pNode> &r);
  size_t recordCount() const noexcept;
  size_t recordSize() const noexcept;
  void cleanup();
 private:
  std::vector<Record> records;
};

#endif /* STRUCTUREDDATAREGION_HPP_ */
