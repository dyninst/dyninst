\section{DynC Language Description}
The DynC language is a subset of C with a \textbf{domain} specification for selecting the location of a resource.

\subsection{Domains}
Domains are special keywords that allow the programmer to precisely indicate
which resource to use.  DynC domains follow the form of
\code{<domain>`<identifier>}, with a back-tick separating the domain and the
identifier. The DynC domains are as follows:

\begin{table}[!th]
\begin{tabular}{ | l | p{12cm} |}
\hline
Domain & Description\\
\hline
\code{inf} & The inferior process (the program being instrumented). Allows access to functions of the mutatee and it's loaded libraries.\\
\hline
\code{dyninst} & Dyninst utility functions. Allows access to context information as well as the \code{break()} function. See Appendix \ref{sec:dyninstdomain}.\\
\hline
\code{local} & A mutatee variable local to function in which the snippet is inserted. \\
\hline
\code{global} & A global mutatee variable. \\
\hline
\code{param} & A parameter of the mutatee function in which the snippet is inserted. \\
\hline
\textit{default} & The default domain (domain not specified) is the domain of snippet variables. \\
\hline

\end{tabular}
\caption{DynC API Domains}
\end{table}

\noindent Example:
\begin{lstlisting}
   inf`printf("n is equal to %d.\n", ++global`n);
\end{lstlisting}
This would increment and print the value of the mutatee global variable n.

\subsection{Control Flow}

\subsubsection{Comments}
Block and line comments work as they do in C or C++.

\noindent Example:
\begin{lstlisting}
   /*
    * This is a comment.
    */
   int i; // So is this.
\end{lstlisting}

\subsubsection{Conditionals}
Use \code{if} to conditionally execute code. 
\noindent Example:
\begin{lstlisting}
   if(x == 0){
      inf`printf("x == 0.\n");
   }
\end{lstlisting}
The \code{else} command can be used to specify code executed if a condition is not true.
\noindent Example:
\begin{lstlisting}
   if(x == 0){
      inf`printf("x == 0.\n");
   }else if(x > 3){
      inf`printf("x > 3.\n");
   }else{
      inf`printf("x < 3 but x }= 0.\n");
   }
\end{lstlisting}

\subsubsection{First-Only Code Block}
\label{sec:firstOnly}
Code enclosed by a pair of \code{{\% <code> \%}} is executed only once by a
snippet. First-only code blocks can be useful for declaring and initilizing
variables, or for any task that needs to be executed only once. Any number of
first-only code blocks can be used in a dynC code snippet.

\noindent A first-only code block is equivalent to the following:
\begin{lstlisting}
   static int firstTime = 0;
   if(firstTime == 0){
     <code>
     firstTime = 1;
   }
\end{lstlisting}

DynC will only execute the code in a first-only section the first time a snippet
is executed. If \code{createSnippet(...)} is called multiple times and is passed
the same name, then the first-only code will be executed only once: the first
time that any of those snippets \underline{with the same name} is executed. In
contrast, if a snippet is created by calling \code{createSnippet(...)} with a
unique snippet name (or if a name is unspecified), the first-only code will be
executed only once upon reaching the first point encountered in the execution of
the mutatee where the returned \code{BPatch\_Snippet} is inserted. 

\noindent Example Touch:
\begin{lstlisting}
   {%
      inf`printf("Function %s has been touched.\n", dyninst`function_name);
   %}
\end{lstlisting}

If \code{createSnippet(...)} is passed the code in Example Touch and the name
\code{"fooTouchSnip"} and the returned \code{BPatch\_snippet} is inserted at the entry to function \code{foo}, the output would be:
\begin{lstlisting}
   Function foo has been touched.
   (mutatee exit)
\end{lstlisting}

If the dynC code in Example Touch is passed to \code{createSnippet(...)} multiple times and each snippet is given the same name, but is inserted at the entries of the functions \code{foo}, \code{bar}, and \code{run} respectively, the output would be:

\begin{lstlisting}
   Function foo has been touched.
   (mutatee exit)
\end{lstlisting}

Creating the snippets with distinct names (e.g. \code{createSnippet(...)} is called with the dynC code in Example Touch multiple times and the snippets are named \code{"fooTouchSnip"}, \code{"barTouchSnip"}, \code{"runTouchSnip"}) would produce an output like:

\begin{lstlisting}
   Function foo has been touched.
   Function bar has been touched.
   Function run has been touched.
   (mutatee exit)
\end{lstlisting}

A cautionary note: the use of first-only blocks can be expensive, as a
conditional must be evaluated each time the snippet is executed. If the option
is available, one may opt to insert a dynC snippet initializing all global
variables at the entry point of \code{main}.

\subsection{Variables}

DynC allows for the creation of \textit{snippet local} variables. These
variables are in scope only within the snippet in which they are created.

\noindent For example,

\begin{lstlisting}
   int i;
   i = 5;
\end{lstlisting}

\noindent would create an uninitialized variable named \code{i} of type integer.
The value of \code{i} is then set to 5.  This is equivalent to:

\begin{lstlisting}
   int i = 5;
\end{lstlisting}

\subsubsection{Static Variables}

Every time a snippet is executed, non-static variables are reinitialized. To create a variable with value that persists across executions of snippets, declare the variable as static. 

\noindent Example: 
\begin{lstlisting}
   int i = 10;
   inf`printf("i is %d.\n", i++);
\end{lstlisting}

\noindent If the above is inserted at the entrance to a function that is called four times, the output would be:

\begin{lstlisting}
i is 10.
i is 10.
i is 10.
i is 10.
\end{lstlisting}

\noindent Adding \code{static} to the variable declaration would make the value of \code{i} persist across executions:
\begin{lstlisting}
   static int i = 10;
   inf`printf("i is %d.\n", i++);
\end{lstlisting}
\noindent Produces:
\begin{lstlisting}
i is 10.
i is 11.
i is 12.
i is 13.
\end{lstlisting}

\noindent A variable declared in a first-only section will also behave statically, as the initialization occurs only once.

\begin{lstlisting}
   {%
      int i = 10;
   %}
\end{lstlisting}

\subsubsection{An Explanation of the Internal Workings of DynC Variable Creation}
\label{sec:varExplain}

DynC uses the DyninstAPI function \code{malloc(...)} to allocate dynC declared
variables when \code{createSnippet(...)} is called. The variable name is mangled
with the name of the snippet passed to createSnippet. Thus, variables declared
in dynC snippets are accessible only to those snippets created by calling
\code{createSnippet(...)} with the same name. 

If the variables are explicitly initialized, dynC sets the value of the variable
with a \code{BPatch\_arithExpr(BPatch\_assign...)} snippet. Because of this, each
time the snippet is executed, the value is reset to the initialized value. If,
however the variables are not explicitly initialized, they are automatically set
to a type-specific zero-value. Scalar variables are set to 0, and c-strings are
set to empty, null-terminated strings (i.e. \code{""}).

If a variable is declared with the \code{static} keyword, then the
initialization is performed as if in a first-only block (see section
\ref{sec:firstOnly}). Thus, a variable is initialized only the first time that
snippet is executed, and subsequent executions of the variable initialization
are ignored. 

\subsubsection{Creating Global Variables That Work With DynC}

To declare a global variable that is accessible to all snippets inserted into a
mutatee, one must use the DyninstAPI \hspace{1pt}
\code{BPatch\_addressSpace::malloc(...)} method (see \underline{Dyninst
Programmer's Guide}). This code is located in mutator code (\emph{not} in dynC
code).


\noindent \textbf{myMutator.C:}
\begin{lstlisting}
   ...
   // Creates a global variable of type in named globalIntN
   myAddressSpace->malloc(myImage->getType("int"), "globalIntN"); 
   
   // file1 and file2 are FILE *, entryPoint and exitPoint are BPatch_point 
   BPatch_snippet *snippet1 = dynC::createSnippet(file1, &entryPoint, "mySnippet1"); 
   BPatch_snippet *snippet2 = dynC::createSnippet(file2, &exitPoint, "mySnippet2");
   
   assert(snippet1);
   assert(snippet2);
   
   myAdressSpace->insertSnippet(snippet1, &entryPoint);
   myAdressSpace->insertSnippet(snippet2, &exitPoint);
   
   // run the mutatee
   ((BPatch_process *)myAdressSpace)->continueExecution();
   ...
\end{lstlisting}
\noindent \textbf{file1:}
\begin{lstlisting}
   {%
      global`globalIntN = 0; // initialize global variable in first-only section
   %}
   inf`printf("Welcome to function %s. Global variable globalIntN = %d.\n", 
        dyninst`function_name, global`globalIntN++);
\end{lstlisting}

\noindent \textbf{file2:}
\begin{lstlisting}
   inf`printf("Goodbye from function %s. Global variable globalIntN = %d.\n", 
        dyninst`function_name, global`globalIntN++);
\end{lstlisting}

\noindent When run, the output from the instrumentation would be:
\begin{lstlisting}
   Welcome to function foo. Global variable globalIntN = 0.
   Goodbye from function foo. Global variable globalIntN = 1.
   Welcome to function foo. Global variable globalIntN = 2.
   Goodbye from function foo. Global variable globalIntN = 3.
   Welcome to function foo. Global variable globalIntN = 4.
   Goodbye from function foo. Global variable globalIntN = 5.
\end{lstlisting}

\subsubsection{Data Types}
\label{dataTypes}
DynC supported data types are restricted by those supported by Dyninst: \code{int}, \code{long}, \code{char *}, and \code{void *}. Integer and c-string primitives are also recognized:\\

\noindent Example:
\begin{lstlisting}
   int i = 12;
   char *s = "hello";
\end{lstlisting}

\subsubsection{Pointers}
Pointers are dereferenced with the prefix \code{*<variable>} and the address of
variable is specified by \code{\&<variable>}.
For example, in reference to the previous example from section \ref{dataTypes}, the statement \code{*s} would evaluate to the character \code{h}.

\subsubsection{Arrays}
Arrays in DynC behave much the same way they do in C. 

\noindent Example:
\begin{lstlisting}
   int array[3] = {1, 2, 3};
   char *names[] = {"Mark", "Phil", "Deb", "Tracy"};
   names[2] = "Gwen" // change Deb to Gwen
   inf`printf("The seventh element of mutArray is %d.\n", global`mutArray[6]); //Mutatee array 
   if(inf`strcmp(*names, "Mark") == 0){} // This will evaluate to true. 
\end{lstlisting}

\subsection{DynC Limitations}
The DynC, while quite expressive, is limited to those actions supported by the DyninstAPI. As such, it lacks certain abilities that many programmers have come to expect. These differences will be discussed in an exploration of those C abilities that dynC lacks.
 
\subsubsection{Loops}
There are no looping structures in DynC.

\subsubsection{Enums, Unions, Structures}
These features present a unique implementation challenge and are in development. Look to future revisions for full support for enums, unions, and structures.

\subsubsection{Preprocessing}
DynC does not allow C-style preprocessing macros or importation. Rather than
\code{\#define} statements, constant variables are recommended.

\subsubsection{Functions}
Specifying functions is beyond the scope of the DynC language. DyninstAPI has methods for dynamically loading code into a mutatee, and these loaded functions can be used in DynC snippets.


