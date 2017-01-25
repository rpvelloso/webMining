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
  TPSExtractor() : StructuredExtractor<DataRegionType>() {
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

    std::set<std::pair<int, std::string>> f;
    for (auto c:classFreq)
    	f.insert(std::make_pair(c.second, c.first));

    std::cout << std::endl;
    for (auto c:f)
    	std::cout << c.second << " = " << c.first << std::endl;
  }

  virtual void extract(pDOM dom) = 0;
  virtual void cleanup() {
    StructuredExtractor < DataRegionType > ::cleanup();
    tagPathMap.clear();
    tagPathSequence.clear();
    nodeSequence.clear();
  }
 protected:
  std::string cleanClassName(std::string v) {
	  auto vend = std::remove_if(v.begin(), v.end(), isdigit);
	  v.erase(vend,v.end());
	  std::transform(v.begin(), v.end(), v.begin(), tolower);
	  return v;
  };

  void computeClassFreq(pNode n) {
	  auto classValues = n->getAttr("class");

	  while (classValues != "") {
    	  auto v = cleanClassName(stringTok(classValues, " \r\n\t"));
    	  if (v != "")
    		  ++classFreq[v];
	  }

      for (auto child = n->child(); child != nullptr; child = child->next())
    	  computeClassFreq(child);
  };

  void buildTagPath(std::string s, pNode n, bool css) {
    static std::vector<std::string> styleAttr = { "style", "class", "color",
        "bgcolor", "width", "height", "align", "valign", "halign", "colspan",
        "rowspan" };
    static std::set<std::string> ignoreClass = { "highlight", "special", "last", "odd", "first", "even", "item", "feature" };

    std::string tagStyle;

    if (s == "") {
      tagPathMap.clear();
      tagPathSequence.clear();
      nodeSequence.clear();
    }

    for (auto attrName : styleAttr) {
      auto attrValue = n->getAttr(attrName);

      if (attrName == "class") {
          std::set<std::pair<int,std::string>> classes;

          while (attrValue != "") {
        	  auto v = cleanClassName(stringTok(attrValue, " \r\n\t"));
        	  bool insert = true;
        	  for (auto i:ignoreClass)
        		  if (v.find(i) != std::string::npos)
        			  insert = false;
        	  if (insert)
        		classes.insert(std::make_pair(classFreq[v],v));
          }

          // consider only tag's most frequent class
          if (classes.size() > 0)
        	  attrValue = (*classes.rbegin()).second;
      }

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
  std::unordered_map<std::string, int> classFreq;
};

#endif /* TPSEXTRACTOR_HPP_ */
