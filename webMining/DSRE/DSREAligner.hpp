/*
 * DSREAligner.hpp
 *
 *  Created on: 30 de mar de 2017
 *      Author: rvelloso
 */

#ifndef DSRE_DSREALIGNER_HPP_
#define DSRE_DSREALIGNER_HPP_

#include <vector>
#include "../base/Node.hpp"
#include "DSREAbstractRecordAligner.hpp"

class DSREAligner: public DSREAbstractRecordAligner {
public:
	DSREAligner();
	virtual ~DSREAligner();
	void align(DSREDataRegion &region, std::set<std::size_t> &recpos);
private:
	void extractRecords(std::vector<std::wstring> &m, std::vector<std::vector<pNode>> &n, DSREDataRegion &region);
};

#endif /* DSRE_DSREALIGNER_HPP_ */
