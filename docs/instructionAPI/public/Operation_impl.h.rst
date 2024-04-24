.. _`sec:Operation_impl.h`:

Operation_impl.h
################

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:class:: Operation : public boost::lockable_adapter<boost::recursive_mutex>

  **A family of opcodes that perform the same task**

  .. cpp:type:: std::set<RegisterAST::Ptr> registerSet
  .. cpp:type:: std::set<Expression::Ptr> VCSet

  .. cpp:function:: Operation(entryID id, std::string m, Architecture arch)

      Creates an operation for the instruction ``id`` with mnemonic ``m`` on architecture ``arch``.

  .. cpp:function:: const Operation::registerSet& implicitReads() const

      Returns the set of registers implicitly read from.

  .. cpp:function:: const Operation::registerSet& implicitWrites() const

      Returns the set of registers implicitly written to.

  .. cpp:function:: std::string format() const

      Returns the mnemonic for the operation.

  .. cpp:function:: entryID getID() const

      Returns the id corresponding to this operation.

  .. cpp:function:: prefixEntryID getPrefixID() const

      Returns the prefix id corresponding to this operation, if any.

  .. cpp:function:: bool isRead(Expression::Ptr candidate) const

      Checks if ``candidate`` is read from.

  .. cpp:function:: bool isWritten(Expression::Ptr candidate) const

      Checks if ``candidate`` is written to.

  .. cpp:function:: const Operation::VCSet& getImplicitMemReads() const

      Returns the set of memory locations implicitly read from.

  .. cpp:function:: const Operation::VCSet& getImplicitMemWrites() const

      Returns the set of memory locations implicitly written to.


.. _`sec:operation-notes`:

Notes
=====

An operation includes information about the number of operands, their read/write semantics,
the implicit register reads and writes, and the control flow behavior of
a particular assembly language operation. It additionally provides
access to the assembly mnemonic, which allows any semantic details that
are not encoded in the Instruction representation to be added by higher
layers of analysis.

As an example, the ``cmp`` operation on IA32/AMD64 processors has the
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

Operations are constructed by the :cpp:class::`InstructionDecoder` as part of the
process of constructing an instruction.
