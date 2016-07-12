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
	cerr << endl << "Processing " << d->getURI() << " ..." << endl;
	SRDE(d->body(), true);
}

SRDEFilter::~SRDEFilter() {
}

unordered_map<int,int> SRDEFilter::symbolFrequency(wstring s, set<int> &alphabet) {
	unordered_map<int, int> symbolCount;

	alphabet.clear();
	// compute symbol frequency
	for (auto c:s) {
		if (c != 0) {
			if (alphabet.count(c) == 0) {
				symbolCount[c]=0;
				alphabet.insert(c);
			}
			symbolCount[c]++;
		}
	}
	return symbolCount;
}

map<int,int> SRDEFilter::frequencyThresholds(unordered_map<int,int> symbolCount) {
	map<int,int> thresholds;

	// create sorted list of frequency thresholds
	for (auto i:symbolCount)
		thresholds[i.second] = i.first;

	return thresholds;
}

void SRDEFilter::buildTagPath(string s, pNode n, bool css) {
	static vector<string> styleAttr = {
			"style", "class", "color", "bgcolor", "width", "height",
			"align", "valign", "halign", "colspan", "rowspan"
	};

	string tagStyle;

	if (s == "") {
		tagPathMap.clear();
		tagPathSequence.clear();
		nodeSequence.clear();
	}

	for (auto attrName:styleAttr) {
		auto attrValue = n->getAttr(attrName);

		if (attrName == "class") // consider only tag's first class
			attrValue = stringTok(attrValue," \r\n\t");

		if (attrValue != "")
			tagStyle = tagStyle + " " + attrName + "=" + attrValue;
	}

	string tagName;

	if (n->isText())
		tagName = "#text";
	else
		tagName = n->getTagName();

	if (tagName != "") {
		if (css && (tagStyle != "")) s = s + "/" + tagName + tagStyle;
		else s = s + "/" + tagName;

		if (tagPathMap.count(s) == 0)
			tagPathMap[s] = tagPathMap.size()+1;

		tagPathSequence = tagPathSequence + wchar_t(tagPathMap[s]);
		nodeSequence.push_back(n);

		for (auto child = n->child(); child != nullptr; child = child->next())
			buildTagPath(s,child,css);
	} else {
		cout << "empty tagName!!! : " << n->toString() << endl;
		//terminate();
	}
}

vector<long int> SRDEFilter::detectStructure(unordered_map<long int, tTPSRegion> &regs) {
	float angCoeffThreshold=0.17453; // 10 degrees
	long int sizeThreshold = (tagPathSequence.size()*3)/100; // % page size
	vector<long int> structured;

	for (auto &r:regs) {
		r.second.lc = linearRegression(r.second.tps);

		cerr << "size: " << r.second.size() << " ang.coeff.: " << r.second.lc.a << endl;

		if (
			(abs(r.second.lc.a) < angCoeffThreshold) && // test for structure
			(r.second.size() >= sizeThreshold) // test for size
			)
			structured.push_back(r.first);
	}
	return structured;
}

list<pair<size_t,size_t> > SRDEFilter::segment_difference(const vector<double> &diff) {
	list<pair<size_t,size_t> > ret;
	size_t start = 0, end = 0;

	for (size_t i = 0; i < diff.size(); ++i) {
		if (diff[i] != 0) {
			if ((end - start + 1) > 3)
				ret.push_back(make_pair(start,end));

			start = end = i+1;
		} else
			end = i;
	}

	if (start != end)
		ret.push_back(make_pair(start,end));

	return ret;
}

void SRDEFilter::merge_regions(list<pair<size_t,size_t> > &regions) {
// merge regions with common alphabet
	auto r = ++regions.begin();
	for (; r != regions.end(); ++r) {
		set<int> alpha, palpha, intersect, setUnion;
		auto prev = r;
		--prev;

		// avoid merging regions too far apart
		/*
		if ((r->pos - (prev->pos+prev->len-1)) > 5)
			continue;
		*/
		auto tps = tagPathSequence.substr((*r).first,(*r).second - (*r).first + 1);
		auto ptps = tagPathSequence.substr((*prev).first,(*prev).second - (*prev).first + 1);
		symbolFrequency(tps, alpha);
		symbolFrequency(ptps, palpha);
		set_intersection(
				palpha.begin(),palpha.end(),
				alpha.begin(),alpha.end(),
				inserter(intersect,intersect.begin()));

		set_union(
				palpha.begin(),palpha.end(),
				alpha.begin(),alpha.end(),
				inserter(setUnion,setUnion.begin()));
		if (!intersect.empty()) {
		//if (((double)intersect.size()/(double)setUnion.size()) > 0.05) {
			(*r).first = (*prev).first;
			regions.erase(prev);
		}
	}
}

