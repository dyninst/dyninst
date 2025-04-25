\appendix
\section{The Dyninst Domain}
\label{sec:dyninstdomain}
The \code{dyninst} domain has quite a few useful values and functions:

\begin{table}[!th]
\begin{tabular}{ | l | l | p{4cm} | p{6.5cm} |}
\hline
Identifier & Type & Where Valid & Description\\
\hline
\code{function\_name} & \code{char *} & Within a function & Evaluates to the
name of the current function. Call to \code{createSnippet(...)} must specify a
\code{BPatch\_point}.\\
\hline
\code{module\_name} & \code{char *} & Anywhere & Evaluates to the name of the
current module. Call to \code{createSnippet(...)} must specify a \code{BPatch\_point}.\\
\hline
\code{bytes\_accessed} & int & At a memory operation & Evaluates to the number of bytes accessed by a memory operation.\\  
\hline
\code{effective\_address} & \code{void *} & At a memory operation & Evaluates the effective address of a memory operation.\\
\hline
\code{original\_address} & \code{void *} & Anywhere & Evaluates to the original address where the snippet was inserted. \\
\hline
\code{actual\_address} & \code{void *} & Anywhere & Evaluates to the actual address of the instrumentation. \\
\hline
\code{return\_value} & \code{void *} & Function exit & Evaluates to the return value of a function.\\ 
\hline
\code{thread\_index} & int & Anywhere &  Returns the index of the thread the snippet is executing on.\\
\hline
\code{tid} & int & Anywhere & Returns the id of the thread the snippet is executing on.\\
\hline
\code{dynamic\_target} & \code{void *} & At calls, jumps, returns & Calculates the target of a control flow instruction.\\ 
\hline
\code{break()} & void & Anywhere & Causes the mutatee to execute a breakpoint.\\
\hline
\code{stopthread()} & void & Anywhere & Stops the thread on which the snippet is executing.\\
\hline

\end{tabular}
\caption{Dyninst Domain Values}
\end{table}


