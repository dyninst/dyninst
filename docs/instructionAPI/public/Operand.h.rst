.. _`sec:Operand.h`:

Operand.h
#########

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:class:: Operand

  **An AST containing read/write information**

  .. cpp:type:: boost::shared_ptr<Operand> Ptr

  .. cpp:function:: explicit Operand(Expression::Ptr val = {}, bool read = false, \
                                     bool written = false, bool implicit = false, \
                                     bool trueP = false, bool falseP = false)

      Creates an operand from an expression and flags describing its read/write properties.

      .. csv-table::
      
        ``read``, the operand is read from
        ``written``, the operand is written to
        ``implicit``, the operand is implicitly read from or written to
        ``trueP``, the operand is predicated on ``true``
        ``falseP``, the operand is predicated on ``false``

    .. Note:: ``trueP`` and ``falseP`` are only used for GPU instructions with predicates.

  .. cpp:function:: void getReadSet(std::set<RegisterAST::Ptr> & regsRead) const

      Appends the set of registers read by this operand into ``regsRead``.

      See :ref:`sec:instruction-read-write-sets` for a more detailed discussion.

  .. cpp:function:: void getWriteSet(std::set<RegisterAST::Ptr> & regsWritten) const

      Appends the set of registers written by this operand into ``regsWritten``.

      See :ref:`sec:instruction-read-write-sets` for a more detailed discussion.

  .. cpp:function:: bool isRead() const

      Checks if this operand is read from.

  .. cpp:function:: bool isWritten() const

      Checks if this operand is written to.

  .. cpp:function:: bool isRead(Expression::Ptr candidate) const

      Checks if ``candidate`` is read by this operand.

  .. cpp:function:: bool isWritten(Expression::Ptr candidate) const

      Checks if ``candidate`` is written to by this operand.

  .. cpp:function:: bool isImplicit() const

      Checks if this operand is implicitly read from or written to.

  .. cpp:function:: void setImplicit(bool i)

      Sets if this operand is implicitly read from or written to.

  .. cpp:function:: bool isTruePredicate() const

      Checks if this operand is predicated on ``true``.

  .. cpp:function:: bool isFalsePredicate() const

      Checks if this operand is predicated on ``false``.

  .. cpp:function:: bool readsMemory() const

      Checks if this operand reads memory.

  .. cpp:function:: bool writesMemory() const

      Checks if this operand writes memory.

  .. cpp:function:: void addEffectiveReadAddresses(std::set<Expression::Ptr> & memAccessors) const

      Appends the effective addresses being read from into ``memAccessors``, if this operand reads memory.

  .. cpp:function:: void addEffectiveWriteAddresses(std::set<Expression::Ptr> & memAccessors) const

      Appends the effective addresses being written to into ``memAccessors``, if this operand writes memory.

  .. cpp:function:: std::string format(Architecture arch, Address addr = 0) const

      Returns a string representation of this expression using the :cpp:class:`ArchSpecificFormatter`
      associated with ``arch``. The optional ``addr`` parameter specifies the value of
      the program counter.

  .. cpp:function:: Expression::Ptr getValue() const

      Returns the the :cpp:class:`AST` of the operand.

.. _`sec:operand-notes`:

Notes
=====

This classes can be used to determine which of the registers that appear in
the Operand are read and which are written, as well as whether any
memory accesses are reads, writes, or both. An Operand, given full
knowledge of the values of the leaves of the AST, and knowledge of the
logic associated with the tree’s internal nodes, can determine the
result of any computations that are encoded in it. It will rarely be the
case that an :cpp:class:``Instruction`` is built with its Operands’ state fully
specified. This mechanism is instead intended to allow users to fill in
knowledge about the state of the processor at the time the instruction
is executed.
