/*
 * DataRegion.h
 *
 *  Created on: 13 de jul de 2016
 *      Author: rvelloso
 */

#ifndef STRUCTUREDDATAREGION_HPP_
#define STRUCTUREDDATAREGION_HPP_

#include <vector>
#include "Node.hpp"
#include "DataRegion.hpp"

using Record = std::vector<pNode>;

class StructuredDataRegion : public DataRegion {
 public:
  StructuredDataRegion();
  virtual ~StructuredDataRegion();
  const Record getRecord(size_t pos) const;
  void addRecord(const std::vector<pNode> &r);
  size_t recordCount() const noexcept;
  size_t recordSize() const noexcept;
  void cleanup();
 private:
  std::vector<Record> records;
};

#endif /* STRUCTUREDDATAREGION_HPP_ */
