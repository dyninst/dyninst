\subsection{Class AST}
\label{sec:ast}
\definedin{DynAST.h}

We provide a generic AST framework to represent tree structures. 
One example use case is to represent instruction semantics with symbolic expressions. 
The AST framework includes the base class definitions for tree nodes and visitors.
Users can inherit tree node classes to create their own AST structure and 
AST visitors to write their own analyses for the AST. 

All AST node classes should be derived from the AST class.  Currently we have the
following types of AST nodes.

\begin{center}
\begin{tabular}{ll}
\toprule
AST::ID  & Meaning \\
\midrule 
    V\_AST & Base class type \\
    V\_BottomAST & Bottom AST node \\
    V\_ConstantAST & Constant AST node \\
    V\_VariableAST & Variable AST node \\
    V\_RoseAST & ROSEOperation AST node \\
    V\_StackAST & Stack AST node \\
\bottomrule
\end{tabular}
\end{center}

\begin{apient}
typedef boost::shared_ptr<AST> Ptr;
\end{apient}
\apidesc{Shared pointer for class AST.}

\begin{apient}
typedef std::vector<AST::Ptr> Children;      
\end{apient}
\apidesc{The container type for the children of this AST.}

\begin{apient}
bool operator==(const AST &rhs) const; 
bool equals(AST::Ptr rhs);
\end{apient}
\apidesc{Check whether two AST nodes are equal. Return \code{true} when two
nodes are in the same type and are equal according to the \code{==} operator of that type.}

\begin{apient}
virtual unsigned numChildren() const; 
\end{apient}
\apidesc{Return the number of children of this node.}

\begin{apient}
virtual AST::Ptr child(unsigned i) const;
\end{apient}
\apidesc{Return the \code{i}th child.}
 
\begin{apient}
virtual const std::string format() const = 0;
\end{apient}
\apidesc{Return the string representation of the node.}

\begin{apient}
static AST::Ptr substitute(AST::Ptr in, AST::Ptr a, AST::Ptr b); 
\end{apient}
\apidesc{Substitute every occurrence of \code{a} with \code{b} in AST \code{in}.
Return a new AST after the substitution.}

\begin{apient} 
virtual AST::ID AST::getID() const;
\end{apient}
\apidesc{Return the class type ID of this node.}

\begin{apient}
virtual Ptr accept(ASTVisitor *v);
\end{apient}
\apidesc{Apply visitor \code{v} to this node. Note that this method will not
automatically apply the visitor to its children.}

\begin{apient}
virtual void AST::setChild(int i, AST::Ptr c);
\end{apient}
\apidesc{Set the \code{i}th child of this node to \code{c}.}
