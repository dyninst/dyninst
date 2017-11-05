#include <atomic>
#include <iterator>


template<typename T>
class WaitFreeQueueItem {
  typedef WaitFreeQueueItem<T> item_type;
public:
  WaitFreeQueueItem(T __value) : _next(0), _value(__value) {};
  void setNext(item_type *__next) { _next.store(__next); };
  item_type *next() { return _next.load(); };
  T value() { return _value; };
private:
  std::atomic<item_type *> _next;
  T _value;
};


template<typename T>
class WaitFreeQueueIterator {
  typedef WaitFreeQueueItem<T> item_type;
  typedef WaitFreeQueueIterator<T> iterator;
public:
  typedef T value_type;
  typedef T * pointer;
  typedef T & reference;
  typedef std::ptrdiff_t difference_type;
  typedef std::forward_iterator_tag iterator_category;
public:
  WaitFreeQueueIterator(item_type *_item) : item(_item) {};
  T operator *() { return item->value(); };
  bool operator != (iterator i) { return item != i.item; };
  iterator operator ++() { 
    if (item) item = item->next(); 
    return *this; 
  };
  iterator operator ++(int) { 
    iterator clone(*this);
    if (item) item = item->next(); 
    return clone;
  };
private:
  item_type *item;
};


// wait-free concurrent operations:
//    insert: either an value or a chain of items can be concurrently added to the front of the queue
//    splice: an entire queue of items can be concurrently added to the front of the destination queue
//       note: it is unsafe to concurrenty insert into the source for splice while it is being spliced
template<typename T>
class WaitFreeQueue {
public:
  typedef WaitFreeQueueIterator<T> iterator;
  typedef WaitFreeQueueItem<T> item_type; 
public:
  WaitFreeQueue() : head(0), tail(0) {};
  void insert(T value) {
    item_type *entry = new item_type(value);
    item_type *oldhead = head.exchange(entry);
    entry->setNext(oldhead);
    if (!oldhead) tail = entry;
  };
  void insert(item_type *seqFirst, item_type *seqLast) { 
    item_type *oldhead = head.exchange(seqFirst);
    seqLast->setNext(oldhead);
    if (!oldhead) tail = seqLast;
  };
  void splice(WaitFreeQueue<T> &other) {
    if (other.head) {
      insert(other.head, other.tail);
      other.reset();
    }
  }
  iterator begin() { return iterator(head.load()); };
  iterator end() { return iterator(0); };
  ~WaitFreeQueue() { clear(); };
private:
  item_type *pop() { 
    item_type *first = head.load();
    if (first) head.store(first->next());
    return first;
  };
  void clear() { 
    item_type *first;
    while((first = pop())) { 
      delete first;
    }
  };
  void reset() {
    head.store(0);
    tail = 0;
  };
private:
  std::atomic<item_type *> head;
  item_type * tail;
};
