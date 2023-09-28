\section{Simple Examples}

To illustrate the ideas in the API, this section presents several short examples
that demonstrate how the API can be used.  SymtabAPI has the ability to parse
files that are on-disk or present in memory. The user program starts by
requesting SymtabAPI to parse an object file. SymtabAPI returns a handle if the
parsing succeeds, whcih can be used for further interactions with the SymtabAPI
library. The following example shows how to parse a shared object file on disk.

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left, numberstyle=\tiny, stepnumber=5, numbersep=5pt}
\begin{lstlisting}
using namespace Dyninst;
using namespace SymtabAPI;

//Name the object file to be parsed:
std::string file = "libfoo.so";

//Declare a pointer to an object of type Symtab; this represents the file.
Symtab *obj = NULL;

// Parse the object file
bool err = Symtab::openFile(obj, file);
\end{lstlisting}

Once the object file is parsed successfully and the handle is obtained, symbol look up and update operations can be performed in the following way:

\begin{lstlisting}
using namespace Dyninst;
using namespace SymtabAPI;
std::vector <Symbol *> syms;
std::vector <Function *> funcs;

// search for a function with demangled (pretty) name "bar".
if (obj->findFunctionsByName(funcs, "bar")) {
       // Add a new (mangled) primary name to the first function
       funcs[0]->addMangledName("newname", true);
}

// search for symbol of any type with demangled (pretty) name "bar".
if (obj->findSymbol(syms, "bar", Symbol::ST_UNKNOWN)) { 

    // change the type of the found symbol to type variable(ST_OBJECT)
    syms[0]->setType(Symbol::ST_OBJECT);

    // These changes are automatically added to symtabAPI; no further
    // actions are required by the user.
}
\end{lstlisting}

\newpage

New symbols, functions, and variables can be created and added to the library at any point using the handle returned by successful parsing of the object file. When possible, add a function or variable rather than a symbol directly. 

\begin{lstlisting}
using namespace Dyninst;
using namespace SymtabAPI;

// obj represents a handle to a parsed object file.
for(auto *m : obj->findModulesByName("/path/to/foo.c")) {

  // Create a new function symbol
  Variable *newVar = m->createVariable("newIntVar",  // Name of new variable
                                       0x12345,      // Offset from data section
                                       sizeof(int)); // Size of symbol 
}
\end{lstlisting}

SymtabAPI gives the ability to query type information present in the object file. Also, new user defined types can be added to SymtabAPI. The following example shows both how to query type information after an object file is successfully parsed and also add a new structure type.

\begin{lstlisting}
// create a new struct Type
// typedef struct{
//int field1,
//int field2[10]
// } struct1;

using namespace Dyninst;
using namespace SymtabAPI;

// Find a handle to the integer type; obj represents a handle to a parsed object file
Type *lookupType;
obj->findType(lookupType, "int");

// Convert the generic type object to the specific scalar type object
typeScalar *intType = lookupType->getScalarType();

// container to hold names and types of the new structure type
vector<pair<string, Type *> >fields;

//create a new array type(int type2[10])
typeArray *intArray = typeArray::create("intArray",intType,0,9, obj);

//types of the structure fields
fields.push_back(pair<string, Type *>("field1", intType));
fields.push_back(pair<string, Type *>("field2", intArray));

//create the structure type
typeStruct *struct1 = typeStruct::create("struct1", fields, obj);
\end{lstlisting}

Users can also query line number information present in an object file. The following example shows how to use SymtabAPI to get the address range for a line number within a source file.

\begin{lstlisting}
using namespace Dyninst;
using namespace SymtabAPI;

// obj represents a handle to a parsed object file using symtabAPI
// Container to hold the address range
vector< pair< Offset, Offset > > ranges;

// Get the address range for the line 30 in source file foo.c
obj->getAddressRanges(ranges, "foo.c", 30);
\end{lstlisting}

Local variable information can be obtained using symtabAPI. You can query for a local variable within the entire object file or just within a function. The following example shows how to find local variable foo within function bar.

\begin{lstlisting}
using namespace Dyninst;
using namespace SymtabAPI;

// Obj represents a handle to a parsed object file using symtabAPI
// Get the Symbol object representing function bar
vector<Symbol *> syms;
obj->findSymbol(syms, "bar", Symbol::ST_FUNCTION);

// Find the local var foo within function bar
vector<localVar *> *vars = syms[0]->findLocalVarible("foo");
\end{lstlisting}

The rest of this document describes the class hierarchy and the API in detail.
