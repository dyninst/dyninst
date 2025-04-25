\section{DynC API}
\subsection{Motivation}
Dyninst is a powerful instrumentation tool, but specifying instrumentation code
(known as an Abstract Syntax Tree) in the \code{BPatch\_snippet} language can be
cumbersome. DynC API answers these concerns, enabling a programmer to easily and
quickly build \code{BPatch\_snippets} using a simple C-like language. Other
advantages to specifying \code{BPatch\_snippets} using dynC include cleaner,
more readable mutator code, automatic variable handling, and runtime-compiled
snippets.

As a motivating example, the following implements a function tracer that
notifies the user when entering and exiting functions, and keeps track of the
number of times each function is called.

\subsubsection{Dyninst API}

When creating a function tracer using the Dyninst API, the programmer must
perform many discrete lookups and create many \code{BPatch\_snippet objects},
which are then combined and inserted into the mutatee.

\noindent Look up \code{printf}:

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left, numberstyle=\tiny, stepnumber=5, numbersep=5pt}
\lstset{showstringspaces=false}
\begin{lstlisting}
  std::vector<BPatch_function *> *printf_func;
  appImage->findFunction("printf", printf_func);
  BPatch_function *BPF_printf = printf_func[0];
\end{lstlisting}

\noindent Create each \code{printf} pattern:
\begin{lstlisting}
  BPatch_constExpr entryPattern("Entering %s, called %d times.\n");
  BPatch_constExpr exitPattern("Exiting %s.\n");
\end{lstlisting}

\noindent \textbf{For each function, do the following:}

Create snippet vectors:
\begin{lstlisting}
     std::vector<BPatch_snippet *> entrySnippetVect;
     std::vector<BPatch_snippet *> exitSnippetVect;
\end{lstlisting}

Create the \code{intCounter} global variable:
\begin{lstlisting}
     appProc->malloc(appImage->findType("int"), std::string("intCounter"));
\end{lstlisting}

Get the name of the function:
\begin{lstlisting}
     char fName[128];
     BPatch_constExpr funcName(functions[i]->getName(fName, 128));
\end{lstlisting}

Build the entry \code{printf}:
\begin{lstlisting}
     std::vector<BPatch_snippet *> entryArgs;
     entryArgs.push_back(entryPattern);
     entryArgs.push_back(funcName);
     entryArgs.push_back(intCounter);
\end{lstlisting}

Build the exit \code{printf}:
\begin{lstlisting}
     std::vector<BPatch_snippet *> exitArgs;
     exitArgs.push_back(exitPattern);
     exitArgs.push_back(funcName);
\end{lstlisting}

Add \code{printf} to the snippet:
\begin{lstlisting}
     entrySnippetVect.push_back(BPatch_functionCallExpr(*printf_func, entryArgs));
     exitSnippetVect.push_back(BPatch_functionCallExpr(*printf_func, exitArgs));
\end{lstlisting}

Increment the counter:
\begin{lstlisting}
     BPatch_arithExpr addOne(BPatch_assign, *intCounter, 
            BPatch_arithExpr(BPatch_plus, *intCounter, BPatch_constExpr(1)));
\end{lstlisting}

Add increment to the entry snippet:
\begin{lstlisting}
     entrySnippetVect.push_back(&addOne);
\end{lstlisting}

Insert the snippets:
\begin{lstlisting}
     appProc->insertSnippet(*entrySnippetVect, functions[i]->findPoint(BPatch_entry));
     appProc->insertSnippet(*exitSnippetVect, functions[i]->findPoint(BPatch_exit));
\end{lstlisting}

\begin{center} 
%\rule{6.5in}{1pt}
\end{center}

\begin{comment}
  // find points
  std::vector<BPatch_point *> *entryPoints = functions[i]->findPoint(BPatch_entry);
  std::vector<BPatch_point *> *exitPoints = functions[i]->findPoint(BPatch_exit);

  // insert snippets
  appProc->insertSnippet(BPatch_sequence(entrySnippetVect), entryPoints);
  appProc->insertSnippet(BPatch_sequence(exitSnippetVect), exitPoints);
}

  // run mutatee
  appProc->continueExecution();
\end{comment}

\pagebreak


\subsubsection{DynC API}

A function tracer is much easier to build in DynC API, especially if reading dynC code from file. Storing dynC code in external files not only cleans up mutator code, but also allows the programmer to modify snippets without recompiling.

\vspace{0.5cm}

%\begin{center} 
%\rule{6.5in}{1pt}
%\end{center}

\noindent In this example, the files \code{myEntryDynC.txt} and \code{myExitDynC.txt} contain dynC code:

%\lstset{frame=single}
\begin{lstlisting}
  // myEntryDynC.txt
  static int intCounter;
  printf("Entering %s, called %d times.\n", dyninst`function_name, intCounter++);
\end{lstlisting}

\begin{lstlisting}
  // myExitDynC.txt
  printf("Leaving %s.\n", dyninst`function_name);
