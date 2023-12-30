Instruction.h
=============

.. cpp:namespace:: Dyninst::InstructionAPI

Instruction Class
-----------------

The Instruction class is a generic instruction representation that
contains operands, read/write semantic information about those operands,
and information about what other registers and memory locations are
affected by the operation the instruction performs.

The purpose of an Instruction object is to join an Operation with a
sequence of Operands, and provide an interface for some common summary
analyses (namely, the read/write sets, memory access information, and
control flow information).

The Operation contains knowledge about its mnemonic and sufficient
semantic details to answer the following questions:

-  What Operands are read/written?

-  What registers are implicitly read/written?

-  What memory locations are implicitly read/written?

-  What are the possible control flow successors of this instruction?

Each Operand is an AST built from RegisterAST and Immediate leaves. For
each Operand, you may determine:

-  Registers read

-  Registers written

-  Whether memory is read or written

-  Which memory addresses are read or written, given the state of all
   relevant registers

Instructions should be constructed from an ``unsigned`` ``char\ast``
pointing to machine language, using the InstructionDecoder class. See
InstructionDecoder for more details.

Instruction (Operation::Ptr what, size_t size, const unsigned char \*
raw, Dyninst::Architecture arch)

``what`` is the opcode of the instruction, ``size`` contains the number
of bytes occupied by the corresponding machine instruction, and ``raw``
contains a pointer to the buffer from which this ``Instruction`` object
was decoded. The architecture is specified by ``arch``, and may be an
element from the following set:
``{Arch_x86, Arch_x86_64, Arch_ppc32, Arch_ppc64}`` (as defined in
``dyn_regs.h``).

Construct an Instruction from an Operation and a collection of
Expressions. This method is not intended to be used except by the
InstructionDecoder class, which serves as a factory class for producing
Instruction objects.

While an Instruction object may be built “by hand” if desired, using the
decoding interface ensures that the operation and operands are a
sensible combination, and that the size reported is based on the actual
size of a legal encoding of the machine instruction represented.


.. code-block:: cpp
    
    const Operation & getOperation() const

Returns the ``Operation`` used by the ``Instruction``. See
Section `3.2 <#sec:operation>`__ for details of the ``Operation``
interface.

.. code-block:: cpp

    void getOperands(std::vector<Operand> & operands) const

The vector operands has the instruction’s operands appended to it in the
same order that they were decoded.

.. code-block:: cpp

    Operand getOperand(int index) const

The ``getOperand`` method returns the operand at position ``index``, or
an empty operand if ``index`` does not correspond to a valid operand in
this instruction.

.. code-block:: cpp

    unsigned char rawByte(unsigned int index) const

Returns the index\ :math:`^{th}` byte in the instruction.

.. code-block:: cpp

    size_t size() const

Returns the size of the corresponding machine instruction, in bytes.

.. code-block:: cpp

    const void * ptr() const

Returns a pointer to the raw byte representation of the corresponding
machine instruction.

.. code-block:: cpp

    void getWriteSet(std::set<RegisterAST::Ptr> & regsWritten) const

Insert the set of registers written by the instruction into
``regsWritten``. The list of registers returned in ``regsWritten``
includes registers that are explicitly written as destination operands
(like the destination of a move). It also includes registers that are
implicitly written (like the stack pointer in a push or pop
instruction). It does not include any registers used only in computing
the effective address of a write. ``pop`` ``\asteax``, for example,
writes to ``esp``, reads ``esp``, and reads ``eax``, but despite the
fact that ``\asteax`` is the destination operand, ``eax`` is not itself
written.

For both the write set and the read set (below), it is possible to
determine whether a register is accessed implicitly or explicitly by
examining the Operands. An explicitly accessed register appears as an
operand that is written or read; also, any registers used in any address
calculations are explicitly read. Any element of the write set or read
set that is not explicitly written or read is implicitly written or
read.

.. code-block:: cpp

    void getReadSet(std::set<RegisterAST::Ptr> & regsRead) const

Insert the set of registers read by the instruction into ``regsRead``.

If an operand is used to compute an effective address, the registers
involved are read but not written, regardless of the effect on the
operand.

.. code-block:: cpp

    bool isRead(Expression::Ptr candidate) const

``candidate`` is the subexpression to search for among the values read
by this ``Instruction`` object.

Returns ``true`` if ``candidate`` is read by this ``Instruction``.

.. code-block:: cpp

    bool isWritten(Expression::Ptr candidate) const

``candidate`` is the subexpression to search for among the values
written by this ``Instruction`` object.

Returns ``true`` if ``candidate`` is written by this ``Instruction``.

