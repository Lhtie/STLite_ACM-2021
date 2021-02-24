#ifndef SJTU_PRIORITY_QUEUE_HPP
#define SJTU_PRIORITY_QUEUE_HPP

#include <cstddef>
#include <functional>
#include "exceptions.hpp"

namespace sjtu {

/**
 * a container like std::priority_queue which is a heap internal.
 */
template<typename T, class Compare = std::less<T>>
class priority_queue {
private:
	struct node{
		T key;
		size_t dist;
		node *lc, *rc;
	};
	node *root;
	size_t num;
	Compare cmp;
	void copy(node *x, node *y){
		if (y->lc) x->lc = new node(*y->lc), copy(x->lc, y->lc);
		if (y->rc) x->rc = new node(*y->rc), copy(x->rc, y->rc);
	}
	void del(node *x){
		if (!x) return ;
		if (x->lc) del(x->lc);
		if (x->rc) del(x->rc);
		delete x;
	}
	node *merge(node *x, node *y){
		if (!x) return y;
		if (!y) return x;
		if (cmp(x->key, y->key)) std::swap(x, y);
		x->rc = merge(x->rc, y);
		if (!x->lc || x->lc->dist < x->rc->dist)
		std::swap(x->lc, x->rc);
		x->dist = (x->rc ? x->rc->dist : 0) + 1;
		return x;
	}
public:
	/**
	 * TODO constructors
	 */
	priority_queue() : root(nullptr), num(0) {}
	priority_queue(const priority_queue &other) {
		num = other.num;
		if (!other.root) return ;
		root = new node(*other.root);
		copy(root, other.root);
	}
	/**
	 * TODO deconstructor
	 */
	~priority_queue() { del(root); }
	/**
	 * TODO Assignment operator
	 */
	priority_queue &operator=(const priority_queue &other) {
		if (this == &other) return *this;
		del(root);
		num = other.num;
		if (!other.root) return *this;
		root = new node(*other.root);
		copy(root, other.root);
		return *this;
	}
	/**
	 * get the top of the queue.
	 * @return a reference of the top element.
	 * throw container_is_empty if empty() returns true;
	 */
	const T & top() const {
		if (empty()) throw container_is_empty();
		return root->key;
	}
	/**
	 * TODO
	 * push new element to the priority queue.
	 */
	void push(const T &e) {
		num++;
		node *other = new node({e, 0, nullptr, nullptr});
		root = merge(root, other);
	}
	/**
	 * TODO
	 * delete the top element.
	 * throw container_is_empty if empty() returns true;
	 */
	void pop() {
		if (empty()) throw container_is_empty();
		num--;
		node *l = root->lc, *r = root->rc;
		delete root;
		root = merge(l, r);
	}
	/**
	 * return the number of the elements.
	 */
	size_t size() const {
		return num;
	}
	/**
	 * check if the container has at least an element.
	 * @return true if it is empty, false if it has at least an element.
	 */
	bool empty() const {
		return root == nullptr;
	}
	/**
	 * return a merged priority_queue with at least O(logn) complexity.
	 */
	void merge(priority_queue &other) {
		num += other.num;
		root = merge(root, other.root);
		other.root = nullptr;
	}
};

}

#endif
