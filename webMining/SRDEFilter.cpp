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

#include "SRDEFilter.h"

#include <queue>
#include <iostream>
#include <set>
#include <functional>
#include <algorithm>
#include <map>
#include <unordered_map>
#include <list>
#include <vector>
#include <tidy.h>
#include "misc.h"
#include "hsfft.h"
#include "dom.hpp"
//#include "Ckmeans.1d.dp.h"

using namespace std;

SRDEFilter::SRDEFilter(DOM *d) : ExtractorInterface() {
	SRDE(d->body(), true);
}

SRDEFilter::~SRDEFilter() {
}

unordered_map<int,int> SRDEFilter::symbolFrequency(wstring s, unordered_set<int> &alphabet) {
	unordered_map<int, int> symbolCount;

	// compute symbol frequency
	for (size_t i=0;i<s.size();i++) {
		if (s[i] != 0) {
			if (alphabet.count(s[i]) == 0) {
				symbolCount[s[i]]=0;
				alphabet.insert(s[i]);
			}
			symbolCount[s[i]]++;
		}
	}
	return symbolCount;
}

map<int,int> SRDEFilter::frequencyThresholds(unordered_map<int,int> symbolCount) {
	map<int,int> thresholds;

	// create sorted list of frequency thresholds
	for (auto i=symbolCount.begin();i!=symbolCount.end();i++)
		thresholds[(*i).second] = (*i).first;

	return thresholds;
}

void SRDEFilter::buildTagPath(string s, pNode n, bool css) {
	static vector<string> styleAttr = {"style", "class", "bgcolor", "width", "height", "align", "valign", "halign"};
	string tagStyle;

	if (s == "") {
		tagPathMap.clear();
		tagPathSequence.clear();
		nodeSequence.clear();
	}

	for (auto attrName:styleAttr) {
		auto attrValue = n->getAttr(attrName);
		if (attrValue != "")
			tagStyle = tagStyle + " " + attrName + "=" + attrValue;
	}

	auto tagName = n->tagName();
	if (tagName != "") {
		if (css && (tagStyle != "")) s = s + "/" + tagName + tagStyle;
		else s = s + "/" + tagName;
	}

	if (tagPathMap.count(s) == 0)
		tagPathMap[s] = tagPathMap.size()+1;

	tagPathSequence = tagPathSequence + wchar_t(tagPathMap[s]);
	nodeSequence.push_back(n);

	for (auto child = n->child(); child; child = child->next())
		buildTagPath(s,child,css);
}

map<long int, tTPSRegion> SRDEFilter::detectStructure(unordered_map<long int, tTPSRegion> &r) {
	float angCoeffThreshold=0.17633; // 10 degrees
	long int sizeThreshold = (tagPathSequence.size()*3)/100; // % page size
	map<long int,tTPSRegion> structured;

	for (auto i=r.begin();i!=r.end();i++) {
		(*i).second.lc = linearRegression((*i).second.tps);

		cerr << "size: " << (*i).second.len << " ang.coeff.: " << (*i).second.lc.a << endl;

		if (
			(abs((*i).second.lc.a) < angCoeffThreshold) && // test for structure
			((*i).second.len >= sizeThreshold) // test for size
			)
			structured.insert(*i);
	}
	return structured;
}

