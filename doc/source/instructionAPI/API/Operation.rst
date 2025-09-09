.. _sec:operation:

Operation Class
---------------

An Operation object represents a family of opcodes (operation encodings)
that perform the same task (e.g. the family). It includes information
about the number of operands, their read/write semantics, the implicit
register reads and writes, and the control flow behavior of a particular
assembly language operation. It additionally provides access to the
assembly mnemonic, which allows any semantic details that are not
encoded in the Instruction representation to be added by higher layers
of analysis.

As an example, the operation on IA32/AMD64 processors has the following
properties:

-  Operand 1 is read, but not written

-  Operand 2 is read, but not written

-  The following flags are written:

   -  Overflow

   -  Sign

   -  Zero

   -  Parity

   -  Carry

   -  Auxiliary

-  No other registers are read, and no implicit memory operations are
   performed

Operations are constructed by the as part of the process of constructing
an Instruction.

OpCode = operation + encoding

-  hex value

-  information on how to decode operands

-  operation encoded

Operation = what an instruction does

-  read/write semantics on explicit operands

-  register implicit read/write lists, including flags

-  string/enum representation of the operation

Some use cases include

OpCode + raw instruction -> Operation + ExpressionPtrs Operation +
ExpressionPtrs -> Instruction + Operands

const Operation::registerSet & implicitReads () const

const Operation::registerSet & implicitWrites () const

std::string format() const

entryID getID() const

prefixEntryID getPrefixID() const

bool isRead(Expression::Ptr candidate) const

bool isWritten(Expression::Ptr candidate) const

const Operation::VCSet & getImplicitMemReads() const

const Operation::VCSet & getImplicitMemWrites() const
