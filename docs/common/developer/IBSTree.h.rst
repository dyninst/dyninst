.. _`sec:IBSTree.h`:

IBSTree.h
#########

.. cpp:namespace:: Dyninst

.. cpp:class:: template<class ITYPE = SimpleInterval<>> IBSTree

  **Interval Binary Search Tree**

  The implementation is based on a red-black tree (derived from our codeRange implementation)
  to control the tree height and thus insertion and search cost.

  Unlike our codeRangeTree, this data structure can represent overlapping
  intervals. It is useful for executing stabbing queries and more
  generally for finding invervals that overlap a particular interval.
  A stabbing query requires ``O(log(N) + L)`` time in the worst case, where
  L is the number of overlapping intervals, and insertion requires
  ``O(log^2(N))`` time (compare to ``O(log(N)``) for a standard RB-tree).

  This class requires a worst case storage ``O(N log(N))``.

  For more information::

    @TECHREPORT{Hanson90theibs-tree:,
      author = {Eric N. Hanson and Moez Chaabouni},
      title = {The IBS-tree: A data structure for finding all intervals that overlap a point},
      institution = {},
      year = {1990}
    }

  .. Attention:: All intervals must have lower bound predicate <= and upper bound predicate > ; that is, intervals are [a,b)

  .. cpp:type:: typename ITYPE::type interval_type
  .. cpp:type:: IBSNode<ITYPE>* iterator
  .. cpp:type:: const IBSNode<ITYPE>* const_iterator
  .. cpp:type:: ITYPE value_type
  .. cpp:type:: value_type& reference
  .. cpp:type:: const value_type& const_reference
  .. cpp:type:: size_t difference_type
  .. cpp:type:: size_t size_type

  .. cpp:member:: IBSNode<ITYPE> *nil

  .. cpp:function:: friend std::ostream& operator<<(std::ostream& stream, const IBSTree<ITYPE>& tree)
  .. cpp:function:: IBSTree()
  .. cpp:function:: size_type size() const
  .. cpp:function:: const_iterator begin() const
  .. cpp:function:: const_iterator end() const
  .. cpp:function:: int CountMarks() const
  .. cpp:function:: bool empty() const
  .. cpp:function:: void insert(ITYPE *)
  .. cpp:function:: void remove(ITYPE *)
  .. cpp:function:: int find(interval_type, std::set<ITYPE *> &) const
  .. cpp:function:: int find(ITYPE *I, std::set<ITYPE *> &) const
  .. cpp:function:: void successor(interval_type X, std::set<ITYPE *> &) const
  .. cpp:function:: ITYPE * successor(interval_type X) const
  .. cpp:function:: void clear()
  .. cpp:function:: void PrintPreorder()

.. cpp:namespace-push:: IBS

.. cpp:enum:: color_t

  .. cpp:enumerator:: TREE_RED
  .. cpp:enumerator:: TREE_BLACK

.. cpp:namespace-pop::

.. cpp:class:: template <typename T = int, typename U = void*> SimpleInterval

  .. cpp:type:: T type
  .. cpp:function:: SimpleInterval(T low, T high, U id)
  .. cpp:function:: virtual T low() const
  .. cpp:function:: virtual T high() const
  .. cpp:function:: virtual U id() const

.. cpp:class:: template<class ITYPE = SimpleInterval<>> IBSNode

  .. cpp:function:: IBSNode()
  .. cpp:function:: IBSNode(interval_type value, IBSNode *n)
  .. cpp:function:: interval_type value() const
  .. cpp:function:: interval_type operator*() const
  .. cpp:function:: friend std::ostream& operator<<(std::ostream& stream, const IBSNode<ITYPE>& node)
