\section{Plugin API Reference}
\label{sec-plugin-api}

This section describes the various plugin interfaces for extending PatchAPI. We
expect that most users should not have to ever explicitly use an interface from
this section; instead, they will use plugins previously implemented by PatchAPI
developers.

As with the public interface, all objects and methods in this section are in the
``Dyninst::PatchAPI'' namespace.

\subsection{AddrSpace}
\label{sec-3.2.1}

\textbf{Declared in}: AddrSpace.h

The AddrSpace class represents the address space of a \textbf{Mutatee}, where it
contains a collection of \textbf{PatchObjects} that represent shared libraries
or a binary executable. In addition, programmers implement some memory
management interfaces in the AddrSpace class to determine the type of the code
patching - 1st party, 3rd party, or binary rewriting.


\begin{apient}
virtual bool write(PatchObject* obj, Address to, Address from, size_t size);
\end{apient}


\apidesc{

This method copies \emph{size}-byte data stored at the address \emph{from} on
the \textbf{Mutator} side to the address \emph{to} on the \textbf{Mutatee}
side. The parameter \emph{to} is the relative offset for the PatchObject
\emph{obj}, if the instrumentation is for binary rewriting; otherwise \emph{to}
is an absolute address.

If the write operation succeeds, this method returns true; otherwise, false.
}

\begin{apient}
virtual Address malloc(PatchObject* obj, size_t size, Address near);
\end{apient}


\apidesc{

This method allocates a buffer of \emph{size} bytes on the \textbf{Mutatee}
side. The address \emph{near} is a relative address in the object \emph{obj}, if
the instrumentation is for binary rewriting; otherwise, \emph{near} is an
absolute address, where this method tries to allocate a buffer near the address
\emph{near}.

If this method succeeds, it returns a non-zero address; otherwise, it returns 0.
}

\begin{apient}
virtual Address realloc(PatchObject* obj, Address orig, size_t size);
\end{apient}


\apidesc{
This method reallocates a buffer of \emph{size} bytes on the \textbf{Mutatee} side. The
original buffer is at the address \emph{orig}. This method tries to reallocate the
buffer near the address \emph{orig}, where \emph{orig} is a relative address in the
PatchObject \emph{obj} if the instrumentation is for binary rewriting; otherwise,
\emph{orig} is an absolute address.

If this method succeeds, it returns a non-zero address; otherwise, it returns 0.
}

\begin{apient}
virtual bool free(PatchObject* obj, Address orig);
\end{apient}


\apidesc{
This method deallocates a buffer on the \textbf{Mutatee} side at the address \emph{orig}.
If the instrumentation is for binary rewriting, then the parameter \emph{orig} is a
relative address in the object \emph{obj}; otherwise, \emph{orig} is an absolute address.

If this method succeeds, it returns true; otherwise, it returns false.
}

\begin{apient}
virtual bool loadObject(PatchObject* obj);
\end{apient}


\apidesc{
This method loads a PatchObject into the address space. If this method succeeds, it
returns true; otherwise, it returns false.
}

\begin{apient}
typedef std::map<const ParseAPI::CodeObject*, PatchObject*> AddrSpace::ObjMap;

ObjMap& objMap();
\end{apient}

\apidesc{

Returns a set of mappings from ParseAPI::CodeObjects to PatchObjects, where
PatchObjects in all mappings represent all binary objects (either executable or
libraries loaded) in this address space.

}

\begin{apient}
PatchObject* executable();
\end{apient}


\apidesc{
Returns the PatchObject of the executable of the \textbf{Mutatee}.
}

\begin{apient}
PatchMgrPtr mgr();
\end{apient}


\apidesc{
Returns the PatchMgr's pointer, where the PatchMgr contains this address space.
}

\subsection{Snippet}
\label{sec-3.2.2}

\textbf{Declared in}: Snippet.h

The Snippet class allows programmers to customize their own snippet
representation and the corresponding mini-compiler to translate the
representation into the binary code.


\begin{apient}
static Ptr create(Snippet* a);
\end{apient}


\apidesc{
Creates an object of the Snippet.
}

\begin{apient}
virtual bool generate(Point *pt, Buffer &buf);
\end{apient}


\apidesc{

Users should implement this virtual function for generating binary code for
the snippet.

Returns false if code generation failed catastrophically. Point {\em pt} is an
in-param that identifies where the snippet is being generated.  Buffer {\em buf}
is an out-param that holds the generated code.  }

\subsection{Command}
\label{sec-3.2.3}

\textbf{Declared in}: Command.h

The Command class represents an instrumentation request (e.g., snippet insertion
or removal), or an internal logical step in the code patching (e.g., install
instrumentation).


