\section{Abstractions}
\label{sec:abstractions}

\begin{figure}
    \centering
    \tikzstyle{symbol} = [rectangle, draw, fill=green!25]
    \tikzstyle{type} = [rectangle, draw, rounded corners, fill=yellow!25]
    \tikzstyle{local} = [ellipse, draw, fill=red!25]
    \tikzstyle{line} = [trapezium, trapezium left angle=70, trapezium right angle=70, draw, fill=cyan!25]
    \tikzstyle{annotation} = []
    \begin{tikzpicture}[
            level distance=1cm,
            level 3/.style={level distance=1cm}, 
            growth parent anchor=south,
            sibling distance=2.2cm,
            child anchor=north,
        ]

        \node [symbol] (Archive) {Archive} [->]
        child {
            node [symbol] (Symtab) {Symtab}
            child {
                node [symbol] (Module) {Module}
                child  {
                    node [symbol] (ExceptionBlock) {ExceptionBlock}
%                    edge from parent node[sloped, above, draw=none] {N:1}
                }
                child [xshift=.5cm]{
                    node [symbol] (Function) {Function}
%                    edge from parent node[sloped, above, draw=none] {N:1}
                }
                child [yshift=-1cm-1em] {
                    node [symbol] (Symbol) {Symbol}
                    child {
                        node [local] (localVar) {localVar}
%                        edge from parent node[left, draw=none] {1:N}
                    }
%                    edge from parent node[sloped, above, draw=none] {N:1}
                }
                child [xshift=-.25cm] {
                    node [symbol] (Variable) {Variable}
%                    edge from parent node[sloped, above, draw=none] {N:1}
                }
                child [xshift=-.5cm] {
                    node [type] (Type) {Type}
%                    edge from parent node[sloped, above, draw=none] {1:N}
                }
                child {
                    node [line] (LineInformation) {LineInformation}
                    child {
                        node [line] (Statement) {Statement}
%                        edge from parent node[left, draw=none] {1:N}
                    }
%                    edge from parent node[sloped, above, draw=none] {1:1}
                }
%                edge from parent node[left, draw=none] {1:N}
            }
%            edge from parent node[left, draw=none] {1:N}
        }
        ;

%        \draw [->] (Function) edge node[sloped, below] {1:N} (Symbol);
%        \draw [->] (Variable) edge node[sloped, below] {N:1} (Symbol);
        \draw [->] (Function) -- (Symbol);
        \draw [->] (Variable) -- (Symbol);

    \end{tikzpicture}
    
    \vspace{1cm}

    % legend
    \begin{tikzpicture}
    \node [symbol, minimum width=1.5cm, minimum height=1em] (symbol) {A};
    \node [annotation, right=.25cm of symbol] (classSymbol) {Class A belongs to the};
    \node [annotation, below=0cm of classSymbol] (classSymbol2) {symbol table interface};

    \node [type, minimum width=1.5cm, minimum height=1em, below=1cm of symbol] (type) {A};
    \node [annotation, right=.25cm of type] (classType) {Class A belongs to the};
    \node [annotation, below=0cm of classType] (classType2) {Type interface};

    \node [local, minimum width=1.5cm, minimum height=1em, right=5cm of symbol] (local) {A};
    \node [annotation, right=.25cm of local] (classLocal) {Class A belongs to the};
    \node [annotation, below=0cm of classLocal] (classLocal2) {Local Variable interface};

    \node [line, below=1cm of local] (line) {~~A~~};
    \node [annotation, right=.25cm of line] (classLine) {Class A belongs to the};
    \node [annotation, below=0cm of classLine] (classLine2) {Line Number interface};

