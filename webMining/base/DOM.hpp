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

#ifndef DOM_HPP_
#define DOM_HPP_

#include <tidy.h>
#include <tidybuffio.h>
#include <string>
#include <unordered_map>

#include "../3rdparty/sol.hpp"

class Node;
using pNode = Node *;
class DOM;
using pDOM = DOM *;

class DOM {
  friend class Node;
 public:
  DOM(const std::string &uri, const std::string &html);
  ~DOM();
  void printHTML() const;
  pNode body();
  std::string getURI() const noexcept;
  void traverse(int, pNode = nullptr);
  void setVisitFunction(sol::function v);

  static void luaBinding(sol::state &lua);
  size_t size() const;
 private:
  void mapNodes(TidyNode node);
  std::unordered_map<TidyNode, pNode> domNodes;
  void clear();
  sol::function visit;

  TidyDoc tdoc;
  TidyBuffer output = { 0 };
  TidyBuffer errbuf = { 0 };
  DOM() = delete;
  std::string uri;
};

using pDOM = DOM *;

#endif /* DOM_HPP_ */
