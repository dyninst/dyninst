\section{Public API Reference}
\label{sec-public-api}

This section describes public interfaces in PatchAPI. The API is organized as a
collection of C++ classes. The classes in PatchAPI fall under the C++ namespace
Dyninst::PatchAPI. To access them, programmers should refer to them using the
``Dyninst::PatchAPI::'' prefix, e.g., Dyninst::PatchAPI::Point. Alternatively,
programmers can add the C++ \emph{using} keyword above any references to
PatchAPI objects, e.g.,\emph{using namespace Dyninst::PatchAPI} or \emph{using
  Dyninst::PatchAPI::Point}.

Classes in PatchAPI use either the C++ raw pointer or the boost shared pointer
(\emph{boost::shared\_ptr<T>}) for memory management. A class uses a raw pointer
whenever it is returning a handle to the user that is controlled and destroyed
by the PatchAPI runtime library. Classes that use a raw pointer include the CFG
objects, a Point, and various plugins, e.g., AddrSpace, CFGMaker, PointMaker,
and Instrumenter.  A class uses a shared\_pointer whenever it is handing
something to the user that the PatchAPI runtime library is not controlling and
destroying. Classes that use a boost shared pointer include a Snippet, PatchMgr,
and Instance, where we typedef a class's shared pointer by appending the Ptr to
the class name, e.g., PatchMgrPtr for PatchMgr.

\subsection{CFG Interface}

\subsubsection{PatchObject}
\label{sec-3.2.8}

\textbf{Declared in}: PatchObject.h

The PatchObject class is a wrapper of ParseAPI's CodeObject class (has-a), which
represents an individual binary code object, such as an executable or a library.


\begin{apient}
static PatchObject* create(ParseAPI::CodeObject* co, Address base,
                           CFGMaker* cm = NULL, PatchCallback *cb = NULL);
\end{apient}


\apidesc{

Creates an instance of PatchObject, which has \emph{co} as its on-disk
representation (ParseAPI::CodeObject), and \emph{base} as the base address where
this object is loaded in the memory. For binary rewriting, base should be 0. The
\emph{cm} and \emph{cb} parameters are for registering plugins. If \emph{cm} or
\emph{cb} is NULL, then we use the default implementation of CFGMaker or
PatchCallback.

}

\begin{apient}
static PatchObject* clone(PatchObject* par_obj, Address base,
                          CFGMaker* cm = NULL, PatchCallback *cb = NULL);
\end{apient}


\apidesc{

Returns a new object that is copied from the specified object \emph{par\_obj} at
the loaded address \emph{base} in the memory.  For binary rewriting, base should
be 0. The \emph{cm} and \emph{cb} parameters are for registering plugins. If
\emph{cm} or \emph{cb} is NULL, then we use the default implementation of
CFGMaker or PatchCallback.

}

\begin{apient}
Address codeBase();
\end{apient}

\apidesc{

Returns the base address where this object is loaded in memory.

}

\begin{apient}
PatchFunction *getFunc(ParseAPI::Function *func, bool create = true);
\end{apient}


\apidesc{

Returns an instance of PatchFunction in this object, based on the \emph{func}
parameter. PatchAPI creates a PatchFunction on-demand, so if there is not any
PatchFunction created for the ParseAPI function \emph{func}, and the
\emph{create} parameter is false, then no any instance of PatchFunction will be
created.

It returns NULL in two cases. First, the function \emph{func} is not in this
PatchObject. Second, the PatchFunction is not yet created and the \emph{create}
is false. Otherwise, it returns a PatchFunction.

}

\begin{apient}
template <class Iter>
void funcs(Iter iter);
\end{apient}

\apidesc{

Outputs all instances of PatchFunction in this PatchObject to the STL inserter
\emph{iter}.

}

\begin{apient}
PatchBlock *getBlock(ParseAPI::Block* blk, bool create = true);
\end{apient}


\apidesc{

Returns an instance of PatchBlock in this object, based on the \emph{blk}
parameter. PatchAPI creates a PatchBlock on-demand, so if there is not any
PatchBlock created for the ParseAPI block \emph{blk}, and the \emph{create}
parameter is false, then no any instance of PatchBlock will be created.

It returns NULL in two cases. First, the ParseAPI block \emph{blk} is not in
this PatchObject. Second, the PatchBlock is not yet created and the
\emph{create} is false. Otherwise, it returns a PatchBlock.

}

\begin{apient}
template <class Iter>
void blocks(Iter iter);
\end{apient}

\apidesc{

Outputs all instances of PatchBlock in this object to the STL inserter
\emph{iter}.

}

\begin{apient}
PatchEdge *getEdge(ParseAPI::Edge* edge, PatchBlock* src, PatchBlock* trg,
                   bool create = true);
\end{apient}


