/*
 * CVSREDataRegion.hpp
 *
 *  Created on: 23 de jul de 2016
 *      Author: roberto
 */

#ifndef CVSREDATAREGION_HPP_
#define CVSREDATAREGION_HPP_

#include "../base/StructuredDataRegion.hpp"
#include "../base/util.hpp"

class CVSREDataRegion: public StructuredDataRegion {
public:
	CVSREDataRegion(size_t offset, double period, std::wstring tps,
			std::vector<pNode>::iterator b, std::vector<pNode>::iterator e);
	virtual ~CVSREDataRegion();
	const std::vector<pNode>& getNodeSequence() const;

	size_t size();
	size_t getEndPos();
	size_t getStartPos();
	const std::wstring &getTps();
	LinearRegression getLinearRegression();
	bool isStructured();
	double getScore();
	bool isContent();
	double getEstPeriod();
	int getPeriodEstimator();

	static void luaBinding(sol::state &lua);
private:
	size_t endPos, startPos;
	double period;
	std::wstring tps;
	std::vector<pNode> nodeSequence;
};

#endif /* CVSREDATAREGION_HPP_ */
