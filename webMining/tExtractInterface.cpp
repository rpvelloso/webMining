/*
 * tExtractInterface.cpp
 *
 *  Created on: 27/07/2014
 *      Author: roberto
 */

#include "node.hpp"
#include "tExtractInterface.h"

tExtractInterface::tExtractInterface() {
}

tExtractInterface::~tExtractInterface() {
}

void tExtractInterface::cleanRegion(vector<vector<pNode> > &recs) {
	if (!(recs.size())) return;
	if (!(recs[0].size())) return;

	bool field[recs[0].size()];

	for (size_t i=0;i<recs[0].size();i++) field[i]=false;

	for (size_t i=0;i<recs.size();i++) {
		for (size_t j=0;j<recs[i].size();j++) {
			if (recs[i][j] && (
					recs[i][j]->isImage() ||
					recs[i][j]->isLink() ||
					recs[i][j]->isText())

			) field[j]=true;
		}
	}

	int k=0;
	for (size_t j=0;j<recs[0].size();j++,k++) {
		if (!field[j]) {
			for (size_t i=0;i<recs.size();i++) {
				recs[i].erase(recs[i].begin()+k);
			}
			k--;
		}
	}
}
