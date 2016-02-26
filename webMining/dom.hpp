/*
 * dom.hpp
 *
 *  Created on: 25 de fev de 2016
 *      Author: rvelloso
 */

#ifndef DOM_HPP_
#define DOM_HPP_

#include <string>
#include <unordered_map>
#include <tidy.h>
#include <tidybuffio.h>
#include "tTPSFilter.h"

class Node;
using pNode = Node *;

class DOM {
friend class Node;
public:
	DOM(const std::string filename);
	~DOM();
	bool isLoaded();
	void printHTML();
	pNode body();
	pNode html();
	void traverse(pNode, size_t ident = 0);

	tTPSFilter tpsf;
private:
	void mapNodes(TidyNode node);
	unordered_map<TidyNode, pNode> domNodes;

	TidyDoc tdoc;
	TidyBuffer output = {0};
	TidyBuffer errbuf = {0};
	bool loaded = false;
	DOM() = delete;
};

#endif /* DOM_HPP_ */
