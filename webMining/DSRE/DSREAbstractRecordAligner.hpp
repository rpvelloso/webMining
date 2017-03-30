/*
 * DSRERecordAligner.hpp
 *
 *  Created on: 30 de mar de 2017
 *      Author: rvelloso
 */

#ifndef DSRE_DSREABSTRACTRECORDALIGNER_HPP_
#define DSRE_DSREABSTRACTRECORDALIGNER_HPP_

#include <set>
#include "DSREDataRegion.hpp"

enum class AlignmentStrategy {
	DefaultAlignment = 0,
	CenterStarAlignment
};

class DSREAbstractRecordAligner {
public:
	DSREAbstractRecordAligner() {};
	virtual ~DSREAbstractRecordAligner() {};
	virtual void align(DSREDataRegion &region, std::set<std::size_t> &recpos) = 0;
};

#endif /* DSRE_DSREABSTRACTRECORDALIGNER_HPP_ */
