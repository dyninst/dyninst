.. _`sec:NodeIterator.h`:

NodeIterator.h
##############

.. cpp:namespace:: Dyninst

.. cpp:class:: NodeIteratorImpl

  .. cpp:function:: virtual void inc() = 0
  .. cpp:function:: virtual Node::Ptr get() = 0
  .. cpp:function:: virtual bool equals(NodeIteratorImpl *) = 0
  .. cpp:function:: virtual NodeIteratorImpl *copy() = 0
  .. cpp:function:: virtual ~NodeIteratorImpl()

.. cpp:class:: NodeIteratorSet : public NodeIteratorImpl

  **Iterator over a set of nodes**

  .. cpp:function:: virtual void inc()
  .. cpp:function:: virtual Node::Ptr get()
  .. cpp:function:: virtual bool equals(NodeIteratorImpl *rhs)
  .. cpp:function:: virtual NodeIteratorImpl *copy()
  .. cpp:function:: virtual ~NodeIteratorSet()
  .. cpp:function:: NodeIteratorSet(const std::unordered_set<Node::Ptr, Node::NodePtrHasher>::iterator iter)

.. cpp:class:: NodeFromEdgeSet : public NodeIteratorImpl

  .. cpp:enum:: iterType

    .. cpp:enumerator:: target
    .. cpp:enumerator:: source
    .. cpp:enumerator:: unset

  .. cpp:function:: virtual void inc()
  .. cpp:function:: virtual Node::Ptr get()
  .. cpp:function:: virtual bool equals(NodeIteratorImpl *rhs)
  .. cpp:function:: virtual NodeIteratorImpl *copy ()
  .. cpp:function:: virtual ~NodeFromEdgeSet()
  .. cpp:function:: NodeFromEdgeSet(const std::unordered_set<Edge::Ptr, Edge::EdgePtrHasher>::iterator iter, iterType type)

.. cpp:class:: NodeSearchIterator : public NodeIteratorImpl

  Linearizes access to a tree/graph structure.

  Since we operate over a graph there may be cycles. This
  is handled by keeping a visited set; once a node is encountered
  it is placed in the visited set.

  The iterator is defined to be "end" if ``current`` ``NULL``.

  The iterator is one step from "end" (that is, iter->inc == end) if ``current`` is
  not ``NULL`` and every element in the worklist has been visited.
  Due to this, we *must* describe the worklist and visited sets in terms
  of nodes, rather than iterators. Given an iterator, we cannot determine
  (without deep inspection) whether it contains an unvisited node. Deep inspection
  really obviates the point.

  .. Note:: TODO: reverse iteration

  .. cpp:enum:: Type

      .. cpp:enumerator:: depth
      .. cpp:enumerator:: breadth

  .. cpp:enum:: Direction

    .. cpp:enumerator:: in
    .. cpp:enumerator:: out

  .. cpp:function:: virtual void inc()
  .. cpp:function:: virtual Node::Ptr get()
  .. cpp:function:: virtual NodeIteratorImpl *copy()
  .. cpp:function:: NodeSearchIterator()
  .. cpp:function:: NodeSearchIterator(Node::Ptr cur, Direction d, Type t)
  .. cpp:function:: NodeSearchIterator(Node::Ptr cur, Direction d, Type t, std::deque<Node::Ptr> wl, std::set<Node::Ptr> v)
  .. cpp:function:: NodeSearchIterator(NodeIterator &rangeBegin, NodeIterator &rangeEnd, Direction d, Type t)
  .. cpp:function:: virtual ~NodeSearchIterator()

.. cpp:class:: NodeIteratorPredicateObj : public NodeIteratorImpl

  .. cpp:function:: virtual void inc()
  .. cpp:function:: virtual Node::Ptr get()
  .. cpp:function:: virtual bool equals(NodeIteratorImpl *rhs)
  .. cpp:function:: virtual NodeIteratorImpl *copy()
  .. cpp:function:: virtual ~NodeIteratorPredicateObj()
  .. cpp:function:: NodeIteratorPredicateObj(Graph::NodePredicate::Ptr p, NodeIterator &c, NodeIterator &n, NodeIterator &e)
  .. cpp:function:: NodeIteratorPredicateObj(Graph::NodePredicate::Ptr p, NodeIterator &b, NodeIterator &e)
  .. cpp:function:: void setNext()

.. cpp:class:: NodeIteratorPredicateFunc : public NodeIteratorImpl

  .. cpp:function:: virtual void inc()
  .. cpp:function:: virtual Node::Ptr get()
  .. cpp:function:: virtual bool equals(NodeIteratorImpl *rhs)
  .. cpp:function:: virtual NodeIteratorImpl *copy()
  .. cpp:function:: virtual ~NodeIteratorPredicateFunc()
  .. cpp:function:: NodeIteratorPredicateFunc(Graph::NodePredicateFunc p, void *u, NodeIterator &c, NodeIterator &n, NodeIterator &e)
  .. cpp:function:: NodeIteratorPredicateFunc(Graph::NodePredicateFunc p, void *u, NodeIterator &b, NodeIterator &e)
  .. cpp:function:: void setNext()