\begin{apient}
virtual bool run() = 0;
\end{apient}


\apidesc{
Executes the normal operation of this Command.

It returns true on success; otherwise, it returns false.
}

\begin{apient}
virtual bool undo() = 0;
\end{apient}


\apidesc{
Undoes the operation of this Command.
}

\begin{apient}
virtual bool commit();
\end{apient}


\apidesc{
Implements the transactional semantics: all succeed, or all fail. Basically, it
performs such logic:
}

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=none}
\begin{lstlisting}
if (run()) {
  return true;
} else {
  undo();
  return false;
}
\end{lstlisting}

\subsection{BatchCommand}
\label{sec-3.2.4}

\textbf{Declared in}: Command.h

The BatchCommand class inherits from the Command class. It is actually a
container of a list of Commands that will be executed in a transaction: all
Commands will succeed, or all will fail.


\begin{apient}
typedef std::list<CommandPtr> CommandList;

CommandList to_do_;
CommandList done_;
\end{apient}


\apidesc{
This class has two protected members \emph{to\_do\_} and \emph{done\_}, where \emph{to\_do\_}
is a list of Commands to execute, and \emph{done\_} is a list of Commands that are
executed.
}

\begin{apient}
virtual bool run();
virtual bool undo();
\end{apient}

\apidesc{
The method run() of BatchCommand invokes the run() method of each Command in
\emph{to\_do\_} in order, and puts the finished Commands in \emph{done\_}. The method
undo() of BatchCommand invokes the undo() method of each Command in \emph{done \_} in
order.
}

\begin{apient}
void add(CommandPtr command);
\end{apient}


\apidesc{
This method adds a Command into \emph{to\_do\_}.
}

\begin{apient}
void remove(CommandList::iterator iter);
\end{apient}

\apidesc{
This method removes a Command from \emph{to\_do\_}.
}

\subsection{Instrumenter}
\label{sec-3.2.5}

\textbf{Declared in}: Command.h

The Instrumenter class inherits BatchCommand to encapsulate the core code
patching logic, which includes binary code generation. Instrumenter would
contain several logical steps that are individual Commands.


\begin{apient}
CommandList user_commands_;
\end{apient}


\apidesc{
This class has a protected data member \emph{user\_commands\_} that contains all
Commands issued by users, e.g., snippet insertion. This is to facilitate the
implementation of the instrumentation engine.
}

\begin{apient}
static InstrumenterPtr create(AddrSpacePtr as);
\end{apient}

\apidesc{
Returns an instance of Instrumenter, and it takes input the address space \emph{as}
that is going to be instrumented.
}

\begin{apient}
virtual bool replaceFunction(PatchFunction* oldfunc, PatchFunction* newfunc);
\end{apient}


\apidesc{
Replaces a function \emph{oldfunc} with a new function \emph{newfunc}.

It returns true on success; otherwise, it returns false.
}

\begin{apient}
virtual bool revertReplacedFunction(PatchFunction* oldfunc);
\end{apient}

\apidesc{
Undoes the function replacement for \emph{oldfunc}.

It returns true on success; otherwise, it returns false.
}

\begin{apient}
typedef std::map<PatchFunction*, PatchFunction*> FuncModMap;
\end{apient}

\apidesc{
The type FuncModMap contains mappings from an PatchFunction to another
PatchFunction.
}

\begin{apient}
virtual FuncModMap& funcRepMap();
\end{apient}

\apidesc{
Returns the FuncModMap that contains a set of mappings from an old function to a
new function, where the old function is replaced by the new function.
}

\begin{apient}
virtual bool wrapFunction(PatchFunction* oldfunc, PatchFunction* newfunc, string name);
\end{apient}


\apidesc{
  Replaces all calls to \emph{oldfunc} with calls to wrapper \emph{newfunc}
  (similar to function replacement). However, we create a copy of original using
  the {\em name} that can be used to call the original. The wrapper code would
  look like follows:
}

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=none}
\begin{lstlisting}
void *malloc_wrapper(int size) {
  // do stuff
  void *ret = malloc_clone(size);
  // do more stuff
  return ret;
}
\end{lstlisting}


\apidesc{
This interface requires the user to give us a name (as represented by clone) for
the original function. This matches current techniques and allows users to use
indirect calls (function pointers).
}

\begin{apient}
virtual bool revertWrappedFunction(PatchFunction* oldfunc);
\end{apient}

\apidesc{
Undoes the function wrapping for \emph{oldfunc}.

It returns true on success; otherwise, it returns false.
}

\begin{apient}
virtual FuncModMap& funcWrapMap();

