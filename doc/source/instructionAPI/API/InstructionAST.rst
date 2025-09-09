\subsection{InstructionAST Class}
\label{sec:instructionAST}

The InstructionAST class is the base class for all nodes in the ASTs used by the
Operand class. It defines the necessary interfaces for traversing and searching
an abstract syntax tree representing an operand. For the purposes of searching
an InstructionAST, we provide two related interfaces. The first, \code{getUses},
will return the registers that appear in a given tree. The second,
\code{isUsed}, will take as input another tree and return true if that tree is a
(not necessarily proper) subtree of this one. \code{isUsed} requires us to
define an equality relation on these abstract syntax trees, and the equality
operator is provided by the InstructionAST, with the details implemented by the
classes derived from InstructionAST. Two AST nodes are equal if the following
conditions hold:

\begin{itemize}
\item They are of the same type
\item If leaf nodes, they represent the same immediate value or the same register
\item If non-\/leaf nodes, they represent the same operation and their corresponding children are equal 
\end{itemize}

\begin{apient}

typedef boost::shared_ptr<InstructionAST> Ptr
\end{apient}
\apidesc{
    A type definition for a reference-counted pointer to an \code{InstructionAST}.
}

\begin{apient}
bool operator==(const InstructionAST &rhs) const  
\end{apient}
\apidesc{
Compare two AST nodes for equality.

Non-leaf nodes are equal if they are of the same type and their children are
equal. \code{RegisterAST}s are equal if they represent the same register.
\code{Immediate}s
are equal if they represent the same value. Note that it is not safe to compare two \code{InstructionAST::Ptr}
variables, as those are pointers. Instead, test the underlying
\code{InstructionAST} objects.
}

\begin{apient}
virtual void getChildren(vector<InstructionAPI::Ptr> & children) const
\end{apient}
\apidesc{
    Children of this node are appended to the vector \code{children}.
}

\begin{apient}
virtual void getUses(set<InstructionAPI::Ptr> & uses)
\end{apient}
\apidesc{
    The use set of an \code{InstructionAST} is defined as follows:
    \begin{itemize}
    \item A \code{RegisterAST} uses itself
    \item A \code{BinaryFunction} uses the use sets of its children
    \item A \code{Immediate} uses nothing
    \item A \code{Dereference uses the use set of its child}
    \end{itemize}

    The use set oft his node is appended to the vector \code{uses}.
}

\begin{apient}
virtual bool isUsed(InstructionAPI::Ptr findMe) const
\end{apient}
\apidesc{
    Unlike \code{getUses}, \code{isUsed} looks for \code{findMe} as a subtree of
    the current tree. \code{getUses} is designed to return a minimal set of
    registers used in this tree, whereas \code{isUsed} is designed to allow
    searches for arbitrary subexpressions. \code{findMe} is the AST node to find
    in the use set of this node. 

    Returns \code{true} if \code{findMe} is used by this AST node.
}

\begin{apient}
virtual std::string format(formatStyle how == defaultStyle) const
\end{apient}
\apidesc{
    The \code{format} interface returns the contents of an \code{InstructionAPI}
    object as a string. By default, \code{format} produces assembly language.
}
