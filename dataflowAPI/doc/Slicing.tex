\subsection{Class Slicer}
\label{sec:slicing}

\definedin{slicing.h}

Class Slicer is the main interface for performing forward and backward slicing.
The slicing algorithm starts with a user provided Assignment and generates a
graph as the slicing results. The nodes in the generated Graph are individual
assignments that affect the starting assignment (backward slicing) or are
affected by the starting assignment (forward slicing). The edges in the graph
are directed and represent either data flow dependencies or control flow
dependencies. 

We provide call back functions
and allow users to control when to stop slicing.
In particular, class \code{Slicer::Predicates} contains a collection of call back functions that can
control the specific behaviors of the slicer. Users can inherit from the Predicates class
to provide customized stopping criteria for the slicer.

\begin{apient}
Slicer(AssignmentPtr a,
       ParseAPI::Block *block,
       ParseAPI::Function *func,
       bool cache = true,
       bool stackAnalysis = true);
\end{apient}
\apidesc{Construct a slicer, which can then be used to perform forward or backward slicing
starting at the assignment \code{a}. \code{block} and \code{func} represent the
context of assignment \code{a}. \code{cache} specifies whether the slicer will
cache the results of conversions from instructions to assignments.
\code{stackAnalysis} specifies whether the slicer will invoke stack analysis to
distinguish stack variables.}

\begin{apient}
GraphPtr forwardSlice(Predicates &predicates);
GraphPtr backwardSlice(Predicates &predicates);
\end{apient}
\apidesc{Perform forward or backward slicing and use \code{predicates} to
control the stopping criteria and return the slicing results as a graph}

A slice is represented as a Graph. The nodes and edges are defined as below:

% We also have SliceNode and SliceEdge
\begin{apient}
class SliceNode : public Node
\end{apient}
\apidesc{The default node data type in a slice graph.}

\begin{apient}
typedef boost::shared_ptr<SliceNode> Ptr;      
static SliceNode::Ptr SliceNode::create(AssignmentPtr ptr,
                                        ParseAPI::Block *block,
                                        ParseAPI::Function *func);
\end{apient}
\apidesc{Create a slice node, which represents assignment \code{ptr} in basic
block \code{block} and function \code{func}.}

Class SliceNode has the following methods to retrieve information associated the
node:

\begin{tabular}{p{1.25in}p{1.125in}p{3.125in}}
\toprule
Method name & Return type & Method description \\
\midrule
block & ParseAPI::Block* & Basic block of this SliceNode. \\
func & ParseAPI::Function* & Function of this SliceNode. \\
addr & Address & Address of this SliceNode. \\
assign & Assignment::Ptr & Assignment of this SliceNode. \\
format & std::string & String representation of this SliceNode.\\
\bottomrule
\end{tabular}


\begin{apient}
class SliceEdge : public Edge 
\end{apient}
\apidesc{The default edge data type in a slice graph.}

\begin{apient}
typedef boost::shared_ptr<SliceEdge> Ptr;
static SliceEdge::Ptr create(SliceNode::Ptr source,
                             SliceNode::Ptr target,
                             AbsRegion const&data);
\end{apient}
\apidesc{Create a slice edge from \code{source} to \code{target} and the edge
presents a dependency about abstract region \code{data}.}

\begin{apient}
const AbsRegion &data() const;
\end{apient}
\apidesc{Get the data annotated on this edge.}

% Below are interfaces for class Predicates

\subsection{Class Slicer::Predicates}
\label{sec:slicing}

\definedin{slicing.h}

Class Predicates abstracts the stopping criteria of slicing. Users can inherit
this class to control slicing in various situations, including whether or not to perform inter-procedural slicing, whether or not to search for control flow
dependencies, and whether or not to stop slicing after discovering certain assignments. We
provide a set of call back functions that allow users to dynamically control the
behavior of the Slicer.

\begin{apient}
Predicates(); 
\end{apient}
\apidesc{Construct a default predicate, which will only search for
intraprocedural data flow
dependencies.}

