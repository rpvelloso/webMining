/*
 Copyright 2011 Roberto Panerai Velloso.
 This file is part of webMining.
 webMining is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 webMining is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with webMining.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "DOM.hpp"
#include "DOMTraverse.hpp"

#include <tidyenum.h>
#include <tidyplatform.h>
#include <iostream>
#include <utility>
#include <vector>
#include <unordered_set>
#include <exception>

#include "Node.hpp"

void DOM::luaBinding(sol::state &lua) {
	lua.new_usertype<DOM>("DOM",
			sol::constructors<sol::types<const std::string>, sol::types<const std::string, const std::string>>(),
			"printHTML", &DOM::printHTML,
			"getURI", &DOM::getURI,
			"traverse", &DOM::traverse,
			"setVisitFunction", &DOM::setVisitFunction,
			"body", &DOM::body,
			"size", &DOM::size
			);
	Node::luaBinding(lua);
}

DOM::DOM(const std::string &uri, const std::string &html = "") {
	this->uri = uri;
	tdoc = tidyCreate();

	tidyOptSetValue(tdoc, TidyIndentContent, "auto");
	tidyOptSetValue(tdoc, TidySortAttributes, "alpha");
	tidyOptSetValue(tdoc, TidyCharEncoding, "utf8");

	tidyOptSetInt(tdoc, TidyIndentSpaces, 2);

	tidyOptSetInt(tdoc, TidyMergeDivs, yes);
	tidyOptSetInt(tdoc, TidyMergeSpans, yes);

	tidyOptSetBool(tdoc, TidyHtmlOut, yes);
	tidyOptSetBool(tdoc, TidyMakeClean, yes);
	tidyOptSetBool(tdoc, TidyJoinClasses, yes);
	tidyOptSetBool(tdoc, TidyJoinStyles, yes);
	tidyOptSetBool(tdoc, TidyCoerceEndTags, yes);
	tidyOptSetBool(tdoc, TidyDropEmptyElems, yes);
	tidyOptSetBool(tdoc, TidyDropEmptyParas, yes);
	tidyOptSetBool(tdoc, TidyIndentCdata, yes);
	tidyOptSetBool(tdoc, TidyFixComments, yes);
	tidyOptSetBool(tdoc, TidyHideComments, yes);
	tidyOptSetBool(tdoc, TidyForceOutput, yes);
	tidyOptSetBool(tdoc, TidySkipNested, yes);
	tidyOptSetBool(tdoc, TidyEscapeCdata, yes);
	tidyOptSetBool(tdoc, TidyFixUri, yes);

	tidySetErrorBuffer(tdoc, &errbuf);

	int parseResult;
	if (html == "")
		parseResult = tidyParseFile(tdoc, uri.c_str());
	else
		parseResult = tidyParseString(tdoc, html.c_str());

	if (parseResult >= 0) {
		tidyCleanAndRepair(tdoc);
		tidyRunDiagnostics(tdoc);
		clear();
		tidySaveBuffer(tdoc, &output);
		mapNodes(tidyGetHtml(tdoc));
	} else
		throw new std::runtime_error("error parsing file " + uri);
}

DOM::~DOM() {
	if (errbuf.allocated)
		tidyBufFree(&errbuf);
	if (output.allocated)
		tidyBufFree(&output);
	tidyRelease(tdoc);
	for (auto n : domNodes)
		delete n.second;
}

void DOM::traverse(int strategy, pNode start) {
	TraverseContainer<pNode> traverseContainer(TraverseStrategyFactory<pNode>::get(strategy));

	traverseContainer.push(start==nullptr?body():start);

	while (!traverseContainer.empty()) {
		auto n = traverseContainer.top();
		traverseContainer.pop();

		int visitResult = visit(this, n);

		if (visitResult != 1)
			break;

		for (auto c = n->child(); c != nullptr; c = c->next())
			traverseContainer.push(c);
	}
}

void DOM::printHTML() const {
	std::cout << output.bp << std::endl;
}
;

pNode DOM::body() {
	return domNodes[tidyGetBody(tdoc)];
}
;

void DOM::mapNodes(TidyNode node) {
	if (domNodes.count(node) == 0) {
		domNodes[node] = new Node(this, node);

		for (auto child = tidyGetChild(node); child; child = tidyGetNext(child))
			mapNodes(child);
	}
}
;

static void cleanHelper(TidyNode n, std::vector<TidyNode> &remove) {
	static std::unordered_set<std::string> removeTags = { "script", "noscript" };

	auto pTagName = tidyNodeGetName(n);
	auto nodeType = tidyNodeGetType(n);
	std::string tagName;

	if (pTagName != nullptr)
		tagName = pTagName;

	if (removeTags.count(tagName) > 0 || nodeType == TidyNode_Comment)
		remove.push_back(n);

	for (auto c = tidyGetChild(n); c; c = tidyGetNext(c))
		cleanHelper(c, remove);
}

std::string DOM::getURI() const noexcept {
	return uri;
}

void DOM::setVisitFunction(sol::function v) {
	this->visit = v;
}

size_t DOM::size() const {
	return domNodes.size();
}

void DOM::clear() {
	std::vector<TidyNode> remove;

	cleanHelper(tidyGetHtml(tdoc), remove);

	while (!remove.empty()) {
		auto node = remove.back();
		remove.pop_back();
		tidyDiscardElement(tdoc, node);
	}
}
