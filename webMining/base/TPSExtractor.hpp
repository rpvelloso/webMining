/*
 * TPSExtractor.hpp
 *
 *  Created on: 23 de jul de 2016
 *      Author: roberto
 */

#ifndef TPSEXTRACTOR_HPP_
#define TPSEXTRACTOR_HPP_

#include "StructuredExtractor.hpp"

template<typename DataRegionType>
class TPSExtractor: public StructuredExtractor<DataRegionType> {
public:
	TPSExtractor() {};
	virtual ~TPSExtractor() {};
protected:
	void buildTagPath(std::string s, pNode n, bool css) {
		static std::vector<std::string> styleAttr = {
				"style", "class", "color", "bgcolor", "width", "height",
				"align", "valign", "halign", "colspan", "rowspan"
		};

		std::string tagStyle;

		if (s == "") {
			tagPathMap.clear();
			tagPathSequence.clear();
			nodeSequence.clear();
		}

		for (auto attrName:styleAttr) {
			auto attrValue = n->getAttr(attrName);

			if (attrName == "class") // consider only tag's first class
				attrValue = stringTok(attrValue," \r\n\t");

			if (attrValue != "")
				tagStyle = tagStyle + " " + attrName + "=" + attrValue;
		}

		std::string tagName;

		if (n->isText())
			tagName = "#text";
		else
			tagName = n->getTagName();

		if (tagName != "") {
			if (css && (tagStyle != "")) s = s + "/" + tagName + tagStyle;
			else s = s + "/" + tagName;

			if (tagPathMap.count(s) == 0)
				tagPathMap[s] = tagPathMap.size()+1;

			tagPathSequence = tagPathSequence + wchar_t(tagPathMap[s]);
			nodeSequence.push_back(n);

			for (auto child = n->child(); child != nullptr; child = child->next())
				buildTagPath(s,child,css);
		} else {
			std::cout << "empty tagName!!! : " << n->toString() << std::endl;
			//terminate();
		}
	};
	std::unordered_map<std::string, int> tagPathMap;
	std::wstring tagPathSequence;
	std::vector<pNode> nodeSequence;
};

#endif /* TPSEXTRACTOR_HPP_ */