\apidesc{

Returns an instance of PatchEdge in this object, according to the parameters
ParseAPI::Edge \emph{edge}, source PatchBlock \emph{src}, and target PatchBlock
\emph{trg}. PatchAPI creates a PatchEdge on-demand, so if there is not any
PatchEdge created for the ParseAPI \emph{edge}, and the \emph{create} parameter
is false, then no any instance of PatchEdge will be created.

It returns NULL in two cases. First, the ParseAPI \emph{edge} is not in this
PatchObject. Second, the PatchEdge is not yet created and the \emph{create} is
false. Otherwise, it returns a PatchEdge.

}

\begin{apient}
template <class Iter>
void edges(Iter iter);
\end{apient}


\apidesc{

Outputs all instances of PatchEdge in this object to the STL inserter
\emph{iter}.

}

\begin{apient}
PatchCallback *cb() const;
\end{apient}


\apidesc{
Returns the PatchCallback object associated with this PatchObject.
}

\subsubsection{PatchFunction}
\label{sec-3.2.9}

\textbf{Declared in}: PatchCFG.h

The PatchFunction class is a wrapper of ParseAPI's Function class (has-a), which
represents a function.

\begin{apient}
const string &name();
\end{apient}


\apidesc{
Returns the function's mangled name.
}

\begin{apient}
Address addr() const;
\end{apient}


\apidesc{
Returns the address of the first instruction in this function.
}

\begin{apient}
ParseAPI::Function *function();
\end{apient}



\apidesc{
Returns the ParseAPI::Function associated with this PatchFunction.
}

\begin{apient}
PatchObject* obj();
\end{apient}

\apidesc{
Returns the PatchObject associated with this PatchFunction.
}

\begin{apient}
typedef std::set<PatchBlock *> PatchFunction::Blockset;

const Blockset &blocks();
\end{apient}


\apidesc{
Returns a set of all PatchBlocks in this PatchFunction.
}

\begin{apient}
PatchBlock *entry();
\end{apient}


\apidesc{
Returns the entry block of this PatchFunction.
}

\begin{apient}
const Blockset &exitBlocks();
\end{apient}


\apidesc{
Returns a set of exit blocks of this PatchFunction.
}

\begin{apient}
const Blockset &callBlocks();
\end{apient}


\apidesc{
Returns a set of all call blocks of this PatchFunction.
}

\begin{apient}
PatchCallback *cb() const;
\end{apient}

\apidesc{
Returns the PatchCallback object associated with this PatchFunction.
}

\begin{apient}
PatchLoopTreeNode* getLoopTree()
\end{apient}
\apidesc{Return the nesting tree of the loops in the function. See class \code{PatchLoopTreeNode} for more details}

\begin{apient}
PatchLoop* findLoop(const char *name)
\end{apient}
\apidesc{Return the loop with the given nesting name. See class \code{PatchLoopTreeNode} for more details about how loop nesting names are assigned.}

\begin{apient}
bool getLoops(vector<PatchLoop*> &loops);
\end{apient}
\apidesc{Fill \code{loops} with all the loops in the function}

\begin{apient}
bool getOuterLoops(vector<PatchLoop*> &loops);
\end{apient}
\apidesc{Fill \code{loops} with all the outermost loops in the function}

\begin{apient}
bool dominates(PatchBlock* A, PatchBlock *B);
\end{apient}
\apidesc{Return true if block \code{A} dominates block \code{B}}

\begin{apient}
PatchBlock* getImmediateDominator(PatchBlock *A);
\end{apient}
\apidesc{Return the immediate dominator of block \code{A}, \code{NULL} if the block \code{A} does not have an immediate dominator.}

\begin{apient}
void getImmediateDominates(PatchBlock *A, set<PatchBlock*> &imm);
\end{apient}
\apidesc{Fill \code{imm} with all the blocks immediate dominated by block \code{A}}

\begin{apient}
void getAllDominates(PatchBlock *A, set<PatchBlock*> &dom);
\end{apient}
\apidesc{Fill \code{dom} with all the blocks dominated by block \code{A}}


\begin{apient}
bool postDominates(PatchBlock* A, PatchBlock *B);
\end{apient}
\apidesc{Return true if block \code{A} post-dominates block \code{B}}

\begin{apient}
PatchBlock* getImmediatePostDominator(PatchBlock *A);
\end{apient}
\apidesc{Return the immediate post-dominator of block \code{A}, \code{NULL} if the block \code{A} does not have an immediate post-dominator.}

\begin{apient}
void getImmediatePostDominates(PatchBlock *A, set<PatchBlock*> &imm);
\end{apient}
\apidesc{Fill \code{imm} with all the blocks immediate post-dominated by block \code{A}}

