/*
 * DSREAligner.cpp
 *
 *  Created on: 30 de mar de 2017
 *      Author: rvelloso
 */

#include <set>
#include <vector>
#include <unordered_map>
#include <iostream>
#include "DSREAligner.hpp"
#include "../base/DataRegion.hpp"

DSREAligner::DSREAligner() {
}

DSREAligner::~DSREAligner() {
}

void DSREAligner::align(DSREDataRegion& region, std::set<std::size_t>& recpos) {
	  std::vector<std::wstring> m;
	  std::vector<std::vector<pNode>> n;

	  // create a sequence for each record found
	  auto firstNode = region.getNodeSequence().begin();
	  auto firstTP = region.getTps().begin();
	  size_t max_size = 0;
	  if (recpos.size() > 1) {
	    auto prev = recpos.begin();
	    for (auto rp = ++(recpos.begin()); rp != recpos.end(); ++rp, ++prev) {
	      if (((*rp) - (*prev)) > 0) {
	        m.emplace_back(firstTP + *prev, firstTP + *rp);
	        n.emplace_back(firstNode + *prev, firstNode + *rp);

	        max_size = std::max((*rp) - (*prev), max_size);
	      }
	    }

	    if (*prev < region.getTps().size() - 1) {
	      m.emplace_back(region.getTps().substr(*prev, max_size));
	      n.emplace_back(firstNode + *prev, firstNode + *prev + max_size);
	    }
	  }

	  if (m.size() > 1 && m[0].size() > 1) {

		  extractRecords(m, n, region);
	  }
}

void DSREAligner::extractRecords(std::vector<std::wstring> &m, std::vector<std::vector<pNode>> &n, DSREDataRegion &region) {
	std::vector<std::vector<std::pair<wchar_t, pNode>>> recs;

	auto rec = n.begin();
	for (auto i:m) {
		auto node = rec->begin();
		std::vector<std::pair<wchar_t, pNode>> currRec;
		for (auto j:i) {
			currRec.emplace_back(std::make_pair(j, *node));
			++node;
		}
		std::stable_sort(currRec.begin(), currRec.end(), [](auto &a, auto &b){
			return a.first < b.first;
		});
		recs.emplace_back(currRec);
		++rec;
	}

	// quantidade minima que ele aparece em todos os registros.
	std::unordered_map<wchar_t, int> fieldCount;

	for (auto r:recs) {
		for (auto i = r.begin(); i != r.end();) {
			auto last = std::upper_bound(i, r.end(), *i, [](auto &a, auto &b){
				return a.first < b.first;
			});
			auto count = std::distance(i, last);
			//std::cerr << "* " << i->first << " = " << count << std::endl;
			auto sym = fieldCount.find(i->first);
			if (sym == fieldCount.end() || sym->second > count)
				fieldCount[i->first] = count;
			i = last;
		}
	}

	std::cerr << "field count:" << std::endl;
	for (auto x:fieldCount)
		std::cerr << x.first << " = " << x.second << std::endl;
	std::cerr << std::endl;

	// em quantos registros o simbolo aparece.
	std::unordered_map<wchar_t, int> recCount;

	for (auto r:recs) {
		auto e = std::unique(r.begin(), r.end(), [](auto &a, auto &b){
			return a.first == b.first;
		});
		for (auto i = r.begin(); i != e; ++i)
			++recCount[i->first];
	}

	std::cerr << "rec count:" << std::endl;
	for (auto x:recCount)
		std::cerr << x.first << " = " << x.second << std::endl;
	std::cerr << std::endl;

	int numRecs = recs.size();

	for (auto &r:recs) {
		std::stable_sort(r.begin(), r.end(), [&numRecs, &recCount, &fieldCount](auto &a, auto &b){
			if (recCount[a.first] == recCount[b.first]) {
				return a.first < b.first;
			} else
				return (numRecs - recCount[a.first]) < (numRecs - recCount[b.first]);
		});
	}

	if (numRecs == 0)
		return;

	for (auto r:recs) {
		for (auto f:r)
			std::cerr << f.first << " ";
		std::cerr << std::endl;
	}
	std::cerr << std::endl;

	decltype(recs) opt(numRecs);
	//decltype(recs) req(numRecs);
	std::vector<Record> table(numRecs);
	auto currSym = recs[0][0].first;
	std::vector<size_t> recPos(numRecs);
	std::map<wchar_t, int> symbols;
	while (recCount[currSym] == numRecs) {
		for (int recNo = 0; recNo < numRecs; ++recNo) {
			auto rec = recs[recNo];
			for (int i = 0; i < fieldCount[currSym]; ++i) {
				table[recNo].emplace_back(rec[recPos[recNo]].second);
				//req[recNo].push_back(rec[recPos[recNo]]);
				++recPos[recNo];
			}
			while (recPos[recNo] < rec.size() && rec[recPos[recNo]].first == currSym) {
				opt[recNo].push_back(rec[recPos[recNo]]);
				++symbols[currSym];
				++recPos[recNo];
			}
		}
		if (recPos[0] < recs[0].size())
			currSym = recs[0][recPos[0]].first;
		else
			break;
	}

	for (int recNo = 0; recNo < numRecs; ++recNo) {
		auto rec = recs[recNo];
		for (auto i = recPos[recNo]; i < rec.size(); ++i) {
			opt[recNo].push_back(rec[i]);
			++symbols[rec[i].first];
		}
		std::stable_sort(opt[recNo].begin(), opt[recNo].end(), [](auto &a, auto &b){
			return a.first > b.first;
		});
	}

	auto s = symbols.begin();
	while (!symbols.empty()) {
		if (s->second == 0) {
			s = symbols.erase(s);
		}
		for (int i = 0; i < numRecs; ++i) {
			if (opt[i].empty())
				table[i].emplace_back(nullptr);
			else {
				if (opt[i].back().first == s->first) {
					table[i].emplace_back(opt[i].back().second);
					opt[i].pop_back();
					--(s->second);
				} else {
					table[i].emplace_back(nullptr);
				}
			}
		}
	}

	for (auto &r:table)
		region.addRecord(r);
	region.cleanup();

	for (auto r:table) {
		for (auto f:r) {
			if (f != nullptr)
			std::cerr << f->toString() << " ";
		}
		std::cerr << std::endl;
	}
}
