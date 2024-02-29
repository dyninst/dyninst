\subsection{Class CodeObject}

\definedin{CodeObject.h}

The CodeObject class describes an individual binary code object, such as an
executable or library. It is the top-level container for parsing the object as
well as accessing that parse data. The following API routines and data types
are provided to support parsing and retrieving parsing products.

\begin{apient}

typedef std::set<Function *, Function::less> funclist
\end{apient}
\apidesc{Container for access to functions. Refer to Section \ref{sec:containers} for details. Library users \emph{must not} rely on the underlying container type of std::set, as it is subject to change.}

\begin{apient}
CodeObject(CodeSource * cs,
           CFGFactory * fact = NULL,
           ParseCallback * cb = NULL,
           bool defensiveMode = false)
\end{apient}
\apidesc{Constructs a new CodeObject from the provided CodeSource and
optional object factory and callback handlers. Any parsing hints provided
by the CodeSource are processed, but the binary is not parsed when this
constructor returns. The passed CodeSource is \textbf{not} owned by this
object. However, it must have the same lifetime as the CodeObject.

\medskip\noindent The \code{defensiveMode}
parameter optionally trades off coverage for safety; this mode is not
recommended for most applications as it makes very conservative assumptions
about control flow transfer instructions (see Section \ref{sec:defmode}).}

\begin{apient}
void parse()
\end{apient}
\apidesc{Recursively parses the binary represented by this CodeObject from all
known function entry points (i.e., the hints provided by the CodeSource). This
method and the following parsing methods may safely be invoked repeatedly if
new information about function locations is provided through the CodeSource.
Note that these parsing methods do not automatically perform speculative gap parsing.
parseGaps should be used for this purpose.}

\begin{apient}
void parse(Address target,
           bool recursive)
\end{apient}
\apidesc{Parses the binary starting with the instruction at the provided target address. If \code{recursive} is {\scshape true}, recursive traversal parsing is used as in the default \code{parse()} method; otherwise only instructions reachable through intraprocedural control flow are visited.}

\begin{apient}
void parse(CodeRegion * cr,
           Address target,
           bool recursive)
\end{apient}
\apidesc{Parses the specified core region of the binary starting with the 
instruction at the provided target address. If \code{recursive} is 
{\scshape true}, recursive traversal parsing is used as in the default 
\code{parse()} method; otherwise only instructions reachable through 
intraprocedural control flow are visited.
}

\begin{apient}
struct NewEdgeToParse {
    Block *source;
    Address target;
    EdgeTypeEnum type;
}
bool parseNewEdges( vector<NewEdgeToParse> & worklist )
\end{apient}
\apidesc{Parses a set of newly created edges specified in the worklist supplied 
that were not included when the function was originally parsed. 
}

ParseAPI is able to speculatively parse gaps 
(regions of binary that has not been identified as code or data yet)
to identify function entry points and
perform control flow traversal.

\begin{center}
\begin{tabular}{lp{10cm}}
\toprule
GapParsingType & Technique description \\
\midrule
PreambleMatching & If instruction patterns are matched at an adderss, the address is a function entry point  \\
IdiomMatching & Based on a pre-trained model, this technique calculates the probability of an address to be a function entry point and predicts whether which addresses are function entry points\\
\bottomrule
\end{tabular}
\end{center}

\begin{apient}
void parseGaps(CodeRegion *cr,
               GapParsingType type=IdiomMatching)
\end{apient}
\apidesc{Speculatively parse the indicated region of the binary using the specified technique to find likely function entry points, enabled on the x86 and x86-64 platforms.}

\fbox{\begin{minipage}[t]{1\columnwidth}%
\begin{center}{\textbf{A note on using the lookup functions}}\end{center}
When parsing binary objects such as .o files and static libraries which may have multiple
\texttt{CodeRegion} objects that overlap in the address space, the \texttt{CodeRegion} argument
\textit{must} be passed. For executable binaries and shared libraries that are fully linked, there
is no ambiguity, and {\scshape null} can be passed. The only exception is \texttt{findFuncsByBlock}
which always requires a valid \texttt{CodeRegion}.%
\end{minipage}}

\begin{apient}
Function * findFuncByEntry(CodeRegion * cr,
                           Address entry)
\end{apient}
\apidesc{Find the function starting at address \code{entry} in the indicated CodeRegion. Returns {\scshape null} if no such function exists.}

\begin{apient}
int findFuncs(CodeRegion * cr,
              Address addr,
              std::set<Function*> & funcs)
\end{apient}
\apidesc{Finds all functions spanning \code{addr} in the code region, adding each to \code{funcs}. The number of results of this stabbing query are returned.}

\begin{apient}
int findFuncs(CodeRegion * cr,
              Address start,
              Address end,
              std::set<Function*> & funcs)
\end{apient}
\apidesc{Finds all functions overlapping the range \code{[start,end)} in the code region, adding each to \code{funcs}. The number of results of this stabbing query are returned.}

\begin{apient}
const funclist & funcs()
\end{apient}
\apidesc{Returns a const reference to a container of all functions in the binary. Refer to Section \ref{sec:containers} for container access details.}

\begin{apient}
Block * findBlockByEntry(CodeRegion * cr,
                         Address entry)
\end{apient}
\apidesc{Find the basic block starting at address \code{entry}. Returns {\scshape null} if no such block exists.}

\begin{apient}
int findBlocks(CodeRegion * cr,
               Address addr,
               std::set<Block*> & blocks)
\end{apient}
\apidesc{Finds all blocks spanning \code{addr} in the code region, adding each to \code{blocks}. Multiple blocks can be returned only on platforms with variable-length instruction sets (such as IA32) for which overlapping instructions are possible; at most one block will be returned on all other platforms.}

\begin{apient}
Block * findNextBlock(CodeRegion * cr,
                      Address addr)
\end{apient}
\apidesc{Find the next reachable basic block starting at address \code{entry}. Returns {\scshape null} if no such block exists.}

\begin{apient}
CodeSource * cs()
\end{apient}
\apidesc{Return a reference to the underlying CodeSource.}

\begin{apient}
CFGFactory * fact()
\end{apient}
\apidesc{Return a reference to the CFG object factory.}

\begin{apient}
bool defensiveMode()
\end{apient}
\apidesc{Return a boolean specifying whether or not defensive mode is enabled.}

\begin{apient}
bool isIATcall(Address insn,
               std::string &calleeName)
\end{apient}
\apidesc{Returns a boolean specifying if the address at \code{addr} is located at the call named in \code{calleeName}. }

\begin{apient}
void startCallbackBatch()
\end{apient}
\apidesc{Starts a batch of callbacks that have been registered.}

\begin{apient}
void finishCallbackBatch()
\end{apient}
\apidesc{Completes all callbacks in the current batch.}

\begin{apient}
void registerCallback(ParseCallback *cb);
\end{apient}
\apidesc{Register a callback \code{cb}}

\begin{apient}
void unregisterCallback(ParseCallback *cb);
\end{apient}
\apidesc{Unregister an existing callback \code{cb}}


\begin{apient}
void finalize()
\end{apient}
\apidesc{Force complete parsing of the CodeObject; parsing operations are otherwise completed only as needed to answer queries.}

\begin{apient}
void destroy(Edge *)
\end{apient}
\apidesc{Destroy the edge listed.}

\begin{apient}
void destroy(Block *)
\end{apient}
\apidesc{Destroy the code block listed.}

\begin{apient}
void destroy(Function *)
\end{apient}
\apidesc{Destroy the function listed.}