%    \node [annotation, below=1cm of type] (ownA) {A};
%    \node [annotation, right=1cm of ownA] (ownB) {B};
%    \draw [->] (ownA) edge node[above] {1:1} (ownB);
%    \node [annotation, right=.25cm of ownB] (own) {Each instance of class A owns
%    one instance of class B};
%
%    \node [annotation, below=.5cm of ownA] (ownmultA) {A};
%    \node [annotation, right=1cm of ownmultA] (ownmultB) {B};
%    \draw [->] (ownmultA) edge node[above] {1:N} (ownmultB);
%    \node [annotation, right=.25cm of ownmultB] (ownmult) {Each instance of class A owns
%    multiple instances of class B};
%
    \node [rectangle, draw,
    fit=(symbol)(classSymbol)(classSymbol2)(type)(classType)(classType2)(local)(classLocal)(classLocal2)(line)(classLine)(classLine2)]
    (legendBox) {};
    \node [rectangle, label=left: \rotatebox{90}{LEGEND}, fit=(legendBox)] {};

    \end{tikzpicture}
    
    \caption{SymtabAPI Object Ownership Diagram}
    \label{fig:object-ownership}
\end{figure}

SymtabAPI provides a simple set of abstractions over complicated data structures
which makes it straight-forward to use. The SymtabAPI consists of five classes of
interfaces: the symbol table interface, the type interface, the line map
interface, the local variable interface, and the address translation interface. 

Figure~\ref{fig:object-ownership} shows the ownership hierarchy for the
SymtabAPI classes. Ownership here is a ``contains'' relationship; if one
class owns another, then instances of the owner class maintain an exclusive
instance of the other. For example, each Symtab class instance contains multiple
instances of class Symbol and each Symbol class instance belongs to one Symtab
class instance. Each of four interfaces and the classes belonging to these
interfaces are described in the rest of this section. The API functions in each
of the classes are described in detail in Section \ref{sec:symtabAPI}.

\subsection{Symbol Table Interface}

The symbol table interface is responsible for parsing the object file and
handling the look-up and addition of new symbols. It is also responsible for the
emit functionality that SymtabAPI supports. The Symtab and the Module classes
inherit from the LookupInterface class, an abstract class, ensuring the same
lookup function signatures for both Module and Symtab classes. 

\begin{description}
\item[Symtab] A Symtab class object represents either an object file on-disk or in-memory that the SymtabAPI library operates on.
\item[Symbol] A Symbol class object represents an entry in the symbol table.
\item[Module] A Module class object represents a particular source file in cases where multiple files were compiled into a single binary object; if this information is not present, we use a single default module.
\item[Archive] An Archive class object represents a collection of binary objects stored in a single file (e.g., a static archive). 
\item[ExceptionBlock] An ExceptionBlock class object represents an exception block which contains the information necessary for run-time exception handling.
\end{description}

In addition, we define two symbol aggregates, Function and Variable. These classes collect multiple symbols with the same address and type but different names; for example, weak and strong symbols for a single function. 

