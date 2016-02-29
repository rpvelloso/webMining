/*
    Copyright 2011 Roberto Panerai Velloso.
    This file is part of libsockets.
    libsockets is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    libsockets is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with libsockets.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <string>
#include <unordered_map>
#include "dom.hpp"
#include "node.hpp"

DOM::DOM(const std::string filename) {
	tdoc = tidyCreate();

	tidyOptSetBool(tdoc, TidyXhtmlOut, yes);
	tidyOptSetValue(tdoc, TidyIndentContent, "auto");
	tidyOptSetInt(tdoc,TidyIndentSpaces,2);
	tidyOptSetBool(tdoc, TidyIndentCdata, yes);

	tidySetErrorBuffer(tdoc, &errbuf);

	if (tidyParseFile(tdoc, filename.c_str()) >= 0) {
		tidyCleanAndRepair(tdoc);
		tidyOptSetBool(tdoc, TidyForceOutput, yes);
		tidySaveBuffer(tdoc, &output);
		clean();
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

bool DOM::isLoaded() const {
	return loaded;
}

void DOM::printHTML() const {
	if (loaded)
		std::cout << output.bp << std::endl;
};

pNode DOM::body() {
	return domNodes[tidyGetBody(tdoc)];
};

pNode DOM::html() {
	return domNodes[tidyGetHtml(tdoc)];
};

void DOM::mapNodes(TidyNode node) {
	if (domNodes.count(node) == 0) {
		domNodes[node] = new Node(this, node);

		for (auto child = tidyGetChild(node); child; child = tidyGetNext(child))
			mapNodes(child);
	}
}
;

static void cleanHelper(TidyNode n, unordered_set<TidyNode> &remove) {
	static unordered_set<string> removeTags = {"script", "noscript"};

	auto pTagName = tidyNodeGetName(n);
	auto nodeType = tidyNodeGetType(n);
	string tagName;

	if (pTagName != nullptr)
		tagName = pTagName;

	if (removeTags.count(tagName) > 0 ||
		nodeType == TidyNode_Comment)
		remove.insert(n);

	for (auto c = tidyGetChild(n); c ; c = tidyGetNext(c))
		cleanHelper(c, remove);
}

void DOM::clean() {

	unordered_set<TidyNode> remove;

	cleanHelper(tidyGetHtml(tdoc), remove);
	for (auto node:remove)
		tidyDiscardElement(tdoc, node);
}

ExtractorInterface* DOM::getExtractor() const {
	return extractor.get();
}

void DOM::setExtractor(ExtractorInterface* extractor) {
	this->extractor = unique_ptr<ExtractorInterface>(extractor);
}
