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

#ifndef TPSEXTRACTOR_HPP_
#define TPSEXTRACTOR_HPP_

#include <iomanip>

#include "StructuredExtractor.hpp"
#include "Node.hpp"
#include "../base/util.hpp"

// TODO: refactor. Use composition to add TPS functionality to extractor classes instead of inhertance
template<typename DataRegionType>
class TPSExtractor : public StructuredExtractor<DataRegionType> {
 public:
  TPSExtractor() {
  }
  virtual ~TPSExtractor() {
  }
  std::wstring getTps() const noexcept {
    return tagPathSequence;
  }

  void printTps() const {
    std::unordered_map<int, std::string> tpcMap;

    for (auto tps : tagPathMap)
      tpcMap.insert(make_pair(tps.second, tps.first));

    size_t i = 0;
    for (auto tpc : tagPathSequence)
      std::cout << std::setfill('0') << std::setw(5) << i++ << ":" <<
	  std::setfill('0') << std::setw(5) << tpc << ":" <<
	  tpcMap[tpc] << std::endl;
  }

  virtual void extract(pDOM dom) = 0;
  virtual void cleanup() {
    StructuredExtractor < DataRegionType > ::cleanup();
    tagPathMap.clear();
    tagPathSequence.clear();
    nodeSequence.clear();
  }
 protected:
  void buildTagPath(std::string s, pNode n, bool css) {
    static std::vector<std::string> styleAttr = { "style", "class", "color",
        "bgcolor", "width", "height", "align", "valign", "halign", "colspan",
        "rowspan" };

    std::string tagStyle;

    if (s == "") {
      tagPathMap.clear();
      tagPathSequence.clear();
      nodeSequence.clear();
    }

    for (auto attrName : styleAttr) {
      auto attrValue = n->getAttr(attrName);

      if (attrName == "class")  // consider only tag's first class
        attrValue = stringTok(attrValue, " \r\n\t");

      if (attrValue != "")
        tagStyle = tagStyle + " " + attrName + "=" + attrValue;
    }

    std::string tagName;

    if (n->isText())
      tagName = "#text";
    else
      tagName = n->getTagName();

    if (tagName != "") {
      if (css && (tagStyle != ""))
        s = s + "/" + tagName + tagStyle;
      else
        s = s + "/" + tagName;

      if (tagPathMap.count(s) == 0)
        tagPathMap[s] = tagPathMap.size() + 1;

      tagPathSequence = tagPathSequence + wchar_t(tagPathMap[s]);
      nodeSequence.push_back(n);

      for (auto child = n->child(); child != nullptr; child = child->next())
        buildTagPath(s, child, css);
    } else {
      std::cout << "empty tagName!!! : " << n->toString() << std::endl;
      //terminate();
    }
  }
  ;
  std::unordered_map<std::string, int> tagPathMap;
  std::wstring tagPathSequence;
  std::vector<pNode> nodeSequence;
};

#endif /* TPSEXTRACTOR_HPP_ */
