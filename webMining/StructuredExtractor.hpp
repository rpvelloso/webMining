/*
 * StructuredExtractor.hpp
 *
 *  Created on: 15 de jul de 2016
 *      Author: roberto
 */

#ifndef STRUCTUREDEXTRACTOR_HPP_
#define STRUCTUREDEXTRACTOR_HPP_

#include "Extractor.hpp"

template <typename DataRegionType>
class StructuredExtractor : public Extractor<DataRegionType> {
public:
	StructuredExtractor() {};
	virtual ~StructuredExtractor() {};
	virtual void extract(pDOM dom) = 0;
	virtual void clear() = 0;
};

#endif /* STRUCTUREDEXTRACTOR_HPP_ */
