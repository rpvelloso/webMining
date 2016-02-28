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
#include <vector>
#include <string>
//#include "dom.hpp"
#include "node.hpp"
#include "tExtractInterface.h"
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

class SRDEFilter : public tExtractInterface {
public:
	SRDEFilter();
	virtual ~SRDEFilter();

	void SRDE(pNode, bool);
	const wstring& getTagPathSequence(int = -1);
	tTPSRegion *getRegion(size_t);
	virtual size_t getRegionCount();
	virtual vector<pNode> getRecord(size_t, size_t);
protected:
    void buildTagPath(string, pNode, bool, bool, bool);
	map<long int, tTPSRegion> filter(pNode, bool);
	vector<size_t> locateRecords(tTPSRegion &, double &);
	unordered_map<int,int> symbolFrequency(wstring, set<int> &);
	map<int,int> frequencyThresholds(unordered_map<int,int>);
	double estimatePeriod(vector<double>);
	map<long int, tTPSRegion> detectStructure(unordered_map<long int, tTPSRegion> &);

	virtual void onDataRecordFound(vector<wstring> &, vector<size_t> &, tTPSRegion *);

	unordered_map<string, int> tagPathMap;
	wstring tagPathSequence;
	vector<pNode> nodeSequence;
	int count=0,pathCount=0;

	unordered_map<long int, tTPSRegion> _regions;
	vector<tTPSRegion> regions;
};

#endif /* SRDEFILTER_H_ */
