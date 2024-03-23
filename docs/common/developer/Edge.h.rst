.. _`sec:dev-Edge.h`:

Edge.h
######

.. cpp:namespace:: Dyninst::dev

.. cpp:class:: Edge

  .. cpp:type:: private boost::shared_ptr<Node> NodeSharedPtr
  .. cpp:type:: private boost::weak_ptr<Node> NodePtr

  These member functions are technically all private because :cpp:type:`NodeSharedPtr` and
  :cpp:type:`NodePtr` are private.

  .. cpp:function:: static Ptr createEdge(const NodeSharedPtr source, const NodeSharedPtr target);

    Create a new directed edge from ``source`` to ``target``.

  .. cpp:function:: NodeSharedPtr source() const
  .. cpp:function:: NodeSharedPtr target() const

    Return the source / target Dyninst::Node.

  .. cpp:function:: void setSource(NodeSharedPtr source)
  .. cpp:function:: void setTarget(NodeSharedPtr target)

.. cpp:class:: EdgeIterator

  .. cpp:type:: protected EdgeIteratorImpl *iter_

     We hide the internal iteration behavior behind a pointer.
     This allows us to override.

  .. cpp:function:: EdgeIterator()

    Make sure this is explicitly *not* allowed (no vectors of iterators).


.. cpp:class:: EdgeIteratorImpl

    .. cpp:function:: virtual void inc() = 0;
    .. cpp:function:: virtual Dyninst::Edge::Ptr get() = 0;
    .. cpp:function:: virtual bool equals(EdgeIteratorImpl *) = 0;
    .. cpp:function:: virtual EdgeIteratorImpl *copy() = 0;


.. cpp:class:: EdgeIteratorSet : public EdgeIteratorImpl

  Types of edge iteration: over a set of edges

  .. cpp:function:: virtual void inc()
  .. cpp:function:: virtual Edge::Ptr get()
  .. cpp:function:: virtual bool equals(EdgeIteratorImpl *rhs)
  .. cpp:function:: virtual EdgeIteratorImpl *copy()
  .. cpp:function:: virtual ~EdgeIteratorSet()
  .. cpp:function:: EdgeIteratorSet(const std::unordered_set<Edge::Ptr, Edge::EdgePtrHasher>::iterator iter)
  .. cpp:member:: private std::unordered_set<Edge::Ptr, Edge::EdgePtrHasher>::iterator internal_
