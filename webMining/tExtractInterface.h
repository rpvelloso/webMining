/*
 * tExtractInterface.h
 *
 *  Created on: 27/07/2014
 *      Author: roberto
 */

#ifndef TEXTRACTINTERFACE_H_
#define TEXTRACTINTERFACE_H_

#include <vector>
#include <deque>
#include "node.hpp"

using namespace std;

#define LEAF(n) ((n->getNodes().size()==0) && (n->getText()!=""))

class tExtractInterface {
public:
	tExtractInterface();
	virtual ~tExtractInterface();

	virtual vector<pNode> getRecord(size_t, size_t) = 0;
	virtual size_t getRegionCount() = 0;

	void cleanRegion(vector<vector<pNode> > &);
};

#endif /* TEXTRACTINTERFACE_H_ */
