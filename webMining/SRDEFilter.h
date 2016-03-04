/*
    Copyright 2011 Roberto Panerai Velloso.

    This file is part of libsockets.

    libsockets is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libsockets is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libsockets.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SRDEFILTER_H_
#define SRDEFILTER_H_

#include <map>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>
#include "ExtractorInterface.h"
#include "node.hpp"
#include "misc.h"

using namespace std;

struct tTPSRegion {
	long int len,pos;
	wstring tps;
	vector<pNode> nodeSeq;
	tLinearCoeff lc;
	vector<vector<pNode> > records;
	double stddev=0;
	double score=0;
	bool content=false;
};

class SRDEFilter : public ExtractorInterface {
public:
	SRDEFilter(DOM *d);
	virtual ~SRDEFilter();

	const wstring& getTagPathSequence(int);
	tTPSRegion *getRegion(size_t);
	size_t getRegionCount();
	vector<pNode> getRecord(size_t, size_t);
	void printTagPathSequence();
protected:
	SRDEFilter() = delete;
	void SRDE(pNode, bool);
    void buildTagPath(string, pNode, bool);
	vector<long int> segment(pNode, bool, unordered_map<long int, tTPSRegion> &);
	set<size_t> locateRecords(tTPSRegion &);
	unordered_map<int,int> symbolFrequency(wstring, set<int> &);
	map<int,int> frequencyThresholds(unordered_map<int,int>);
	double estimatePeriod(vector<double>);
	vector<long int> detectStructure(unordered_map<long int, tTPSRegion> &);

	virtual void onDataRecordFound(vector<wstring> &, set<size_t> &, tTPSRegion &);

	unordered_map<string, int> tagPathMap;
	wstring tagPathSequence;
	vector<pNode> nodeSequence;

	vector<tTPSRegion> regions;
};

#endif /* SRDEFILTER_H_ */
