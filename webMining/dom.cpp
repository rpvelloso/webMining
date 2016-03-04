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
#include <algorithm>
#include "dom.hpp"
#include "node.hpp"

/*static void traverse(DOM *dom, TidyNode node, size_t ident = 0) {
	Node n(dom, node);

	for (size_t i = 0; i < ident; ++i)
		cout << " ";
	cout << n.toString() << endl;

	for (auto c = tidyGetChild(node); c; c = tidyGetNext(c))
		traverse(dom, c, ident+2);
}*/

DOM::DOM(const std::string uri) {
	this->uri = uri;
	tdoc = tidyCreate();

	tidyOptSetValue(tdoc, TidyIndentContent, "auto");
	tidyOptSetValue(tdoc, TidySortAttributes, "alpha");

	tidyOptSetInt(tdoc,TidyIndentSpaces, 2);

	tidyOptSetBool(tdoc, TidyXhtmlOut, yes);
	tidyOptSetBool(tdoc, TidyMakeClean, yes);
	tidyOptSetBool(tdoc, TidyJoinClasses, yes);
	tidyOptSetBool(tdoc, TidyJoinStyles, yes);
	tidyOptSetBool(tdoc, TidyCoerceEndTags, yes);
	tidyOptSetBool(tdoc, TidyDropEmptyElems, yes);
	tidyOptSetBool(tdoc, TidyDropEmptyParas, yes);
	tidyOptSetBool(tdoc, TidyIndentCdata, yes);
	tidyOptSetBool(tdoc, TidyHideComments, yes);
	tidyOptSetBool(tdoc, TidyForceOutput, yes);

	tidySetErrorBuffer(tdoc, &errbuf);

	if (tidyParseFile(tdoc, uri.c_str()) >= 0) {
		tidyCleanAndRepair(tdoc);
		tidyRunDiagnostics(tdoc);
		clean();
		tidySaveBuffer(tdoc, &output);
		mapNodes(tidyGetHtml(tdoc));
		loaded = true;
		//traverse(this,tidyGetHtml(tdoc));
	}
};

DOM::~DOM() {
	if (errbuf.allocated)
		tidyBufFree(&errbuf);
	if (output.allocated)
		tidyBufFree(&output);
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

/*pNode DOM::html() {
	return domNodes[tidyGetHtml(tdoc)];
};*/

void DOM::mapNodes(TidyNode node) {
	if (domNodes.count(node) == 0) {
		domNodes[node] = new Node(this, node);

		for (auto child = tidyGetChild(node); child; child = tidyGetNext(child))
			mapNodes(child);
	}
}
;

static void cleanHelper(TidyNode n, vector<TidyNode> &remove) {
	static unordered_set<string> removeTags = {"script", "noscript"};

	auto pTagName = tidyNodeGetName(n);
	auto nodeType = tidyNodeGetType(n);
	string tagName;

	if (pTagName != nullptr)
		tagName = pTagName;

	if (removeTags.count(tagName) > 0 ||
		nodeType == TidyNode_Comment)
		remove.push_back(n);

	for (auto c = tidyGetChild(n); c ; c = tidyGetNext(c))
		cleanHelper(c, remove);
}

string DOM::getURI() const {
	return uri;
}

void DOM::clean() {
	vector<TidyNode> remove;

	cleanHelper(tidyGetHtml(tdoc), remove);

	while (!remove.empty()) {
		auto node = remove.back();
		remove.pop_back();
		tidyDiscardElement(tdoc, node);
	}
}

ExtractorInterface* DOM::getExtractor() const {
	return extractor.get();
}

void DOM::setExtractor(ExtractorInterface* extractor) {
	this->extractor = unique_ptr<ExtractorInterface>(extractor);
}
