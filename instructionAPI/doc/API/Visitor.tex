\subsection{Visitor Paradigm}\label{sec:visitor}

An alternative to the bind/eval mechanism is to use a \emph{visitor}
\footnote{From \emph{Elements of Reusable Object-Oriented Software} by
  Gamma, Helm, Johnson, and Vlissides}
over an expression tree. The visitor concept applies a user-specified
visitor class to all nodes in an expression tree (in a post-order
traversal). The visitor paradigm can be used as a more efficient
replacement for bind/eval, to identify whether an expression has a
desired pattern, or to locate children of an expression tree.

A \code{Visitor} performs postfix-order traversal of the AST represented by
an \code{Expression}.  The \code{Visitor} class specifies the interface from which
users may derive \code{Visitors} that perform specific tasks.

The \code{visit} method must be overridden for each type of \code{Expression} node
(\code{BinaryFunction}, \code{Immediate}, \code{RegisterAST}, and \code{Dereference}).  Any state that
the \code{Visitor} needs to preserve between nodes must be kept within the class.
\code{Visitors} are invoked by calling \code{Expression::apply(Visitor* v);} the \code{visit} method
should not be invoked by user code ordinarily.

A visitor is a user-defined class that inherits from the
\code{Visitor} class defined in \code{Visitor.h}. That class is
repeated here for reference:

\begin{apient}
class Visitor {
  public:
    Visitor() {}
    virtual ~Visitor() {}
    virtual void visit(BinaryFunction* b) = 0;
    virtual void visit(Immediate* i) = 0;
    virtual void visit(RegisterAST* r) = 0;
    virtual void visit(MultiRegisterAST* r) = 0;
    virtual void visit(Dereference* d) = 0;
};
\end{apient}

A user provides implementations of the four \code{visit} methods. When
applied to an \code{Expression} (via the \code{Expression::apply}
method) the InstructionAPI will perform a post-order traversal of the
tree, calling the appropriate \code{visit} method at each node. 

As a simple example, the following code prints out the name of each
register used in an \code{Expression}:

\lstset{language=[GNU]C++,basicstyle=\ttfamily\selectfont\small}
\lstset{numbers=none}
\lstinputlisting{examples/Visitor-regexample.C}

Visitors may also set and use internal state. For example, the
following visitor (presented without surrounding use code) matches x86
and x86-64 instructions that add 0 to a register (effectively a
noop). 

\lstinputlisting{examples/Visitor-noop.C}
