/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
	class Key,
	class T,
	class Compare = std::less<Key>
>
class map {
public:
	/**
	 * the internal type of data.
	 * it should have a default constructor, a copy constructor.
	 * You can use sjtu::map as value_type by typedef.
	 */
	typedef pair<const Key, T> value_type;
	enum Color {
		red, black
	};
	/**
	 * see BidirectionalIterator at CppReference for help.
	 *
	 * if there is anything wrong throw invalid_iterator.
	 *     like it = map.begin(); --it;
	 *       or it = map.end(); ++end();
	 */
private:
	struct node {
		value_type *key;
		Color color; // 0: red 1: black
		node *fa, *lc, *rc;
		node() : key(nullptr), fa(nullptr), lc(nullptr), rc(nullptr) {}
		node(const value_type &key_, Color color_) :
			color(color_), fa(nullptr), lc(nullptr), rc(nullptr) { key = new value_type(key_); }
		~node() { delete key; }
	};
	node *nil, *root, *head;
	size_t num;
	Compare cmper;
	void copy(node *&cur, node *fa, node *other_cur, node *other_nil) {
		if (other_cur == other_nil) {
			cur = nil;
			return;
		}
		cur = new node;
		cur->key = new value_type(*other_cur->key), cur->color = other_cur->color, cur->fa = fa;
		copy(cur->lc, cur, other_cur->lc, other_nil), copy(cur->rc, cur, other_cur->rc, other_nil);
	}
	void del(node *cur) {
		if (cur == nil) return;
		del(cur->lc), del(cur->rc);
		delete cur;
	}
	void left_rotate(node *x){
		node *fa = x->fa, *y = x->rc, *z = y->lc;
		x->rc = z, z->fa = x;
		y->lc = x, x->fa = y;
		if (x == fa->lc) fa->lc = y; else fa->rc = y;
		y->fa = fa;
		std::swap(x->color, y->color);
		if (fa == nil) root = y;
	}
	void right_rotate(node *x){
		node *fa = x->fa, *y = x->lc, *z = y->rc;
		x->lc = z, z->fa = x;
		y->rc = x, x->fa = y;
		if (x == fa->lc) fa->lc = y; else fa->rc = y;
		y->fa = fa;
		std::swap(x->color, y->color);
		if (fa == nil) root = y;
	}
	void transplant(node *x, node *y){
		node *fa = x->fa, *lc = x->lc, *rc = x->rc;
		if (y == y->fa->lc) y->fa->lc = x; else y->fa->rc = x;
		x->fa = y->fa, x->lc = y->lc, x->rc = y->rc;
		y->lc->fa = x, y->rc->fa = x;
		if (x == fa->lc) fa->lc = y; else fa->rc = y;
		y->fa = fa, y->lc = lc, y->rc = rc;
		lc->fa = y, rc->fa = y;
		std::swap(x->color, y->color);
		if (root == x) root = y;
		else if (root == y) root = x;
	}
	void insert_fixup(node *x){
		for (node *fa = x->fa; fa != nil && fa->color == red && fa->fa != nil; ){
			node *y = fa == fa->fa->lc ? fa->fa->rc : fa->fa->lc;
			if (y->color == red){
				fa->color = y->color = black, fa->fa->color = red;
				x = fa->fa, fa = x->fa;
			} else {
				if (fa == fa->fa->lc){
					if (x == fa->rc) left_rotate(fa), std::swap(x, fa);
					right_rotate(fa->fa);
				} else {
					if (x == fa->lc) right_rotate(fa), std::swap(x, fa);
					left_rotate(fa->fa);
				}
				break;
			}
		}
		root->color = black;
	}
	void erase_fixup(node *x){
		if (x->fa == nil) return ;
		node *y = x == x->fa->lc ? x->fa->rc : x->fa->lc;
		if (y->color == red){
			if (y == x->fa->lc) right_rotate(x->fa);
			else left_rotate(x->fa);
		}
		y = x == x->fa->lc ? x->fa->rc : x->fa->lc;
		if (y == nil){
			x->fa->color = black; return ;
		}
		if (y->lc->color == red && y->rc->color == red)
			y->lc->color = y->rc->color = black;
		if (y->lc->color == black && y->rc->color == black){
			y->color = red;
			if (y->fa->color == black){
				erase_fixup(y->fa);
			} else y->fa->color = black;
		} else if (y->lc->color == red){
			y->lc->color = black;
			if (y == x->fa->lc) right_rotate(x->fa);
			else right_rotate(y), left_rotate(x->fa);
		} else {
			y->rc->color = black;
			if (y == x->fa->rc) left_rotate(x->fa);
			else left_rotate(y), right_rotate(x->fa);
		}
	}
	node *loc(const Key &key) const {
		node *cur = root;
		for (; cur != nil; ){
			if (cmper(key, cur->key->first)) cur = cur->lc;
			else if (cmper(cur->key->first, key)) cur = cur->rc;
			else break;
		}
		return cur;
	}
	node *getmin(node *cur) const {
		node *ret = cur;
		for (; cur != nil; ret = cur, cur = cur->lc) ;
		return ret;
	}
	node *getmax(node *cur) const {
		node *ret = cur;
		for (; cur != nil; ret = cur, cur = cur->rc) ;
		return ret;
	}
	node *prv(node *cur) const {
		if (cur->lc != nil) return getmax(cur->lc);
		else {
			for (; cur->fa != nil; cur = cur->fa)
				if (cur == cur->fa->rc) return cur->fa;
		}
		return nil;
	}
	node *suf(node *cur) const {
		if (cur->rc != nil) return getmin(cur->rc);
		else {
			for (; cur->fa != nil; cur = cur->fa)
				if (cur == cur->fa->lc) return cur->fa;
		}
		return nil;
	}
public:
	class const_iterator;
	class iterator {
		friend class map;
	private:
		/**
		 * TODO add data members
		 *   just add whatever you want.
		 */
		 node *node_ptr;
		 map *map_ptr;
	public:
		iterator() : node_ptr(nullptr), map_ptr(nullptr) {}
		iterator(node *node_ptr_, map *map_ptr_) : node_ptr(node_ptr_), map_ptr(map_ptr_) {}
		iterator(const iterator &other) : node_ptr(other.node_ptr), map_ptr(other.map_ptr) {}
		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
			if (node_ptr == map_ptr->nil) throw invalid_iterator();
			iterator ret = *this;
			node_ptr = map_ptr->suf(node_ptr);
			return ret;
		}
		/**
		 * TODO ++iter
		 */
		iterator & operator++() {
			if (node_ptr == map_ptr->nil) throw invalid_iterator();
			node_ptr = map_ptr->suf(node_ptr);
			return *this;
		}
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
			if (node_ptr == map_ptr->head) throw invalid_iterator();
			iterator ret = *this;
			if (node_ptr == map_ptr->nil)
				node_ptr = map_ptr->getmax(map_ptr->root);
			else node_ptr = map_ptr->prv(node_ptr);
			return ret;
		}
		/**
		 * TODO --iter
		 */
		iterator & operator--() {
			if (node_ptr == map_ptr->head) throw invalid_iterator();
			if (node_ptr == map_ptr->nil)
				node_ptr = map_ptr->getmax(map_ptr->root);
			else node_ptr = map_ptr->prv(node_ptr);
			return *this;
		}
		/**
		 * an operator to check whether two iterators are same (pointing to the same memory).
		 */
		value_type & operator*() const {
			if (node_ptr == map_ptr->nil) throw invalid_iterator();
			else return *node_ptr->key;
		}
		bool operator==(const iterator &rhs) const { return node_ptr == rhs.node_ptr; }
		bool operator==(const const_iterator &rhs) const { return node_ptr == rhs.node_ptr; }
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const { return node_ptr != rhs.node_ptr; }
		bool operator!=(const const_iterator &rhs) const { return node_ptr != rhs.node_ptr; }

		/**
		 * for the support of it->first. 
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		value_type* operator->() const noexcept {
			if (node_ptr == map_ptr->nil) throw invalid_iterator();
			else return node_ptr->key;
		}
	};
	class const_iterator {
		// it should has similar member method as iterator.
		//  and it should be able to construct from an iterator.
		friend class map;
	private:
		// data members.
		node *node_ptr;
		const map *map_ptr;
	public:
		const_iterator() : node_ptr(nullptr), map_ptr(nullptr) {}
		const_iterator(node *node_ptr_, const map *map_ptr_) : node_ptr(node_ptr_), map_ptr(map_ptr_) {}
		const_iterator(const const_iterator &other) : node_ptr(other.node_ptr), map_ptr(other.map_ptr) {}
		const_iterator(const iterator &other) : node_ptr(other.node_ptr), map_ptr(other.map_ptr) {}
		/**
		 * TODO iter++
		 */
		const_iterator operator++(int) {
			if (node_ptr == map_ptr->nil) throw invalid_iterator();
			const_iterator ret = *this;
			node_ptr = map_ptr->suf(node_ptr);
			return ret;
		}
		/**
		 * TODO ++iter
		 */
		const_iterator & operator++() {
			if (node_ptr == map_ptr->nil) throw invalid_iterator();
			node_ptr = map_ptr->suf(node_ptr);
			return *this;
		}
		/**
		 * TODO iter--
		 */
		const_iterator operator--(int) {
			if (node_ptr == map_ptr->head) throw invalid_iterator();
			const_iterator ret = *this;
			if (node_ptr == map_ptr->nil)
				node_ptr = map_ptr->getmax(map_ptr->root);
			else node_ptr = map_ptr->prv(node_ptr);
			return ret;
		}
		/**
		 * TODO --iter
		 */
		const_iterator & operator--() {
			if (node_ptr == map_ptr->head) throw invalid_iterator();
			if (node_ptr == map_ptr->nil)
				node_ptr = map_ptr->getmax(map_ptr->root);
			else node_ptr = map_ptr->prv(node_ptr);
			return *this;
		}
		/**
		 * an operator to check whether two iterators are same (pointing to the same memory).
		 */
		const value_type & operator*() const {
			if (node_ptr == map_ptr->nil) throw invalid_iterator();
			else return *node_ptr->key;
		}
		bool operator==(const iterator &rhs) const { return node_ptr == rhs.node_ptr; }
		bool operator==(const const_iterator &rhs) const { return node_ptr == rhs.node_ptr; }
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const { return node_ptr != rhs.node_ptr; }
		bool operator!=(const const_iterator &rhs) const { return node_ptr != rhs.node_ptr; }

		/**
		 * for the support of it->first.
		 * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
		 */
		const value_type* operator->() const noexcept {
			if (node_ptr == map_ptr->nil) throw invalid_iterator();
			else return node_ptr->key;
		}
	};
	/**
	 * TODO two constructors
	 */
	map() : num(0) {
		nil = new node; nil->color = black;
		root = head = nil;
	}
	map(const map &other) : num(other.num) {
		nil = new node; nil->color = black;
		copy(root, nil, other.root, other.nil);
		head = getmin(root);
	}
	/**
	 * TODO assignment operator
	 */
	map & operator=(const map &other) {
		if (this == &other) return *this;
		del(root);
		num = other.num;
		copy(root, nil, other.root, other.nil);
		head = getmin(root);
		return *this;
	}
	/**
	 * TODO Destructors
	 */
	~map() { del(root); delete nil; }
	/**
	 * TODO
	 * access specified element with bounds checking
	 * Returns a reference to the mapped value of the element with key equivalent to key.
	 * If no such element exists, an exception of type `index_out_of_bound'
	 */
	T & at(const Key &key) {
		node *tmp = loc(key);
		if (tmp == nil) throw index_out_of_bound();
		else return tmp->key->second;
	}
	const T & at(const Key &key) const {
		node *tmp = loc(key);
		if (tmp == nil) throw index_out_of_bound();
		else return tmp->key->second;
	}
	/**
	 * TODO
	 * access specified element 
	 * Returns a reference to the value that is mapped to a key equivalent to key,
	 *   performing an insertion if such key does not already exist.
	 */
	T & operator[](const Key &key) {
		node *tmp = loc(key);
		if (tmp == nil)
			return insert(value_type(key, T())).first.node_ptr->key->second;
		else return tmp->key->second;
	}
	/**
	 * behave like at() throw index_out_of_bound if such key does not exist.
	 */
	const T & operator[](const Key &key) const {
		node *tmp = loc(key);
		if (tmp == nil) throw index_out_of_bound();
		else return tmp->key->second;
	}
	/**
	 * return a iterator to the beginning
	 */
	iterator begin() { return iterator(head, this); }
	const_iterator cbegin() const { return const_iterator(head, this); }
	/**
	 * return a iterator to the end
	 * in fact, it returns past-the-end.
	 */
	iterator end() { return iterator(nil, this); }
	const_iterator cend() const { return const_iterator(nil, this); }
	/**
	 * checks whether the container is empty
	 * return true if empty, otherwise false.
	 */
	bool empty() const { return root == nil; }
	/**
	 * returns the number of elements.
	 */
	size_t size() const { return num; }
	/**
	 * clears the contents
	 */
	void clear() {
		num = 0, del(root);
		root = head = nil;
	}
	/**
	 * insert an element.
	 * return a pair, the first of the pair is
	 *   the iterator to the new element (or the element that prevented the insertion), 
	 *   the second one is true if insert successfully, or false.
	 */
	pair<iterator, bool> insert(const value_type &value) {
		node *pos = loc(value.first);
		if (pos != nil) return pair<iterator, bool>(iterator(pos, this), false);
		else {
			node *cur = root, *x;
			if (empty()){
				root = new node(value, black);
				root->fa = root->lc = root->rc = nil;
				cur = root;
			} else {
				for (; cur != nil;) {
					x = cur;
					if (cmper(value.first, cur->key->first)) cur = cur->lc; else cur = cur->rc;
				}
				cur = new node(value, red);
				if (cmper(value.first, x->key->first)) x->lc = cur; else x->rc = cur;
				cur->fa = x, cur->lc = cur->rc = nil;
				insert_fixup(cur);
			}
			num++, head = getmin(root);
			return pair<iterator, bool>(iterator(cur, this), true);
		}
	}
	/**
	 * erase the element at pos.
	 *
	 * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
	 */
	void erase(iterator pos) {
		if (pos.node_ptr == nil || pos.map_ptr != this)
			throw invalid_iterator();
		node *cur = pos.node_ptr;
		if (cur->lc != nil && cur->rc != nil){
			node *p = prv(cur);
			if (p->fa == cur) right_rotate(cur);
			else transplant(cur, p);
		}
		node *fa = cur->fa;
		if (cur->lc == nil && cur->rc == nil) {
			if (cur->color == black) erase_fixup(cur);
			if (fa == nil) root = nil;
			else if (cur == fa->lc) fa->lc = nil; else fa->rc = nil;
		} else {
			node *nxt = cur->lc == nil ? cur->rc : cur->lc;
			if (cur->color == black) nxt->color = black;
			if (fa == nil) root = nxt;
			else if (cur == fa->lc) fa->lc = nxt; else fa->rc = nxt;
			nxt->fa = fa;
		}
		delete cur;
		num--, head = getmin(root);
	}
	/**
	 * Returns the number of elements with key 
	 *   that compares equivalent to the specified argument,
	 *   which is either 1 or 0 
	 *     since this container does not allow duplicates.
	 * The default method of check the equivalence is !(a < b || b > a)
	 */
	size_t count(const Key &key) const {
		node *tmp = loc(key);
		return tmp == nil ? 0 : 1;
	}
	/**
	 * Finds an element with key equivalent to key.
	 * key value of the element to search for.
	 * Iterator to an element with key equivalent to key.
	 *   If no such element is found, past-the-end (see end()) iterator is returned.
	 */
	iterator find(const Key &key) { return iterator(loc(key), this); }
	const_iterator find(const Key &key) const { return const_iterator(loc(key), this); }
};

}

#endif
