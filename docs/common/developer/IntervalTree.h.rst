.. _`sec:IntervalTree.h`:

IntervalTree.h
##############

.. cpp:namespace:: Dyninst

.. cpp:class:: template <class K, class V> IntervalTree

  .. cpp:type:: private typename std::map<K, std::pair<K, V> > Tree
  .. cpp:type:: private typename Tree::const_iterator c_iter
  .. cpp:type:: private typename Tree::const_reverse_iterator c_r_iter
  .. cpp:type:: private typename Tree::iterator Iter

  .. cpp:type:: typename std::pair<K, K> Range
  .. cpp:type:: typename std::pair<Range, V> Entry
  .. cpp:type:: typename Tree::iterator iterator
  .. cpp:type:: typename Tree::const_iterator const_iterator

  .. cpp:function:: iterator begin()
  .. cpp:function:: iterator end()
  .. cpp:function:: c_r_iter rbegin()
  .. cpp:function:: c_r_iter rend()
  .. cpp:function:: const_iterator begin() const
  .. cpp:function:: const_iterator end() const
  .. cpp:function:: int size() const
  .. cpp:function:: bool empty() const
  .. cpp:function:: void insert(K lb, K ub, V v)
  .. cpp:function:: void remove(K lb)
  .. cpp:function:: void erase(K lb)
  .. cpp:function:: bool find(K key, V &value) const
  .. cpp:function:: bool find(K key, K &l, K &u, V &value)
  .. cpp:function:: bool precessor(K key, K &l, K &u, V &v) const
  .. cpp:function:: bool precessor(K key, Entry &e) const
  .. cpp:function:: void elements(std::vector<Entry> &buffer) const
  .. cpp:function:: K lowest() const
  .. cpp:function:: K highest() const
  .. cpp:function:: void clear()
  .. cpp:function:: bool empty()
  .. cpp:function:: bool update(K lb, K newUB)
  .. cpp:function:: bool updateValue(K lb, V newVal)
