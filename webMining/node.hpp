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

#ifndef NODE_HPP_
#define NODE_HPP_

#include <string>
#include <map>
#include "tidy.h"

using namespace std;

class Node;
using pNode = Node *;

class DOM;

class Node {
friend class DOM;
public:
	Node(DOM *d, TidyNode n);
	~Node();
	pNode next() const;
	string getAttr(string attrName);
	pNode child() const;
	int type() const;
	string tagName() const;
	bool isImage() const;
	bool isLink() const;
	bool isText() const;
	string toString() const;
private:
	Node() = delete;
	TidyNode node;
	DOM *dom;
	map<string, string> attrs;
};

#endif /* NODE_HPP_ */
