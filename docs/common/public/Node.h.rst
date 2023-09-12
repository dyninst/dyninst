.. _`sec:Node.h`:

Node.h
######

.. cpp:namespace:: Dyninst

.. cpp:class:: Node

   .. cpp:type:: boost::shared_ptr<Node> Ptr;
      
      Shared pointer for Node
      
   .. cpp:function:: void ins(EdgeIterator &begin, EdgeIterator &end)
   .. cpp:function:: void outs(EdgeIterator &begin, EdgeIterator &end)
      
      Iterate over incoming/outgoing edges of this node.
      
   .. cpp:function:: void ins(NodeIterator &begin, NodeIterator &end)
   .. cpp:function:: void outs(NodeIterator &begin, NodeIterator &end)
      
      Iterate over adjacent nodes connected with incoming/outgoing edges of
      this node.
      
   .. cpp:function:: bool hasInEdges()
   .. cpp:function:: bool hasOutEdges()
      
      Return ``true`` if this node has incoming/outgoing edges.
      
   .. cpp:function:: void deleteInEdge(EdgeIterator e)
   .. cpp:function:: void deleteOutEdge(EdgeIterator e)
      
      Delete an incoming/outgoing edge.
      
   .. cpp:function:: virtual Address addr() const
      
      Return the address of this node.
      
   .. cpp:function:: virtual std::string format() const = 0;
      
      Return the string representation.
      
.. cpp:class:: NodeIterator
   
   Iterator for nodes. Common iterator operations including ``++``, ``–``,
   and dereferencing are supported.
   