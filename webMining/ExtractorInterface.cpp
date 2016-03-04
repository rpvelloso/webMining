/*
 * tExtractInterface.cpp
 *
 *  Created on: 27/07/2014
 *      Author: roberto
 */

#include <iostream>
#include <algorithm>
#include "ExtractorInterface.h"

using namespace std;

#include "node.hpp"

ExtractorInterface::ExtractorInterface() {
}

ExtractorInterface::~ExtractorInterface() {
}

void ExtractorInterface::cleanRegion(vector<vector<pNode> > &recs) {
	if (recs.size() == 0) return;
	if (recs[0].size() == 0) return;

	vector<bool> removeRecord(recs.size(), true);
	vector<bool> removeField(recs[0].size(), true);

	for (size_t i=0;i<recs.size();i++) {
		for (size_t j=0;j<recs[i].size();j++) {
			auto node = recs[i][j];
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
	auto recsEnd = remove_if(recs.begin(), recs.end(),
	[&l, removeRecord](const vector<pNode> &a)->bool{
		return removeRecord[l++];
	});
	recs.erase(recsEnd,recs.end());

	for (auto &r:recs) {
		l = 0;
		auto rEnd = remove_if(r.begin(), r.end(),
		[&l, removeField](const pNode &a)->bool{
			return removeField[l++];
		});
		r.erase(rEnd, r.end());
	}
}