\begin{apient}
void getAllPostDominates(PatchBlock *A, set<PatchBlock*> &dom);
\end{apient}
\apidesc{Fill \code{dom} with all the blocks post-dominated by block \code{A}}

\subsubsection{PatchBlock}
\label{sec-3.2.10}

\textbf{Declared in}: PatchCFG.h

The PatchBlock class is a wrapper of ParseAPI's Block class (has-a), which
represents a basic block.


\begin{apient}
Address start() const;
\end{apient}


\apidesc{
Returns the lower bound of this block (the address of the first instruction).
}

\begin{apient}
Address end() const;
\end{apient}


\apidesc{

Returns the upper bound (open) of this block (the address immediately following
the last byte in the last instruction).

}

\begin{apient}
Address last() const;
\end{apient}


\apidesc{
Returns the address of the last instruction in this block.
}

\begin{apient}
Address size() const;
\end{apient}


\apidesc{
Returns end() - start().
}

\begin{apient}
bool isShared();
\end{apient}


\apidesc{
Indicates whether this block is contained by multiple functions.
}

\begin{apient}
int containingFuncs() const;
\end{apient}

\apidesc{
Returns the number of functions that contain this block.
}

\begin{apient}
typedef std::map<Address, InstructionAPI::Instruction::Ptr> Insns;
void getInsns(Insns &insns) const;
\end{apient}


\apidesc{
This function outputs Instructions that are in this block to \emph{insns}.
}

\begin{apient}
InstructionAPI::Instruction::Ptr getInsn(Address a) const;
\end{apient}


\apidesc{
Returns an Instruction that has the address \emph{a} as its starting address. If no
any instruction can be found in this block with the starting address \emph{a}, it
returns InstructionAPI::Instruction::Ptr().
}

\begin{apient}
std::string disassemble() const;
\end{apient}


\apidesc{
Returns a string containing the disassembled code for this block. This is mainly
for debugging purpose.
}

\begin{apient}
bool containsCall();
\end{apient}


\apidesc{
Indicates whether this PatchBlock contains a function call instruction.
}

\begin{apient}
bool containsDynamicCall();
\end{apient}

\apidesc{
Indicates whether this PatchBlock contains any indirect function call, e.g., via
function pointer.
}

\begin{apient}
PatchFunction* getCallee();
\end{apient}

\apidesc{
Returns the callee function, if this PatchBlock contains a function call;
otherwise, NULL is returned.
}

\begin{apient}
PatchFunction *function() const;
\end{apient}

\apidesc{
Returns a PatchFunction that contains this PatchBlock. If there are multiple
PatchFunctions containing this PatchBlock, then a random one of them is
returned.
}

\begin{apient}
ParseAPI::Block *block() const;
\end{apient}


\apidesc{
Returns the ParseAPI::Block associated with this PatchBlock.
}

\begin{apient}
PatchObject* obj() const;
\end{apient}

\apidesc{
Returns the PatchObject that contains this block.
}

\begin{apient}
typedef std::vector<PatchEdge*> PatchBlock::edgelist;

const edgelist &sources();
\end{apient}

\apidesc{
Returns a list of the source PatchEdges. This PatchBlock is the target block of
the returned edges.
}

\begin{apient}
const edgelist &targets();
\end{apient}

\apidesc{
Returns a list of the target PatchEdges. This PatchBlock is the source block of
the returned edges.
}

\begin{apient}
template <class OutputIterator>
void getFuncs(OutputIterator result);
\end{apient}

\apidesc{
Outputs all functions containing this PatchBlock to the STL inserter \emph{result}.
}

\begin{apient}
PatchCallback *cb() const;
\end{apient}

\apidesc{
Returns the PatchCallback object associated with this PatchBlock.
}

\subsubsection{PatchEdge}
\label{sec-3.2.11}

\textbf{Declared in}: PatchCFG.h

The PatchEdge class is a wrapper of ParseAPI's Edge class (has-a), which joins
two PatchBlocks in the CFG, indicating the type of control flow transfer
instruction that joins the basic blocks to each other.


\begin{apient}
ParseAPI::Edge *edge() const;
\end{apient}

\apidesc{
Returns a ParseAPI::Edge associated with this PatchEdge.
}

\begin{apient}
PatchBlock *src();
\end{apient}

\apidesc{
Returns the source PatchBlock.
}

\begin{apient}
PatchBlock *trg();
\end{apient}

\apidesc{
Returns the target PatchBlock.
}

\begin{apient}
ParseAPI::EdgeTypeEnum type() const;
\end{apient}

