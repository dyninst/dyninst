.. _`sec:IBSTree-fast.h`:

IBSTree-fast.h
##############

.. cpp:namespace:: Dyninst

.. cpp:class:: template <typename ITYPE> IBSTree_fast

  .. cpp:type:: typename ITYPE::type interval_type
  .. cpp:member:: IBSTree<ITYPE> overlapping_intervals
  .. cpp:type:: boost::multi_index_container<ITYPE*, \
                boost::multi_index::indexed_by< \
                        boost::multi_index::ordered_unique< \
                                boost::multi_index::const_mem_fun<ITYPE, interval_type, &ITYPE::high> > \
                > > interval_set
  .. cpp:member:: interval_set unique_intervals

  .. cpp:function:: int size() const
  .. cpp:function:: bool empty() const
  .. cpp:function:: void insert(ITYPE*)
  .. cpp:function:: void remove(ITYPE*)
  .. cpp:function:: int find(interval_type, std::set<ITYPE*> &) const
  .. cpp:function:: int find(ITYPE* I, std::set<ITYPE*>&) const
  .. cpp:function:: void successor(interval_type X, std::set<ITYPE*>& ) const
  .. cpp:function:: ITYPE* successor(interval_type X) const
  .. cpp:function:: void clear()
  .. cpp:function:: friend std::ostream& operator<<(std::ostream& stream, const IBSTree_fast<ITYPE>& tree)
