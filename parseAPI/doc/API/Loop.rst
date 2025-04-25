\subsection{Class Loop}
\definedin{CFG.h}


The Loop class represents code that may execute repeatedly.
We detect both natural loops (loops that have a single entry block)
and irreducible loops (loops that have multiple entry blocks).
A back edge is defined as an edge that has its source in
the loop and has its target being an entry block of the loop.
It represents the end of an iteration of the loop.
For all the loops detected in a function, we also build a loop nesting 
tree to represent the nesting relations between the loops.
See class \code{LoopTreeNode} for more details.

\begin{apient}
Loop* parent
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
int getLoopEntries(vector<Block*>& entries);
\end{apient}
\apidesc{Fills \code{entries} with the set of entry basic blocks of the loop. Return the number of the entries that this loop has}
        
		   
\begin{apient}
int getBackEdges(vector<Edge*> &edges)
\end{apient}
\apidesc{Sets \code{edges} to the set of back edges in this loop. 
It returns the number of back edges that are in this loop. 
}
	
\begin{apient}
bool getContainedLoops(vector<Loop*> &loops)
\end{apient}
\apidesc{Returns a vector of loops that are nested under this loop.}

\begin{apient}
bool getOuterLoops(vector<Loop*> &loops)
\end{apient}
\apidesc{Returns a vector of loops that are directly nested under this loop.}

	
\begin{apient}
bool getLoopBasicBlocks(vector<Block*> &blocks)
\end{apient}
\apidesc{Fills \code{blocks} with all basic blocks in the loop}

        
\begin{apient}
bool getLoopBasicBlocksExclusive(vector<Block*> &blocks)
\end{apient}
\apidesc{Fills \code{blocks} with all basic blocks in this loop, excluding the blocks of its sub loops.}
     
\begin{apient}
bool hasBlock(Block *b);
\end{apient}
\apidesc{Returns \code{true} if this loop contains basic block \code{b}.}

\begin{apient}
bool hasBlockExclusive(Block *b);
\end{apient}
\apidesc{Returns \code{true} if this loop contains basic block \code{b} and \code{b} is not in its sub loops.}

\begin{apient}
bool hasAncestor(Loop *loop)
\end{apient}
\apidesc{Returns true if this loop is a descendant of the given loop.}

\begin{apient}
Function * getFunction();
\end{apient}
\apidesc{Returns the function that this loop is in.}

