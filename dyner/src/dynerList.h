/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#ifndef __DYNERLIST__
#define __DYNERLIST__

#ifdef USE_STL_VECTOR
#include <list>
#include <algorithm>
#define DynerList	std::list
#else

template<class T> 
class DynerList {
	struct ListItem {
		T _item;
		ListItem *_next;
		ListItem() {_item = DynerList<T>::_nullT; _next = NULL; }
		ListItem(const T& item) { _item = item; _next = NULL; }
	};

	struct iterType {
		ListItem *_ptr;
		iterType() { _ptr = NULL; }
		void operator++() {
			if (!_ptr)
				return;
			_ptr = _ptr->_next;
		}
		void operator++(int) {
			if (!_ptr)
				return;
			_ptr = _ptr->_next;
		}
		void operator=(ListItem *item) {
			_ptr = item;
		}
		bool operator==(iterType& iter) {
			return _ptr == iter._ptr;
		}
		bool operator!=(iterType& iter) {
			return _ptr != iter._ptr;
		}
		T& operator*() {
			return _ptr->_item;
		}
	};

	iterType _begin;
	iterType _end;
	ListItem *_vector;
	int _size;
public:
	static T _nullT;
	typedef iterType iterator;

        DynerList();
        ~DynerList();

        const T& operator[](int n) const;
        bool erase(iterType &x);
        void clear();
        void push_back(const T& x);
        void push_front(const T& x);
	int size() { return _size; }
	iterator& begin() { 
		if (_vector)
			_begin._ptr = _vector;
		else
			_begin = _end;
		return _begin; 
	}
	iterator& end() { return _end; }
};

template<class T>
DynerList<T>::DynerList() { 
	_vector = NULL;
	_size = 0;
}

template<class T>
DynerList<T>::~DynerList() { 
	clear();
}

template<class T>
const T& DynerList<T>::operator[](int n) const {
	if ( (n<0) || (n>_size-1) )
		return _nullT;

	ListItem *item = _vector;

	for(int i=0; i <= n; ++i)
		item = item->_next;

	return item->_item;
}

template<class T>
bool DynerList<T>::erase(iterType &x) {
	ListItem *item, *prev;

	prev = item = _vector;

	while(item) {
		if (item == x._ptr)
			break;

		prev = item;
		item = item->_next;
	}

	if (!item)
		return false;

	if (item == _vector) {
		_vector = _vector->_next;
		x._ptr = _vector;    
	}else {
		prev->_next = item->_next;
		x._ptr = prev;  
	}

	delete item;
	return true;
}

template<class T>
void DynerList<T>::clear() {
	ListItem *prev;

	while(_vector) {
		prev = _vector;
		_vector = _vector->_next;
		delete prev;
	}
	_vector = NULL;
	_size = 0;
}


template<class T>
void DynerList<T>::push_back(const T& x) {
	if (!_vector) {
		_vector = new ListItem(x);
		return;
	}

	ListItem *item = _vector;
	while(item->_next)
		item = item->_next;

	item->_next = new ListItem(x);
}

template<class T>
void DynerList<T>::push_front(const T& x) {

	ListItem *item = new ListItem(x);
	item->_next = _vector;
	_vector = item;
}

template<class T> T DynerList<T>::_nullT;

#endif

#endif //__DYNERLIST__
