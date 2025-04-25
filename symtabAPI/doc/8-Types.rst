\section{API Reference - Type Interface}
This section describes the type interface for the SymtabAPI library. Currently this interface has the following capabilities:
\begin{itemize}
    \item Look up types within an object file.
    \item Extend the types to create new types and add them to the Symtab file representation. These types will be available for lookup but will not be added if a new object file is produced.
\end{itemize}
    
The rest of the section describes the classes that are part of the type interface.

\input{API/Types/Type}
