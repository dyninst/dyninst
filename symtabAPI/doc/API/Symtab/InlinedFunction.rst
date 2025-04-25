\subsection{Class InlinedFunction}\label{InlinedFunction}
The \code{InlinedFunction} class represents an inlined function, as found in DWARF information. Its interface is almost entirely inherited from \code{FunctionBase}.

\begin{apient}
std::pair<std::string, Dyninst::Offset> getCallsite()
\end{apient}
\apidesc{
Returns the file and line corresponding to the call site of an inlined function.
}