map<long int, tTPSRegion> SRDEFilter::filter(pNode n, bool css, unordered_map<long int, tTPSRegion> &regs) {
	unordered_set<int> alphabet;
	wstring s;
	long int lastRegPos = -1;
	map<long int, tTPSRegion> ret;

	buildTagPath("",n,css);
	s = tagPathSequence;

	auto symbolCount = symbolFrequency(s,alphabet);
	auto thresholds = frequencyThresholds(symbolCount);
	auto threshold = thresholds.begin();
	threshold++;
	//threshold++;

	do {
		regs.clear();
		threshold++;

		cerr << "threshold: " << (*threshold).first << " / " << (*(thresholds.rbegin())).first << endl;

		for (size_t i=0;i<s.size();i++)
			if (symbolCount[s[i]] <= (*threshold).first) s[i]=0;

		bool regionOpened=false;
		int regionStart=0;
		int regionEnd=0;

		for (size_t i=0;i<s.size();i++) {
			if (!regionOpened) {
				 if (s[i] != 0) {
					regionOpened = true;
					regionStart = i;
				}
			} else {
				if ((s[i] == 0) || (i == s.size()-1)) {
					tTPSRegion r;

					regionOpened = false;
					regionEnd = i-1;
					if (i == s.size()-1) regionEnd++;
					r.pos = regionStart;
					r.len = regionEnd - regionStart + 1;
					if (r.len > 3) {
						r.tps = tagPathSequence.substr(r.pos,r.len);

						if (lastRegPos != -1) {
							unordered_set<int> palpha,alpha,intersect;

							symbolFrequency(regs[lastRegPos].tps,palpha);
							symbolFrequency(r.tps,alpha);

							set_intersection(palpha.begin(),palpha.end(),alpha.begin(),alpha.end(),inserter(intersect,intersect.begin()));

							if (!intersect.empty()) {
								regs[lastRegPos].len = regionEnd - regs[lastRegPos].pos + 1;
								regs[lastRegPos].tps = tagPathSequence.substr(regs[lastRegPos].pos,regs[lastRegPos].len);
								//cout << "merge " << _regions[lastRegPos].pos << " " << _regions[lastRegPos].len << endl;
								continue;
							}
						}
						regs[regionStart] = r;
						lastRegPos = regionStart;
						//cout << "new   " << _regions[lastRegPos].pos << " " << _regions[lastRegPos].len << endl;
					}
				}
			}
		}

		if (regs.size() == 0) {
			regs[0].content = true;
			regs[0].len = tagPathSequence.size();
			regs[0].pos = 0;
			regs[0].nodeSeq = nodeSequence;
			regs[0].tps = tagPathSequence;
		}

		/*buildTagPath("",n,false); // rebuild TPS without CSS to increase periodicity
		for (auto i=_regions.begin();i!=_regions.end();i++) {
			(*i).second.tps = tagPathSequence.substr((*i).second.pos,(*i).second.len);
		}*/
		ret=detectStructure(regs);
	} while ((ret.size()==0) && (threshold != thresholds.end()));

	return ret;
}

void SRDEFilter::SRDE(pNode n, bool css) {
	vector<size_t> recpos;
	vector<wstring> m;
	map<long int, tTPSRegion> structured;
	double period;
	unordered_map<long int, tTPSRegion> regs;

	structured=filter(n,css, regs); // segment page and detects structured regions
	//structured = tagPathSequenceFilter(n,css);

	for (auto i=structured.begin();i!=structured.end();i++) {
		auto firstNode = nodeSequence.begin()+(*i).first;
		auto lastNode = firstNode + (*i).second.len;

		regs[(*i).first].nodeSeq.assign(firstNode,lastNode);
		m.clear();
		recpos.clear();

		cerr << "TPS: " << endl;
		for (size_t j=0;j<regs[(*i).first].tps.size();j++)
			cerr << regs[(*i).first].tps[j] << " ";
		cerr << endl;

		// identify the start position of each record
		recpos = locateRecords(regs[(*i).first],period);

		// consider only leaf nodes when performing field alignment
		auto j = regs[(*i).first].nodeSeq.begin();
		auto t=regs[(*i).first].tps.begin();
		size_t k=0;
		while (k < regs[(*i).first].tps.size()) {
			bool erase = (!(*j)->isImage() && !(*j)->isLink() && !(*j)->isText());
			for (size_t w=0;w<recpos.size();w++) {
				if (recpos[w] == k) {
					erase=false;
					break;
				}
			}

			if (erase) {
				j = regs[(*i).first].nodeSeq.erase(j);
				t = regs[(*i).first].tps.erase(t);
				for (size_t w=0;w<recpos.size();w++) {
					if (recpos[w] > k) recpos[w]--;
				}
			} else {
				j++;
				t++;
				k++;
			}
		}

		// create a sequence for each record found
		int prev=-1;
		size_t max_size=0;
		for (size_t j=0;j<recpos.size();j++) {
			if (prev==-1) prev=recpos[j];
			else {
				if ((recpos[j]-prev) > 0) {
					m.push_back(regs[(*i).first].tps.substr(prev,recpos[j]-prev));
					max_size = max(recpos[j]-prev,max_size);
					prev = recpos[j];
				}
			}
		}
		if (prev != -1) {
			if (period > max_size) max_size = period;
			m.push_back(regs[(*i).first].tps.substr(prev,max_size));
		}

		if (m.size()) {
			// align the records (one alternative to 'center star' algorithm is ClustalW)
			//align(m);
			regs[(*i).first].score = centerStar(m);

			// and extracts them
			onDataRecordFound(m,recpos,&regs[(*i).first]);
		}
	}

	// remove regions with only a single record
	for (auto i=structured.begin();i!=structured.end();) {
		if (regs[(*i).first].records.size() < 2) structured.erase(i++);
		else {
			auto stddev = regs[(*i).first].stddev;
			auto recCount = regs[(*i).first].records.size();
			auto recSize = regs[(*i).first].records[0].size();

			regs[(*i).first].score = //stddev;
					((min((double)recCount,(double)recSize) /
					max((double)recCount,(double)recSize))) * stddev * ((double)regs[(*i).first].len / (double)tagPathSequence.size());
					//(double)recCount * (double)recSize * stddev;
			++i;
		}
	}

	if (structured.size()) {
		vector<double> ckmeansScoreInput;
		ClusterResult scoreResult;

		ckmeansScoreInput.push_back(0);
		for (auto i=structured.begin();i!=structured.end();i++) {
			ckmeansScoreInput.push_back(regs[(*i).first].score);
		}
		scoreResult = kmeans_1d_dp(ckmeansScoreInput,2,2);

		auto j=++(scoreResult.cluster.begin());
		for (auto i=structured.begin();i!=structured.end();i++,j++) {
			regs[(*i).first].content = (((*j) == 2) || (scoreResult.nClusters < 2));

			// restore the original region's tps
			regs[(*i).first].tps = tagPathSequence.substr((*i).first,(*i).second.len);
		}

		// -----

		/*ckmeansScoreInput.clear();
		ckmeansScoreInput.push_back(0);
		for (auto i=structured.begin();i!=structured.end();i++) {
			ckmeansScoreInput.push_back(_regions[(*i).first].stddev);
		}
		scoreResult = kmeans_1d_dp(ckmeansScoreInput,2,2);

		j=++(scoreResult.cluster.begin());
		for (auto i=structured.begin();i!=structured.end();i++,j++)
			_regions[(*i).first].content |= (((*j) == 2) || (scoreResult.nClusters < 2));*/
	}

	for (auto i = regs.begin();i!=regs.end();) {
		if ((*i).second.records.size() < 2) regs.erase(i++);
		else ++i;
	}

	regions.clear();

	for (auto i=regs.begin();i!=regs.end();i++) {
		(*i).second.pos = (*i).first;
		regions.push_back((*i).second);
	}

    sort(regions.begin(),regions.end(),[](const tTPSRegion &a, const tTPSRegion &b) {
    		return (a.score > b.score);
    });
}