\begin{apient}
bool searchForControlFlowDep();
\end{apient}
\apidesc{Return \code{true} if this predicate will search for control flow
dependencies. Otherwise, return \code{false}.}

\begin{apient}
void setSearchForControlFlowDep(bool cfd);
\end{apient}
\apidesc{Change whether or not to search for control flow dependencies according
to \code{cfd}.}

\begin{apient}
virtual bool widenAtPoint(AssignmentPtr) { return false; }
\end{apient}
\apidesc{The default behavior is to return \code{false}.}

\begin{apient}
virtual bool endAtPoint(AssignmentPtr);
\end{apient}
\apidesc{In backward slicing, after we find a match for an assignment, we pass
it to this function. This function should return \code{true} if the user does
not want to continue searching for this assignment. Otherwise, it should return
\code{false}. The default behavior of this function is to always return \code{false}.}


\begin{apient}
typedef std::pair<ParseAPI::Function *, int> StackDepth_t;
typedef std::stack<StackDepth_t> CallStack_t;
virtual bool followCall(ParseAPI::Function * callee,
                        CallStack_t & cs,
                        AbsRegion argument);

\end{apient}
\apidesc{This predicate function is called when the slicer reaches a direct call site.
If it returns \code{true}, the slicer will follow into the callee function \code{callee}.
This function also takes input \code{cs}, which represents the call stack of the
followed callee functions from the starting point of the slicing to this call
site, and \code{argument}, which represents the variable to slice with in the
callee function. This function defaults to always returning \code{false}.
Note that as Dyninst currently does not try to resolve indirect
calls, the slicer will NOT call this function at an indirect call site. }
%virtual bool followCallee(ParseAPI::Function * callee,

%\begin{apient}
%virtual bool followCall(ParseAPI::Function * /*callee*/,
%                        CallStack_t & /*cs*/,
%			AbsRegion /*argument*/); 
%\end{apient}
%\apidesc{The same as \code{followCallee}. This function is going to be
%depreciated and exists for backward compatibility.}

\begin{apient}
virtual std::vector<ParseAPI::Function *> 
          followCallBackward(ParseAPI::Block * caller,
                             CallStack_t & cs,
                             AbsRegion argument);       
\end{apient}
\apidesc{This predicate function is called when the slicer reaches the entry of
a function in the case of backward slicing or reaches a return instruction in
the case of forward slicing. It returns a vector of caller functions that the
user wants the slicer to continue to follow. This function takes input
\code{caller}, which represents the call block of the caller, \code{cs}, which
represents the caller functions that have been followed to this place, and
\code{argument}, which represents the variable to slice with in the caller
function. This function defaults to always
returning an empty vector.}

%        followCaller(ParseAPI::Block * caller,

%\begin{apient}
%virtual std::vector<ParseAPI::Function *> 
%        followCallBackward(ParseAPI::Block * /*callerB*/,
%                           CallStack_t & /*cs*/,
%                           AbsRegion /*argument*/) 
%\end{apient}	
%\apidesc{The same as \code{followCaller}. This function is going to be
%depreciated and exists for backward compatibility. }

\begin{apient}
virtual bool addPredecessor(AbsRegion reg);
\end{apient}
\apidesc{In backward slicing, after we match an assignment at a location, the
matched AbsRegion \code{reg} is
passed to this predicate function. This function should return \code{true} if the user
wants to continue to search for dependencies for this AbsRegion. Otherwise, this
function should return \code{true}. The default behavior of this function is to
always return \code{true}.}

\begin{apient}
virtual bool addNodeCallback(AssignmentPtr assign,
                             std::set<ParseAPI::Edge*> &visited);
\end{apient}
\apidesc{In backward slicing, this function is called when the slicer adds a new node to the slice. 
The newly added assignment \code{assign} and the set of control flow edges
\code{visited} that have been visited so far are
passed to this function. This function should return \code{true} if the user
wants to continue slicing. If this function returns \code{false}, the Slicer
will not continue to search along the path. The default behavior of this
function is to always return \code{true}.}


