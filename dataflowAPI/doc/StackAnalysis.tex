\subsection{Class StackAnalysis}
\label{sec:stackanalysis}

The StackAnalysis interface is used to determine the possible stack heights of abstract locations at any instruction in a function.  Due to there often being many paths through the CFG to reach a given instruction, abstract locations may have different stack heights depending on the path taken to reach that instruction.  In other cases, StackAnalysis is unable to adequately determine what is contained in an abstract location.  In both situations, StackAnalysis is conservative in its reported stack heights.  The table below explains what the reported stack heights mean.

\begin{tabular}{p{1.25in}p{4.25in}}
	\toprule
	Reported stack height & Meaning \\
	\midrule
	TOP & On all paths to this instruction, the specified abstract location contains a value that does not point to the stack. \\\\
	\emph{x} (some number) & On at least one path to this instruction, the specified abstract location has a stack height of \emph{x}.  On all other paths, the abstract location either has a stack height of \emph{x} or doesn't point to the stack. \\\\
	BOTTOM & There are three possible meanings:
	  \begin{enumerate}
		\item On at least one path to this instruction, StackAnalysis was unable to determine whether or not the specified abstract location points to the stack.
		\item On at least one path to this instruction, StackAnalysis determined that the specified abstract location points to the stack but could not determine the exact stack height.
		\item On at least two paths to this instruction, the specified abstract location pointed to different parts of the stack.
	  \end{enumerate} \\
	\bottomrule
\end{tabular}

\begin{apient}
	StackAnalysis(ParseAPI::Function *f)
\end{apient}
\apidesc{
	Constructs a StackAnalysis object for function \code{f}.
}

\begin{apient}
StackAnalysis(ParseAPI::Function *f,
              const std::map<Address, Address> &crm,
              const std::map<Address, TransferSet> &fs)
\end{apient}
\apidesc{
	Constructs a StackAnalysis object for function \code{f} with interprocedural analysis activated. A call resolution map is passed in \code{crm} mapping addresses of call sites to the resolved inter-module target address of the call.  Generally the call resolution map is created with DyninstAPI where PLT resolution is done.  Function summaries are passed in \code{fs} which maps function entry addresses to summaries.  The function summaries are then used at all call sites to those functions.
}

\begin{apient}
	StackAnalysis::Height find(ParseAPI::Block *b, Address addr, Absloc loc)
\end{apient}
\apidesc{
	Returns the stack height of abstract location \code{loc} before execution of the instruction with address \code{addr} contained in basic block \code{b}.  The address \code{addr} must be contained in block \code{b}, and block \code{b} must be contained in the function used to create this StackAnalysis object.
}

\begin{apient}
	StackAnalysis::Height findSP(ParseAPI::Block *b, Address addr)
	StackAnalysis::Height findFP(ParseAPI::Block *b, Address addr)
\end{apient}
\apidesc{
	Returns the stack height of the stack pointer and frame pointer, respectively, before execution of the instruction with address \code{addr} contained in basic block \code{b}.  The address \code{addr} must be contained in block \code{b}, and block \code{b} must be contained in the function used to create this StackAnalysis object.
}


\begin{apient}
void findDefinedHeights(ParseAPI::Block *b,
                        Address addr,
                        std::vector<std::pair<Absloc, StackAnalysis::Height>> &heights)
\end{apient}
\apidesc{
	Writes to the vector \code{heights} all defined <abstract location, stack height> pairs before execution of the instruction with address \code{addr} contained in basic block \code{b}. Note that abstract locations with stack heights of TOP (i.e. they do not point to the stack) are not written to \code{heights}.  The address \code{addr} must be contained in block \code{b}, and block \code{b} must be contained in the function used to create this StackAnalysis object.
}


\begin{apient}
	bool canGetFunctionSummary()
\end{apient}
\apidesc{
	Returns true if the function associated with this StackAnalysis object returns on some execution path.
}

\begin{apient}
	bool getFunctionSummary(TransferSet &summary)
\end{apient}
\apidesc{
	Returns in \code{summary} a summary for the function associated with this StackAnalysis object.  Function summaries can then be passed to the constructors for other StackAnalysis objects to enable interprocedural analysis.  Returns true on success.
}




\subsection{Class StackAnalysis::Height}
\definedin{stackanalysis.h}

The Height class is used to represent the abstract notion of stack heights.  Every Height object represents a stack height of either TOP, BOTTOM, or \emph{x}, where \emph{x} is some integral number.  The Height class also defines methods for comparing, combining, and modifying stack heights in various ways.

\begin{apient}
typedef signed long Height_t
\end{apient}
\apidesc{
	The underlying data type used to convert between Height objects and integral values.
}

\begin{tabular}{p{1.25in}p{1.125in}p{3.125in}}
	\toprule
	Method name & Return type & Method description \\
	\midrule
	height & Height\_t & This stack height as an integral value.\\
	format & std::string & This stack height as a string.\\
	isTop & bool & True if this stack height is TOP.\\
	isBottom & bool & True if this stack height is BOTTOM.\\
	\bottomrule
\end{tabular}

\begin{apient}
Height(const Height_t h)
\end{apient}
\apidesc{
	Creates a Height object with stack height \code{h}.
}

\begin{apient}
Height()
\end{apient}
\apidesc{
	Creates a Height object with stack height TOP.
}

\begin{apient}
bool operator<(const Height &rhs) const
bool operator>(const Height &rhs) const
bool operator<=(const Height &rhs) const
bool operator>=(const Height &rhs) const
bool operator==(const Height &rhs) const
bool operator!=(const Height &rhs) const
\end{apient}
\apidesc{
	Comparison operators for Height objects.  Compares based on the integral stack height treating TOP as MAX\_HEIGHT and BOTTOM as MIN\_HEIGHT.
}

\begin{apient}
Height &operator+=(const Height &rhs)
Height &operator+=(const signed long &rhs)
const Height operator+(const Height &rhs) const
const Height operator+(const signed long &rhs) const
const Height operator-(const Height &rhs) const
\end{apient}
\apidesc{
	Returns the result of basic arithmetic on Height objects according to the following rules, where \emph{x} and \emph{y} are integral stack heights and \emph{S} represents any stack height:
	\begin{itemize}[leftmargin=1in]
		\item \(TOP + TOP = TOP\)
		\item \(TOP + x = BOTTOM\)
		\item \(x + y = (x+y) \)
		\item \(BOTTOM + S = BOTTOM\)
	\end{itemize}
	Note that the subtraction rules can be obtained by replacing all + signs with - signs.

	The \code{operator+} and \code{operator-} methods leave this Height object unmodified while the \code{operator+=} methods update this Height object with the result of the computation.  For the methods where \code{rhs} is a \code{const signed long}, it is not possible to set \code{rhs} to TOP or BOTTOM.
}



