\section{Examples} \label{sec-example}
To illustrate the ideas of PatchAPI, we present some simple code examples that
demonstrate how the API can be used.

\subsection{Using the public interface}
The basic flow of doing code patching is to first find some points in a program,
and then to insert, delete or update a piece of code at these points.
\subsubsection{CFG Traversal}
\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left}
\begin{lstlisting}[caption=Example of CFG traversal]
ParseAPI::CodeObject* co = ...
PatchObject* obj = PatchObject::create(co, code_base);

// Find all functions in the object
std::vector<PatchFunction*> all;
obj->funcs(back_inserter(all));

for (std::vector<PatchFunction*>::iterator fi = all.begin();
     fi != all.end(); fi++) {
  // Print out each function's name
  PatchFunction* func = *fi;
  std::cout << func->name() << std::endl;

  const PatchFunction::Blockset& blks = func->blocks();
  for (PatchFunction::BlockSet::iterator bi = blks.begin();
       bi != blks.end(); bi++) {
    // Print out each block's size
    PatchBlock* blk = *bi;
    std::cout << "\tBlock size:" << blk->size() << std::endl;
  }
}
\end{lstlisting}
In the above code, we illustrate how to traverse CFG structures in
PatchAPI. First, we construct an instance of PatchObject using an instance of
ParseAPI's CodeObject. Then, we traverse all functions in that object, and print
out each function's name. For each function, we also print out the size of each
basic block.

\subsubsection{Point Finding} \label{sec-example-pt}

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left}
\begin{lstlisting}[caption=Example of point finding]
PatchFunction *func = ...;
PatchBlock *block = ...;
PatchEdge *edge = ...;

PatchMgr *mgr = ...;

std::vector<Point*> pts;
mgr->findPoints(Scope(func),
                Point::FuncEntry | 
                Point::PreCall | 
                Point::FuncExit,
                back_inserter(pts));
mgr->findPoints(Scope(block),
                Point::BlockEntry,
                back_inserter(pts));
mgr->findPoints(Scope(edge),
                Point::EdgeDuring,
                back_inserter(pts));
\end{lstlisting}
The above code shows how to use the PatchMgr::findPoints method to find some
instrumentation points. There are three invocations of findPoints. For the first
invocation (Line 8), it finds points only within a specific function
\emph{func}, and output the found points to a vector \emph{pts}. The result
should include all points at this function's entry, before all function calls
inside this function, and at the function's exit. Similarly, for the second
invocation (Line 13), it finds points only within a specific basic \emph{block},
and the result should include the point at the block entry. Finally, for the
third invocation (Line 16), it finds the point at a specific CFG \emph{edge}
that connects two basic blocks.

\subsubsection{Code Patching}

\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left}
\begin{lstlisting}[caption=Example of code patching]
MySnippet::ptr snippet = MySnippet::create(new MySnippet);

Patcher patcher(mgr);
for (vector<Point*>::iterator iter = pts.begin();
     iter != pts.end(); ++iter) {
  Point* pt = *iter;
  patcher.add(PushBackCommand::create(pt, snippet));
}
patcher.commit();
\end{lstlisting}

The above code is to insert the same code \emph{snippet} to all points
\emph{pts} found in Section~\ref{sec-example-pt}. We'll explain the snippet
(Line 1) in the example in Section~\ref{sec-example-snip}. Each point maintains
a list of snippet instances, and the PushBackCommand is to push a snippet
instance to the end of that list. An instance of Patcher is to represent a
transaction of code patching. In this example, all snippet insertions (or all
PushBackCommands) are performed atomically when the Patcher::commit method is
invoked. That is, all snippet insertions would succeed or all would fail.

\subsection{Using the plugin interface}

