.. _`sec:Instruction.h`:

Instruction.h
#############

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:class:: Instruction

  **A generic instruction representation**

  .. cpp:type:: boost::shared_ptr<Instruction> Ptr

  .. cpp:function:: Instruction(Operation::Ptr what, size_t size, const unsigned char *raw, Architecture arch)

      Constructs an instruction from the ``arch`` ISA that consumes ``size`` bytes from ``raw``
      and executes the operation ``what``.

  .. cpp:function:: const Operation & getOperation() const

      Returns the operation executed by the instruction.

  .. cpp:function:: void getOperands(std::vector<Operand> & operands) const

      Appends the operands into ``operands`` in the order they were decoded.

  .. cpp:function:: std::vector<Operand> getDisplayOrderedOperands() const;

      Returns the non-implicit operands in printed order.

  .. cpp:function:: Operand getOperand(int index) const

      Returns the operand at position ``index``.

      Returns an empty operand if ``index`` does not correspond to a valid operand.

  .. cpp:function:: unsigned char rawByte(unsigned int index) const

      Returns the byte of the instruction at zero-based position ``index``.

  .. cpp:function:: size_t size() const

      Returns the size of the corresponding machine instruction in **bytes**.

  .. cpp:function:: const void * ptr() const

      Returns a pointer to the raw bytes from which this instruction was decoded.

  .. cpp:function:: void getWriteSet(std::set<RegisterAST::Ptr> & regsWritten) const

      Appends the set of registers written by this instruction into ``regsWritten``.

      See :ref:`sec:instruction-read-write-sets` for details.

  .. cpp:function:: void getReadSet(std::set<RegisterAST::Ptr> & regsRead) const

      Appends the set of registers read by this instruction into ``regsRead``.

      If an operand is used to compute an effective address, the registers
      involved are read but not written, regardless of the effect on the
      operand. See :ref:`sec:instruction-read-write-sets` for details.

  .. cpp:function:: bool isRead(Expression::Ptr candidate) const

      Checks if ``candidate`` is an operand read from by this instruction.

      See :ref:`sec:instruction-read-write-sets` for details.

  .. cpp:function:: bool isWritten(Expression::Ptr candidate) const

      Checks if ``candidate`` is an operand written to by this instruction.

      See :ref:`sec:instruction-read-write-sets` for details.

  .. cpp:function:: bool readsMemory() const

      Checks if the instruction reads at least one memory address as data.

      If any operand containing a ``Dereference`` object is read, the
      instruction reads the memory at that address. Also, on platforms where a
      stack pop is guaranteed to read memory, ``readsMemory`` will return
      ``true`` for a pop instruction. See :ref:`sec:instruction-read-write-sets` for details.

  .. cpp:function:: bool writesMemory() const

      Checks if the instruction writes at least one memory address as data.

      If any operand containing a ``Dereference`` object is write, the
      instruction writes the memory at that address. Also, on platforms where
      a stack push is guaranteed to write memory, ``writesMemory`` will return
      ``true`` for a pop instruction. See :ref:`sec:instruction-read-write-sets` for details.

  .. cpp:function:: void getMemoryReadOperands(std::set<Expression::Ptr> & memAccessors) const

      Appends the set of memory addresses read by this instruction into ``memAccessors``.

      The addresses read are in the form of expressions which may be
      evaluated once all of the registers that they use have had their values
      set. See :ref:`sec:instruction-read-write-sets` for details.

      .. Note:: This method returns ASTs representing address computations, not address accesses.

  .. cpp:function:: void getMemoryWriteOperands(std::set<Expression::Ptr> & memAccessors) const

      Appends the set of memory addresses written to by this instruction into ``memAccessors``.

      The addresses written to are in the same form as those returned by :cpp:func:`getMemoryReadOperands`.
      See :ref:`sec:instruction-read-write-sets` for details

  .. cpp:function:: Expression::Ptr getControlFlowTarget() const

      Returns an expression to the non-fallthrough control targets, if any, of this instruction.

      See :ref:`sec:instruction-control-flow-targets` for details.

  .. cpp:function:: bool allowsFallThrough() const

      Checks if control flow will unconditionally go to the result of
      :cpp:func:`getControlFlowTarget` after executing this instruction.

      When called on an explicitly control-flow altering instruction, returns
      the non-fallthrough control flow destination. When called on any other
      instruction, returns ``NULL``. See :ref:`sec:instruction-control-flow-targets` for details.

  .. cpp:function:: std::string format(Address addr = 0)

      Returns the instruction as a string of assembly language.

      If ``addr`` is specified, the value of the program counter as used by the instruction
      (e.g., a branch) is set to ``addr``.

  .. cpp:function:: ArchSpecificFormatter& getFormatter() const

      Returns a specialized formatter for this instruction.

  .. cpp:function:: bool isValid() const

      Checks if this instruction is valid.

      Invalid instructions indicate than an :cpp:class::`InstructionDecoder` has reached
      the end of its assigned range, and that decoding should terminate.

  .. cpp:function:: bool isLegalInsn() const

      Checks if this Instruction is a legal instruction as specified by the architecture used
      to decode it.

  .. cpp:function:: Architecture getArch() const

      Returns the architecture containing the instruction.

  .. cpp:function:: InsnCategory getCategory() const

      Returns the category of this instruction.

  .. cpp:type:: std::list<CFT>::const_iterator cftConstIter
  .. cpp:function:: cftConstIter cft_begin() const
  .. cpp:function:: cftConstIter cft_end() const

