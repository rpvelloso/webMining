/*
 * DSRE.h
 *
 *  Created on: 13 de jul de 2016
 *      Author: rvelloso
 */

#ifndef DSRE_HPP_
#define DSRE_HPP_

#include <unordered_map>
#include <string>
#include <vector>
#include <list>
#include <set>

#include "DSREDataRegion.hpp"
#include "sol.hpp"
#include "StructuredExtractor.hpp"

class DSRE: public StructuredExtractor<DSREDataRegion> {
public:
	DSRE();
	virtual ~DSRE();
	void extract(pDOM dom) override;
	void clear() override;
	std::wstring getTps() const noexcept;
	void printTps() const;

	static void luaBinding(sol::state &lua);
private:
	void buildTagPath(std::string s, pNode n, bool css);

	void segment(pNode n, bool css);
	std::vector<size_t> detectStructure();
	std::set<size_t> locateRecords(size_t regNum);
	double estimatePeriod(std::vector<double> signal);

	std::list<std::pair<size_t,size_t> > segmentDifference(const std::vector<double> &);
	void mergeRegions(std::list<std::pair<size_t,size_t> > &);
	std::unordered_map<int,int> symbolFrequency(std::wstring, std::set<int> &);
	void extractRecords(std::vector<std::wstring> &m, std::set<size_t> &recpos, size_t regNum);

	std::unordered_map<std::string, int> tagPathMap;
	std::wstring tagPathSequence;
	std::vector<pNode> nodeSequence;
};

#endif /* DSRE_HPP_ */
