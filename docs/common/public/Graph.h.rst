.. _`sec:Graph.h`:

Graph.h
#######

.. cpp:namespace:: Dyninst

.. cpp:class:: Graph
   
   We provide a generic graph interface, which allows users to add, delete,
   and iterate nodes and edges in a graph. Our slicing algorithms are
   implemented upon this graph interface, so users can inherit the defined
   classes for customization.
   
   .. cpp:type:: boost::shared_ptr<Graph> Ptr;
      
      Shared pointer for Graph
      
   .. cpp:function:: virtual void entryNodes(NodeIterator &begin, NodeIterator &end)
      
      The entry nodes (nodes without any incoming edges) of the graph.
      
   .. cpp:function:: virtual void exitNodes(NodeIterator &begin, NodeIterator &end)
      
      The exit nodes (nodes without any outgoing edges) of the graph.
      
   .. cpp:function:: virtual void allNodes(NodeIterator &begin, NodeIterator &end)
      
      Iterate all nodes in the graph.
      
   .. cpp:function:: bool printDOT(const std::string& fileName)
      
      Output the graph in dot format.
      
   .. cpp:function:: static Graph::Ptr createGraph()
      
      Return an empty graph.
      
   .. cpp:function:: void insertPair(NodePtr source, NodePtr target, EdgePtr edge = EdgePtr())
      
      Insert a pair of nodes into the graph and create a new edge ``edge``
      from ``source`` to ``target``.
      
   .. cpp:function:: virtual void insertEntryNode(NodePtr entry)
   .. cpp:function:: virtual void insertExitNode(NodePtr exit)
      
      Insert a node as an entry/exit node
      
   .. cpp:function:: virtual void markAsEntryNode(NodePtr entry)
   .. cpp:function:: virtual void markAsExitNode(NodePtr exit)
      
      Mark a node that has been added to this graph as an entry/exit node.
      
   .. cpp:function:: void deleteNode(NodePtr node)
   .. cpp:function:: void addNode(NodePtr node)
      
      Delete / Add a node.
      
   .. cpp:function:: bool isEntryNode(NodePtr node)
   .. cpp:function:: bool isExitNode(NodePtr node)
      
      Check whether a node is an entry / exit node
      
   .. cpp:function:: void clearEntryNodes()
   .. cpp:function:: void clearExitNodes()
      
      Clear the marking of entry / exit nodes. Note that the nodes are not
      deleted from the graph.
      
   .. cpp:function:: unsigned size() const
      
      Return the number of nodes in the graph.