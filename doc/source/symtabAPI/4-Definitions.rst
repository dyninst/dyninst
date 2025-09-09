\section{Definitions and Basic Types}

The following definitions and basic types are referenced throughout the rest of this document.

\subsection{Definitions}

\begin{description}
\item[Offset] Offsets represent an address relative to the start address(base) of the object file. For executables, the Offset represents an absolute address.
The following definitions deal with the symbol table interface.
\item[Object File] An object file is the representation of code that a compiler or assembler generates by processing a source code file. It represents .o's, a.out's and shared libraries.
\item[Region] A region represents a contiguous area of the file that contains executable code or readable data; for example, an ELF section.
\item[Symbol] A symbol represents an entry in the symbol table, and may identify a function, variable or other structure within the file.
\item[Function] A function represents a code object within the file represented by one or more symbols.
\item[Variable] A variable represents a data object within the file represented by one or more symbols.
\item[Module] A module represents a particular source file in cases where multiple files were compiled into a single binary object; if this information is not present, or if the binary object is a shared library, we use a single default module.
\item[Archive] An archive represents a collection of binary objects stored in a single file (e.g., a static archive). 
\item[Relocations] These provide the necessary information for inter-object references between two object files.
\item[Exception Blocks] These contain the information necessary for run-time exception handling
The following definitions deal with members of the Symbol class.
\item[Mangled Name] A mangled name for a symbol provides a way of encoding additional information about a function, structure, class or another data type in a symbol name.
It is a technique used to produce unique names for programming entities in many modern programming languages.
For example, the method \emph{foo} of class C with signature \emph{int C::foo(int, int)} has a mangled name \emph{\_ZN1C3fooEii} when compiled with gcc.
Mangled names may include a sequence of clone suffixes (begins with `.' that indicate a compiler synthesized function), and this may be followed by a version suffix (begins with `@').
\item[Pretty Name] A pretty name for a symbol is the demangled user-level symbolic name without type information for the function parameters and return types.
For non-mangled names, the pretty name is the symbol name.
Any function clone suffixes of the symbol are appended to the result of the demangler.
For example, a symbol with a mangled name \emph{\_ZN1C3fooEii} for the method \emph{int C::foo(int, int)} has a pretty name \emph{C::foo}.
Version suffixes are removed from the mangled name before conversion to the pretty name.
The pretty name can be obtained by running the command line tool \code{c++filt} as \code{c++filt -i -p \emph{name}}, or using the libiberty library function \code{cplus\_demangle} with options of \code{DMGL\_AUTO | DMGL\_ANSI}.
\item[Typed Name] A typed name for a symbol is the demangled user-level symbolic name including type information for the function parameters.
Typically, but not always, function return type information is not included. Any function clone information is also included.
For non-mangled names, the typed name is the symbol name.
For example, a symbol with a mangled name \emph{\_ZN1C3fooEii} for the method \emph{int C::foo(int, int)} has a typed name \emph{C::foo(int, int)}.
Version suffixes are removed from the mangled name before conversion to the typed name.
The typed name can be obtained by running the command line tool \code{c++filt} as \code{c++filt -i \emph{name}}, or using the libiberty library function \code{cplus\_demangle} with options of \code{DMGL\_AUTO | DMGL\_ANSI | DMGL\_PARAMS}.
\item[Symbol Linkage] The symbol linkage for a symbol gives information on the visibility (binding) of this symbol, whether it is visible only in the object file where it is defined (local), if it is visible to all the object files that are being linked (global), or if its a weak alias to a global symbol.
\item[Symbol Type] Symbol type for a symbol represents the category of symbols to which it belongs. It can be a function symbol or a variable symbol or a module symbol.
The following definitions deal with the type and the local variable interface.
\item[Type] A type represents the data type of a variable or a parameter. This can represent language pre-defined types (e.g. int, float), pre-defined types in the object (e.g., structures or unions), or user-defined types.
\item[Local Variable] A local variable represents a variable that has been declared within the scope of a sub-routine or a parameter to a sub-routine.
\end{description}

\subsection{Basic Types}

\begin{apient}
typedef unsigned long Offset
\end{apient}
\apidesc{An integer value that contains an offset from base address of the object file.}
\begin{apient}
typedef int typeId_t
\end{apient}
\apidesc{A unique handle for identifying a type. Each of types is assigned a globally unique ID. This way it is easier to identify any data type of a variable or a parameter.}
\begin{apient}
typedef ... PID
\end{apient}
\apidesc{A handle for identifying a process that is used by the dynamic components of SymtabAPI. On UNIX platforms PID is a int, on Windows it is a HANDLE that refers to a process.}
\begin{apient}
typedef unsigned long Address
\end{apient}
\apidesc{An integer value that represents an address in a process. This is used by the dynamic components of SymtabAPI.}
