\subsection{Dereference Class}
\label{sec:dereference}

A \code{Dereference} object is an \code{Expression} that dereferences another
\code{ValueComputation}.

A \code{Dereference} contains an \code{Expression} representing an effective address
computation. Its use set is the same as the use set of the \code{Expression} being
dereferenced.

It is not possible, given the information in a single instruction, to evaluate the
result of a dereference. \code{eval} may still be called on an \code{Expression}
that includes dereferences, but the expected use case is as follows:

\begin{itemize}
\item Determine the address being used in a dereference via the \code{eval} mechanism
\item Perform analysis to determine the contents of that address
\item If necessary, fill in the \code{Dereference} node with the contents of that addresss, using \code{setValue} 
\end{itemize}

The type associated with a \code{Dereference} node will be the type of the value
{\itshape read\/} {\itshape from\/} {\itshape memory\/}, not the type used for
the address computation. Two \code{Dereference}s that access the same address but
interpret the contents of that memory as different types will produce different values.
The children of a \code{Dereference} at a given address are identical,
regardless of the type of dereference being performed at that address. For example,
the \code{Expression} shown in Figure 6 could have its root \code{Dereference}, which
interprets the memory being dereferenced as a unsigned 16-\/bit integer, replaced
with a \code{Dereference} that interprets the memory being dereferenced as any
other type. The remainder of the \code{Expression} tree would, however, remain
unchanged.

\begin{apient}
Dereference (Expression::Ptr addr, Result_Type result_type)
\end{apient}
\apidesc{
A \code{Dereference} is constructed from an \code{Expression} pointer (raw or shared) representing the address
to be dereferenced and a type indicating how the memory at the address in question is to be interpreted.
}

\begin{apient}
virtual void getChildren (vector< InstructionAST::Ptr > & children) const
\end{apient}
\apidesc{
A \code{Dereference} has one child, which represents the address being dereferenced.
Appends the child of this \code{Dereference} to \code{children}.
}

\begin{apient}
virtual void getUses (set< InstructionAST::Ptr > & uses)
\end{apient}
\apidesc{
The use set of a \code{Dereference} is the same as the use set of its children.
The use set of this \code{Dereference} is inserted into \code{uses}.
}

\begin{apient}
virtual bool isUsed (InstructionAST::Ptr findMe) const
\end{apient}
\apidesc{
An \code{InstructionAST} is used by a \code{Dereference} if it is equivalent to the \code{Dereference} or it is used by
the lone child of the \code{Dereference}
}
