\subsection{Operation Class}
\label{sec:operation}

An Operation object represents a family of opcodes (operation encodings) that
perform the same task (e.g. the \code{MOV} family). It includes information
about the number of operands, their read/write semantics, the implicit register
reads and writes, and the control flow behavior of a particular assembly
language operation. It additionally provides access to the assembly mnemonic,
which allows any semantic details that are not encoded in the Instruction
representation to be added by higher layers of analysis.

As an example, the \code{CMP} operation on IA32/AMD64 processors has the following properties:
\begin{itemize}
\item Operand 1 is read, but not written
\item Operand 2 is read, but not written
\item The following flags are written:
\begin{itemize}
\item Overflow
\item Sign
\item Zero
\item Parity
\item Carry
\item Auxiliary
\end{itemize}
\item No other registers are read, and no implicit memory operations are performed
\end{itemize}

Operations are constructed by the \code{InstructionDecoder} as part of the
process of constructing an Instruction.

OpCode = operation + encoding
\begin{itemize}
\item hex value
\item information on how to decode operands
\item operation encoded
\end{itemize}

Operation = what an instruction does
\begin{itemize}
\item read/write semantics on explicit operands
\item register implicit read/write lists, including flags
\item string/enum representation of the operation
\end{itemize}

Some use cases include

OpCode + raw instruction -> Operation + ExpressionPtrs
Operation + ExpressionPtrs -> Instruction + Operands


\begin{apient}
const Operation::registerSet & implicitReads () const
\end{apient}
\apidesc{
Returns the set of registers implicitly read (i.e. those not included in the
operands, but read
anyway).
}

\begin{apient}
const Operation::registerSet & implicitWrites () const
\end{apient}
\apidesc{
Returns the set of registers implicitly written (i.e. those not included in the
operands, but written
anyway).
}

\begin{apient}
std::string format() const
\end{apient}
\apidesc{
    Returns the mnemonic for the operation. Like \code{instruction::format}, this is
exposed for debug-
ging and will be replaced with stream operators in the public interface.
}

\begin{apient}
entryID getID() const
\end{apient}
\apidesc{
    Returns the entry ID corresponding to this operation. Entry IDs are
    enumerated values that correspond to assembly mnemonics.
}

\begin{apient}
prefixEntryID getPrefixID() const
\end{apient}
\apidesc{
Returns the prefix entry ID corresponding to this operation, if any. Prefix IDs
are enumerated values that correspond to assembly prefix mnemonics.
}

\begin{apient}
bool isRead(Expression::Ptr candidate) const
\end{apient}
\apidesc{
    Returns \code{true} if the expression represented by \code{candidate} is
    read implicitly.
}

\begin{apient}
bool isWritten(Expression::Ptr candidate) const
\end{apient}
\apidesc{
    Returns \code{true} if the expression represented by \code{candidate} is
    written implicitly.
}

\begin{apient}
const Operation::VCSet & getImplicitMemReads() const
\end{apient}
\apidesc{
Returns the set of memory locations implicitly read.
}

\begin{apient}
const Operation::VCSet & getImplicitMemWrites() const
\end{apient}
\apidesc{
Returns the set of memory locations implicitly write.
}