\subsection{Type Interface}
\label{subsec:typeInterface}
 The Type interface is responsible for parsing type information from the object file and handling the look-up and addition of new type information. Figure \ref{fig:class-inherit} shows the class inheritance diagram for the type interface. Class Type is the base class for all of the classes that are part of the interface. This class provides the basic common functionality for all the types, such as querying the name and size of a type. The rest of the classes represent specific types and provide more functionality based on the type. 

 \begin{figure}
    \centering
    \tikzstyle{class} = [rectangle, draw, rounded corners, fill=yellow!100]
    \tikzstyle{class2} = [rectangle, draw, rounded corners, fill=yellow!100,
node distance=.65cm]
    \tikzstyle{abstract} = [rectangle, draw, rounded corners,
pattern=north west lines, pattern color=yellow]
    \tikzstyle{annotation} = []
    \begin{tikzpicture}[
        level distance=1cm, 
        growth parent anchor=south,
        level 1/.style={sibling distance=3cm},
    ]

    \node [class] (type) {Type} [<-]
    child [level distance=1.95cm+1em] {
        node [class2] (typeEnum) {typeEnum}
        node [class2, below of=typeEnum] (typeFunction) {typeFunction}
        node [class2, below of=typeFunction] (typeScalar) {typeScalar}
    }
    child [xshift=1cm] {
        node [abstract] (derivedType) {derivedType}
        child {
            node [class2] (typeRef) {typeRef}
            node [class2, below of=typeRef] (typePointer) {typePointer}
            node [class2, below of=typePointer] (typeTypedef) {typeTypedef}
        }
    }
    child [xshift=1cm] {
        node [abstract] (filedListType) {fieldListType}
        child {
            node [class2] (typeStruct) {typeStruct}
            node [class2, below of=typeStruct] (typeUnion) {typeUnion}
            node [class2, below of=typeUnion] (typeCommon) {typeCommon}
        }
    }
    child [xshift=1cm] {
        node [abstract] (rangedType) {rangedType}
        child {
            node [class2] (typeArray) {typeArray}
            node [class2, below of=typeArray] (typeSubrange) {typeSubrange}
        }
    }
    ;

    \node [class, above=1cm-1em of typeEnum] (CBlock) {CBlock};

    \end{tikzpicture}

    \vspace{1cm}

    \begin{tikzpicture}[]
    % legend
    \node [class, minimum width=2cm, minimum height=1em] (A) {A};
    \node [annotation, right=.25cm of A] (classA) {Class A belongs to the};
    \node [annotation, below=0cm of classA] (classA2) {Type interface};

    \node [annotation, right=1.5cm of classA] (littleA) {A};
    \node [annotation, right of=littleA] (littleB) {B};
    \draw [->] (littleA) -- (littleB);
    \node [annotation, right=.25cm of littleB] (inherits) {A inherits from B};

    \node [abstract, below=.5cm of littleA, minimum width=2cm, minimum
height=1.25em] (abstract) {};
    \node [annotation, right=.25cm of abstract] (abstractClass) {Represents an abstract class};

    \node [rectangle, draw, fit=(A)(classA)(classA2)(littleA)(littleB)(inherits)(abstract)(abstractClass)]
(legendBox) {};   
    \node [rectangle, label=left: \rotatebox{90}{LEGEND}, fit=(legendBox)] {};

    \end{tikzpicture}
     \caption{SymtabAPI Type Interface - Class Inheritance Diagram}
     \label{fig:class-inherit}
 \end{figure}

Some of the types inherit from a second level of type classes, each representing a separate category of types. 
\begin{description}
\item[fieldListType] - This category of types represent the container types that contain a list of fields. Examples of this category include structure and the union types. 
\item[derivedType] - This category of types represent types derived from a base type. Examples of this category include typedef, pointer and reference types. 
\item[rangedType] - This category represents range types. Examples of this category include the array and the sub-range types. 
\end{description}
The enum, function, common block and scalar types do not fall under any of the above category of types. Each of the specific types is derived from Type.

\subsection{Line Number Interface}

The Line Number interface is responsible for parsing line number information from the object file debug information and handling the look-up and addition of new line information. The main classes for this interface are LineInformation and LineNoTuple. 

\begin{description}
\item[LineInformation] - A LineInformation class object represents a mapping of line numbers to address range within a module (source file). 
\item[Statement/LineNoTuple] - A Statement class object represents a location in source code with a source file, line number in that source file and start column in that line. For backwards compatibility, Statements may also be referred to as LineNoTuples. 
\end{description}

\subsection{Local Variable Interface}
The Local Variable Interface is responsible for parsing local variable and parameter information of functions from the object file debug information and handling the look-up and addition of new add new local variables. All the local variables within a function are tied to the Symbol class object representing that function.
\begin{description}
\item[localVar] - A localVar class object represents a local variable or a parameter belonging to a function.
\end{description}

\subsection{Dynamic Address Translation}
The AddressLookup class is a component for mapping between absolute addresses found in a running process and SymtabAPI objects. This is useful because libraries can load at different addresses in different processes. Each AddressLookup instance is associated with, and provides mapping for, one process.