vector<size_t> SRDEFilter::locateRecords(tTPSRegion &region, double &period) {
	wstring s = region.tps;
	vector<double> signal(s.size());
	float avg;
	set<float> candidates;
	vector<size_t> recpos,ret;
	unordered_set<int> alphabet;
	unordered_map<int,int> reencode;
	double estPeriod,estFreq;
	double maxCode=0,maxScore=0;

	// reencode signal
	symbolFrequency(s,alphabet);
	for (size_t i=0,j=0;i<s.size();i++) {
		if (alphabet.count(s[i]) > 0) {
			alphabet.erase(s[i]);
			j++;
			reencode[s[i]]=j;
		}
		//signal[i]=reencode[s[i]];
		signal[i]=s[i];
	}
	avg = mean(signal);

	// remove DC & compute signal's std.dev.
	region.stddev = 0;
	for (size_t i=0;i<s.size();i++) {
		signal[i] = signal[i] - avg;
		region.stddev += signal[i]*signal[i];
		if (signal[i] < 0) candidates.insert(signal[i]);
		if (abs(signal[i]) > maxCode) maxCode = abs(signal[i]);
	}
	region.stddev = sqrt(region.stddev/max((double)1,(double)(s.size()-2)));

	estPeriod = estimatePeriod(signal);
	estFreq = ((double)signal.size() / estPeriod);

	cout << endl;

	for (auto value = candidates.begin(); value != candidates.end(); value ++ ) {
		double stddev,avgsize;

		recpos.clear();
		stddev=0;
		avgsize=0;

		for (size_t i=0;i<s.size();i++) {
			if ((signal[i] == *value))
				recpos.push_back(i);
		}

		if (recpos.size() > 1) {
			for (size_t i=1;i<recpos.size();i++) {
				avgsize += (recpos[i] - recpos[i-1]);
			}
			avgsize /= (float)(recpos.size()-1);

			for (size_t i=1;i<recpos.size();i++) {
				float diff = (float)(recpos[i] - recpos[i-1])-avgsize;
				stddev += (diff*diff);
			}
			stddev = sqrt(stddev/max((float)(recpos.size()-2),(float)1));

			double regionCoverage = min(avgsize*(double)recpos.size()/(double)signal.size(), (double)1);
			double estRegionCoverage = min(estPeriod*estFreq/(double)signal.size(), (double)1);
			double regCoverageRatio = min(regionCoverage,estRegionCoverage)/max(regionCoverage,estRegionCoverage);
			double freqRatio =
					min( (double)recpos.size() ,estFreq) /
					max( (double)recpos.size() ,estFreq);
			if (stddev>1) avgsize /= stddev; // SNR
			double recSizeRatio = min( avgsize, estPeriod )/max( avgsize, estPeriod );
			//recSizeRatio = sqrt(recSizeRatio);
			double tpcRatio = (double)abs(*value)/maxCode; // DNR

			double score = (regCoverageRatio + freqRatio + recSizeRatio + tpcRatio)/(double)4;
			//double score = regionCoverage * recCountRatio * recSizeRatio * tpcRatio;
			if (score > maxScore) {
				maxScore = score;
				ret = recpos;
			}
			printf("value=%.2f, cov=%.2f, #=%.2f, size=%.2f, t=%.2f, s=%.4f - %.2f\n",*value,regCoverageRatio,freqRatio,recSizeRatio,tpcRatio,score,estPeriod);
		}
	}

	return ret.size()?ret:recpos;
}

