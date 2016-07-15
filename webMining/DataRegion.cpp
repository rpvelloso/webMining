/*
 * DataRegion.cpp
 *
 *  Created on: 13 de jul de 2016
 *      Author: rvelloso
 */

#include <exception>
#include <algorithm>
#include "DataRegion.hpp"

DataRegion::DataRegion() {
	// TODO Auto-generated constructor stub

}

DataRegion::~DataRegion() {
	// TODO Auto-generated destructor stub
}

const Record DataRegion::getRecord(size_t pos) const {
	if (pos < records.size())
		return records[pos];

	throw new std::out_of_range("record not found");
}

void DataRegion::addRecord(const std::vector<pNode>& r) {
	records.emplace_back(r);
}

size_t DataRegion::recordCount() const noexcept {
	return records.size();
}

size_t DataRegion::recordSize() const noexcept {
	if (records.size() > 0)
		return (*records.begin()).size();
	else
		return 0;
}

void DataRegion::clean() {
	if (recordCount() == 0) return;
	if (recordSize() == 0) return;

	std::vector<bool> removeRecord(recordCount(), true);
	std::vector<bool> removeField(recordSize(), true);

	for (size_t i=0;i<recordCount();i++) {
		for (size_t j=0;j<recordSize();j++) {
			auto node = records[i][j];
			if (node && (
					node->isImage() ||
					node->isLink() ||
					node->isText())

			) { // do not erase this node
				removeField[j]=false;
				removeRecord[i]=false;
			}
		}
	}

	size_t l=0;
	auto recsEnd = std::remove_if(records.begin(), records.end(),
	[&l, removeRecord](const std::vector<pNode> &a)->bool{
		return removeRecord[l++];
	});
	records.erase(recsEnd,records.end());

	for (auto &r:records) {
		l = 0;
		auto rEnd = std::remove_if(r.begin(), r.end(),
		[&l, removeField](const pNode &a)->bool{
			return removeField[l++];
		});
		r.erase(rEnd, r.end());
	}
}

