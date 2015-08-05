\subsection{Class LoopTreeNode}
\definedin{CFG.h}
The LoopTreeNode class provides a tree interface to a collection of instances of class 
Loop contained in a function. The structure of the tree 
follows the nesting relationship of the loops in a function. 
Each LoopTreeNode contains a pointer to a loop (represented by Loop), and a set
of sub-loops (represented by other LoopTreeNode objects). The \code{loop} field
at the root node is always \code{NULL} since a function may contain multiple outer 
loops. The \code{loop} field is never \code{NULL} at any other node since it
always corresponds to a real loop.
Therefore, the outer most loops in the function are contained in the vector of \code{children} of the root. 

Each instance of LoopTreeNode is given a name that indicates its position in the hierarchy of loops.
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

The \code{foo} function will have a root LoopTreeNode, containing a NULL loop entry and 
two LoopTreeNode children representing the functions outermost loops. These children 
would have names \code{loop\_1} and \code{loop\_2}, respectively representing the \code{x} and \code{i} loops. \code{loop\_2} has 
no children. \code{loop\_1} has two child LoopTreeNode objects, named
\code{loop\_1.1} and 
\code{loop\_1.2}, respectively representing the \code{y} and \code{z} loops. 

\begin{apient}
Loop *loop;
\end{apient}
\apidesc{The Loop instance it points to.}

\begin{apient}
std::vector<LoopTreeNode *> children;
\end{apient}
\apidesc{The LoopTreeNode instances nested within this loop.}
    
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
bool getCallees(vector<Function *> &v);
\end{apient}
\apidesc{Fills \code{v} with a vector of the functions called inside this loop.}
    
    
\begin{apient}
Loop * findLoop(const char *name);
\end{apient}
\apidesc{Looks up a loop by the hierarchical name}
    