.. cpp:union:: Instruction::raw_insn_T

  From 8f5df773905b:

  By making this union .small_insn a uintptr_t, so it's the same size as
  .large_insn, we can avoid some memory allocation without taking any more
  memory in Instruction itself.  (PPC instructions are always 32-bit, so
  there it's left as unsigned int.)

  With this optimization on x86_64, the number of large_insn allocations
  required is greatly reduced.  Many instructions are more than 4 bytes,
  but it's less common to have more than 8 bytes.

.. cpp:struct:: Instruction::CFT
  
  **A Control Flow Target**

    On certain platforms (e.g., PowerPC with conditional call/return
    instructions) the ``getControlFlowTarget`` function is insufficient to
    represent the successors of an instruction. The :cpp:func:`cft_begin` and
    :cpp:func:`cft_end` functions return iterators into a list of all control flow
    target expression. In most cases, :cpp:func:`getControlFlowTarget` suffices.

  .. cpp:member:: Expression::Ptr target
  .. cpp:member:: bool isCall
  .. cpp:member:: bool isIndirect
  .. cpp:member:: bool isConditional
  .. cpp:member:: bool isFallthrough

  .. cpp:function:: CFT(Expression::Ptr t, bool call, bool indir, bool cond, bool ft)


.. _`sec:instruction-notes`:


Notes
=====

An instruction contains operands, read/write semantic information about those operands,
and information about what other registers and memory locations are
affected by the operation the instruction performs. The purpose of an Instruction object
is to join an Operation with a sequence of Operands, and provide an interface for some
common summary analyses: namely, the read/write sets, memory access information, and
control flow information.

The Operation contains knowledge about its mnemonic and sufficient
semantic details to answer the following questions:

-  What Operands are read/written?
-  What registers are implicitly read/written?
-  What memory locations are implicitly read/written?
-  What are the possible control flow successors of this instruction?

Each Operand is an AST that allows you to determine:

-  Registers read
-  Registers written
-  Whether memory is read or written
-  Which memory addresses are read or written, given the state of all
   relevant registers

Instructions should be constructed from an ``unsigned char*``
pointing to machine language, using an :cpp:class:`InstructionDecoder`.

.. _`sec:instruction-read-write-sets`:

Read-Write Sets
^^^^^^^^^^^^^^^

The list of registers returned by :cpp:func:`Instruction::getWriteSet`
includes registers that are explicitly written as destination operands
(like the destination of a move). It also includes registers that are
implicitly written (like the stack pointer in a push or pop
instruction). It does not include any registers used only in computing
the effective address of a write. ``pop eax``, for example,
writes to ``esp``, reads ``esp``, and reads ``eax``, but despite the
fact that ``eax`` is the destination operand, ``eax`` is not itself
written.

For both the write set and the read sets, it is possible to
determine whether a register is accessed implicitly or explicitly by
examining the Operands. An explicitly accessed register appears as an
operand that is written or read; also, any registers used in any address
calculations are explicitly read. Any element of the write set or read
set that is not explicitly written or read is implicitly written or
read.

For instance, an instruction accessing memory through a register dereference would return an
expression tree containing just the register that determines the
address being accessed, not a tree representing a dereference of that
register. Also note that the type of this expression is the type of
an effective address (generally a word or double word), not the type of
the memory being accessed. For the memory being accessed, use
:cpp:func:`getOperands` directly.


.. _`sec:instruction-control-flow-targets`:

Control Flow Targets
^^^^^^^^^^^^^^^^^^^^

For direct absolute branch instructions, :cpp:func:`Instruction::getControlFlowTarget`
returns an :cpp:class:`Immediate` value. For direct relative branch instructions,
it returns the expression ``PC + offset``. In
the case of indirect branches and calls, it returns a dereference of a
register or possibly a dereference of a more complicated expression.
In this case, data flow analysis will often allow the determination of
the possible targets of the instruction. We do not do analysis beyond
the single-instruction level. If other code
performs this type of analysis, it may update the information in the
Dereference object using the setValue method in the Expression
interface.

