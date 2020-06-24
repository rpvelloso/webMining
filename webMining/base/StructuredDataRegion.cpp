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

#include "StructuredDataRegion.hpp"

#include <iostream>
#include <algorithm>
#include <iterator>
#include <stdexcept>

StructuredDataRegion::StructuredDataRegion() : DataRegion() {
}

StructuredDataRegion::~StructuredDataRegion() {
}

Record StructuredDataRegion::getRecord(size_t pos) const {
  if (pos < records.size())
    return records[pos];

  throw new std::out_of_range("record not found");
}

const Node &StructuredDataRegion::getCell(size_t row, size_t col) const {
  if (row < recordCount() && col < recordSize())
    return *records[row][col];

  throw new std::out_of_range("record not found");

}

std::vector<std::vector<std::string>> StructuredDataRegion::getTable() const {
  auto rows = recordCount();
  auto cols = recordSize();

  if (rows > 0 && cols > 0) {
    std::vector<std::vector<std::string>> result(rows, std::vector<std::string>(cols, ""));

    for (size_t i = 0; i < rows; ++i) {
      for (size_t j = 0; j < cols; ++j) {
        if (records[i][j] != nullptr)
          result[i][j] = records[i][j]->toString();
      }
    }

    return result;
  }
  return std::vector<std::vector<std::string>>();
}

void StructuredDataRegion::addRecord(const std::vector<pNode>& r) {
  records.emplace_back(r);
}

size_t StructuredDataRegion::recordCount() const noexcept {
  return records.size();
}

size_t StructuredDataRegion::recordSize() const noexcept {
  if (records.size() > 0)
    return (*records.begin()).size();
  else
    return 0;
}

void StructuredDataRegion::cleanup() {
  if (recordCount() == 0)
    return;
  if (recordSize() == 0)
    return;

  std::vector<bool> removeRecord(recordCount(), true);
  std::vector<bool> removeField(recordSize(), true);

  for (size_t i = 0; i < recordCount(); i++) {
    for (size_t j = 0; j < recordSize(); j++) {
      auto node = records[i][j];
      if (node && (node->isImage() || node->isLink() || node->isText())

      ) {  // do not erase this node
        removeField[j] = false;
        removeRecord[i] = false;
      }
    }
  }

  size_t l = 0;
  auto recsEnd = std::remove_if(
      records.begin(), records.end(),
      [&l, removeRecord](const std::vector<pNode> &a)->bool {
        return removeRecord[l++];
      });
  records.erase(recsEnd, records.end());

  for (auto &r : records) {
    l = 0;
    auto rEnd = std::remove_if(r.begin(), r.end(),
                               [&l, removeField](const pNode &a)->bool {
                                 return removeField[l++];
                               });
    r.erase(rEnd, r.end());
  }
}

