/*
 * DataRegion.h
 *
 *  Created on: 13 de jul de 2016
 *      Author: rvelloso
 */

#ifndef DATAREGION_HPP_
#define DATAREGION_HPP_

#include <vector>
#include "Node.hpp"

using Record = std::vector<pNode>;

class DataRegion {
public:
	DataRegion();
	virtual ~DataRegion();
	const Record getRecord(size_t pos) const;
	void addRecord(const std::vector<pNode> &r);
	size_t recordCount() const noexcept;
	size_t recordSize() const noexcept;
	void clean();
private:
	std::vector<Record> records;
};

#endif /* DATAREGION_HPP_ */
