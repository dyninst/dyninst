\subsection{Class Function}

\definedin{CFG.h}

The Function class represents the portion of the program CFG that is reachable through intraprocedural control flow transfers from the function's entry block. Functions in the ParseAPI have only a single entry point; multiple-entry functions such as those found in Fortran programs are represented as several functions that ``share'' a subset of the CFG. Functions may be non-contiguous and may share blocks with other functions. 

\begin{center}
\begin{tabular}{ll}
\toprule
FuncSource & Meaning \\
\midrule
RT & recursive traversal (default) \\
HINT & specified in CodeSource hints \\
GAP & speculative parsing heuristics \\
GAPRT & recursive traversal from speculative parse \\
ONDEMAND & dynamically discovered at runtime \\
MODIFICATION & Added via user modification \\
\bottomrule
\end{tabular}
\end{center}

\apidesc{Return status of an function, which indicates whether this function will return to its caller or not; see description below.}

\begin{center}
\begin{tabular}{ll}
\toprule
FuncReturnStatus & Meaning \\
\midrule
UNSET & unparsed function (default) \\
NORETURN & will not return \\
UNKNOWN & cannot be determined statically \\
RETURN & may return \\
\bottomrule
\end{tabular}
\end{center}

\begin{apient}
typedef boost::transform_iterator<selector, blockmap::iterator> bmap_iterator
typedef boost::transform_iterator<selector, blockmap::const_iterator> bmap_const_iterator
typedef boost::iterator_range<bmap_iterator> blocklist
typedef boost::iterator_range<bmap_const_iterator> const_blocklist
typedef std::set<Edge*> edgelist
\end{apient}
\apidesc{Containers for block and edge access. Library users \emph{must not} rely on the underlying container type of std::set/std::vector lists, as it is subject to change.}

\begin{tabular}{p{1.25in}p{1.125in}p{3.125in}}
\toprule
Method name & Return type & Method description \\
\midrule
name & string & Name of the function. \\
addr & Address & Entry address of the function.  \\
entry & Block * & Entry block of the function. \\
parsed & bool & Whether the function has been parsed. \\
blocks & blocklist \& & List of blocks contained by this function sorted by entry address. \\
callEdges & const edgelist \& & List of outgoing call edges from this function. \\
returnBlocks & const\_blocklist \& & List of all blocks ending in return edges. \\
exitBlocks & const\_blocklist \& & List of all blocks that end the function, including blocks with no out-edges. \\
hasNoStackFrame & bool & True if the function does not create a stack frame. \\
savesFramePointer & bool & True if the function saves a frame pointer (e.g. \%ebp). \\
cleansOwnStack & bool & True if the function tears down stack-passed arguments upon return. \\
region & CodeRegion * & Code region that contains the function. \\
isrc & InstructionSource * & The InstructionSource for this function. \\
obj & CodeObject * & CodeObject that contains this function. \\
src & FuncSrc & The type of hint that identified this function's entry point. \\
restatus & FuncReturnStatus * & Returns the best-effort determination of whether this function may return or not. Return status cannot always be statically determined, and at most can guarantee that a function \emph{may} return, not that it \emph{will} return. \\
getReturnType & Type * & Type representing the return type of the function. \\

\bottomrule
\end{tabular}

\begin{apient}
Function(Address addr,
         string name,
         CodeObject * obj,
         CodeRegion * region,
         InstructionSource * isource)
\end{apient}
\apidesc{Creates a function at \code{addr} in the code region specified. 
Insructions for this function are given in \code{isource}.}

\begin{apient}
LoopTreeNode* getLoopTree()
\end{apient}
\apidesc{Return the nesting tree of the loops in the function. See class \code{LoopTreeNode} for more details}

\begin{apient}
Loop* findLoop(const char *name)
\end{apient}
\apidesc{Return the loop with the given nesting name. See class \code{LoopTreeNode} for more details about how loop nesting names are assigned.}

\begin{apient}
bool getLoops(vector<Loop*> &loops);
\end{apient}
\apidesc{Fill \code{loops} with all the loops in the function}

\begin{apient}
bool getOuterLoops(vector<Loop*> &loops);
\end{apient}
\apidesc{Fill \code{loops} with all the outermost loops in the function}

\begin{apient}
bool dominates(Block* A, Block *B);
\end{apient}
\apidesc{Return true if block \code{A} dominates block \code{B}}

\begin{apient}
Block* getImmediateDominator(Block *A);
\end{apient}
\apidesc{Return the immediate dominator of block \code{A}, \code{NULL} if the block \code{A} does not have an immediate dominator.}

\begin{apient}
void getImmediateDominates(Block *A, set<Block*> &imm);
\end{apient}
\apidesc{Fill \code{imm} with all the blocks immediate dominated by block \code{A}}

\begin{apient}
void getAllDominates(Block *A, set<Block*> &dom);
\end{apient}
\apidesc{Fill \code{dom} with all the blocks dominated by block \code{A}}


\begin{apient}
bool postDominates(Block* A, Block *B);
\end{apient}
\apidesc{Return true if block \code{A} post-dominates block \code{B}}

\begin{apient}
Block* getImmediatePostDominator(Block *A);
\end{apient}
\apidesc{Return the immediate post-dominator of block \code{A}, \code{NULL} if the block \code{A} does not have an immediate post-dominator.}

\begin{apient}
void getImmediatePostDominates(Block *A, set<Block*> &imm);
\end{apient}
\apidesc{Fill \code{imm} with all the blocks immediate post-dominated by block \code{A}}

\begin{apient}
void getAllPostDominates(Block *A, set<Block*> &dom);
\end{apient}
\apidesc{Fill \code{dom} with all the blocks post-dominated by block \code{A}}




\begin{apient}
std::vector<FuncExtent *> const& extents()
\end{apient}
\apidesc{Returns a list of contiguous extents of binary code within the function.}

\begin{apient}
void setEntryBlock(block * new_entry)
\end{apient}
\apidesc{Set the entry block for this function to \code{new\_entry}.}

\begin{apient}
void set_retstatus(FuncReturnStatus rs)
\end{apient}
\apidesc{Set the return status for the function to \code{rs}.}

\begin{apient}
bool contains(Block *b)
\end{apient}
\apidesc{Return true if this function contains the given block \code{b};
otherwise false.}


\begin{apient}
void removeBlock(Block *)
\end{apient}
\apidesc{Remove a basic block from the function.}
