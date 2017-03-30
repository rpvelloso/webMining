/*
 * DSRECenterStar.cpp
 *
 *  Created on: 30 de mar de 2017
 *      Author: rvelloso
 */

#include "DSRECenterStar.hpp"

DSRECenterStar::DSRECenterStar() {
}

DSRECenterStar::~DSRECenterStar() {
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
