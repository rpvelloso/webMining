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

#ifndef NODE_HPP_
#define NODE_HPP_

#include <tidy.h>
#include <map>
#include <string>

#include "../3rdparty/sol.hpp"

class Node;
using pNode = Node *;

class DOM;

class Node {
  friend class DOM;
 public:
  Node(DOM *d, TidyNode n);
  ~Node();
  pNode next() const;
  std::string getAttr(std::string attrName);
  pNode child() const;
  int type() const;
  std::string getTagName() const;
  bool isImage() const;
  bool isLink() const;
  bool isText() const;
  std::string toString() const;
  static void luaBinding(sol::state &lua);
 private:
  Node() = delete;
  TidyNode node;
  DOM *dom;
  std::map<std::string, std::string> attrs;
  std::string tagName;
  std::string value;
};

#endif /* NODE_HPP_ */
