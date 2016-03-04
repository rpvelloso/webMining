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

#ifndef DOM_HPP_
#define DOM_HPP_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <tidy.h>
#include <tidybuffio.h>
#include "ExtractorInterface.h"

class Node;
using pNode = Node *;

class DOM {
friend class Node;
public:
	DOM(const std::string uri);
	~DOM();
	bool isLoaded() const;
	void printHTML() const;
	pNode body();
	//pNode html();
	string getURI() const;
	ExtractorInterface* getExtractor() const;
	void setExtractor(ExtractorInterface* extractor);

 private:
	void mapNodes(TidyNode node);
	unordered_map<TidyNode, pNode> domNodes;
	void clean();

	TidyDoc tdoc;
	TidyBuffer output = {0};
	TidyBuffer errbuf = {0};
	bool loaded = false;
	DOM() = delete;
	unique_ptr<ExtractorInterface> extractor;
	string uri;
};

#endif /* DOM_HPP_ */
