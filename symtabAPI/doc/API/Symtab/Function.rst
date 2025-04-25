\subsection{Class Function}\label{Function}
The \code{Function} class represents a collection of symbols that have the same address
and a type of \code{ST\_FUNCTION}. When appropriate, use this representation instead of the underlying \code{Symbol} objects. 

\begin{tabular}{p{1.25in}p{1.125in}p{3.125in}}
\toprule
Method name & Return type & Method description \\
\midrule
getModule & const Module * & Module this function belongs to. \\
getOffset & Offset & Offset in the file associated with the function. \\
getSize & unsigned & Size encoded in the symbol table; may not be actual function size. \\
mangled\_names\_begin & Aggregate::name\_iter & Beginning of a range of unique names of symbols pointing to this function. \\
mangled\_names\_end & Aggregate::name\_iter & End of a range of unique names of symbols pointing to this function. \\
pretty\_names\_begin &  Aggregate::name\_iter & As above, but prettified with the demangler. \\
pretty\_names\_end &  Aggregate::name\_iter & As above, but prettified with the demangler. \\
typed\_names\_begin & Aggregate::name\_iter  & As above, but including full type strings. \\
typed\_names\_end & Aggregate::name\_iter  & As above, but including full type strings. \\
getRegion & Region * & Region containing this function. \\
getReturnType & Type * & Type representing the return type of the function. \\
\bottomrule
\end{tabular}
	
\begin{apient}
bool getSymbols(vector<Symbol *> &syms) const
\end{apient}
\apidesc{
This method returns the vector of \code{Symbol}s that refer to the function.
}

\begin{apient}
bool setModule (Module *module)
\end{apient}
\apidesc{
This function changes the module to which the function belongs to \code{module}. Returns \code{true} if it succeeds.
}

\begin{apient}
bool setSize (unsigned size)
\end{apient}
\apidesc{
This function changes the size of the function to \code{size}. Returns \code{true} if it succeeds.
}

\begin{apient}
bool setOffset (Offset offset)
\end{apient}
\apidesc{
The method changes the offset of the function to \code{offset}. Returns \code{true} if it succeeds.
}

\begin{apient}
bool addMangledName(string name,
                    bool isPrimary)
\end{apient}
\apidesc{
This method adds a mangled name \code{name} to the function. If \code{isPrimary} is \code{true} then it becomes the default name for the function.
This method returns \code{true} on success and \code{false} on failure.
}

\begin{apient}
bool addPrettyName(string name,
                   bool isPrimary)
\end{apient}
\apidesc{
This method adds a pretty name \code{name} to the function. If \code{isPrimary} is \code{true} then it becomes the default name for the function. 
This method returns \code{true} on success and \code{false} on failure.
}

\begin{apient}
bool addTypedName(string name,
                  bool isPrimary)
\end{apient}
\apidesc{
This method adds a typed name \code{name} to the function. If \code{isPrimary} is \code{true} then it becomes the default name for the function. 
This method returns \code{true} on success and \code{false} on failure.
}

\begin{apient}
bool getLocalVariables(vector<localVar *> &vars)
\end{apient}
\apidesc{
This method returns the local variables in the function. \code{vars} contains the list of variables found.
If there is no debugging information present then it returns \code{false} with the
error code set to \code{NO\_DEBUG\_INFO} accordingly. Otherwise it returns \code{true}.
}

\begin{apient}
std::vector<VariableLocation> &getFramePtr()
\end{apient}
\apidesc{
This method returns a list of frame pointer offsets (abstract top of the stack) for the function. See the \code{VariableLocation} class description for more information. 
}

\begin{apient}
bool getParams(vector<localVar *> &params)
\end{apient}
\apidesc{
This method returns the parameters to the function. \code{params} contains the list of parameters.
If there is no debugging information present then it returns \code{false} with the
error code set to \code{NO\_DEBUG\_INFO} accordingly. Returns \code{true} on success.
}

\begin{apient}
bool findLocalVariable(vector<localVar *> &vars,
                       string name)
\end{apient}
\apidesc{
This method returns a list of local variables within a function that have name \code{name}. \code{vars} contains the list of variables found.
Returns \code{true} on success and \code{false} on failure.
}