\apidesc{
Returns the edge type (ParseAPI::EdgeTypeEnum, please see \href{ftp://ftp.cs.wisc.edu/paradyn/releases/release7.0/doc/parseapi.pdf}{ParseAPI Manual}).
}

\begin{apient}
bool sinkEdge() const;
\end{apient}

\apidesc{
Indicates whether this edge targets the special sink block, where a sink block
is a block to which all unresolvable control flow instructions will be
linked.
}

\begin{apient}
bool interproc() const;
\end{apient}

\apidesc{
Indicates whether the edge should be interpreted as interprocedural (e.g.,
calls, returns, direct branches under certain circumstances).
}

\begin{apient}
PatchCallback *cb() const;
\end{apient}

\apidesc{
Returns a Patchcallback object associated with this PatchEdge.
}

\subsubsection{PatchLoop}
\label{sec-3.2.12}

\textbf{Declared in}: PatchCFG.h

The PatchLoop class is a wrapper of ParseAPI's Loop class (has-a).
It represents code structure that may execute repeatedly.

\begin{apient}
PatchLoop* parent
\end{apient}
\apidesc{Returns the loop which directly encloses this loop. NULL if no such loop.} 


\begin{apient}
bool containsAddress(Address addr)
\end{apient}
\apidesc{Returns true if the given address is within the range of this loop's basic blocks.}
	

\begin{apient}
bool containsAddressInclusive(Address addr)
\end{apient}
\apidesc{Returns true if the given address is within the range of this loop's basic blocks or its children.}

\begin{apient}
int getLoopEntries(vector<PatchBlock*>& entries);
\end{apient}
\apidesc{Fills \code{entries} with the set of entry basic blocks of the loop. Return the number of the entries that this loop has}
        
		   
\begin{apient}
int getBackEdges(vector<PatchEdge*> &edges)
\end{apient}
\apidesc{Sets \code{edges} to the set of back edges in this loop. 
It returns the number of back edges that are in this loop. 
}
	
\begin{apient}
bool getContainedLoops(vector<PatchLoop*> &loops)
\end{apient}
\apidesc{Returns a vector of loops that are nested under this loop.}

\begin{apient}
bool getOuterLoops(vector<PatchLoop*> &loops)
\end{apient}
\apidesc{Returns a vector of loops that are directly nested under this loop.}

	
\begin{apient}
bool getLoopBasicBlocks(vector<PatchBlock*> &blocks)
\end{apient}
\apidesc{Fills \code{blocks} with all basic blocks in the loop}

        
\begin{apient}
bool getLoopBasicBlocksExclusive(vector<PatchBlock*> &blocks)
\end{apient}
\apidesc{Fills \code{blocks} with all basic blocks in this loop, excluding the blocks of its sub loops.}
     
\begin{apient}
bool hasBlock(PatchBlock *b);
\end{apient}
\apidesc{Returns \code{true} if this loop contains basic block \code{b}.}

\begin{apient}
bool hasBlockExclusive(PatchBlock *b);
\end{apient}
\apidesc{Returns \code{true} if this loop contains basic block \code{b} and \code{b} is not in its sub loops.}

\begin{apient}
bool hasAncestor(PatchLoop *loop)
\end{apient}
\apidesc{Returns true if this loop is a descendant of the given loop.}

\begin{apient}
PatchFunction * getFunction();
\end{apient}
\apidesc{Returns the function that this loop is in.}

\subsubsection{PatchLoopTreeNode}
\label{sec-3.2.13}

\textbf{Declared in}: PatchCFG.h

The PatchLoopTreeNode class provides a tree interface to a collection of instances of class 
PatchLoop contained in a function. The structure of the tree 
follows the nesting relationship of the loops in a function. 
Each PatchLoopTreeNode contains a pointer to a loop (represented by PatchLoop), and a set
of sub-loops (represented by other PatchLoopTreeNode objects). The \code{loop} field
at the root node is always \code{NULL} since a function may contain multiple outer 
loops. The \code{loop} field is never \code{NULL} at any other node since it
always corresponds to a real loop.
Therefore, the outer most loops in the function are contained in the vector of \code{children} of the root. 


Each instance of PatchLoopTreeNode is given a name that indicates its position in the hierarchy of loops.
The name of each outermost loop takes the form of \code{loop\_x}, 
where \code{x} is an integer from 1 to n,
where n is the number of outer loops in the function.
Each sub-loop has the name of its parent,
followed by a \code{.y}, where \code{y} is 1 to m, where m is the number of sub-loops under the outer loop.  
For example, consider the following C function:

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{showstringspaces=false, numbers=none}

\begin{lstlisting}

void foo() {
  int x, y, z, i;
  for (x=0; x<10; x++) {
    for (y = 0; y<10; y++)
      ...
    for (z = 0; z<10; z++)
      ...
  }
  for (i = 0; i<10; i++) {
     ...
  }
}
\end{lstlisting}

The \code{foo} function will have a root PatchLoopTreeNode, containing a NULL loop entry and 
two PatchLoopTreeNode children representing the functions outermost loops. These children 
would have names \code{loop\_1} and \code{loop\_2}, respectively representing the \code{x} and \code{i} loops. \code{loop\_2} has 
no children. \code{loop\_1} has two child PatchLoopTreeNode objects, named
\code{loop\_1.1} and 
\code{loop\_1.2}, respectively representing the \code{y} and \code{z} loops. 

\begin{apient}
PatchLoop *loop;
\end{apient}
\apidesc{The PatchLoop instance it points to.}

\begin{apient}
std::vector<PatchLoopTreeNode *> children;
\end{apient}
\apidesc{The PatchLoopTreeNode instances nested within this loop.}
    
\begin{apient}
const char * name(); 
\end{apient}
\apidesc{Returns the hierarchical name of this loop.}
    
\begin{apient}
const char * getCalleeName(unsigned int i)
\end{apient}
\apidesc{Returns the function name of the ith callee.}

\begin{apient}
unsigned int numCallees()
\end{apient}    
\apidesc{Returns the number of callees contained in this loop's body.}

\begin{apient}
bool getCallees(vector<PatchFunction *> &v);
\end{apient}
\apidesc{Fills \code{v} with a vector of the functions called inside this loop.}
    
    
\begin{apient}
PatchLoop * findLoop(const char *name);
\end{apient}
\apidesc{Looks up a loop by the hierarchical name}
 

\subsection{Point/Snippet Interface}
\label{sec-3.1}

\subsubsection{PatchMgr}
\label{sec-3.1.1}

\textbf{Declared in}: PatchMgr.h

The PatchMgr class is the top-level class for finding instrumentation \textbf{Points},
inserting or deleting \textbf{Snippets}, and registering user-provided plugins.


\begin{apient}
static PatchMgrPtr create(AddrSpace* as, Instrumenter* inst = NULL,
                          PointMaker* pm = NULL);
\end{apient}


\apidesc{

This factory method creates a new PatchMgr object that performs binary code
patching. It takes input three plugins, including AddrSpace \emph{as},
Instrumenter \emph{inst}, and PointMaker \emph{pm}. PatchAPI uses default
plugins for PointMaker and Instrumenter, if \emph{pm} and \emph{inst} are not
specified (NULL by default).

This method returns PatchMgrPtr() if it was unable to create a new PatchMgr
object.

}

\begin{apient}
Point *findPoint(Location loc, Point::Type type, bool create = true);
\end{apient}

\apidesc{

This method returns a unique Point according to a Location \emph{loc} and a Type
\emph{type}. The Location structure is to specify a physical location of a Point
(e.g., at function entry, at block entry, etc.), details of Location will be
covered in Section~\ref{sec-3.1.2}. PatchAPI creates Points on demand, so if
a Point is not yet created, the \emph{create} parameter is to indicate whether
to create this Point. If the Point we want to find is already created, this
method simply returns a pointer to this Point from a buffer, no matter whether
\emph{create} is true or false. If the Point we want to find is not yet created,
and \emph{create} is true, then this method constructs this Point and put it in
a buffer, and finally returns a Pointer to this Point. If the Point creation
fails, this method also returns false. If the Point we want to find is not yet
created, and \emph{create} is false, this method returns NULL. The basic logic
of finding a point can be found in the Listing~\ref{findpt}.

}

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=none}
\begin{lstlisting}[label=findpt, caption=Pseudocode of finding a point]
if (point is in the buffer) {
  return point;
} else {
  if (create == true) {
    create point
    if (point creation fails) return NULL;
    put the point in the buffer
  } else {
    return NULL;
  }
}
\end{lstlisting}

\begin{apient}
template <class OutputIterator>
bool findPoint(Location loc, Point::Type type, OutputIterator outputIter,
               bool create = true);
\end{apient}

\apidesc{

This method finds a Point at a physical Location \emph{loc} with a \emph{type}.
It adds the found Point to \emph{outputIter} that is a STL inserter. The point
is created on demand. If the Point is already created, then this method outputs
a pointer to this Point from a buffer. Otherwise, the \emph{create} parameter
indicates whether to create this Point.

This method returns true if a point is found, or the \emph{create} parameter is
false; otherwise, it returns false.

}

\begin{apient}
template <class OutputIterator>
bool findPoints(Location loc, Point::Type types, OutputIterator outputIter,
                bool create = true);
\end{apient}

\apidesc{

This method finds Points at a physical Location \emph{loc} with composite
\emph{types} that are combined using the overloaded operator ``|''. This
function outputs Points to the STL inserter \emph{outputIter}. The point is
created on demand. If the Point is already created, then this method outputs a
pointer to this Point from a buffer. Otherwise, the \emph{create} parameter
indicates whether to create this Point.

This method returns true if a point is found, or the \emph{create} parameter is
false; otherwise, it returns false.
}

\begin{apient}
template <class FilterFunc,
          class FilterArgument,
          class OutputIterator>
bool findPoints(Location loc, Point::Type types, FilterFunc filter_func,
                FilterArgument filter_arg, OutputIterator outputIter, 
                bool create = true);
\end{apient}

\apidesc{

This method finds Points at a physical Location \emph{loc} with composite
\emph{types} that are combined using the overloaded operator ``|''. Then, this
method applies a filter functor \emph{filter\_func} with an argument
\emph{filter\_arg} on each found Point.  The method outputs Points to the
inserter \emph{outputIter}. The point is created on demand. If the Point is
already created, then this method returns a pointer to this Point from a
buffer. Otherwise, the \emph{create} parameter indicates whether to create this
Point.

If no any Point is created, then this method returns false; otherwise, true is
returned. The code below shows the prototype of an example functor.
}

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=none}
\begin{lstlisting}[caption=Code template for the filter function in findPoint]
template <class T>
class FilterFunc {
  public:
    bool operator()(Point::Type type, Location loc, T arg) {
      // The logic to check whether this point is what we need
      return true;
    }
};
\end{lstlisting}

\apidesc{
In the functor FilterFunc above, programmers check each candidate Point by
looking at the Point::Type, Location, and the user-specified parameter \emph{arg}.
If the return value is true, then the Point being checked will be put in the STL
inserter \emph{outputIter}; otherwise, this Point will be discarded.
}

\begin{apient}
struct Scope {
  Scope(PatchBlock *b);
  Scope(PatchFunction *f, PatchBlock *b);
  Scope(PatchFunction *f);
};
\end{apient}

\apidesc{ The Scope structure specifies the scope to find points, where a scope
  could be a function, or a basic block. This is quite useful if programmers
  don't know the exact Location, then they can use Scope as a wildcard. A basic
  block can be contained in multiple functions. The second constructor only
  specifies the block \emph{b} in a particular function \emph{f}.}

\begin{apient}
template <class FilterFunc,
          class FilterArgument,
          class OutputIterator>
bool findPoints(Scope scope, Point::Type types, FilterFunc filter_func,
                FilterArgument filter_arg, OutputIterator output_iter, 
                bool create = true);
\end{apient}

\apidesc{

This method finds points in a \emph{scope} with certain \emph{types} that are
combined together by using the overloaded operator ``|''. Then, this method
applies the filter functor \emph{filter\_func} on each found Point. It outputs
Points where \emph{filter\_func} returns true to the STL inserter
\emph{output\_iter}. Points are created on demand. If some points are already
created, then this method outputs pointers to them from a buffer. Otherwise, the
\emph{create} parameter indicates whether to create Points.

If no any Point is created, then this function returns false; otherwise, true is
returned.

}


\begin{apient}
template <class OutputIterator>
bool findPoints(Scope scope, Point::Type types, OutputIterator output_iter, bool create = true);
\end{apient}

\apidesc{

This method finds points in a \emph{scope} with certain \emph{types} that are
combined together by using the overloaded operator ``|''. It outputs the found
points to the STL inserter \emph{output\_iter}. If some points are already
created, then this method outputs pointers to them from a buffer. Otherwise, the
\emph{create} parameter indicates whether to create Points.

If no any Point is created, then this method returns false; otherwise, true is
returned.
}

\begin{apient}
bool removeSnippet(InstancePtr);
\end{apient}

\apidesc{
This method removes a snippet Instance.

It returns false if the point associated with this Instance cannot be found;
otherwise, true is returned.
}

\begin{apient}
template <class FilterFunc,
          class FilterArgument>
bool removeSnippets(Scope scope, Point::Type types, FilterFunc filter_func,
                    FilterArgument filter_arg);
\end{apient}

\apidesc{

This method deletes ALL snippet instances at certain points in certain
\emph{scope} with certain \emph{types}, and those points pass the test of
\emph{filter\_func}.

If no any point can be found, this method returns false; otherwise, true is
returned.
}

\begin{apient}
bool removeSnippets(Scope scope, Point::Type types);
\end{apient}

\apidesc{
This method deletes ALL snippet instances at certain points in certain \emph{scope}
with certain \emph{types}.

If no any point can be found, this method returns false; otherwise, true is
returned.

}

\begin{apient}
void destroy(Point *point);
\end{apient}

\apidesc{
This method is to destroy the specified \emph{Point}.
}

\begin{apient}
AddrSpace* as() const;
PointMaker* pointMaker() const;
Instrumenter* instrumenter() const;
\end{apient}

\apidesc{
The above three functions return the corresponding plugin: AddrSpace,
PointMaker, Instrumenter.
}

\subsubsection{Point}
\label{sec-3.1.2}

\textbf{Declared in}: Point.h

The Point class is in essence a container of a list of snippet
instances. Therefore, the Point class has methods similar to those in STL.

\begin{apient}
struct Location {
   static Location Function(PatchFunction *f);
   static Location Block(PatchBlock *b);
   static Location BlockInstance(PatchFunction *f, PatchBlock *b, bool trusted = false);
   static Location Edge(PatchEdge *e);
   static Location EdgeInstance(PatchFunction *f, PatchEdge *e);
   static Location Instruction(PatchBlock *b, Address a);
   static Location InstructionInstance(PatchFunction *f, PatchBlock *b, Address a);
   static Location InstructionInstance(PatchFunction *f, PatchBlock *b, Address a,
                                       InstructionAPI::Instruction::Ptr i,
                                       bool trusted = false);
   static Location EntrySite(PatchFunction *f, PatchBlock *b, bool trusted = false);
   static Location CallSite(PatchFunction *f, PatchBlock *b);
   static Location ExitSite(PatchFunction *f, PatchBlock *b);
};
\end{apient}

\apidesc{

The Location structure uniquely identifies the physical location of a point. A
Location object plus a Point::Type value uniquely identifies a point, because
multiple Points with different types can exist at the same physical location.
The Location structure provides a set of static functions to create an object of
Location, where each function takes the corresponding CFG structures to identify
a physical location. In addition, some functions above (e.g.,
InstructionInstance) takes input the \emph{trusted} parameter that is to
indicate PatchAPI whether the CFG structures passed in is trusted. If the
\emph{trusted} parameter is false, then PatchAPI would have additional checking
to verify the CFG structures passed by users, which causes nontrivial overhead.

}


\begin{apient}
enum Point::Type {
  PreInsn,
  PostInsn,
  BlockEntry,
  BlockExit,
  BlockDuring,
  FuncEntry,
  FuncExit,
  FuncDuring,
  EdgeDuring,
  PreCall,
  PostCall,
  OtherPoint,
  None,
  InsnTypes = PreInsn | PostInsn,
  BlockTypes = BlockEntry | BlockExit | BlockDuring,
  FuncTypes = FuncEntry | FuncExit | FuncDuring,
  EdgeTypes = EdgeDuring,
  CallTypes = PreCall | PostCall
};
\end{apient}

\apidesc{ The enum Point::Type specifies the logical point type. Multiple enum
  values can be OR-ed to form a composite type. For example, the composite type
  of ``PreCall | BlockEntry | FuncExit'' is to specify a set of points with the
  type PreCall, or BlockEntry, or FuncExit.}

\begin{apient}
typedef std::list<InstancePtr>::iterator instance_iter;
instance_iter begin();
instance_iter end();
\end{apient}

\apidesc{

The method begin() returns an iterator pointing to the beginning of the
container storing snippet Instances, while the method end() returns an iterator
pointing to the end of the container (past the last element).
}

\begin{apient}
InstancePtr pushBack(SnippetPtr);
InstancePtr pushFront(SnippetPtr);
\end{apient}


\apidesc{

Multiple instances can be inserted at the same Point. We maintain the instances
in an ordered list. The pushBack method is to push the specified Snippet to the
end of the list, while the pushFront method is to push to the front of the
list.

Both methods return the Instance that uniquely identifies the inserted snippet.
}

\begin{apient}
bool remove(InstancePtr instance);
\end{apient}


\apidesc{
This method removes the given snippet \emph{instance} from this Point.
}

\begin{apient}
void clear();
\end{apient}


\apidesc{
This method removes all snippet instances inserted to this Point.
}

\begin{apient}
size_t size();
\end{apient}


\apidesc{
Returns the number of snippet instances inserted at this Point.
}

\begin{apient}
Address addr() const;
\end{apient}


\apidesc{
Returns the address associated with this point, if it has one; otherwise, it
returns 0.
}

\begin{apient}
Type type() const;
\end{apient}


\apidesc{
Returns the Point type of this point.
}

\begin{apient}
bool empty() const;
\end{apient}


\apidesc{
Indicates whether the container of instances at this Point is empty or not.
}

\begin{apient}
PatchFunction* getCallee();
\end{apient}


\apidesc{
Returns the function that is invoked at this Point, which should have
Point::Type of Point::PreCall or Point::PostCall. It there is not a function
invoked at this point, it returns NULL.
}

\begin{apient}
const PatchObject* obj() const;
\end{apient}


\apidesc{
Returns the PatchObject where the Point resides.
}

\begin{apient}
const InstructionAPI::Instruction::Ptr insn() const;
\end{apient}


\apidesc{
Returns the Instruction where the Point resides.
}

\begin{apient}
PatchFunction* func() const;
\end{apient}


\apidesc{
Returns the function where the Point resides.
}

\begin{apient}
PatchBlock* block() const;
\end{apient}


\apidesc{
Returns the PatchBlock where the Point resides.
}

\begin{apient}
PatchEdge* edge() const;
\end{apient}


\apidesc{
Returns the Edge where the Point resides.
}

\begin{apient}
PatchCallback *cb() const;
\end{apient}


\apidesc{
Returns the PatchCallback object that is associated with this Point.
}

\begin{apient}
static bool TestType(Point::Type types, Point::Type type);
\end{apient}


\apidesc{
This static method tests whether a set of \emph{types} contains a specific \emph{type}.
}

\begin{apient}
static void AddType(Point::Type& types, Point::Type type);
\end{apient}


\apidesc{
This static method adds a specific \emph{type} to a set of \emph{types}.
}

\begin{apient}
static void RemoveType(Point::Type& types, Point::Type trg);
\end{apient}


\apidesc{
This static method removes a specific \emph{type} from a set of \emph{types}.
}

\subsubsection{Instance}
\label{sec-3.1.3}

\textbf{Declared in}: Point.h

The Instance class is a representation of a particular snippet inserted at a
particular point. If a Snippet is inserted to N points or to the same point for
N times (N $>$ 1), then there will be N Instances.


\begin{apient}
bool destroy();
\end{apient}


\apidesc{
This method destroys the snippet Instance itself.
}

\begin{apient}
Point* point() const;
\end{apient}


\apidesc{
Returns the Point where the Instance is inserted.
}

\begin{apient}
SnippetPtr snippet() const;
\end{apient}


\apidesc{
Returns the Snippet. Please note that, the same Snippet may have multiple
instances inserted at different Points or the same Point.
}

\subsection{Callback Interface}
\label{sec-3.1}


\subsubsection{PatchCallback}
\label{sec-3.2.7}

\textbf{Declared in}: PatchCallback.h

The PatchAPI CFG layer may change at runtime due to program events
(e.g., a program loading additional code or overwriting its own code
with new code). The \texttt{PatchCallback} interface allows users to
specify callbacks they wish to occur whenever the PatchAPI CFG
changes.

\begin{apient}
virtual void destroy_cb(PatchBlock *);
virtual void destroy_cb(PatchEdge *);
virtual void destroy_cb(PatchFunction *);
virtual void destroy_cb(PatchObject *);
\end{apient}


\apidesc{ Programmers implement the above virtual methods to handle
  the event of destroying a PatchBlock, a PatchEdge, a PatchFunction,
  or a PatchObject respectively. All the above methods will be called
  before corresponding object destructors are called. }

\begin{apient}
virtual void create_cb(PatchBlock *);
virtual void create_cb(PatchEdge *);
virtual void create_cb(PatchFunction *);
virtual void create_cb(PatchObject *);
\end{apient}


\apidesc{

Programmers implement the above virtual methods to handle the event of
creating a PatchBlock, a PatchEdge, a PatchFunction, or a PatchObject
respectively. All the above methods will be called after the objects
are created. }

\begin{apient}
virtual void split_block_cb(PatchBlock *first, PatchBlock *second);
\end{apient}


\apidesc{

Programmers implement the above virtual method to handle the event of
splitting a PatchBlock as a result of a new edge being discovered. The
above method will be called after the block is split.  }

\begin{apient}
virtual void remove_edge_cb(PatchBlock *, PatchEdge *, edge_type_t);
virtual void add_edge_cb(PatchBlock *, PatchEdge *, edge_type_t);

\end{apient}


\apidesc{
Programmers implement the above virtual methods to handle the events of removing
or adding an PatchEdge respectively. The method remove\_edge\_cb will be called
before the event triggers, while the method add\_edge\_cb will be called after
the event triggers.
}

\begin{apient}
virtual void remove_block_cb(PatchFunction *, PatchBlock *);
virtual void add_block_cb(PatchFunction *, PatchBlock *);
\end{apient}


\apidesc{
Programmers implement the above virtual methods to handle the events of removing
or adding a PatchBlock respectively. The method remove\_block\_cb will be called
before the event triggers, while the method add\_block\_cb will be called after
the event triggers.
}

\begin{apient}
virtual void create_cb(Point *pt);
virtual void destroy_cb(Point *pt);
\end{apient}


\apidesc{
Programmers implement the create\_cb method above, which will be called after
the Point \emph{pt} is created. And, programmers implement the destroy\_cb method,
which will be called before the point \emph{pt} is deleted.
}

\begin{apient}
virtual void change_cb(Point *pt, PatchBlock *first, PatchBlock *second);
\end{apient}


\apidesc{
Programmers implement this method, which is to be invoked after a block is
split. The provided Point belonged to the first block and is being
moved to the second. 
}