.. code-block:: cpp

    bool readsMemory() const

Returns ``true`` if the instruction reads at least one memory address as
data.

If any operand containing a ``Dereference`` object is read, the
instruction reads the memory at that address. Also, on platforms where a
stack pop is guaranteed to read memory, ``readsMemory`` will return
``true`` for a pop instruction.

.. code-block:: cpp

    bool writesMemory() const

Returns ``true`` if the instruction writes at least one memory address
as data.

If any operand containing a ``Dereference`` object is write, the
instruction writes the memory at that address. Also, on platforms where
a stack push is guaranteed to write memory, ``writesMemory`` will return
``true`` for a pop instruction.

.. code-block:: cpp

    void getMemoryReadOperands(std::set<Expression::Ptr> & memAccessors) const

Addresses read by this instruction are inserted into ``memAccessors``.

The addresses read are in the form of ``Expression``\ s, which may be
evaluated once all of the registers that they use have had their values
set. Note that this method returns ASTs representing address
computations, and not address accesses. For instance, an instruction
accessing memory through a register dereference would return an
``Expression`` tree containing just the register that determines the
address being accessed, not a tree representing a dereference of that
register. Also note that the type of this ``Expression`` is the type of
an effective address (generally a word or double word), not the type of
the memory being accessed. For the memory being accessed, use
``getOperands`` directly.

.. code-block:: cpp

    void getMemoryWriteOperands(std::set<Expression::Ptr> & memAccessors) const

Addresses written by this instruction are inserted into
``memAccessors``.

The addresses written are in the same form as those returned by
``getMemoryReadOperands`` above.

.. code-block:: cpp

    Expression::Ptr getControlFlowTarget() const

When called on an explicitly control-flow altering instruction, returns
the non-fallthrough control flow destination. When called on any other
instruction, returns ``NULL``.

For direct absolute branch instructions, ``getControlFlowTarget`` will
return an immediate value. For direct relative branch instructions,
``getControlFlowTarget`` will return the expression ``PC`` + offset. In
the case of indirect branches and calls, it returns a dereference of a
register (or possibly a dereference of a more complicated expression).
In this case, data flow analysis will often allow the determination of
the possible targets of the instruction. We do not do analysis beyond
the single-instruction level in the Instruction API; if other code
performs this type of analysis, it may update the information in the
Dereference object using the setValue method in the Expression
interface. More details about this may be found in
Section `3.5 <#sec:expression>`__ and
Section `3.11 <#sec:dereference>`__.

Returns an ``Expression`` evaluating to the non-fallthrough control
targets, if any, of this instruction.

.. code-block:: cpp

    bool allowsFallThrough() const

Returns ``false`` if control flow will unconditionally go to the result
of ``getControlFlowTarget`` after executing this instruction.

.. code-block:: cpp

    std::string format(Address addr = 0)

Returns the instruction as a string of assembly language. If ``addr`` is
specified, the value of the program counter as used by the instruction
(e.g., a branch) is set to ``addr``.

.. code-block:: cpp

    bool isValid() const

Returns ``true`` if this ``Instruction`` object is valid. Invalid
instructions indicate than an ``InstructionDecoder`` has reached the end
of its assigned range, and that decoding should terminate.

.. code-block:: cpp

    bool isLegalInsn() const

Returns ``true`` if this Instruction object represents a legal
instruction, as specified by the architecture used to decode this
instruction.

.. code-block:: cpp

    Architecture getArch() const

Returns the architecture containing the instruction. As above, this will
be an element from the set
``{Arch_x86, Arch_x86_64, Arch_ppc32, Arch_ppc64}``.

.. code-block:: cpp

    InsnCategory getCategory() const

Currently, the valid categories are ``c_CallInsn``, ``c_ReturnInsn``,
``c_BranchInsn``, ``c_CompareInsn``, ``c_PrefetchInsn``,
``c_SysEnterInsn``, ``c_SyscallInsn``, ``c_VectorInsn``, and
``c_NoCategory``, as defined in ``InstructionCategories.h``.

.. code-block:: cpp

    struct CFT Expression::Ptr target; bool isCall; bool isIndirect; bool isConditional; bool isFallthrough;
    typedef ... cftConstIter; cftConstIter cft_begin() const; cftConstIter cft_end() const;

On certain platforms (e.g., PowerPC with conditional call/return
instructions) the ``getControlFlowTarget`` function is insufficient to
represent the successors of an instruction. The ``cft_begin`` and
``cft_end`` functions return iterators into a list of all control flow
target expressions as represented by a list of ``CFT`` structures. In
most cases, ``getControlFlowTarget`` suffices.