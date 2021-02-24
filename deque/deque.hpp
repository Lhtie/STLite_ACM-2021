#ifndef SJTU_DEQUE_HPP
#define SJTU_DEQUE_HPP

#include "exceptions.hpp"

#include <cstddef>

namespace sjtu { 

template<class T>
class deque {
private:
	const static int BLOCK_SIZE = 400;
	struct block{
		T* arr[BLOCK_SIZE+10];
		size_t size;
		block *prv, *nxt;
		block() : size(0), prv(nullptr), nxt(nullptr) {
			for (int i = 0; i < BLOCK_SIZE + 10; ++i)
				arr[i] = nullptr;
		}
		void load(block *other) {
			size = other->size;
			for (int i = 0; i < BLOCK_SIZE + 10; ++i)
				if (other->arr[i]) arr[i] = new T(*other->arr[i]);
		}
		~block() {
			for (int i = 0; i < BLOCK_SIZE + 10; ++i)
				delete arr[i];
		}
	};
	block *head, *tail;
	size_t num;
	void del(){
		block *p = head, *q = p->nxt;
		while (q){
			delete p;
			p = q, q = q->nxt;
		}
		delete p;
		head = tail = nullptr, num = 0;
	}
	void copy(block *p){
		head = new block, head->load(p);
		head->prv = nullptr;
		block *q = head;
		while (p->nxt){
			q->nxt = new block, q->nxt->load(p->nxt);
			q->nxt->prv = q;
			q = q->nxt, p = p->nxt;
		}
		q->nxt = nullptr, tail = q;
	}
	void assign(T *&p, const T &x){
		if (!p) p = new T(x);
		else *p = x;
	}
	void destroy(T *&p){
		delete p; p = nullptr;
	}
	T &loc(size_t pos) const{
		block *p = head;
		while (pos){
			if (p == tail) throw index_out_of_bound();
			if (pos < p->size) return *p->arr[pos];
			pos -= p->size, p = p->nxt;
		}
		if (p == tail) throw index_out_of_bound();
		else return *p->arr[0];
	}
public:
	class const_iterator;
	class iterator {
		friend class deque<T>;
	private:
		/**
		 * TODO add data members
		 *   just add whatever you want.
		 */
		block *block_ptr;
		int offset;
		deque *deque_ptr;
	public:
		iterator() : block_ptr(nullptr), offset(0), deque_ptr(nullptr) {}
		iterator(block *block_ptr_, int offset_, deque *deque_ptr_):
			block_ptr(block_ptr_), offset(offset_), deque_ptr(deque_ptr_) {}
		bool valid() const {
			return deque_ptr && block_ptr && block_ptr != deque_ptr->tail
				&& offset >= 0 && offset < block_ptr->size;
		}
		/**
		 * return a new iterator which pointer n-next elements
		 *   if there are not enough elements, iterator becomes invalid
		 * as well as operator-
		 */
		iterator operator+(const int &n) const {
			//TODO
			if (n < 0) return *this - (-n);
			if (offset + n < block_ptr->size){
				return iterator(block_ptr, offset + n, deque_ptr);
			} else{
				block *p = block_ptr->nxt;
				if (!p) throw invalid_iterator();
				int r = n - (block_ptr->size - offset);
				while (p != deque_ptr->tail && r >= p->size) r -= p->size, p = p->nxt;
				if (!r) return iterator(p, 0, deque_ptr);
				if (p == deque_ptr->tail) throw invalid_iterator();
				return iterator(p, r, deque_ptr);
			}
		}
		iterator operator-(const int &n) const {
			//TODO
			if (n < 0) return *this + (-n);
			if (offset - n >= 0){
				return iterator(block_ptr, offset - n, deque_ptr);
			} else{
				block *p = block_ptr->prv, *q = block_ptr;
				int r = n - offset;
				while (p && r >= p->size) r -= p->size, q = p, p = p->prv;
				if (!r) return iterator(q, 0, deque_ptr);
				if (!p) throw invalid_iterator();
				return iterator(p, p->size - r, deque_ptr);
			}
		}
		// return th distance between two iterator,
		// if these two iterators points to different vectors, throw invaild_iterator.
		int operator-(const iterator &rhs) const {
			//TODO
			if (deque_ptr != rhs.deque_ptr) throw invalid_iterator();
			if (rhs.block_ptr == block_ptr) return offset - rhs.offset;
			int ret = block_ptr->size - offset;
			block *p = block_ptr->nxt;
			while (p && p != rhs.block_ptr) ret += p->size, p = p->nxt;
			if (p == rhs.block_ptr) return -(ret + rhs.offset);
			ret = offset;
			p = block_ptr->prv;
			while (p && p != rhs.block_ptr) ret += p->size, p = p->prv;
			return ret + (p->size - rhs.offset);
		}
		iterator& operator+=(const int &n) {
			//TODO
			return *this = *this + n;
		}
		iterator& operator-=(const int &n) {
			//TODO
			return *this = *this - n;
		}
		/**
		 * TODO iter++
		 */
		iterator operator++(int) {
			iterator ret = *this;
			*this += 1;
			return ret;
		}
		/**
		 * TODO ++iter
		 */
		iterator& operator++() { return *this += 1; }
		/**
		 * TODO iter--
		 */
		iterator operator--(int) {
			iterator ret = *this;
			*this -= 1;
			return ret;
		}
		/**
		 * TODO --iter
		 */
		iterator& operator--() { return *this -= 1; }
		/**
		 * TODO *it
		 * 		throw if iterator is invalid
		 */
		T &operator*() const {
			if (!valid()) throw invalid_iterator();
			return *block_ptr->arr[offset];
		}
		/**
		 * TODO it->field
		 * 		throw if iterator is invalid
		 */
		T *operator->() const noexcept {
			if (!valid()) throw invalid_iterator();
			return block_ptr->arr[offset];
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &rhs) const {
			return block_ptr == rhs.block_ptr && offset == rhs.offset;
		}
		bool operator==(const const_iterator &rhs) const {
			return block_ptr == rhs.block_ptr && offset == rhs.offset;
		}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
		bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
	};
	class const_iterator {
	// it should has similar member method as iterator.
	//  and it should be able to construct from an iterator.
		friend class deque<T>;
	private:
		// data members.
		block *block_ptr;
		int offset;
		const deque *deque_ptr;
	public:
		const_iterator() : block_ptr(nullptr), offset(0), deque_ptr(nullptr) {}
		const_iterator(block *block_ptr_, int offset_, const deque *deque_ptr_):
				block_ptr(block_ptr_), offset(offset_), deque_ptr(deque_ptr_) {}
		const_iterator(const iterator &other) :
			block_ptr(other.block_ptr), offset(other.offset), deque_ptr(other.deque_ptr) {}
		bool valid() const {
			return deque_ptr && block_ptr && block_ptr != deque_ptr->tail
				&& offset >= 0 && offset < block_ptr->size;
		}
		// And other methods in iterator.
		const_iterator operator+(const int &n) const {
			//TODO
			if (n < 0) return *this - (-n);
			if (offset + n < block_ptr->size){
				return const_iterator(block_ptr, offset + n, deque_ptr);
			} else{
				block *p = block_ptr->nxt;
				if (!p) throw invalid_iterator();
				int r = n - (block_ptr->size - offset);
				while (p != deque_ptr->tail && r >= p->size) r -= p->size, p = p->nxt;
				if (!r) return const_iterator(p, 0, deque_ptr);
				if (p == deque_ptr->tail) throw invalid_iterator();
				return const_iterator(p, r, deque_ptr);
			}
		}
		const_iterator operator-(const int &n) const {
			//TODO
			if (n < 0) return *this + (-n);
			if (offset - n >= 0){
				return const_iterator(block_ptr, offset - n, deque_ptr);
			} else{
				block *p = block_ptr->prv, *q = block_ptr;
				int r = n - offset;
				while (p && r >= p->size) r -= p->size, q = p, p = p->prv;
				if (!r) return const_iterator(q, 0, deque_ptr);
				if (!p) throw invalid_iterator();
				return const_iterator(p, p->size - r, deque_ptr);
			}
		}
		// return th distance between two iterator,
		// if these two iterators points to different vectors, throw invaild_iterator.
		int operator-(const const_iterator &rhs) const {
			//TODO
			if (deque_ptr != rhs.deque_ptr) throw invalid_iterator();
			if (rhs.block_ptr == block_ptr) return offset - rhs.offset;
			int ret = block_ptr->size - offset;
			block *p = block_ptr->nxt;
			while (p && p != rhs.block_ptr) ret += p->size, p = p->nxt;
			if (p == rhs.block_ptr) return -(ret + rhs.offset);
			ret = offset;
			p = block_ptr->prv;
			while (p && p != rhs.block_ptr) ret += p->size, p = p->prv;
			return ret + (p->size - rhs.offset);
		}
		const_iterator& operator+=(const int &n) {
			//TODO
			return *this = *this + n;
		}
		const_iterator& operator-=(const int &n) {
			//TODO
			return *this = *this - n;
		}
		/**
		 * TODO iter++
		 */
		const_iterator operator++(int) {
			const_iterator ret = *this;
			*this += 1;
			return ret;
		}
		/**
		 * TODO ++iter
		 */
		const_iterator& operator++() { return *this += 1; }
		/**
		 * TODO iter--
		 */
		const_iterator operator--(int) {
			const_iterator ret = *this;
			*this -= 1;
			return ret;
		}
		/**
		 * TODO --iter
		 */
		const_iterator& operator--() { return *this -= 1; }
		/**
		 * TODO *it
		 * 		throw if iterator is invalid
		 */
		const T &operator*() const {
			if (!valid()) throw invalid_iterator();
			return *block_ptr->arr[offset];
		}
		/**
		 * TODO it->field
		 * 		throw if iterator is invalid
		 */
		const T *operator->() const noexcept {
			if (!valid()) throw invalid_iterator();
			return block_ptr->arr[offset];
		}
		/**
		 * a operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &rhs) const {
			return block_ptr == rhs.block_ptr && offset == rhs.offset;
		}
		bool operator==(const const_iterator &rhs) const {
			return block_ptr == rhs.block_ptr && offset == rhs.offset;
		}
		/**
		 * some other operator for iterator.
		 */
		bool operator!=(const iterator &rhs) const { return !(*this == rhs); }
		bool operator!=(const const_iterator &rhs) const { return !(*this == rhs); }
	};
	/**
	 * TODO Constructors
	 */
	deque() : num(0) {
		head = tail = new block;
		tail->prv = tail->nxt = nullptr, tail->size = 1;
	}
	deque(const deque &other) : num(other.num) {
		copy(other.head);
	}
	/**
	 * TODO Deconstructor
	 */
	~deque() { del(); }
	/**
	 * TODO assignment operator
	 */
	deque &operator=(const deque &other) {
		if (this == &other) return *this;
		del(), copy(other.head);
		num = other.num;
		return *this;
	}
	/**
	 * access specified element with bounds checking
	 * throw index_out_of_bound if out of bound.
	 */
	T & at(const size_t &pos) { return loc(pos); }
	const T & at(const size_t &pos) const { return loc(pos); }
	T & operator[](const size_t &pos) { return loc(pos); }
	const T & operator[](const size_t &pos) const { return loc(pos); }
	/**
	 * access the first element
	 * throw container_is_empty when the container is empty.
	 */
	const T & front() const {
		if (empty()) throw container_is_empty();
		return *head->arr[0];
	}
	/**
	 * access the last element
	 * throw container_is_empty when the container is empty.
	 */
	const T & back() const {
		if (empty()) throw container_is_empty();
		return *tail->prv->arr[tail->prv->size-1];
	}
	/**
	 * returns an iterator to the beginning.
	 */
	iterator begin() {
		return iterator(head, 0, this);
	}
	const_iterator cbegin() const {
		return const_iterator(head, 0, this);
	}
	/**
	 * returns an iterator to the end.
	 */
	iterator end() {
		return iterator(tail, 0, this);
	}
	const_iterator cend() const {
		return const_iterator(tail, 0, this);
	}
	/**
	 * checks whether the container is empty.
	 */
	bool empty() const { return !num; }
	/**
	 * returns the number of elements
	 */
	size_t size() const { return num; }
	/**
	 * clears the contents
	 */
	void clear() {
		del();
		head = tail = new block;
		tail->prv = tail->nxt = nullptr, tail->size = 1;
	}
	/**
	 * inserts elements at the specified locat on in the container.
	 * inserts value before pos
	 * returns an iterator pointing to the inserted value
	 *     throw if the iterator is invalid or it point to a wrong place.
	 */
	iterator insert(iterator pos, const T &value) {
		if (pos.deque_ptr != this) throw invalid_iterator();
		if (pos.block_ptr == tail){
			if (empty()){
				head = new block;
				head->size = num = 1, assign(head->arr[0], value);
				head->nxt = tail, tail->prv = head;
			} else {
				block *p = tail->prv;
				assign(p->arr[p->size++], value);
				num++;
				if (p->size > BLOCK_SIZE){
					block *q = new block;
					q->size = p->size / 2;
					p->size -= q->size;
					for (int i = 0, j = p->size; j < p->size + q->size; ++i, ++j)
						assign(q->arr[i], *p->arr[j]), destroy(p->arr[j]);
					p->nxt = q, q->nxt = tail, tail->prv = q, q->prv = p;
				}
			}
			return end() - 1;
		}
		if (!pos.valid()) throw invalid_iterator();
		block *p = pos.block_ptr;
		size_t offset = pos.offset;
		for (int i = p->size; i > offset; --i)
			assign(p->arr[i], *p->arr[i-1]);
		assign(p->arr[offset], value);
		num++, p->size++;
		if (p->size > BLOCK_SIZE){
			block *q = new block;
			q->size = p->size / 2;
			p->size -= q->size;
			for (int i = 0, j = p->size; j < p->size + q->size; ++i, ++j)
				assign(q->arr[i], *p->arr[j]), destroy(p->arr[j]);
			block *r = p->nxt;
			p->nxt = q, q->nxt = r, r->prv = q, q->prv = p;
			if (offset < p->size) return iterator(p, offset, this);
			else return iterator(q, offset - p->size, this);
		}
		return iterator(p, offset, this);
	}
	/**
	 * removes specified element at pos.
	 * removes the element at pos.
	 * returns an iterator pointing to the following element, if pos pointing to the last element, end() will be returned.
	 * throw if the container is empty, the iterator is invalid or it points to a wrong place.
	 */
	iterator erase(iterator pos) {
		if (empty()) throw container_is_empty();
		if (pos.deque_ptr != this || !pos.valid())
			throw invalid_iterator();
		block *p = pos.block_ptr;
		size_t offset = pos.offset;
		for (int i = offset; i < p->size - 1; ++i)
			assign(p->arr[i], *p->arr[i+1]);
		destroy(p->arr[p->size-1]);
		num--, p->size--;
		if (empty()){
			delete head; head = tail;
			return iterator(tail, 0, this);
		}
		if (p->nxt != tail && p->size + p->nxt->size <= BLOCK_SIZE){
			block *q = p->nxt, *r = q->nxt;
			for (int i = p->size, j = 0; j < q->size; ++i, ++j)
				assign(p->arr[i], *q->arr[j]), destroy(q->arr[j]);
			p->size += q->size;
			p->nxt = r, r->prv = p;
			delete q;
		}
		if (offset == p->size) return iterator(p->nxt, 0, this);
		else return iterator(p, offset, this);
	}
	/**
	 * adds an element to the end
	 */
	void push_back(const T &value) {
		insert(end(), value);
	}
	/**
	 * removes the last element
	 *     throw when the container is empty.
	 */
	void pop_back() {
		if (empty()) throw container_is_empty();
		erase(end() - 1);
	}
	/**
	 * inserts an element to the beginning.
	 */
	void push_front(const T &value) {
		insert(begin(), value);
	}
	/**
	 * removes the first element.
	 *     throw when the container is empty.
	 */
	void pop_front() {
		if (empty()) throw container_is_empty();
		erase(begin());
	}
};

}

#endif
