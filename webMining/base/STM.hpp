/*
 * STM.hpp
 *
 *  Created on: 21 de jan de 2020
 *      Author: rvelloso
 */

#ifndef BASE_STM_HPP_
#define BASE_STM_HPP_

#include <vector>
#include "Node.hpp"

class STM {
public:
	STM() = delete;
	static int match(pNode a, pNode b, bool align = false);
private:
	static void alignTrees(
			pNode a,
			pNode b,
			const std::vector<std::vector<int>> &m);
};

#endif /* BASE_STM_HPP_ */
