Operand.h
=========

.. cpp:namespace:: Dyninst::InstructionAPI

Operand Class
-------------

An Operand object contains an AST built from RegisterAST and Immediate
leaves, and information about whether the Operand is read, written, or
both. This allows us to determine which of the registers that appear in
the Operand are read and which are written, as well as whether any
memory accesses are reads, writes, or both. An Operand, given full
knowledge of the values of the leaves of the AST, and knowledge of the
logic associated with the tree’s internal nodes, can determine the
result of any computations that are encoded in it. It will rarely be the
case that an Instruction is built with its Operands’ state fully
specified. This mechanism is instead intended to allow a user to fill in
knowledge about the state of the processor at the time the Instruction
is executed.

.. code-block:: cpp

    Operand(Expression::Ptr val, bool read, bool written)

Create an operand from an ``Expression`` and flags describing whether
the ValueComputation is read, written, or both.

``val`` is a reference-counted pointer to the ``Expression`` that will
be contained in the ``Operand`` being constructed. ``read`` is true if
this operand is read. ``written`` is true if this operand is written.

.. code-block:: cpp

    void getReadSet(std::set<RegisterAST::Ptr> & regsRead) const

Get the registers read by this operand. The registers read are inserted
into ``regsRead``.

.. code-block:: cpp

    void getWriteSet(std::set<RegisterAST::Ptr> & regsWritten) const

Get the registers written by this operand. The registers written are
inserted into ``regsWritten``.

.. code-block:: cpp

    bool isRead() const

Returns ``true`` if this operand is read.

.. code-block:: cpp
    
    bool isWritten() const

Returns ``true`` if this operand is written.

.. code-block:: cpp
    
    bool isRead(Expression::Ptr candidate) const

Returns ``true`` if ``candidate`` is read by this operand.

.. code-block:: cpp

    bool isWritten(Expression::Ptr candidate) const

Returns ``true`` if ``candidate`` is written to by this operand.

.. code-block:: cpp

    bool readsMemory() const

Returns ``true`` if this operand reads memory.

.. code-block:: cpp

    bool writesMemory() const

Returns ``true`` if this operand writes memory.

.. code-block:: cpp
    
    void addEffectiveReadAddresses(std::set<Expression::Ptr> & memAccessors) const

If ``Operand`` is a memory read operand, insert the ``ExpressionPtr``
representing the address being read into ``memAccessors``.

.. code-block:: cpp

    void addEffectiveWriteAddresses(std::set<Expression::Ptr> & memAccessors) const

If ``Operand`` is a memory write operand, insert the ``ExpressionPtr``
representing the address being written into ``memAccessors``.

.. code-block:: cpp

    std::string format(Architecture arch, Address addr = 0) const

Return a printable string representation of the operand. The ``arch``
parameter must be supplied, as operands do not record their
architectures. The optional ``addr`` parameter specifies the value of
the program counter.

.. code-block:: cpp

    Expression::Ptr getValue() const

The ``getValue`` method returns an ``ExpressionPtr`` to the AST
contained by the operand.