vector<long int> SRDEFilter::segment(pNode n, bool css, unordered_map<long int, tTPSRegion> &regs) {
	buildTagPath("",n,css);
	auto s = tagPathSequence;
	list<tTPSRegion> regions;

	// contour
	s = contour(s);

	// 1st difference of contour
	auto diff = difference(s);

	// region segmentation
	auto segs = segment_difference(diff);

	merge_regions(segs);

	for (auto i = segs.begin(); i != segs.end();) {
		auto seg = *i;
		auto tps = tagPathSequence.substr(seg.first, seg.second-seg.first+1);
		reverse(tps.begin(), tps.end());
		auto c = contour(tps);
		auto d = difference(c);
		auto ss = segment_difference(d);
		if (ss.size() > 1) {
			reverse(ss.begin(), ss.end());
			for (auto j:ss) {
				j.first = (tps.size()-1-j.first) + seg.first;
				j.second = (tps.size()-1-j.second) + seg.first;
				swap(j.first,j.second);
				segs.insert(i,j);
			}
			i = segs.erase(i);
		} else
			++i;
	}

	merge_regions(segs);

	for (auto seg:segs) {
		tTPSRegion r;

		r.setStartPos(seg.first);
		r.setEndPos(seg.second);
		r.tps = tagPathSequence.substr(r.getStartPos(),r.size());
		regions.push_back(r);
	}

	// correct region boundaries
	for (auto &r:regions) {
		set<int> alpha;
		//size_t maxTPC, minTPC;

		symbolFrequency(r.tps, alpha);
		/*minTPC = *alpha.begin();
		maxTPC = *alpha.rbegin();*/

		// pad left
		//while (r.getStartPos() > 0 && tagPathSequence[r.getStartPos()-1] >= minTPC && tagPathSequence[r.getStartPos()-1] <= maxTPC) {
		while (r.getStartPos() > 0 && alpha.count(tagPathSequence[r.getStartPos()-1]) > 0) {
			r.shiftStartPos(-1);
		}

		// pad right
		//while ((static_cast<size_t>(r.getEndPos()) < tagPathSequence.size()-1) && tagPathSequence[r.getEndPos()+1] >= minTPC && tagPathSequence[r.getEndPos()+1] <= maxTPC) {
		while ((static_cast<size_t>(r.getEndPos()) < tagPathSequence.size()-1) && alpha.count(tagPathSequence[r.getEndPos()+1]) > 0) {
			r.shiftEndPos(+1);
		}

		r.tps = tagPathSequence.substr(r.getStartPos(),r.size());
	}

	regs.clear();
	for (auto r:regions)
		regs[r.getStartPos()] = r;

	return detectStructure(regs);
}
/*
vector<long int> SRDEFilter::segment(pNode n, bool css, unordered_map<long int, tTPSRegion> &regs) {
	set<int> alphabet;
	wstring s;
	long int lastRegPos = -1;
	vector<long int> ret;

	buildTagPath("",n,css);
	s = tagPathSequence;

	auto symbolCount = symbolFrequency(s,alphabet);
	auto thresholds = frequencyThresholds(symbolCount);
	auto threshold = thresholds.begin();

	while ((*threshold).first < (*(thresholds.rbegin())).first*0.10)
		threshold++;

	do {
		cerr << "threshold: " << (*threshold).first << " / " << (*(thresholds.rbegin())).first << endl;

		regs.clear();

		for (size_t i = 0; i < s.size(); ++i)
			if (symbolCount[s[i]] <= (*threshold).first)
				s[i]=0;

		bool regionOpened=false;
		int regionStart=0;
		int regionEnd=0;

		for (size_t i=0; i < s.size(); i++) {
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
							set<int> palpha,alpha,intersect;

							symbolFrequency(regs[lastRegPos].tps,palpha);
							symbolFrequency(r.tps,alpha);

							set_intersection(
									palpha.begin(),palpha.end(),
									alpha.begin(),alpha.end(),
									inserter(intersect,intersect.begin()));

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

		//buildTagPath("",n,false); // rebuild TPS without CSS to increase periodicity
		//for (auto &r:regs)
			//r.second.tpsClean = tagPathSequence.substr(r.second.pos, r.second.len);
		ret=detectStructure(regs);
		++threshold;
	} while ((ret.size()==0) && (threshold != thresholds.end()));

	return ret;
}
*/
void SRDEFilter::SRDE(pNode n, bool css) {
	vector<long int> structured;
	unordered_map<long int, tTPSRegion> regs;

	structured=segment(n,css, regs); // segment page and detects structured regions
	//structured = tagPathSequenceFilter(n,css);

	for (auto i:structured) {
		vector<wstring> m;
		auto &region = regs[i];
		auto firstNode = nodeSequence.begin()+i;
		auto lastNode = firstNode + region.size();

		region.nodeSeq.assign(firstNode,lastNode);

		cerr << "TPS: " << endl;
		for (auto j:region.tps)
			cerr << j << " ";
		cerr << endl;

		// identify the start position of each record
		auto recpos = locateRecords(region);

		// consider only leaf nodes when performing field alignment
		vector<bool> remove(region.tps.size());
		size_t pos = 0;

		auto nodeSeqEnd = remove_if(region.nodeSeq.begin(), region.nodeSeq.end(),
			[&remove, recpos, &pos](pNode n)->bool{
				remove[pos] = !(n->isImage() || n->isLink() || n->isText());
				remove[pos] = remove[pos] && (recpos.count(pos) == 0);
				return remove[pos++]; // remove node if not content and not record start
			});

		pos = 0;
		auto tpsEnd = remove_if(region.tps.begin(), region.tps.end(),
			[remove, &pos](wchar_t c)->bool{
				return remove[pos++];
			});

		// adjust records start position after removing nodes
		unordered_map<size_t, size_t> newRecPos;
		for (auto r:recpos)
			newRecPos[r] = r;

		for (size_t i = 0; i < remove.size(); ++i) {
			if (remove[i]) {
				for (auto rp = upper_bound(recpos.begin(), recpos.end(), i); rp != recpos.end(); ++rp)
					--newRecPos[*rp];
			}
		}

		// commit changes
		recpos.clear();
		for (auto r:newRecPos)
			recpos.insert(r.second);

		region.nodeSeq.erase(nodeSeqEnd, region.nodeSeq.end());
		region.tps.erase(tpsEnd, region.tps.end());

		// create a sequence for each record found
		size_t max_size=0;
		if (recpos.size() > 1) {
			auto prev = recpos.begin();
			for (auto rp = ++(recpos.begin()); rp != recpos.end(); ++rp, ++prev) {
				if (((*rp)-(*prev)) > 0) {
					m.push_back(region.tps.substr(*prev,(*rp)-(*prev)));
					max_size = max((*rp)-(*prev),max_size);
				}
			}

			if (*prev < region.tps.size()-1)
				m.push_back(region.tps.substr(*prev,max_size));
		}

		if (m.size() > 1 && m[0].size() > 1) {
			// align the records (one alternative to 'center star' algorithm is ClustalW)
			//align(m);
			cerr << "1-RxC = " << m.size() << " " << m[0].size() << endl;
			region.score = centerStar(m);
			cerr << "2-RxC = " << m.size() << " " << m[0].size() << endl;

			// and extracts them
			onDataRecordFound(m,recpos,region);
		}
	}

	// remove regions with only a single record or single column
	auto struEnd = remove_if(structured.begin(), structured.end(), [&regs](long int i)->bool{
		return (regs[i].records.size() < 2) || (regs[i].records[0].size() < 2);
	});
	structured.erase(struEnd, structured.end());

	if (structured.size()) {
		// compute content score
		float tpsSize = tagPathSequence.size();
		float tpsCenter = (tpsSize - 1) / 2;
		float maxDistance = tpsSize / 2;

		for (auto i:structured) {
			float recCount = regs[i].records.size();
			float recSize = regs[i].records[0].size();
			/*auto stddev = regs[i].stddev;*/
			float regionCenter = regs[i].getStartPos() + (regs[i].size()/2);

			/*regs[i].score = log(//stddev;
					((min((double)recCount,(double)recSize) /
					max((double)recCount,(double)recSize))) *
					stddev *
					((double)regs[i].len / (double)tagPathSequence.size()));
					//(double)recCount * (double)recSize * stddev;
			 */

			float positionScore = 1-(abs(tpsCenter - regionCenter)/maxDistance);
			float sizeScore = regs[i].size()/tpsSize;
			float recScore = min(recCount,recSize) / max(recCount,recSize);

			regs[i].score = (positionScore + sizeScore + recScore)/3;
		}

		// k-mean clustering to identify content
		vector<double> ckmeansScoreInput;
		ckmeansScoreInput.push_back(0);
		for (auto i:structured)
			ckmeansScoreInput.push_back(regs[i].score); // cluster by score
		auto scoreResult = kmeans_1d_dp(ckmeansScoreInput,2,2);

		auto j=++(scoreResult.cluster.begin());
		for (auto i:structured) {
			regs[i].content = (((*j) == 2) || (scoreResult.nClusters < 2));

			// restore the original region's tps
			regs[i].tps = tagPathSequence.substr(i,regs[i].size());
			++j;
		}

		/*ckmeansScoreInput.clear();
		ckmeansScoreInput.push_back(0);
		for (auto i:structured)
			ckmeansScoreInput.push_back(regs[i].stddev); // cluster by stddev
		scoreResult = kmeans_1d_dp(ckmeansScoreInput,2,2);

		j=++(scoreResult.cluster.begin());
		for (auto i:structured) {
			regs[i].content |= (((*j) == 2) || (scoreResult.nClusters < 2));
			++j;
		}*/
	}

	regions.clear();

	for (auto r:regs) {
		if (r.second.records.size() > 1) {
			r.second.setStartPos(r.first);
			regions.push_back(r.second);
		}
	}

    sort(regions.begin(),regions.end(),[](tTPSRegion &a, tTPSRegion &b) {
    	if (a.content == b.content)
    		return (a.score > b.score);
    	else {
    		if (a.content)
    			return true;
    		else
    			return false;
    	}
    });
}

