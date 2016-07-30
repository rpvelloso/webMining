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

#include "Node.hpp"

#include <tidybuffio.h>
#include <sstream>
#include <unordered_map>
#include <utility>

#include "DOM.hpp"

void Node::luaBinding(sol::state &lua) {
  lua.new_usertype<Node>("Node",
                         "toString", &Node::toString,
                         "getAttr",&Node::getAttr,
                         "type",&Node::type,
                         "getTagName",&Node::getTagName,
                         "isImage",&Node::isImage,
                         "isLink",&Node::isLink,
                         "isText",&Node::isText
                         );
}

Node::Node(DOM *d, TidyNode n)
    : node(n),
      dom(d) {
  for (auto attr = tidyAttrFirst(node); attr; attr = tidyAttrNext(attr)) {
    if (auto attrName = tidyAttrName(attr)) {
      if (auto attrValue = tidyAttrValue(attr))
        attrs[attrName] = attrValue;
    }
  }

  TidyBuffer buf = { 0 };
  tidyNodeGetValue(dom->tdoc, node, &buf);
  if (buf.allocated) {
    value = (char *) buf.bp;
    tidyBufFree(&buf);
  }

  auto name = tidyNodeGetName(node);
  if (name)
    tagName = name;
}

Node::~Node() {
}
;

pNode Node::next() const {
  auto n = tidyGetNext(node);
  if (n && dom->domNodes.count(n) > 0)
    return dom->domNodes[n];
  else
    return nullptr;
}

pNode Node::child() const {
  auto c = tidyGetChild(node);
  if (c && dom->domNodes.count(c) > 0)
    return dom->domNodes[c];
  else
    return nullptr;
}

int Node::type() const {
  return tidyNodeGetType(node);
}

std::string Node::getTagName() const {
  return tagName;
}

std::string Node::getAttr(std::string attrName) {
  if (attrs.count(attrName) > 0)
    return attrs[attrName];
  else
    return "";
}

bool Node::isImage() const {
  return tidyNodeIsIMG(node);
}
bool Node::isLink() const {
  return tidyNodeIsLINK(node);
}
bool Node::isText() const {
  return tidyNodeIsText(node);
}

std::string Node::toString() const {
  std::stringstream result;

  if (tagName != "")
    result << "<" << tagName << " ";

  for (auto attr : attrs)
    result << attr.first << "=" << attr.second << " ";

  if (tagName != "")
    result << "/> ";

  if (value != "")
    result << value;

  return result.str();
}
