/*
 * DSREDataRegion.h
 *
 *  Created on: 13 de jul de 2016
 *      Author: rvelloso
 */

#ifndef DSREDATAREGION_H_
#define DSREDATAREGION_H_

#include <vector>
#include <functional>
#include "Node.hpp"
#include "DataRegion.hpp"
#include "util.hpp"

class DSREDataRegion : public DataRegion {
public:
	DSREDataRegion();
	virtual ~DSREDataRegion();
	size_t size() const noexcept;
	size_t getEndPos() const noexcept;
	void setEndPos(size_t endPos) noexcept;
	size_t getStartPos() const noexcept;
	void setStartPos(size_t startPos) noexcept;
	void shiftStartPos(long int shift);
	void shiftEndPos(long int shift);
	const std::vector<pNode>& getNodeSequence() const;
	void setNodeSequence(const std::vector<pNode>& nodeSequence);
	void assignNodeSequence(
			std::vector<pNode>::iterator first,
			std::vector<pNode>::iterator last);
	void eraseNodeSequence(std::function<bool(const pNode &)>);
	void eraseTPS(std::function<bool(const wchar_t &)>);
	const std::wstring& getTps() const;
	void setTps(const std::wstring& tps);
	const LinearRegression& getLinearRegression() const;
	void detectStructure();
	bool isStructured() const;
	void setStructured(bool structured = false);
	double getScore() const;
	void setScore(double score);
	bool isContent() const;
	void setContent(bool content = false);
	double getStdDev() const;
	void setStdDev(double stddev = 0);

private:
	std::wstring tps;
	std::vector<pNode> nodeSequence;
	LinearRegression linearRegression;
	size_t startPos = 0, endPos = 0;
	bool structured = false;
	bool content = false;
	double score = 0;
	double stdDev = 0;
};

#endif /* DSREDATAREGION_H_ */
