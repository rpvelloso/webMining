/*
 * DSRECenterStar.hpp
 *
 *  Created on: 30 de mar de 2017
 *      Author: rvelloso
 */

#ifndef DSRE_DSRECENTERSTAR_HPP_
#define DSRE_DSRECENTERSTAR_HPP_

#include "DSREAbstractRecordAligner.hpp"

class DSRECenterStar: public DSREAbstractRecordAligner {
public:
	DSRECenterStar();
	virtual ~DSRECenterStar() = default;
	void align(DSREDataRegion &region, std::set<std::size_t> &recpos);
private:
	void extractRecords(std::vector<std::wstring> &m, std::set<std::size_t> &recpos, DSREDataRegion &region);
};

class DSREPartialTreeAlign: public DSREAbstractRecordAligner {
public:
	DSREPartialTreeAlign();
	virtual ~DSREPartialTreeAlign() = default;
	void align(DSREDataRegion &region, std::set<std::size_t> &recpos);
private:
	std::vector<pNode> buildRecTrees(DSREDataRegion &region, std::set<std::size_t> &recpos);
};

#endif /* DSRE_DSRECENTERSTAR_HPP_ */