\subsubsection{Address Space}
\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left}
\begin{lstlisting}[caption=Example of implementing address space plugin]
class MyAddrSpace : public AddrSpace {
  public:
    ...
    virtual Address malloc(PatchObject* obj, size_t size, Address near) {
      Address buffer = ...
      // do memory allocation here
      return buffer;
    }
    virtual bool write(PatchObject* obj, Address to_addr, Address from_addr,
                       size_t size) {
      // copy data from the address from_addr to the address to_addr
      return true;
    }
    ...
};
\end{lstlisting}
The above code is to implement the address space plugin, in which, a set of
memory management methods should be specified, including malloc, free, realloc,
write and so forth. The instrumentation engine will utilize these memory
management methods during the code patching process. For example, the
instrumentation engine needs to \emph{malloc} a buffer in Mutatee's address
space, and then \emph{write} the code snippet into this buffer.

\subsubsection{Snippet Representation} \label{sec-example-snip}
\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left}
\begin{lstlisting}[caption=Example of implementing snippet plugin]
class MySnippet : public Snippet {
  public:
    virtual bool generate(Point *pt, Buffer &buf) {
      // Generate and store binary code in the Buffer buf
      return true;
    }
};
MySnippet::ptr snippet = MySnippet::create(new MySnippet);
\end{lstlisting}

The above code illustrates how to customize a user-defined snippet
\emph{MySnippet} by implementing the ``mini-compiler'' in the {\em generate}
method, which will be used later in the instrumentation engine to generate
binary code.

\subsubsection{Code Parsing}
\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left}
\begin{lstlisting}[caption=Example of customizing CFG parsing]
class MyFunction : public PatchFunction {
  ...
};
class MyCFGMaker : public CFGMaker {
  public:
    ...
    virtual PatchFunction* makeFunction(ParseAPI::Function *f, PatchObject* o) {
      return new MyFunction(f, o);
    }
    ...
};
\end{lstlisting}
Programmers can augment PatchAPI's CFG structures by annotating their own data.
In this case, a factory class should be built by inheriting from the
CFGMaker class, to create the augmented CFG structures. The factory class will
be used for CFG parsing.

\subsubsection{Point Making}
\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left}
\begin{lstlisting}[caption=Example of point making]
class MyPoint : public Point {
  public:
    MyPoint(Point::Type t, PatchMgrPtr m, PatchFunction *f);
    ...
};

class MyPointMaker: public PointMaker {
  protected:
    virtual Point *mkFuncPoint(Point::Type t, PatchMgrPtr m, PatchFunction *f) {
      return new MyPoint(t, m, f);
    }
};
\end{lstlisting}
In the above example, the MyPoint class inherits from the Point class, and the
MyPointMaker class inherits from the PointMaker class. The mkFuncPoint method in
MyPointMaker simply returns a new instance of MyPoint. The mkFuncPoint method
will be invoked by PatchMgr::findPoint(s).


\subsubsection{Instrumentation Engine}
\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left}
\begin{lstlisting}[caption=Example of customizing instrumentation engine]
class MyInstrumenter : public Instrumenter {
  public:
    virtual bool run() {
      // Specify how to install instrumentation
    }
};
\end{lstlisting}
Programmers can customize the instrumentation engine by extending the
Instrumenter class, and implement the installation of instrumentation inside the
method \emph{run()}.

\subsubsection{Plugin Registration}
\lstset{language=[GNU]C++,basicstyle=\fontfamily{fvm}\selectfont\small}
\lstset{numbers=left}
\begin{lstlisting}[caption=Example of registering plugins]
MyCFGMakerPtr cm = ...
PatchObject* obj = PatchObject::create(..., cm);

MyAddrSpacePtr as = ...
as->loadObject(obj);

MyInstrumenter inst = ...
PatchMgrPtr mgr = PatchMgr::create(as, ..., inst);

MySnippet::ptr snippet = MySnippet::create(new MySnippet);
\end{lstlisting}
The above code shows how to register the above four types of plugins.  An
instance of the factory class for creating CFG structures is registered to an
PatchObject (Line 1 and 2), which is in turn loaded into an instance of
AddrSpace (Line 4 and 5). The AddrSpace (or its subclass implemented by
programmers) instance is passed to PatchMgr::create (Line 7 and 8), together
with an instance of Instrumenter (or its subclass). Finally, a snippet of custom
snippet representation MySnippet is created (Line 10). Therefore, all plugins
are glued together in PatchAPI.
