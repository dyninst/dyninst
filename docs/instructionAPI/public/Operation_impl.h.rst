Operation_impl.h
================

.. cpp:namespace:: Dyninst::InstructionAPI

Operation Class
---------------

An Operation object represents a family of opcodes (operation encodings)
that perform the same task (e.g. the ``MOV`` family). It includes
information about the number of operands, their read/write semantics,
the implicit register reads and writes, and the control flow behavior of
a particular assembly language operation. It additionally provides
access to the assembly mnemonic, which allows any semantic details that
are not encoded in the Instruction representation to be added by higher
layers of analysis.

As an example, the ``CMP`` operation on IA32/AMD64 processors has the
following properties:

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

Operations are constructed by the ``InstructionDecoder`` as part of the
process of constructing an Instruction.

.. code-block:: cpp

    const Operation::registerSet & implicitReads () const

Returns the set of registers implicitly read (i.e. those not included in
the operands, but read anyway).

.. code-block:: cpp

    const Operation::registerSet & implicitWrites () const

Returns the set of registers implicitly written (i.e. those not included
in the operands, but written anyway).

.. code-block:: cpp

    std::string format() const

Returns the mnemonic for the operation. Like ``instruction::format``,
this is exposed for debug- ging and will be replaced with stream
operators in the public interface.

.. code-block:: cpp

    entryID getID() const

Returns the entry ID corresponding to this operation. Entry IDs are
enumerated values that correspond to assembly mnemonics.

.. code-block:: cpp

    prefixEntryID getPrefixID() const

Returns the prefix entry ID corresponding to this operation, if any.
Prefix IDs are enumerated values that correspond to assembly prefix
mnemonics.

.. code-block:: cpp

    bool isRead(Expression::Ptr candidate) const

Returns ``true`` if the expression represented by ``candidate`` is read
implicitly.

.. code-block:: cpp
    
    bool isWritten(Expression::Ptr candidate) const

Returns ``true`` if the expression represented by ``candidate`` is
written implicitly.

.. code-block:: cpp

    const Operation::VCSet & getImplicitMemReads() const

Returns the set of memory locations implicitly read.

.. code-block:: cpp

   const Operation::VCSet & getImplicitMemWrites() const

Returns the set of memory locations implicitly write.