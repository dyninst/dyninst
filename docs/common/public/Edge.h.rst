.. _`sec:Edge.h`:

Edge.h
######

.. cpp:namespace:: Dyninst

.. cpp:class:: Edge

   .. cpp:type:: boost::shared_ptr<Edge> Edge::Ptr;
      
      Shared pointer for ``Edge``.
      
   .. cpp:function:: static Edge::Ptr Edge::createEdge(const Node::Ptr source, const Node::Ptr target)
      
      Create a new directed edge from ``source`` to ``target``.
      
   .. cpp:function:: Node::Ptr Edge::source() const
   .. cpp:function:: Node::Ptr Edge::target() const
      
      Return the source / target node.
      
   .. cpp:function:: void Edge::setSource(Node::Ptr source)
   .. cpp:function:: void Edge::setTarget(Node::Ptr target)
      
      Set the source / target node.
      
.. cpp:namespace:: EdgeIterator

.. cpp:class:: EdgeIterator
   
   Iterator for edges. Common iterator operations including ``++``, ``–``,
   and dereferencing are supported.
