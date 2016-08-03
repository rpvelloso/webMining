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

template<typename Item>
class TraverseStrategyFactory {
public:
	static TraverseStrategyInterface<Item> *get(int strategy) {
		switch (strategy) {
		case 0:
			return new StackTraverseStrategy<Item>();
		case 1:
			return new QueueTraverseStrategy<Item>();
		}
		return new StackTraverseStrategy<Item>();
	}
private:
	TraverseStrategyFactory() = delete;
};

#endif /* BASE_DOMTRAVERSE_HPP_ */