\end{apient}


\apidesc{
The type FuncModMap contains mappings from the original PatchFunction to the
wrapper PatchFunction.
}

\begin{apient}
bool modifyCall(PatchBlock *callBlock, PatchFunction *newCallee,
                PatchFunction *context = NULL);
\end{apient}


\apidesc{
Replaces the function that is invoked in the basic block \emph{callBlock} with the
function \emph{newCallee}. There may be multiple functions containing the same
\emph{callBlock}, so the \emph{context} parameter specifies in which function the
\emph{callBlock} should be modified. If \emph{context} is NULL, then the \emph{callBlock} would
be modified in all PatchFunctions that contain it. If the \emph{newCallee} is NULL,
then the \emph{callBlock} is removed.

It returns true on success; otherwise, it returns false.
}

\begin{apient}
bool revertModifiedCall(PatchBlock *callBlock, PatchFunction *context = NULL);
\end{apient}


\apidesc{
Undoes the function call modification for \emph{oldfunc}. There may be multiple
functions containing the same \emph{callBlock}, so the \emph{context} parameter specifies
in which function the \emph{callBlock} should be modified. If \emph{context} is NULL, then
the \emph{callBlock} would be modified in all PatchFunctions that contain it.

It returns true on success; otherwise, it returns false.
}

\begin{apient}
bool removeCall(PatchBlock *callBlock, PatchFunction *context = NULL);
\end{apient}


\apidesc{
Removes the \emph{callBlock}, where a function is invoked. There may be multiple
functions containing the same \emph{callBlock}, so the \emph{context} parameter specifies
in which function the \emph{callBlock} should be modified. If \emph{context} is NULL, then
the \emph{callBlock} would be modified in all PatchFunctions that contain it.

It returns true on success; otherwise, it returns false.
}


\begin{apient}
typedef map<PatchBlock*,        // B  : A call block
            map<PatchFunction*, // F_c: Function context
                PatchFunction*> // F  : The function to be replaced
           > CallModMap;
\end{apient}


\apidesc{

The type CallModMap maps from B -> F$_c$ -> F, where B identifies a call block,
and F$_c$ identifies an (optional) function context for the replacement. If F$_c$ is
not specified, we use NULL. F specifies the replacement callee; if we want to
remove the call entirely, we use NULL.
}

\begin{apient}
CallModMap& callModMap();
\end{apient}


\apidesc{
Returns the CallModMap for function call replacement / removal.
}

\begin{apient}
AddrSpacePtr as() const;
\end{apient}


\apidesc{
Returns the address space associated with this Instrumenter.
}

\subsection{Patcher}
\label{sec-3.2.6}

\textbf{Declared in}: Command.h

The class Patcher inherits from the class BatchCommand. It accepts
instrumentation requests from users, where these instrumentation requests are
Commands (e.g., snippet insertion). Furthermore, Patcher implicitly adds an
instance of Instrumenter to the end of the Command list to generate binary code
and install the instrumentation.


\begin{apient}
Patcher(PatchMgrPtr mgr)
\end{apient}


\apidesc{
The constructor of Patcher takes input the relevant PatchMgr \emph{mgr}.
}

\begin{apient}
virtual bool run();
\end{apient}


\apidesc{
Performs the same logic as BatchCommand::run(), except that this function
implicitly adds an internal Command -- Instrumenter, which is executed after all
other Commands in the \emph{to\_do\_}.
}

\subsection{CFGMaker}
\label{sec-3.2.12}

\textbf{Declared in}: CFGMaker.h

The CFGMaker class is a factory class that constructs the above CFG structures
(PatchFunction, PatchBlock, and PatchEdge). The methods in this class are used
by PatchObject. Programmers can extend PatchFunction, PatchBlock and PatchEdge
by annotating their own data, and then use this class to instantiate these CFG
structures.


\begin{apient}
virtual PatchFunction* makeFunction(ParseAPI::Function* func, PatchObject* obj);
virtual PatchFunction* copyFunction(PatchFunction* func, PatchObject* obj);

virtual PatchBlock* makeBlock(ParseAPI::Block* blk, PatchObject* obj);
virtual PatchBlock* copyBlock(PatchBlock* blk, PatchObject* obj);

virtual PatchEdge* makeEdge(ParseAPI::Edge* edge, PatchBlock* src,
                            PatchBlock* trg, PatchObject* obj);
virtual PatchEdge* copyEdge(PatchEdge* edge, PatchObject* obj);
\end{apient}

\apidesc{
Programmers implement the above virtual methods to instantiate a CFG structure
(either a PatchFunction, a PatchBlock, or a PatchEdge) or to copy (e.g., when
forking a new process).
}

