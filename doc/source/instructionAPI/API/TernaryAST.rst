\subsection{TernaryAST Class}
\label{sec:TernaryAST}

THIS CLASS IS ONLY PARTIALLY IMPLEMENTED.


A \code{TernaryAST} object represents the value of a ternary assignment. As a \code{TernaryAST}
is an \code{Expression}, it may contain the physical register's contents if they
are known.

\begin{apient}
  typedef dyn\_detail::boost::shared\_ptr<TernaryAST> Ptr
\end{apient}
\apidesc{ A type definition for a reference-counted pointer to a \code{TernaryAST}. }

\begin{apient}
  TernaryAST(Expression::Ptr cond , Expression::Ptr first , Expression::Ptr second, Result_Type result_type);
\end{apient}
\apidesc{
  Construct a register, assigning it the ID id.
}

\begin{apient}
  void getChildren (vector< InstructionAST::Ptr > & children) const
\end{apient}
\apidesc{
  By definition, a TernaryAST has three children, the condition, the first and the second child.
  and should all be added to the children.
}

\begin{apient}
  void getUses (set< InstructionAST::Ptr > & uses)
\end{apient}
\apidesc{ By definition, add the use of all its children to.. }

\begin{apient}
  bool isUsed (InstructionAST::Ptr findMe) const
\end{apient}
\apidesc{
  Checks if findMe is a TernaryAST that represents the same register as this TernaryAST.
}

\begin{apient}
  std::string format (formatStyle how = defaultStyle) const
\end{apient}
\apidesc{ The format method on a \code{TernaryAST} object returns the name associated with its ID. }

\begin{apient}
  TernaryAST makePC (Dyninst::Architecture arch) [static]
\end{apient}
\apidesc{ Utility function to get a \code{Register} object that represents the program counter. \code{makePC} 
is provided to support platform-independent control flow analysis. }

\begin{apient}
  bool operator< (const TernaryAST & rhs) const
\end{apient}
\apidesc{ We define a partial ordering on registers by their register number so that they may be placed into 
sets or other sorted containers. }

\begin{apient}
  MachRegister getID () const
\end{apient}
\apidesc{ The \code{getID} function returns underlying register represented by this AST. }

\begin{apient}
  TernaryAST::Ptr promote (const InstructionAST::Ptr reg) [static]
\end{apient}
\apidesc{ Utility function to hide aliasing complexity on platforms (IA-32) that allow addressing part 
or all of a register }

