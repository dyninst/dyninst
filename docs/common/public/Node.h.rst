.. _`sec:Node.h`:

Node.h
######

.. cpp:namespace:: Dyninst

.. cpp:class:: Node

  **A node in a graph**

  This is the node type used in :cpp:class:`Graph` to make an :cpp:class:`AST`.

  .. cpp:type:: boost::shared_ptr<Node> Ptr

    Shared pointer for Node.

  .. cpp:function:: void ins(EdgeIterator &begin, EdgeIterator &end)

    Iterates over incoming edges of this node.

  .. cpp:function:: void outs(EdgeIterator &begin, EdgeIterator &end)

    Iterates over outgoing edges of this node.

  .. cpp:function:: void ins(NodeIterator &begin, NodeIterator &end)

    Iterates over adjacent nodes connected with incoming edges of this node.

  .. cpp:function:: void outs(NodeIterator &begin, NodeIterator &end)

    Iterates over adjacent nodes connected with outgoing edges of this node.

  .. cpp:function:: bool hasInEdges()

    Checks if this node has any incoming edges.

  .. cpp:function:: bool hasOutEdges()

    Checks if this node has any outgoing edges.

  .. cpp:function:: void deleteInEdge(EdgeIterator e)

    Delete the incoming edge, ``e``.

  .. cpp:function:: void deleteOutEdge(EdgeIterator e)

    Delete the outgoing edge, ``e``.

  .. cpp:function:: void forwardClosure(NodeIterator &begin, NodeIterator &end)
  .. cpp:function:: void backwardClosure(NodeIterator &begin, NodeIterator &end)

  .. cpp:function:: virtual Address addr() const

    Returns the address of this node.

  .. cpp:function:: virtual std::string format() const = 0

    Returns the string representation.

  .. cpp:function:: virtual Node::Ptr copy() = 0

    Returns a deep copy of the current node.

  .. cpp:function:: virtual bool isVirtual() const = 0
  .. cpp:function:: virtual std::string DOTshape() const;
  .. cpp:function:: virtual std::string DOTrank() const;
  .. cpp:function:: virtual std::string DOTname() const;
  .. cpp:function:: virtual bool DOTinclude() const

.. cpp:class:: NodeIterator

  **Iterator for nodes**

  This class satisfies the requirements for
  `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_.