\subsection{PointMaker}
\label{sec-3.2.13}

\textbf{Declared in}: Point.h

The PointMaker class is a factory class that constructs instances of the Point
class. The methods of the PointMaker class are invoked by PatchMgr's findPoint
methods. Programmers can extend the Point class, and then implement a set of
virtual methods in this class to instantiate the subclasses of Point.


\begin{apient}
PointMaker(PatchMgrPtr mgr);
\end{apient}

\apidesc{
The constructor takes input the relevant PatchMgr \emph{mgr}.
}

\begin{apient}
virtual Point *mkFuncPoint(Point::Type t, PatchMgrPtr m, PatchFunction *f);
virtual Point *mkFuncSitePoint(Point::Type t, PatchMgrPtr m, PatchFunction *f, PatchBlock *b);
virtual Point *mkBlockPoint(Point::Type t, PatchMgrPtr m, PatchBlock *b, PatchFunction *context);
virtual Point *mkInsnPoint(Point::Type t, PatchMgrPtr m, PatchBlock *, Address a,
                           InstructionAPI::Instruction::Ptr i, PatchFunction *context);
virtual Point *mkEdgePoint(Point::Type t, PatchMgrPtr m, PatchEdge *e, PatchFunction *context);
\end{apient}

\apidesc{

Programmers implement the above virtual methods to instantiate the subclasses of
Point.

}

\subsection{Default Plugin}
\label{sec-3.3}

\subsection{PushFrontCommand and PushBackCommand}
\label{sec-3.3.1}

\textbf{Declared in}: Command.h

The class PushFrontCommand and the class PushBackCommand inherit from the
Command class. They are to insert a snippet to a point. A point maintains a
list of snippet instances. PushFrontCommand would add the new snippet instance
to the front of the list, while PushBackCommand would add to the end of the
list.


\begin{apient}
static Ptr create(Point* pt, SnippetPtr snip);
\end{apient}

\apidesc{
This static method creates an object of PushFrontCommand or PushBackCommand.
}

\begin{apient}
InstancePtr instance();
\end{apient}

\apidesc{
Returns a snippet instance that is inserted at the point.
}

\subsection{RemoveSnippetCommand}
\label{sec-3.3.2}

\textbf{Declared in}: Command.h

The class RemoveSnippetCommand inherits from the Command class. It is to delete
a snippet Instance.


\begin{apient}
static Ptr create(InstancePtr instance);
\end{apient}

\apidesc{
This static function creates an instance of RemoveSnippetCommand.
}

\subsection{RemoveCallCommand}
\label{sec-3.3.3}

\textbf{Declared in}: Command.h

The class RemoveCallCommand inherits from the class Command. It is to remove a
function call.


\begin{apient}
static Ptr create(PatchMgrPtr mgr, PatchBlock* call_block, PatchFunction* context = NULL);
\end{apient}


\apidesc{
This static method takes input the relevant PatchMgr \emph{mgr}, the \emph{call\_block}
that contains the function call to be removed, and the PatchFunction \emph{context}.
There may be multiple PatchFunctions containing the same \emph{call\_block}. If the
\emph{context} is NULL, then the \emph{call\_block} would be deleted from all
PatchFunctions that contains it; otherwise, the \emph{call\_block} would be deleted
only from the PatchFuncton \emph{context}.
}

\subsection{ReplaceCallCommand}
\label{sec-3.3.4}

\textbf{Declared in}: Command.h

The class ReplaceCallCommand inherits from the class Command. It is to replace a
function call with another function.


\begin{apient}
static Ptr create(PatchMgrPtr mgr, PatchBlock* call_block,
                  PatchFunction* new_callee, PatchFunction* context);
\end{apient}

\apidesc{
This Command replaces the \emph{call\_block} with the new PatchFunction
\emph{new\_callee}. There may be multiple functions containing the same
\emph{call\_block}, so the \emph{context} parameter specifies in which function the
\emph{call\_block} should be replaced. If \emph{context} is NULL, then the \emph{call\_block}
would be replaced in all PatchFunctions that contains it.
}

\subsection{ReplaceFuncCommand}
\label{sec-3.3.5}

\textbf{Declared in}: Command.h

The class ReplaceFuncCommand inherits from the class Command. It is to replace
an old function with the new one.


\begin{apient}
static Ptr create(PatchMgrPtr mgr, PatchFunction* old_func, PatchFunction* new_func);
\end{apient}

\apidesc{
This Command replaces the old PatchFunction \emph{old\_func} with the new
PatchFunction \emph{new\_func}.
}