\end{lstlisting}

%\lstset{frame=none}

\noindent The code to read, build, and insert the snippets would look something like the following:

\noindent First open files:
\begin{lstlisting}
  FILE *entryFile = fopen("myEntryDynC.txt", "r");
  FILE *exitFile = fopen("myExitDynC.txt", "r");
\end{lstlisting}

\noindent Next call DynC API with each function's entry and exit points:
\begin{lstlisting}
  BPatch_snippet *entrySnippet = 
       dynC_API::createSnippet(entryFile, entryPoint, "entrySnippet");
  BPatch_snippet *exitSnippet = 
       dynC_API::createSnippet(exitFile, exitPoint, "exitSnippet");
\end{lstlisting}

\noindent Finally insert the snippets at each function's entry and exit points:
\begin{lstlisting}
  appProc->insertSnippet(*entrySnippet, entryPoint);
  appProc->insertSnippet(*exitSnippet, exitPoint);
\end{lstlisting}

\begin{comment}
\begin{lstlisting}
/*** Create Snippet ***/
// build entryString
std::stringstream entryString;
entryString << "static int intCounter;" << endl;
entryString << "inf`printf(\"Entering %s, which has been called %d times.\\n\"";
entryString << ",dyninst`function_name, intCounter);";

// call to DynC API
BPatch_snippet *entrySnippet = 
       dynC_API::createSnippet(entryString.str().c_str(), entryPoint, "entrySnippet");

// build exitString
std::stringstream exitString;
exitString << "inf`printf(\"Exiting %s.\\n\", dyninst`function_name);";

// call to DynC API
BPatch_snippet *exitSnippet = 
       dynC_API::createSnippet(exitString.str().c_str(), app, "exitSnippet");
/*** Finish Snippet ***/

// find all entry and exit points
std::vector<BPatch_point *> * entry_points = (*functions)[0]->findPoint(BPatch_entry);
std::vector<BPatch_point *> * exit_points = (*functions)[0]->findPoint(BPatch_exit);

for(unsigned int i = 1; i < functions->size(); i++){
  entry_points->push_back((*(*functions)[i]->findPoint(BPatch_entry))[0]);
  exit_points->push_back((*(*functions)[i]->findPoint(BPatch_exit))[0]);
}

// insert Snippets
appProc->insertSnippet(*entrySnippet, entry_points);
appProc->insertSnippet(*exitSnippet, exit_points);
  
//run mutatee
appProc->continueExecution();

\end{lstlisting}
\end{comment}

%\begin{center} 
%\noindent \rule{6.5in}{1pt}
%\end{center}

\subsection{Calling DynC API}
All DynC functions reside in the \code{dynC\_API} namespace. The primary DynC API function is:
\begin{lstlisting}
   BPatch_Snippet *createSnippet(<dynC code>, <location>, char * name);
\end{lstlisting}
where \code{<dynC code>} can be either a constant c-style string or a file
descriptor and \code{<location>} can take the form of a \code{BPatch\_point} or
a \code{BPatch\_addressSpace}. There is also an optional parameter to name a
snippet. A snippet name makes code and error reporting much easier to read, and
allows for the grouping of snippets (see section \ref{sec:varExplain}). If a
snippet name is not specified, the default name \code{Snippet\_[<\#>]} is used.
\\

\begin{centering}

\begin{table}[!th]
\begin{tabular}{|l|p{11cm}|}
\hline
\code{<dynC code>} & Description\\
\hline
\code{std::string str} & A C++ string containing dynC code.\\
\hline
\code{const char *s} & A null terminated string containing dynC code\\
\hline 
\code{FILE *f} & A standard C file descriptor. Facilitates reading dynC code from file.\\
\hline

\end{tabular}
\caption{\code{createSnippet(...)} input options: dynC code}
\end{table}

\begin{table}[!th]
\begin{tabular}{|l|p{8cm}|}
\hline
\code{<location>} & Description\\
\hline
\code{BPatch\_point \&point} & Creates a snippet specific to a single point.\\
\hline
\code{BPatch\_addressSpace \&addSpace} & Creates a more flexible snippet
specific to an address space. See Section \ref{sec:nopoint}.\\
\hline
\end{tabular}
\caption{\code{createSnippet(...)} input options: location}
\end{table}

\end{centering}

The location parameter is the point or address space in which the snippet will be inserted. Inserting a snippet created for one location into another can cause undefined behavior.

\subsection{Creating Snippets Without Point Information}
\label{sec:nopoint}

Creating a snippet without point information (i.e., calling
\code{createSnippet(...)} with a \code{BPatch\_addressSpace}) results in a far
more flexible snippet that may be inserted at any point in the specified address
space. There are, however, a few restrictions on the types of operations that
may be performed by a flexible snippet. No local variables may be accessed,
including parameters and return values. Mutatee variables must be accessed
through the \code{global} domain.

