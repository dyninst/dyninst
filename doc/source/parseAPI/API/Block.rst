\subsection{Class Block}

\definedin{CFG.h}

A Block represents a basic block as defined in Section \ref{sec:abstractions}, and is the lowest level representation of code in the CFG.

\begin{apient}
typedef std::vector<Edge *> edgelist
\end{apient}
\apidesc{Container for edge access. Refer to Section \ref{sec:containers} for details. Library users \emph{must not} rely on the underlying container type of std::vector, as it is subject to change.}

\begin{tabular}{p{1.25in}p{1.125in}p{3.125in}}
\toprule
Method name & Return type & Method description \\
\midrule
start & Address & Address of the first instruction in the block. \\
end & Address & Address immediately following the last instruction in the block. \\
last & Address & Address of the last instruction in the block. \\
lastInsnAddr & Address & Alias of \code{last}. \\
size & Address & Size of the block; \code{end} - \code{start}. \\
parsed & bool & Whether the block has been parsed. \\
obj & CodeObject * & CodeObject containing this block. \\
region & CodeRegion * & CodeRegion containing this block. \\
sources & const edgelist \& & List of all in-edges to the block. \\
targets & const edgelist \& & List of all out-edges from the block. \\
containingFuncs & int & Number of Functions that contain this block. \\
\bottomrule
\end{tabular}

\begin{apient}
Block(CodeObject * o,
      CodeRegion * r,
      Address start,
      Function* f = NULL)
\end{apient}
\apidesc{Creates a block at \code{start} in the code region and code object specified.
Optionally, one can specify the function that will parse the block.
This constructor is used by the ParseAPI parser, which will update its end address during parsing.}

\begin{apient}
Block(CodeObject * o,
      CodeRegion * r,
      Address start,
      Address end,
      Address last,
      Function* f = NULL)
\end{apient}
\apidesc{Creates a block at \code{start} in the code region and code object specified.
The block has its last instruction at address \code{last} and ends at address \code{end}.
This constructor allows external parsers to construct their own blocks.}


\begin{apient}
bool consistent(Address addr,
                Address & prev_insn)
\end{apient}
\apidesc{Check whether address \code{addr} is \emph{consistent} with this basic block. An address is consistent if it is the boundary between two instructions in the block. As long as \code{addr} is within the range of the block, \code{prev\_insn} will contain the address of the previous instruction boundary before \code{addr}, regardless of whether \code{addr} is consistent or not.}

\begin{apient}
void getFuncs(std::vector<Function *> & funcs)
\end{apient}
\apidesc{Fills in the provided vector with all functions that share this basic block.}

\begin{apient}
template <class OutputIterator>
void getFuncs(OutputIterator result)
\end{apient}
\apidesc{Generic version of the above; adds each Function that contains this block to the provided OutputIterator. For example:}

\begin{lstlisting}
       std::set<Function *> funcs;
       block->getFuncs(std::inserter(funcs, funcs.begin()));
\end{lstlisting}

\begin{apient}
typedef std::map<Offset, InstructionAPI::Instruction::Ptr> Insns
void getInsns(Insns &insns) const
\end{apient}
\apidesc{Disassembles the block and stores the result in
  \code{Insns}.}

\begin{apient}
InstructionAPI::Instruction::Ptr getInsn(Offset o) const
\end{apient}
\apidesc{Returns the instruction starting at offset \code{o} within
  the block. Returns \code{InstructionAPI::Instruction::Ptr()} if
  \code{o} is outside the block, or if an instruction does not begin
  at \code{o}.}
