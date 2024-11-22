\subsection{InstructionDecoder Class}
\label{sec:instructionDecoder}

The \code{InstructionDecoder} class decodes instructions, given a buffer of bytes and a
length, and constructs an Instruction. 

The \code{InstructionDecoder} class decodes instructions, given a buffer of bytes and a length, and
the architecture for which to decode instructions,
and constructs shared pointers to \code{Instruction} objects representing those instructions.
\code{InstructionDecoder} objects are given a buffer from which to decode at construction.
Calls to decode will proceed to decode instructions sequentially from that buffer until its
end is reached.  At that point, all subsequent calls to decode will return a null \code{Instruction} pointer.

\begin{apient}
  InstructionDecoder(const unsigned char *buffer, size_t size,
                     Architecture arch)
  InstructionDecoder(const void *buffer, size_t size,
                     Architecture arch)
\end{apient}

\apidesc{
  Construct an \code{InstructionDecoder} over the provided
  \code{buffer} and \code{size}. We consider the buffer to contain
  instructions from the provided \code{arch}
}

\begin{apient}
Instruction::Ptr decode();
\end{apient}

\apidesc{
  Decode the current instruction in this \code{InstructionDecoder} object's buffer, interpreting it as
  machine language of the type understood by this \code{InstructionDecoder}.
  If the buffer does not contain a valid instruction stream, a null \code{Instruction} pointer
  will be returned.  The \code{Instruction}'s size field will contain the size of the instruction decoded.
}

\begin{apient}
  Instruction::Ptr decode(const unsigned char*);
\end{apient}

\apidesc{
  Decode the instruction at buffer, interpreting it as machine language of the type
  understood by this \code{InstructionDecoder}.  If the buffer does not contain a valid instruction stream,
  a null \code{Instruction} pointer will be returned.  The \code{Instruction}\'s size field will contain
  the size of the instruction decoded.
}

\begin{apient}
  struct unknown_instruction
\end{apient}
\apidesc{
  This interface allows registering a callback function to be invoked
  when the InstructionDecoder encounters a byte sequence it is not able
  to successfully convert into a known instruction.
}
