#ifndef __DYNERLIST__
#define __DYNERLIST__

#ifdef USE_STL_VECTOR
#define DynerList	list
#else

template<class T> 
class DynerList {
	struct ListItem {
		T _item;
		ListItem *_next;
		ListItem() {_item = _nullT; _next = NULL; }
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

	T _nullT;
	iterType _begin;
	iterType _end;
	ListItem *_vector;
	int _size;
public:
	typedef iterType iterator;

        DynerList();
        ~DynerList();

        const T& operator[](int n) const;
        bool erase(const iterType &x);
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
bool DynerList<T>::erase(const iterType &x) {
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

	if (item == _vector)
		_vector = _vector->_next;
	else
		prev->_next = item->_next;

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
#endif

#endif //__DYNERLIST__
