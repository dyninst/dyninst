\appendix
\section{PatchAPI for Dyninst Programmers} 
\label{sec-dyn}

The PatchAPI is a Dyninst component and as such is accessible through
the main Dyninst interface (BPatch objects). However, the PatchAPI
instrumentation and CFG models differ from the Dyninst models in
several critical ways that should be accounted for by users. This
section summarizes those differences and describes how to access
PatchAPI abstractions from the DyninstAPI interface. 

\subsection{Differences Between DyninstAPI and PatchAPI}

The DyninstAPI and PatchAPI differ primarily in their CFG
representations and instrumentation point abstractions. In general,
PatchAPI is more powerful and can better represent complex binaries
(e.g., highly optimized code or malware). In order to maintain
backwards compatibility, the DyninstAPI interface has not been
extended to match the PatchAPI. As a result, there are some caveats. 

The PatchAPI uses the same CFG model as the ParseAPI. The primary
representation is an interprocedural graph of basic blocks and
edges. Functions are defined on top of this graph as collections of
blocks. \textbf{A block may be contained by more than one function;}
we call this the \emph{shared block} model. Functions are defined to
have a single entry block, and functions may overlap if they contain
the same blocks. Call and return edges exist in the graph, and
therefore traversing the graph may enter different functions. PatchAPI
users may specify instrumenting a particular block within a particular
function (a \emph{block instance}) by specifying both the block and
the function. 

The DyninstAPI uses a historic CFG model. The primary representation
is the function. Functions contain a intraprocedural graph of blocks
and edges. As a result, a basic block belongs to only one function,
but two blocks from different functions may be \emph{clones} of each
other. No interprocedural edges are represented in the graph, and thus
traversing the CFG from a particular function is guaranteed to remain
inside that function. 

As a result, multiple DyninstAPI blocks may map to the same PatchAPI
block. If instrumenting a particular block instance is desired, the
user should provide both the DyninstAPI basic block and function. 

In addition, DyninstAPI uses a \emph{module} abstraction, where
a \texttt{BPatch\_module} represents a collection of functions from a
particular source file (for the executable) or from an entire library
(for all libraries). PatchAPI, like ParseAPI, instead uses
an \emph{object} representation, where a \texttt{PatchObject} object
represents a collection of functions from a file on disk (executable
or libraries). 

The instrumentation point (\emph{instPoint}) models also differ
between DyninstAPI and PatchAPI. We classify an instPoint either as
a \emph{behavior} point (e.g., function entry) or \emph{location}
point (e.g., a particular instruction). PatchAPI fully supports both
of these models, with the added extension that a location point
explicitly specifies whether instrumentation will execute before or
after the corresponding location. Dyninst does not support the
behavior model, instead mapping behavior instPoints to a corresponding
instruction. For example, if a user requests a function entry
instPoint they instead receive an instPoint for the first instruction
in the function. These may not always be the
same (see \href{ftp://ftp.cs.wisc.edu/paradyn/papers/Bernat11AWAT.pdf}{Bernat\_AWAT}). In addition, location instPoints represent an
instruction, and the user must later specify whether they wish to
instrument before or after that instruction.

As a result, there are complications for using both DyninstAPI and
PatchAPI. We cannot emphasize enough, though, that users \emph{can
combine DyninstAPI and PatchAPI} with some care. Doing so offers
several benefits:
\begin{itemize}
\item The ability to extend legacy code that is written for
DyninstAPI. 
\item The ability to use the DyninstAPI extensions and plugins for
PatchAPI, including snippet-based or dynC-based code generation and
our instrumentation optimizer. 
\end{itemize}
We suggest the following best practices to be followed when coding for
PatchAPI via Dyninst:
\begin{itemize}
\item For legacy code, do not attempt to map between DyninstAPI
instPoints and PatchAPI instPoints. Instead, use DyninstAPI CFG
objects to acquire PatchAPI CFG objects, and use a \texttt{PatchMgr} (acquired
through a \texttt{BPatch\_addressSpace}) to look up
PatchAPI instPoints. 
\item For new code, acquire a \texttt{PatchMgr} directly from
a \texttt{BPatch\_addressSpace} and use its methods to look up both
CFG objects and instPoints. 
\end{itemize}

\subsection{PatchAPI accessor methods in Dyninst}

To access a PatchAPI class from a Dyninst class, use
the \texttt{PatchAPI::convert} function, as in the following example:

\begin{apient}
  BPatch_basicBlock *bp_block = ...;
  PatchAPI::PatchBlock *block = PatchAPI::convert(bp_block);
\end{apient}

We support the following mappings, where all PatchAPI objects
are within the \texttt{Dyninst::PatchAPI} namespace:

\begin{tabular}{|l|l|l|}
\hline
From & To & Comments \\
\hline 
\texttt{BPatch\_function} & \texttt{PatchFunction} & \\
\texttt{BPatch\_basicBlock} & \texttt{PatchBlock} & See above. \\
\texttt{BPatch\_edge} & \texttt{PatchEdge} & See above. \\
\texttt{BPatch\_module} & \texttt{PatchObject} & See above. \\
\texttt{BPatch\_image} & \texttt{PatchMgr} & \\
\texttt{BPatch\_addressSpace} & \texttt{PatchMgr} & \\
\texttt{BPatch\_snippet} & \texttt{Snippet} & \\
\hline
\end{tabular}

We do not support a direct mapping between \texttt{BPatch\_point}s
and \texttt{Point}s, as the failure of Dyninst to properly represent
behavior instPoints leads to confusing results. Instead, use
the PatchAPI point lookup methods. 
    
