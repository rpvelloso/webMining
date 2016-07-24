/*
 * TPSExtractor.hpp
 *
 *  Created on: 23 de jul de 2016
 *      Author: roberto
 */

#ifndef TPSEXTRACTOR_HPP_
#define TPSEXTRACTOR_HPP_

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

    for (auto tpc : tagPathSequence)
      std::cout << tpcMap[tpc] << std::endl;
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
