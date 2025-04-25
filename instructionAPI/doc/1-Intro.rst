\section{Introduction}
\label{sec:intro}

When analyzing and modifying binary code, it is necessary to translate between
raw binary instructions and an abstract form that describes the semantics of the
instructions. As a part of the Dyninst project, we have developed the
Instruction API, an API and library for decoding and representing machine
instructions in a platform-\/independent manner. The Instruction API includes
methods for decoding machine language, convenient abstractions for its analysis,
and methods to produce disassembly from those abstractions. The current
implementation supports the x86, x86-64, ARMv8-64, PowerPC-32, and PowerPC-64 instruction sets.
The Instruction API has the following basic capabilities:

\begin{itemize}
\item Decoding: interpreting a sequence of bytes as a machine instruction in a given machine language.
\item Abstract representation: representing the behavior of that instruction as an abstract syntax tree.
\item Disassembly: translating an abstract representation of a machine instruction into a string representation of the corresponding assembly language instruction.
\end{itemize}

Our goal in designing the Instruction API is to provide a representation of
machine instructions that can be manipulated by higher-\/level algorithms with
minimal knowledge of platform-\/specific details. In addition, users who need
platform-\/specific information should be able to access it. To do so, we
provide an interface that disassembles a machine instruction, extracts an
operation and its operands, converts the operands to abstract syntax trees, and
presents this to the user. A user of the Instruction API can work at a level of
abstraction slightly higher than assembly language, rather than working directly
with machine language. Additionally, by converting the operands to abstract
syntax trees, we make it possible to analyze the operands in a uniform manner,
regardless of the complexity involved in the operand's actual computation.  
