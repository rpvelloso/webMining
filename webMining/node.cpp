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

#include <map>
#include <string>
#include <sstream>
#include "dom.hpp"
#include "node.hpp"
#include "tidy.h"

Node::Node(DOM *d, TidyNode n) : node(n), dom(d) {
	for (auto attr = tidyAttrFirst(node); attr; attr = tidyAttrNext(attr)) {
		if (auto attrName = tidyAttrName(attr)) {
			if (auto attrValue = tidyAttrValue(attr))
				attrs[attrName] = attrValue;
		}
	}
}

Node::~Node() {};

pNode Node::next() {
	auto n = tidyGetNext(node);
	if (dom->domNodes.count(n) > 0)
		return dom->domNodes[n];
	else
		return nullptr;
}

string Node::getAttr(string attrName) {
	if (attrs.count(attrName) > 0)
		return attrs[attrName];
	else
		return "";
}

pNode Node::child() {
	auto c = tidyGetChild(node);
	if (dom->domNodes.count(c) > 0)
		return dom->domNodes[c];
	else
		return nullptr;
}

TidyNodeType Node::type() {
	return tidyNodeGetType(node);
}

string Node::tagName() {
	auto name = tidyNodeGetName(node);
	if (name)
		return string(name);
	else
		return "";
}

bool Node::isImage() {return tidyNodeIsIMG(node);}
bool Node::isLink() {return tidyNodeIsLINK(node);}
bool Node::isText() {return tidyNodeIsText(node);}

string Node::toString() {
	stringstream result;
	auto name = tagName();

	if (name != "")
		result << "<" << name;

	for (auto attr = tidyAttrFirst(node); attr; attr = tidyAttrNext(attr)) {
		if (auto attrName = tidyAttrName(attr)) {
			result << " " << attrName;
			if (auto attrValue = tidyAttrValue(attr))
				result << "=" << attrValue;
		}
	}

	if (name != "")
		result << ">";

	TidyBuffer buf = {0};

	tidyNodeGetValue(dom->tdoc, node, &buf);

	if (buf.allocated) {
		result << buf.bp;
		tidyBufFree(&buf);
	}

	return result.str();
}
