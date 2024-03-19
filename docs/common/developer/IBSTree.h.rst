.. _`sec:IBSTree.h`:

IBSTree.h
#########

.. cpp:namespace:: Dyninst

.. cpp:class:: template <class ITYPE = SimpleInterval<>> IBSTree

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
  .. cpp:type:: IBSNode<ITYPE> *iterator
  .. cpp:type:: const IBSNode<ITYPE> *const_iterator
  .. cpp:type:: ITYPE value_type
  .. cpp:type:: value_type &reference
  .. cpp:type:: const value_type &const_reference
  .. cpp:type:: size_t difference_type
  .. cpp:type:: size_t size_type
  .. cpp:member:: IBSNode<ITYPE> *nil
  .. cpp:member:: private boost::atomic<int> treeSize

    size of tree

  .. cpp:member:: private IBSNode<ITYPE> *root

    pointer to the tree root

  .. cpp:member:: private mutable dyn_rwlock rwlock

    reader-writer lock to coordinate concurrent operations

  .. cpp:function:: private void leftRotate(IBSNode<ITYPE> *)

    RB-tree left rotation with modification to enforce IBS invariants

  .. cpp:function:: private void rightRotate(IBSNode<ITYPE> *)

    RB-tree right rotation with modification to enforce IBS invariants

  .. cpp:function:: private void removeInterval(IBSNode<ITYPE> *R, ITYPE *range)
  .. cpp:function:: private IBSNode<ITYPE> *addLeft(ITYPE *I, IBSNode<ITYPE> *R)

    Insert the left endpoint of an interval into the tree (may or may not add a node).

  .. cpp:function:: private IBSNode<ITYPE> *addRight(ITYPE *I, IBSNode<ITYPE> *R)

    Insert the right endpoint of an interval into the tree (may or may not add a node).

  .. cpp:function:: private interval_type rightUp(IBSNode<ITYPE> *R)

    Find the lowest valued ancestor of node R that has R in its left subtree.

    Used in addLeft to determine whether all of the values in R's right subtree
    are covered by an interval.

    Traverse upward in the tree, looking for the nearest ancestor that has R in its left subtree and
    return that value. Since this routine is used to compute an upper bound on an interval, failure to
    find a node should return ``positive infinity``.

  .. cpp:function:: private interval_type leftUp(IBSNode<ITYPE> *R)

    Same as :cpp:func:`rightUp`, only looking for the nearest ancestor node that has ``R`` in its RIGHT subtree.

    Returns ``NEGATIVE infinity`` upon failure.

  .. cpp:function:: private void insertFixup(IBSNode<ITYPE> *x)

    Tree-balancing algorithm on insertion.

    Restore RB-tree invariants after node insertion

  .. cpp:function:: private void destroy(IBSNode<ITYPE> *)

    Delete all nodes in the subtree rooted at the parameter.

  .. cpp:function:: private void findIntervals(interval_type X, IBSNode<ITYPE> *R, std::set<ITYPE *> &S) const

    Find all intervals that intersect an interval:

      If low is < a node, take the < set (any interval in < contains low)

      If low or high are > a node, take the > set

      If low <= a node and high > a node, take the = set

    Because this traversal may go both directions in the tree, it remains a recursive operation and is less
    efficient than a pointwise stabbing query.

  .. cpp:function:: private void findIntervals(ITYPE *I, IBSNode<ITYPE> *R, std::set<ITYPE *> &S) const
  .. cpp:function:: private void PrintPreorder(IBSNode<ITYPE> *n, int indent)
  .. cpp:function:: private std::ostream &doIndent(int n)
  .. cpp:function:: private int height(IBSNode<ITYPE> *n)
  .. cpp:function:: private int CountMarks(IBSNode<ITYPE> *R) const
  .. cpp:function:: IBSTree()
  .. cpp:function:: ~IBSTree()
  .. cpp:function:: size_type size() const
  .. cpp:function:: const_iterator begin() const
  .. cpp:function:: const_iterator end() const
  .. cpp:function:: int CountMarks() const
  .. cpp:function:: bool empty() const
  .. cpp:function:: void insert(ITYPE *)
  .. cpp:function:: void remove(ITYPE *)
  .. cpp:function:: int find(interval_type, std::set<ITYPE *> &) const

    Finds all intervals that overlap the provided point.

    Returns the number of intervals found.

  .. cpp:function:: int find(ITYPE *I, std::set<ITYPE *> &) const
  .. cpp:function:: void successor(interval_type X, std::set<ITYPE *> &) const

    Finds the very next interval(s) with ``left endpoint = supremum(X)``.

  .. cpp:function:: ITYPE *successor(interval_type X) const

    Use only when no two intervals share the same lower bound

  .. cpp:function:: void clear()

    Delete all entries in the tree

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
  .. cpp:member:: protected T low_
  .. cpp:member:: protected T high_
  .. cpp:member:: protected U id_

    some arbitrary unique identifier


.. cpp:class:: template<class ITYPE = SimpleInterval<>> IBSNode

  .. cpp:function:: IBSNode()
  .. cpp:function:: IBSNode(interval_type value, IBSNode *n)
  .. cpp:function:: ~IBSNode()
  .. cpp:function:: interval_type value() const
  .. cpp:function:: interval_type operator*() const
  .. cpp:member:: private interval_type val_

    The endpoint of an interval range

  .. cpp:member:: private std::set<ITYPE *> less

    Intervals indexed by this node

  .. cpp:member:: private std::set<ITYPE *> greater
  .. cpp:member:: private std::set<ITYPE *> equal
  .. cpp:member:: private IBS::color_t color
  .. cpp:member:: private IBSNode<ITYPE> *left
  .. cpp:member:: private IBSNode<ITYPE> *right
  .. cpp:member:: private IBSNode<ITYPE> *parent