set<size_t> SRDEFilter::locateRecords(tTPSRegion &region) {
	wstring s = region.tps;
	vector<double> signal(s.size());
	float avg;
	set<size_t> recpos,ret;
	set<int> alphabet;
	unordered_map<int,int> reencode;
	double estPeriod,estFreq;
	double maxScore = 0;

	// reencode signal
	symbolFrequency(s,alphabet);
	for (size_t i=0; i < s.size(); ++i) {
		auto c = s[i];

		if (reencode.count(c) == 0)
			reencode[c]=reencode.size()+1;
		signal[i]=reencode[c];
		//signal[i]=c;
	}

	avg = mean(signal);

	set<double> candidates;
	// remove DC & compute signal's std.dev.
	region.stddev = 0;
	for (size_t i=0; i < s.size(); ++i) {
		signal[i] = signal[i] - avg;
		region.stddev += signal[i]*signal[i];
		if (signal[i] < 0) candidates.insert(signal[i]);
	}

	region.stddev = sqrt(region.stddev/max((double)1,(double)(s.size()-2)));

	auto p = estimatePeriod(signal);
	estPeriod = p.first;
	estFreq = ((double)signal.size() / estPeriod);

	cerr << "period: " << estPeriod << "(" << ((double)100.0*p.second) << "%)" << ", freq: " << estFreq << endl;

	for (auto value:candidates) {
		double stddev = 0, avgsize = 0, avgSizeDiff = 0;

		recpos.clear();
		for (size_t i=0; i < s.size(); ++i) {
			if ((signal[i] == value))
				recpos.insert(i);
		}

		if (recpos.size() > 1) {
			auto prev = recpos.begin();
			for (auto rp = ++(recpos.begin()); rp != recpos.end(); ++rp, ++prev) {
				avgsize += (*rp) - (*prev);
				avgSizeDiff += abs(((*rp) - (*prev)) - estPeriod);
			}
			avgsize /= (float)(recpos.size()-1);

			prev = recpos.begin();
			for (auto rp = ++(recpos.begin()); rp != recpos.end(); ++rp, ++prev) {
				float diff = (float)((*rp) - (*prev)) - avgsize;
				stddev += diff * diff;
			}
			stddev = sqrt(stddev/max((float)(recpos.size()-2),(float)1));

			//auto cv = 1.0 - min(stddev, avgsize)/max(stddev, avgsize);
			double regionCoverage =
					(double)(*recpos.rbegin() - *recpos.begin()) /
					(double)(signal.size());

			double freqRatio =
					min( (double)recpos.size() ,estFreq) /
					max( (double)recpos.size() ,estFreq);

			double recSizeRatio = 1 - (
					min( avgSizeDiff, (double)(signal.size()) )/
					max( avgSizeDiff, (double)(signal.size()) ));
					//min( avgsize, estPeriod )/
					//max( avgsize, estPeriod );

			double score = (regionCoverage+recSizeRatio+freqRatio)/3.0;
			fprintf(stderr, "value=%d, cov=%.2f, #=%.2f, size=%.2f, s=%.4f - %.2f\n",int(value+avg),regionCoverage,freqRatio,recSizeRatio,score,estPeriod);

			double haltScore = 0.75;
			if (score > maxScore) {
				maxScore = score;
				ret = recpos;
				if (regionCoverage > haltScore &&
					recSizeRatio > haltScore &&
					freqRatio > haltScore)
					break;
			}
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
	return nullptr;
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

pair<double,double> SRDEFilter::estimatePeriod(vector<double> signal) {
	size_t N = (signal.size() + (signal.size()%2));
	//double maxPeak=-numeric_limits<double>::infinity();

	if (signal.size() != N) { // repeat last sample when signal size is odd
		signal.resize(N);
		signal[N-1]=signal[N-2];
	}

	//auto spectrum = fft(signal);
	auto spectrum = fct(signal);

	double freq = 1;
	double power = spectrum[1], secondPower = spectrum[1];
	for (size_t i = 1; i < spectrum.size()/4; ++i) {
		if (spectrum[i] > power) {
			secondPower = power;
			freq = (double)(i)/2.0;
			power = spectrum[i];
		}
	}
	return make_pair((double)(N)/(double)(freq), ((double)1.0 - (secondPower/power)) /* period confidence */);

	/*auto xcorr = autoCorrelation(signal);

	multimap<double, size_t> candidatePeriods;
	bool considerCandidate = false;
	for (size_t i=0;i<N;i++) {
		if (i > 0 && xcorr[i] > xcorr[i-1])
			considerCandidate = true;

		if (considerCandidate)
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

	return period;*/
}

void SRDEFilter::printTagPathSequence() {
	unordered_map<int, string> tpcMap;

	for (auto tps:tagPathMap)
		tpcMap.insert(make_pair(tps.second, tps.first));

	for (auto tpc:tagPathSequence)
		cout << tpcMap[tpc] << endl;
}

void SRDEFilter::onDataRecordFound(vector<wstring> &m, set<size_t> &recpos, tTPSRegion &reg) {
	if ((m.size() == 0) || (recpos.size() == 0))
		return;

	int rows=m.size(),cols=m[0].size();

	auto rp = recpos.begin();
	for (int i=0;i<rows;i++, ++rp) {
		vector<pNode> rec;

		cerr << endl;
		for (int j=0,k=0;j<cols;j++) {
			if (m[i][j] != 0) {
				auto node = reg.nodeSeq[(*rp)+k];
				rec.push_back(node);

				auto tagName = node->getTagName();
				cerr << "[" << node->toString() << "]; ";

				k++;
			} else rec.push_back(nullptr);
		}
		reg.records.push_back(rec);
	}
	cleanRegion(reg.records);
	cerr << endl;
}
