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

	vector<bool> record(recs.size(), false);
	vector<bool> field(recs[0].size(), false);

	for (size_t i=0;i<recs.size();i++) {
		for (size_t j=0;j<recs[i].size();j++) {
			auto node = recs[i][j];
			if (node && (
					node->isImage() ||
					node->isLink() ||
					node->isText())

			) { // do not erase this node
				field[j]=true;
				record[i]=true;
			}
		}
	}

	size_t l=0;
	auto recsEnd = remove_if(recs.begin(), recs.end(),
	[&l, record](const vector<pNode> &a)->bool{
		return !record[l++];
	});
	recs.erase(recsEnd,recs.end());

	for (auto &r:recs) {
		l = 0;
		auto rEnd = remove_if(r.begin(), r.end(),
		[&l, field](const pNode &a)->bool{
			return !field[l++];
		});
		r.erase(rEnd, r.end());
	}
}