const wstring& SRDEFilter::getTagPathSequence(int dr) {
	if (dr < 0)
		return tagPathSequence;
	else {
		size_t i = dr;

		if (i < regions.size())
			return regions[dr].tps;
		else
			return tagPathSequence;
	}
}

tTPSRegion *SRDEFilter::getRegion(size_t r) {
	if (r < regions.size())
		return &regions[r];
	return NULL;
}

size_t SRDEFilter::getRegionCount() {
	return regions.size();
}

vector<pNode> SRDEFilter::getRecord(size_t dr, size_t rec) {
	if (dr < regions.size()) {
		if (rec < regions[dr].records.size())
			return regions[dr].records[rec];
	}
	return vector<pNode>(0);
}

/*double tTPSFilter::estimatePeriod(vector<double> signal) {
	size_t N = (signal.size() + (signal.size()%2));
	double maxPeak=-INFINITY;
	size_t peakFreq=1;

	if (signal.size() != N) { // repeat last sample when signal size is odd
		signal.resize(N);
		signal[N-1] = signal[N-2];
	}

	for (size_t i = 0; i < N; i++) { // apply 'welch' window to signal
		signal[i] *= (1.0-(((double)i-0.5*(double)(N-1))
					/(0.5*(double)(N+1)))
					*(((double)i-0.5*(double)(N-1))
					/(0.5*(double)(N+1))));
	}


	auto spectrum = fft(signal);

	for (size_t i = 4; i < (N/4)-1; i++) {
		if (spectrum[i] > maxPeak) {
			maxPeak = spectrum[i];
			peakFreq = i;
		}
	}

	return ((double)N/(double)peakFreq);
}*/

#define NUM_PEAKS 15

double SRDEFilter::estimatePeriod(vector<double> signal) {
	size_t N = (signal.size() + (signal.size()%2));
	double maxPeak=-INFINITY;

	if (signal.size() != N) { // repeat last sample when signal size is odd
		signal.resize(N);
		signal[N-1]=signal[N-2];
	}

	auto spectrum = fft(signal);
	auto xcorr = autoCorrelation(signal);

	multimap<double, size_t> candidatePeriods;
	for (size_t i=0;i<N;i++) {
		candidatePeriods.insert(make_pair(xcorr[i],i));
	}

	double period = ceil((double)N/(double)(*(candidatePeriods.begin())).second);
	size_t j=0;
	for (auto i = candidatePeriods.rbegin(); i != candidatePeriods.rend(); i++) {
		if ( ((*i).second > 1) && ((*i).second < N) ) {
			size_t f = ceil((double)N/(double)(*i).second);
			auto peak = spectrum[f];
			if (peak > maxPeak) {
				maxPeak = peak;
				period = (*i).second;
			}
			j++;
			if (j == NUM_PEAKS) break;
		}
	}

	return period;
}

void SRDEFilter::onDataRecordFound(vector<wstring> &m, vector<size_t> &recpos, tTPSRegion *reg) {
	if ((m.size() == 0) || (recpos.size() == 0)) return;// -1;

	int rows=m.size(),cols=m[0].size();
	vector<pNode> rec;
	bool keepRec;

	for (int i=0;i<rows;i++) {
		keepRec = false;

		rec.clear();
		cerr << endl;
		for (int j=0,k=0;j<cols;j++) {
			if (m[i][j] != 0) {
				rec.push_back(reg->nodeSeq[recpos[i]+k]);

				auto tagName = reg->nodeSeq[recpos[i]+k]->tagName();
				if (tagName != "") {
					cerr << tagName << "[" <<
							reg->nodeSeq[recpos[i]+k]->toString() << "];";
				}
				k++;
				if (j>0) keepRec=true;
			} else rec.push_back(NULL);
		}
		if (keepRec) reg->records.push_back(rec);
	}
	cleanRegion(reg->records);
	cerr << endl;
}
