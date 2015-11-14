\section{Examples}
\label{sec:example}

\subsection{Function Disassembly}

The following example uses ParseAPI and InstructionAPI to disassemble
the basic blocks in a function. As an example, it can be built with
G++ as follows: \code{g++ -std=c++0x -o code\_sample code\_sample.cc -L<library
  install path> -I<headers install path> -lparseAPI -linstructionAPI
  -lsymtabAPI -lsymLite -ldynDwarf -ldynElf -lcommon -L<libelf path>
  -lelf -L<libdwarf path> -ldwarf}. Note: this example must be
compiled with C++11x support; for G++ this is enabled with
\code{-std=c++0x}, and it is on by default for Visual Studio.

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left, numberstyle=\tiny, stepnumber=5, numbersep=5pt}
\lstset{showstringspaces=false}
\lstinputlisting{code_sample.cc}

\subsection{Control flow graph traversal}

The following complete example uses the ParseAPI to parse a binary and
dump its control flow graph in the Graphviz file format. As an
example, it can be built with G++ as follows: \code{g++ -std=c++0x -o example
  example.cc -L<library install path> -I<headers install path>
  -lparseAPI -linstructionAPI -lsymtabAPI -lsymLite -ldynDwarf
  -ldynElf -lcommon -L<libelf path> -lelf -L<libdwarf path>
  -ldwarf}. Note: this example must be compiled with C++11x support;
for G++ this is enabled with \code{-std=c++0x}, and it is on by
default for Visual Studio.

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left, numberstyle=\tiny, stepnumber=5, numbersep=5pt}
\lstset{showstringspaces=false}
\lstinputlisting{example.cc}

\subsection{Loop analysis}

The following code example shows how to get loop information using ParseAPI once we have an parsed Function object.

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left, numberstyle=\tiny, stepnumber=5, numbersep=5pt}
\lstset{showstringspaces=false}
\begin{lstlisting}
void GetLoopInFunc(Function *f) {
    // Get all loops in the function
    vector<Loop*> loops;
    f->getLoops(loops);

    // Iterate over all loops
    for (auto lit = loops.begin(); lit != loops.end(); ++lit) {
        Loop *loop = *lit;

        // Get all the entry blocks of the loop
	vector<Block*> entries;
	loop->getLoopEntries(entries);

        // Get all the blocks in the loop
        vector<Block*> blocks;
	loop->getLoopBasicBlocks(blocks);

	// Get all the back edges in the loop
	vector<Edge*> backEdges;
	loop->getBackEdges(backEdges);
    }
}
\end{lstlisting}

