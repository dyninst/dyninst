\subsection{Class Graph}
\definedin{Graph.h}
\label{sec:graph}

We provide a generic graph interface, which allows users to add, delete, and
iterate nodes and edges in a graph. Our slicing algorithms are implemented
upon this graph interface, so users can inherit the defined classes for
customization.

\begin{apient}
typedef boost::shared_ptr<Graph> Ptr;
\end{apient}
\apidesc{Shared pointer for Graph}

\begin{apient}
virtual void entryNodes(NodeIterator &begin, NodeIterator &end);
\end{apient}
\apidesc{The entry nodes (nodes without any incoming edges) of the graph.}

\begin{apient}
virtual void exitNodes(NodeIterator &begin, NodeIterator &end);
\end{apient}
\apidesc{The exit nodes (nodes without any outgoing edges) of the graph.}
    
\begin{apient}
virtual void allNodes(NodeIterator &begin, NodeIterator &end);
\end{apient}
\apidesc{Iterate all nodes in the graph.}

\begin{apient}
bool printDOT(const std::string& fileName);
\end{apient}
\apidesc{Output the graph in dot format.}

\begin{apient}
static Graph::Ptr createGraph();
\end{apient}
\apidesc{Return an empty graph.}

\begin{apient}
void insertPair(NodePtr source, NodePtr target, EdgePtr edge = EdgePtr());
\end{apient}
\apidesc{Insert a pair of nodes into the graph and create a new edge \code{edge} from
\code{source} to \code{target}.}

\begin{apient}
virtual void insertEntryNode(NodePtr entry);
virtual void insertExitNode(NodePtr exit);
\end{apient}
\apidesc{Insert a node as an entry/exit node}

\begin{apient}
virtual void markAsEntryNode(NodePtr entry);
virtual void markAsExitNode(NodePtr exit);
\end{apient}
\apidesc{Mark a node that has been added to this graph as an entry/exit node.}


\begin{apient}
void deleteNode(NodePtr node);
void addNode(NodePtr node);
\end{apient}
\apidesc{Delete / Add a node.}

\begin{apient}
bool isEntryNode(NodePtr node);
bool isExitNode(NodePtr node);
\end{apient}
\apidesc{Check whether a node is an entry / exit node}

\begin{apient}
void clearEntryNodes();
void clearExitNodes();
\end{apient}
\apidesc{Clear the marking of entry / exit nodes. Note that the nodes are not
deleted from the graph.}

\begin{apient}
unsigned size() const;
\end{apient}
\apidesc{Return the number of nodes in the graph.}

\subsection{Class Node}
\definedin{Node.h}

\begin{apient}
typedef boost::shared_ptr<Node> Ptr;
\end{apient}
\apidesc{Shared pointer for Node}

\begin{apient}
void ins(EdgeIterator &begin, EdgeIterator &end);
void outs(EdgeIterator &begin, EdgeIterator &end);
\end{apient}
\apidesc{Iterate over incoming/outgoing edges of this node.}

\begin{apient}
void ins(NodeIterator &begin, NodeIterator &end);
void outs(NodeIterator &begin, NodeIterator &end);
\end{apient}
\apidesc{Iterate over adjacent nodes connected with incoming/outgoing edges of
this node.}

\begin{apient}
bool hasInEdges(); 
bool hasOutEdges();
\end{apient}
\apidesc{Return \code{true} if this node has incoming/outgoing edges.}

\begin{apient}
void deleteInEdge(EdgeIterator e);
void deleteOutEdge(EdgeIterator e);
\end{apient}
\apidesc{Delete an incoming/outgoing edge.}

\begin{apient}
virtual Address addr() const;
\end{apient}
\apidesc{Return the address of this node.}
    
\begin{apient}    
virtual std::string format() const = 0;
\end{apient}
\apidesc{Return the string representation.}

\begin{apient}
class NodeIterator;
\end{apient}
\apidesc{Iterator for nodes. Common iterator operations including \code{++},
\code{--}, and dereferencing are supported.}


\subsection{Class Edge}
\definedin{Edge.h}

\begin{apient}
typedef boost::shared_ptr<Edge> Edge::Ptr;
\end{apient}
\apidesc{Shared pointer for \code{Edge}.}

\begin{apient}
static Edge::Ptr Edge::createEdge(const Node::Ptr source, const Node::Ptr target);
\end{apient}
\apidesc{Create a new directed edge from \code{source} to \code{target}.}

\begin{apient}
Node::Ptr Edge::source() const; 
Node::Ptr Edge::target() const;
\end{apient}
\apidesc{Return the source / target node.}

\begin{apient}
void Edge::setSource(Node::Ptr source); 
void Edge::setTarget(Node::Ptr target);
\end{apient}
\apidesc{Set the source / target node.}

\begin{apient}
class EdgeIterator;
\end{apient}
\apidesc{Iterator for edges. Common iterator operations including \code{++},
\code{--}, and dereferencing are supported.}

