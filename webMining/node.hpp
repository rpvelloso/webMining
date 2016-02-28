/*
 * node.hpp
 *
 *  Created on: 26 de fev de 2016
 *      Author: rvelloso
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
	pNode next();
	string getAttr(string attrName);
	pNode child();
	TidyNodeType type();
	string tagName();
	bool isImage();
	bool isLink();
	bool isText();
	string toString();
private:
	Node() = delete;
	TidyNode node;
	DOM *dom;
	map<string, string> attrs;
};

#endif /* NODE_HPP_ */
