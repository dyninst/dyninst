\subsection{BinaryFunction Class}
\label{sec:binaryFunction}

A \code{BinaryFunction} object represents a function that can combine two \code{Expression}s
and produce another \code{ValueComputation}.

For the purposes of representing a single operand of an instruction, the
\code{BinaryFunction}s of interest are addition and multiplication of integer values;
this allows an \code{Expression} to represent all addressing modes on the architectures
currently supported by the Instruction API. 

\begin{apient}
BinaryFunction(Expression::Ptr arg1,
               Expression::Ptr arg2,
               Result_Type result_type,
               funcT:Ptr func)
\end{apient}
\apidesc{
The constructor for a \code{BinaryFunction} may take a reference-\/counted pointer or a plain C++ pointer
to each of the child \code{Expression}s that represent its arguments. Since the reference-\/counted
implementation requires explicit construction, we provide overloads for all four combinations of plain
and reference-\/counted pointers. Note that regardless of which constructor is used, the pointers
\code{arg1} and \code{arg2} become owned by the \code{BinaryFunction} being constructed, and should
not be deleted. They will be cleaned up when the \code{BinaryFunction} object is destroyed.

The \code{func} parameter is a binary functor on two \code{Result}s. It should be derived from\code{funcT}.
\code{addResult} and \code{multResult}, which respectively add and multiply two \code{Result}s, are provided
as part of the InstructionAPI, as they are necessary for representing address calculations. Other \code{funcTs}
may be implemented by the user if desired. \code{funcTs} have names associated with them for output and
debugging purposes. The addition and multiplication functors provided with the Instruction API are named
"+" and "*", respectively. 
}

\begin{apient}
const Result & eval () const
\end{apient}
\apidesc{
The \code{BinaryFunction} version of \code{eval} allows the \code{eval} mechanism to handle complex addressing
modes. Like all of the \code{ValueComputation} implementations, a \code{BinaryFunction}'s \code{eval} will return the
result of evaluating the expression it represents if possible, or an empty \code{Result} otherwise. A \code{BinaryFunction} may have arguments that can be evaluated, or arguments that cannot. Additionally, it
may have a real function pointer, or it may have a null function pointer. If the arguments can be
evaluated and the function pointer is real, a result other than an empty \code{Result} is guaranteed to
be returned. This result is cached after its initial calculation; the caching mechanism also allows
outside information to override the results of the \code{BinaryFunction}'s internal computation. If the
cached result exists, it is guaranteed to be returned even if the arguments or the function are not
evaluable.
}

\begin{apient}
void getChildren (vector< Expression::Ptr > & children) const
\end{apient}
\apidesc{
The children of a \code{BinaryFunction} are its two arguments.
Appends the children of this BinaryFunction to \code{children}.
}

\begin{apient}
void getUses (set< Expression::Ptr > & uses) 
\end{apient}
\apidesc{
The use set of a \code{BinaryFunction} is the union of the use sets of its children.
Appends the use set of this \code{BinaryFunction} to \code{uses}.
}

\begin{apient}
bool isUsed (Expression::Ptr findMe) const 
\end{apient}
\apidesc{
\code{isUsed} returns \code{true} if \code{findMe} is an argument of this \code{BinaryFunction}, or if it is in the use set of either argument.
}
