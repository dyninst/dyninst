\subsection{Class Variable}

The \code{Variable} class represents a collection of symbols that have the same address and represent data. 

\begin{tabular}{p{1.25in}p{1.125in}p{3.125in}}
\toprule
Method name & Return type & Method description \\
\midrule
getOffset & Offset & Offset associated with this variable. \\
getSize & unsigned & Size of this variable in the symbol table. \\
mangled\_names\_begin & Aggregate::name\_iter & Beginning of a range of unique names of symbols pointing to this variable. \\
mangled\_names\_end & Aggregate::name\_iter & End of a range of unique names of symbols pointing to this variable. \\
getType & Type * & Type of this variable, if known. \\
getModule & const Module * & Module that contains this variable. \\
getRegion & Region * & Region that contains this variable. \\
\bottomrule
\end{tabular}

\begin{apient}
bool getSymbols(vector<Symbol *> &syms) const
\end{apient}
\apidesc{
This method returns the vector of \code{Symbol}s that refer to the variable.
}

\begin{apient}
bool setModule (Module *module)
\end{apient}
\apidesc{
This method changes the module to which the variable belongs. Returns \code{true} if it succeeds.
}

\begin{apient}
bool setSize (unsigned size)
\end{apient}
\apidesc{
This method changes the size of the variable to \code{size}. Returns \code{true} if it succeeds.
}

\begin{apient}
bool setOffset (Offset offset)
\end{apient}
\apidesc{
The method changes the offset of the variable. Returns \code{true} if it succeeds.
}

\begin{apient}
bool addMangledName(string name,
                    bool isPrimary)
\end{apient}
\apidesc{
This method adds a mangled name \code{name} to the variable. If \code{isPrimary} is \code{true} then it becomes the default name for the variable.
This method returns \code{true} on success and \code{false} on failure.
}

\begin{apient}
bool addPrettyName(string name,
                   bool isPrimary)
\end{apient}
\apidesc{
This method adds a pretty name \code{name} to the variable. If \code{isPrimary} is \code{true} then it becomes the default name for the variable. 
This method returns \code{true} on success and \code{false} on failure.
}

\begin{apient}
bool addTypedName(string name,
                  bool isPrimary)
\end{apient}
\apidesc{
This method adds a typed name \code{name} to the variable. If \code{isPrimary} is \code{true} then it becomes the default name for the variable. 
This method returns \code{true} on success and \code{false} on failure.
}


\begin{apient}
bool setType(Type *type)
\end{apient}
\apidesc{
Sets the type of the variable to \code{type}.
}
