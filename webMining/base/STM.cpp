/*
 * STM.cpp
 *
 *  Created on: 21 de jan de 2020
 *      Author: rvelloso
 */

#include "Node.hpp"
#include "STM.hpp"

bool operator==(const Node &a, const Node &b) {
	return a.getTagName() == b.getTagName();
}

bool operator!=(const Node &a, const Node &b) {
	return !(a == b);
}

int STM::match(pNode a, pNode b, bool align) {
	if (*a != *b) {
		return 0;
	}

	size_t k=a->getChildCount();
	size_t n=b->getChildCount();
	std::vector<std::vector<int>> m(k+1,std::vector<int>(n+1));

	for (size_t i=0; i<=k; i++) {
		m[i][0]=0;
	}

	for (size_t j=0; j<=n; j++) {
		m[0][j]=0;
	}

	size_t i = 1;
	for (auto child_a = a->child(); child_a; child_a = child_a->next(), ++i) {
		size_t j = 1;
		for (auto child_b = b->child(); child_b; child_b = child_b->next(), ++j) {
			int z = m[i-1][j-1] + match(child_a, child_b, align);
			m[i][j] = std::max(std::max(m[i][j-1],m[i-1][j]),z);
		}
	}

	if (align) {
		alignTrees(a, b, m);
	}

	return m[k][n]+1;
}

void STM::alignTrees(
		Node* a,
		Node* b,
		const std::vector<std::vector<int>> &m) {

	size_t i = a->getChildCount();
	size_t j = b->getChildCount();
	std::vector<Node *> child_a, child_b;
	for (auto ch = a->child(); ch; ch = ch->next()) {
		child_a.emplace_back(ch);
	}
	for (auto ch = b->child(); ch; ch = ch->next()) {
		child_b.emplace_back(ch);
	}
	auto current_ch_a = child_a.rbegin();
	auto current_ch_b = child_b.rbegin();

	while (m[i][j]) {
		if (m[i-1][j-1] >= m[i-1][j] && m[i-1][j-1] >= m[i][j-1]) {
			(*current_ch_a)->align(*current_ch_b);
			--i;
			--j;
			++current_ch_a;
			++current_ch_b;
		} else if (m[i-1][j] >= m[i-1][j-1] && m[i-1][j] >= m[i][j-1]) {
			--i;
			++current_ch_a;
		} else {
			--j;
			++current_ch_b;
		}
	}
}
