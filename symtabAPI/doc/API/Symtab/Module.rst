\subsection{Class Module}\label{Module}
This class represents the concept of a single source file. Currently, Modules are only identified for the executable file; each shared library is made up of a single Module, ignoring any source file information that may be present. We also create a single module, called \code{DEFAULT\_MODULE}, for each Symtab that contains any symbols for which module information was unavailable. This may be compiler template code, or files produced without module information.

\begin{center}
\begin{tabular}{ll}
\toprule
supportedLanguages & Meaning \\
\midrule
lang\_Unknown & Unknown source language \\
lang\_Assembly & Raw assembly code \\
lang\_C & C source code \\
lang\_CPlusPlus & C++ source code \\
lang\_GnuCPlusPlus & C++ with GNU extensions \\
lang\_Fortran & Fortran source code \\
lang\_Fortran\_with\_pretty\_debug & Fortran with debug annotations \\
lang\_CMFortran & Fortran with CM extensions \\
\bottomrule
\end{tabular}
\end{center}

\begin{tabular}{p{1.25in}p{1.25in}p{3in}}
\toprule
Method name & Return type & Method description \\
\midrule
isShared & bool & True if the module is for a shared library, false for an executable. \\
fullName & std::string \& & Name, including path, of the source file represented by the module. \\
fileName & std::string \& & Name, not including path, of the source file represented by the module. \\
language & supportedLanguages & The source language used by the Module. \\
addr & Offset & Offset of the start of the module, as reported by the symbol table, assuming contiguous modules. \\
exec & Symtab * & Symtab object that contains the module. \\
\bottomrule
\end{tabular}


\subsubsection{Function, Variable, Symbol lookup}

\begin{apient}
typedef enum {
    mangledName,
    prettyName,
    typedName,
    anyName
} NameType;
\end{apient}

\begin{apient}
bool getAllFunctions(vector<Function *> &ret)
\end{apient}
\apidesc{
Returns all functions located within the PC address ranges covered by this module.
}

\begin{apient}
bool findVariablesByOffset(std::vector<Variable *> &ret,
                           const Offset offset)
\end{apient}
\apidesc{
This method returns a vector of \code{Variable}s with the specified offset. 
There may be more than one variable at an offset if they have different sizes.
Returns \code{true} on success and \code{false} if there is no matching variable.
The error value is set to \code{No\_Such\_Variable}.
}

\begin{apient}
bool findVariablesByName(vector<Function> &ret,
                         const string &name,
                         Symtab::NameType nameType,
                         bool isRegex = false,
                         bool checkCase = true)
\end{apient}
\apidesc{
This method finds and returns a vector of \code{Variable}s whose names match the given pattern. The \code{nameType} parameter determines which names are searched: mangled, pretty, typed, or any (note: a \code{Variable} may not have a typed name). If the \code{isRegex} flag is set a regular expression match is performed with the symbol names. \code{checkCase} is applicable only if \code{isRegex} has been set. This indicates if the case be considered while performing regular expression matching. \code{ret} contains the list of matching \code{Variables}, if any.
Returns \code{true} if it finds variables that match the given name, otherwise returns
\code{false}. The error value is set to \code{No\_Such\_Variable}.
}

\begin{apient}
bool getAllSymbols(vector<Symbol *> &ret)
\end{apient}
\apidesc{
This method returns all symbols.
Returns \code{true} on success and \code{false} if there are no symbols. The error value is
set to \code{No\_Such\_Symbol}.
}

\begin{apient}
bool getAllSymbolsByType(vector<Symbol *> &ret, 
                         Symbol::SymbolType sType)
\end{apient}
\apidesc{
This method returns all symbols whose type matches the given type \code{sType}.
Returns \code{true} on success and \code{false} if there are no symbols with the given type.
The error value is set to \code{No\_Such\_Symbol}.}

\subsubsection{Line number information}


\begin{apient}
bool getAddressRanges(vector<pair<unsigned long, unsigned long> > & ranges, 
                      string lineSource, unsigned int lineNo)
\end{apient}
\apidesc{
This method returns the address ranges in \code{ranges} corresponding to the line with line number \code{lineNo} in the source file \code{lineSource}. Searches only this module for the given source.
Return \code{true} if at least one address range corresponding to the line number was found and returns false if none found.
}

\begin{apient}
bool getSourceLines(vector<Statement *> &lines,
                    Offset addressInRange)
\end{apient}
\apidesc{
This method returns the source file names and line numbers corresponding to the given address \code{addressInRange}. Searches only this module for the given source. 
Return \code{true} if at least one tuple corresponding to the offset was found and returns \code{false} if none found. The \code{Statement} class used to be named \code{LineNoTuple}; backwards compatibility is provided via typedef. 
}

\begin{apient}
LineInformation *getLineInformation() const
\end{apient}
\apidesc{
This method returns the line map (section \ref{LineInformation}) corresponding to the module. Returns \code{NULL} if
there is no line information existing for the module.}

\begin{apient}
bool getStatements(std::vector<Statement *> &statements)
\end{apient}
\apidesc{
Returns all line information  (section \ref{Statement}) available for the module.
}

\subsubsection{Type information}
\label{subsubsec:typeInfo}

\begin{apient}
bool findType(Type * &type,
              string name)
\end{apient}
\apidesc{
This method performs a look up and returns a handle to the named \code{type}. 
This method searches all the built-in types, standard types and user-defined types within the module. Returns \code{true} if a type is found with type containing the handle to the type, else return \code{false}.
}

\begin{apient}
bool findLocalVariable(vector<localVar *> &vars,
                       string name)
\end{apient}
\apidesc{
The method returns a list of local variables within the module with name \code{name}. 
Returns \code{true} with vars containing a list of \code{localVar} objects corresponding to the local variables if found or else returns \code{false}.
}

\begin{apient}
bool findVariableType(Type *&type,
                      std::string name)
\end{apient}
\apidesc{
This method looks up a global variable with name \code{name} and returns its type attribute.
Returns \code{true} if a variable is found or returns \code{false} with \code{type} set to \code{NULL}.}
