\subsection{Class Assignment}
\label{sec:assign}
\definedin{Absloc.h}

An assignment represents data dependencies between an output abstract region
that is modified by this instruction and several input abstract regions that are
used by this instruction. An instruction may modify several abstract regions, 
so an instruction can correspond to multiple assignments.


\begin{apient}
typedef boost::shared_ptr<Assignment> Ptr;
\end{apient}
\apidesc{Shared pointer for Assignment class.}

\begin{apient}
const std::vector<AbsRegion> &inputs() const;
std::vector<AbsRegion> &inputs();
\end{apient}
\apidesc{Return the input abstract regions.}

\begin{apient}
const AbsRegion &out() const;
AbsRegion &out();
\end{apient}
\apidesc{Return the output abstract region.}

\begin{apient}
InstructionAPI::Instruction::Ptr insn() const;
\end{apient}
\apidesc{Return the instruction that contains this assignment.}

\begin{apient}
Address addr() const;
\end{apient}
\apidesc{Return the address of this assignment.}

\begin{apient}
ParseAPI::Function *func() const;
\end{apient}
\apidesc{Return the function that contains this assignment.}

\begin{apient}
ParseAPI::Block *block() const;
\end{apient}
\apidesc{Return the block that contains this assignment.}

\begin{apient}
const std::string format() const;
\end{apient}
\apidesc{Return the string representation of this assignment.}


\subsection{Class AssignmentConverter}
\label{sec:assignmentcovnert}
\definedin{AbslocInterface.h}

This class should be used to convert instructions to assignments.

\begin{apient}
AssignmentConverter(bool cache, bool stack = true);
\end{apient}
\apidesc{Construct an AssignmentConverter.
When \code{cache} is \code{true}, this object
will cache the conversion results for converted instructions. When \code{stack}
is \code{true}, stack analysis is used to distinguish stack variables at
different offset. When \code{stack} is \code{false}, the stack is treated as a
single memory region.}

\begin{apient}
void convert(InstructionAPI::Instruction::Ptr insn,
             const Address &addr,
             ParseAPI::Function *func,
             ParseAPI::Block *blk,
             std::vector<Assignment::Ptr> &assign);
\end{apient}
\apidesc{Convert instruction \code{insn} to assignments and return these
assignments in \code{assign}. The user also needs to provide the context of
\code{insn}, including its address \code{addr}, function \code{func}, and block
\code{blk}.}
