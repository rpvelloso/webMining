/*
 * dom.cpp
 *
 *  Created on: 26 de fev de 2016
 *      Author: rvelloso
 */

#include <string>
#include <unordered_map>
#include "dom.hpp"
#include "node.hpp"

DOM::DOM(const std::string filename) {
	tdoc = tidyCreate();
	tidyOptSetBool(tdoc, TidyXhtmlOut, yes);
	tidySetErrorBuffer(tdoc, &errbuf);

	if (tidyParseFile(tdoc, filename.c_str()) >= 0) {
		tidyCleanAndRepair(tdoc);
		tidyOptSetBool(tdoc, TidyForceOutput, yes);
		tidySaveBuffer(tdoc, &output);
		loaded = true;
		mapNodes(tidyGetHtml(tdoc));
	}
};

DOM::~DOM() {
	if (loaded) tidyBufFree(&output);
	tidyBufFree(&errbuf);
	tidyRelease(tdoc);
	for (auto n:domNodes)
		delete n.second;
};

bool DOM::isLoaded() {
	return loaded;
}

void DOM::printHTML() {
	if (loaded)
		std::cout << output.bp << std::endl;
};

pNode DOM::body() {
	return domNodes[tidyGetBody(tdoc)];
};

pNode DOM::html() {
	return domNodes[tidyGetHtml(tdoc)];
};

void DOM::traverse(pNode node, size_t ident) {
	for (size_t i = 0; i < ident; ++i)
		std::cout << " ";

	std::cout << node->type() << ": ";
	std::cout << node->toString();
	std::cout << std::endl;

	for (auto child = node->child(); child != nullptr; child = child->next())
		traverse(child, ident+2);
};

void DOM::mapNodes(TidyNode node) {
	if (domNodes.count(node) == 0) {
		domNodes[node] = new Node(this, node);

		for (auto child = tidyGetChild(node); child; child = tidyGetNext(child))
			mapNodes(child);
	}
};
