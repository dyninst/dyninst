.. _`sec:Graph.h`:

Graph.h
#######

.. cpp:namespace:: Dyninst

.. cpp:class:: Graph
   
  **An interface for adding, deleting, and iterating nodes and edges in a graph**

  The slicing algorithms are implemented upon this graph interface, so users can
  inherit the defined classes for customization.

  .. cpp:type:: boost::shared_ptr<Graph> Ptr

    Shared pointer for Graph

  .. cpp:type:: protected boost::shared_ptr<Node> NodePtr
  .. cpp:type:: protected boost::shared_ptr<Edge> EdgePtr
  .. cpp:type:: protected std::unordered_set<NodePtr, Node::NodePtrHasher> NodeSet
  .. cpp:type:: protected std::unordered_map<Address, NodeSet> NodeMap

  .. cpp:type:: bool (*NodePredicateFunc)(const NodePtr &node, void *user_arg)

  .. cpp:function:: virtual void entryNodes(NodeIterator &begin, NodeIterator &end)

    Returns the entry nodes (nodes without any incoming edges) of the graph.

    If you want to traverse the graph start here.

  .. cpp:function:: virtual void exitNodes(NodeIterator &begin, NodeIterator &end)

    Returns the exit nodes (nodes without any outgoing edges) of the graph.

    If you want to traverse the graph backwards start here.

  .. cpp:function:: virtual void allNodes(NodeIterator &begin, NodeIterator &end)

    Iterates all nodes in the graph.

  .. cpp:function:: virtual bool find(Address addr, NodeIterator &begin, NodeIterator &end)

    Finds all nodes with the address ``addr``.

  .. cpp:function:: virtual bool find(NodePredicate::Ptr p, NodeIterator &begin, NodeIterator &end);

    Finds all nodes that satisfy the predicate ``p``.

  .. cpp:function:: virtual bool find(NodePredicateFunc f, void *user_arg, NodeIterator &begin, NodeIterator &end)

    Finds all nodes that satisfy the predicate ``f``.

  .. cpp:function:: bool printDOT(const std::string& fileName)

    Outputs the graph in dot format.

  .. cpp:function:: static Graph::Ptr createGraph()

    Returns an empty graph.

  .. cpp:function:: void insertPair(NodePtr source, NodePtr target, EdgePtr edge = EdgePtr())

    Inserts a pair of nodes into the graph and create a new edge ``edge``
    from ``source`` to ``target``.

  .. cpp:function:: virtual void insertEntryNode(NodePtr entry)

    Inserts a node as an entry node.

  .. cpp:function:: virtual void insertExitNode(NodePtr exit)

    Inserts a node as an exit node.

  .. cpp:function:: virtual void markAsEntryNode(NodePtr entry)

    Marks a node that has been added to this graph as an entry node.

  .. cpp:function:: virtual void markAsExitNode(NodePtr exit)

    Marks a node that has been added to this graph as an exit node.

  .. cpp:function:: void deleteNode(NodePtr node)

    Deletes a node.

  .. cpp:function:: void addNode(NodePtr node)

    Adds a node.

  .. cpp:function:: bool isEntryNode(NodePtr node)

    Checks if a node is an entry node.

  .. cpp:function:: bool isExitNode(NodePtr node)

    Checks if a node is an exit node.

  .. cpp:function:: void clearEntryNodes()

    Clears the marking of entry nodes.

    .. Note:: The nodes are not deleted from the graph.

  .. cpp:function:: void clearExitNodes()

    Clears the marking of exit nodes.

    .. Note:: The nodes are not deleted from the graph.

  .. cpp:function:: unsigned size() const

    Returns the number of nodes in the graph (cardinality).

.. cpp:class:: Graph::NodePredicate

  **Interface class for predicate-based searches**

   Users can inherit from this class to specify the functor to use
   as a predicate.

  .. cpp:type:: boost::shared_ptr<NodePredicate> Ptr
  .. cpp:function virtual ~NodePredicate()
  .. cpp:function virtual bool predicate(const NodePtr &node) = 0
  .. cpp:function static Ptr getPtr(NodePredicate *p)
