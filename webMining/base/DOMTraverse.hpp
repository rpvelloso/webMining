/*
 * DOMTraverse.hpp
 *
 *  Created on: 2 de ago de 2016
 *      Author: rvelloso
 */

#ifndef BASE_DOMTRAVERSE_HPP_
#define BASE_DOMTRAVERSE_HPP_

#include <unordered_set>
#include <stack>
#include <queue>

template<typename Item>
class TraverseStrategyInterface {
public:
	TraverseStrategyInterface() {};
	virtual ~TraverseStrategyInterface() {};
	virtual Item top() const = 0;
	virtual void pop() = 0;
	virtual void push(const Item &i) = 0;
	virtual bool empty() const = 0;
	virtual size_t size() const = 0;
};

template<typename Item>
class QueueTraverseStrategy : public TraverseStrategyInterface<Item> {
public:
	QueueTraverseStrategy() : TraverseStrategyInterface<Item>() {};

	Item top() const {
		return queue.front();
	}

	void pop() {
		queue.pop();
	}

	void push(const Item &i) {
		queue.push(i);
	}

	bool empty() const {
		return queue.empty();
	}

	size_t size() const {
		return queue.size();
	}
private:
	std::queue<Item> queue;
};

template<typename Item>
class StackTraverseStrategy : public TraverseStrategyInterface<Item> {
public:
	StackTraverseStrategy() : TraverseStrategyInterface<Item>() {};

	Item top() const {
		return stack.top();
	}

	void pop() {
		stack.pop();
	}

	void push(const Item &i) {
		stack.push(i);
	}

	bool empty() const {
		return stack.empty();
	}

	size_t size() const {
		return stack.size();
	}
private:
	std::stack<Item> stack;
};

template<typename Item>
class TraverseContainer : public TraverseStrategyInterface<Item> {
public:
	TraverseContainer(TraverseStrategyInterface<Item> *c) : TraverseStrategyInterface<Item>(), container(c) {};
	Item top() const {
		return container->top();
	}

	void pop() {
		container->pop();
	}

	void push(const Item &i) {
		if (visited.count(i) == 0)
			container->push(i);
	}

	bool empty() const {
		return container->empty();
	}

	size_t size() const {
		return container->size();
	}
private:
	std::unique_ptr<TraverseStrategyInterface<Item> > container;
	std::unordered_set<Item> visited;
};

#endif /* BASE_DOMTRAVERSE_HPP_ */
