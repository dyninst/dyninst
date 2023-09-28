\subsection{Iterating over Line Information}\label{subsec:LineNoIterating}
The \code{LineInformation} class also provides the ability for iterating over its data (line numbers and their corresponding address ranges). The following example shows how to iterate over the line information for a given module using SymtabAPI.

\begin{lstlisting}
//Example showing how to iterate over the line information for a given module.
using namespace Dyninst;
using namespace SymtabAPI;

//Obj represents a handle to a parsed object file using symtabAPI
//Module handle for the module
Module *mod;

//Find the module \lq foo\rq within the object.
for(auto *m : findModulesByName("foo")) {

  // Get the Line Information for module foo.
  LineInformation *info = m->getLineInformation();
  
  //Iterate over the line information
  LineInformation::const_iterator iter;
  for( iter = info->begin(); iter != info->end(); iter++)
  {
  // First component represents the address range for the line
  const std::pair<Offset, Offset> addrRange = iter->first;
  
  //Second component gives information about the line itself.
  LineNoTuple lt = iter->second;
  }
}
\end{lstlisting}
