/*
 * DSRECenterStar.cpp
 *
 *  Created on: 30 de mar de 2017
 *      Author: rvelloso
 */

#include <stack>
#include <set>

#include "DSRECenterStar.hpp"
#include "../base/Node.hpp"
#include "../base/STM.hpp"

class FakeNode : public Node {
public:
	FakeNode(pNode nextNode) : Node(), nextNode(nextNode) {}
    virtual pNode next() const override {return nextNode;};
private:
	pNode nextNode = nullptr;
};

class FakeRoot : public Node {
public:
	FakeRoot(const std::vector<pNode> &children) : Node(), children(children) {
		if (children.size() > 1) {
			for (int i = 1; i < children.size(); ++i) {
				this->children[i-1] = new FakeNode(this->children[i]);
			}
		} else if (children.size() == 1) {
			this->children[0] = new FakeNode(nullptr);
		}
		//dom = children.front()->dom;
	};
	virtual ~FakeRoot() {
		for (auto &ch: children) delete ch;
	}
	  virtual pNode next() const override {return nullptr;};
	  virtual pNode child() const override {return children.front();};
	  virtual size_t getChildCount() const override {return children.size();};
private:
	  std::vector<pNode> children;
};

DSREPartialTreeAlign::DSREPartialTreeAlign() {

}

std::vector<pNode> DSREPartialTreeAlign::buildRecTrees(DSREDataRegion &region, std::set<std::size_t> &recpos) {
	std::vector<pNode> result;

	auto prev = recpos.begin();
	auto curr = prev;
	++curr;
	for (; curr != recpos.end(); ++curr, ++prev) {
		auto comp = [](const pNode a, const pNode b){ return a->getDepth() < b->getDepth();};
		std::set<pNode, decltype(comp)> nodePQ(
				region.getNodeSequence().begin() + *prev,
				region.getNodeSequence().begin() + *curr,
				comp);

		std::vector<pNode> children;
		while (!nodePQ.empty()) {
			auto top = *nodePQ.begin();
			nodePQ.erase(top);
			children.emplace_back(top);

			std::stack<pNode> st;
			st.push(top);
			while (!st.empty()) {
				auto u = st.top();
				st.pop();
				for (auto v = u->child(); u; v = v->next()) {
					st.push(v);
					nodePQ.erase(v);
				}
			}
		}
		result.emplace_back(new FakeRoot(children));
	}

	return result;
}

void DSREPartialTreeAlign::align(DSREDataRegion &region, std::set<std::size_t> &recpos) {
	auto recTrees = buildRecTrees(region, recpos);
	auto seed = recTrees.begin();
	auto curr = seed + 1;
	for (; curr != recTrees.end(); ++curr) {
		STM::match(*seed, *curr, true);
	}
}

DSRECenterStar::DSRECenterStar() {
}

void DSRECenterStar::align(DSREDataRegion& region, std::set<size_t>& recpos) {
  std::vector<std::wstring> m;

  // create a sequence for each record found
  auto firstTP = region.getTps().begin();
  size_t max_size = 0;
  if (recpos.size() > 1) {
	auto prev = recpos.begin();
	for (auto rp = ++(recpos.begin()); rp != recpos.end(); ++rp, ++prev) {
	  if (((*rp) - (*prev)) > 0) {
		m.emplace_back(firstTP + *prev, firstTP + *rp);

		max_size = std::max((*rp) - (*prev), max_size);
	  }
	}

	if (*prev < region.getTps().size() - 1) {
	  m.emplace_back(region.getTps().substr(*prev, max_size));
	}
  }

  if (m.size() > 1 && m[0].size() > 1) {
	// align the records (one alternative to 'center star' algorithm is ClustalW)
	//align(m);
	std::cerr << "1-RxC = " << m.size() << " " << m[0].size() << std::endl;
	region.setScore(centerStar(m));
	std::cerr << "2-RxC = " << m.size() << " " << m[0].size() << std::endl;

	// and extracts them
	extractRecords(m, recpos, region);
  }
}

void DSRECenterStar::extractRecords(std::vector<std::wstring> &m,
                          std::set<size_t> &recpos, DSREDataRegion &region) {
  if ((m.size() == 0) || (recpos.size() == 0))
    return;

  int rows = m.size(), cols = m[0].size();

  auto rp = recpos.begin();
  for (int i = 0; i < rows; i++, ++rp) {
    Record rec;

    std::cerr << std::endl;
    for (int j = 0, k = 0; j < cols; j++) {
      if (m[i][j] != 0) {
        auto node = region.getNodeSequence()[(*rp) + k];
        rec.emplace_back(node);

        auto tagName = node->getTagName();
        std::cerr << "[" << node->toString() << "]; ";

        k++;
      } else
        rec.emplace_back(nullptr);
    }
    region.addRecord(rec);
  }
  region.cleanup();
  std::cerr << std::endl;
}
