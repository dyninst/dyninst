\section{API Reference - Line Number Interface}\label{LineNoInterface}

This section describes the line number interface for the SymtabAPI library. Currently this interface has the following capabilities:
\begin{itemize}
\item Look up address ranges for a given line number.
\item Look up source lines for a given address.
\item Add new line information. This information will be available for lookup, but will not be included with an emitted object file. 
\end{itemize}

In order to look up or add line information, the user/application must have already parsed the object file and should have a Symtab handle to the object file. For more information on line information lookups through the Symtab class refer to Section \ref{sec:symtabAPI}. The rest of this section describes the classes that are part of the line number interface.

\input{API/LineInfo/LineInformation}
\input{API/LineInfo/Statement}
\input{API/LineInfo/Iterating}
