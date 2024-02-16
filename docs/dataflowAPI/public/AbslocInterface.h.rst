.. _`sec:abslocinterface.h`:

AbslocInterface.h
#################

.. cpp:namespace:: Dyninst

.. cpp:class:: AbsRegionConverter

  **Converts instructions to** :ref:`Abstract Regions <sec:dataflow-abstractions>`

  Definition: the first AbsRegion represents the expression.
  If it's a memory reference, any other AbsRegions represent registers used in this expression.

  .. cpp:function:: AbsRegionConverter(bool cache, bool stack)

    Creates a converter that can cache results can use stack analysis.

    When ``cache`` is ``true``, the conversion results are saved for reuse.
    When ``stack`` is ``true``, stack analysis is used to distinguish stack variables at different offsets.
    When ``stack`` is ``false``, the stack is treated as a single memory region.

  .. cpp:function:: void convertAll(InstructionAPI::Expression::Ptr expr, Address addr, \
                                    ParseAPI::Function *func, ParseAPI::Block *block, \
                                    std::vector<AbsRegion> &regions)

    Creates all abstract regions used in ``expr`` and return them in
    ``regions``. All registers appear in ``expr`` will have a separate
    abstract region. If the expression represents a memory access, we will
    also create a heap or stack abstract region depending on where it
    accesses. ``addr``, ``func``, and ``blocks`` specify the contexts of the
    expression. If PC appears in this expression, we assume the expression
    is at address ``addr`` and replace PC with a constant value ``addr``.

  .. cpp:function:: void convertAll(InstructionAPI::Instruction const& insn, Address addr, \
                                    ParseAPI::Function *func, ParseAPI::Block *block, \
                                    std::vector<AbsRegion> &used, std::vector<AbsRegion> &defined)

    Creates abstract regions appearing in instruction ``insn``. Input
    abstract regions of this instructions are returned in ``used`` and
    output abstract regions are returned in ``defined``. If the expression
    represents a memory access, we will also create a heap or stack abstract
    region depending on where it accesses. ``addr``, ``func``, and
    ``blocks`` specify the contexts of the expression. If PC appears in this
    expression, we assume the expression is at address ``addr`` and replace
    PC with a constant value ``addr``.

  .. cpp:function:: AbsRegion convertPredicatedRegister(InstructionAPI::RegisterAST::Ptr r, \
                                                        InstructionAPI::RegisterAST::Ptr p, bool c)

    Creates an abstract region for the register ``r`` using predicated register ``p`` with value ``c``.

  .. cpp:function:: AbsRegion convert(InstructionAPI::RegisterAST::Ptr reg)

    Creates an abstract region representing the register ``reg``.

  .. cpp:function:: AbsRegion convert(InstructionAPI::Expression::Ptr expr, Address addr, \
                                      ParseAPI::Function *func, ParseAPI::Block *block)

    Creates an abstract region represented by ``expr`` at address ``addr`` in the block ``block`` in
    the function ``func``.

  .. cpp:function:: AbsRegion stack(Address addr, ParseAPI::Function *func, ParseAPI::Block *block, bool push)

     Creates a :cpp:enumerator:`Absloc::Type::Stack` abstract region at address ``addr`` in block ``block``
     in function ``func``. If ``push`` is ``true``, the stack height is adjusted to allocate space
     for a word.

  .. cpp:function:: AbsRegion frame(Address addr, ParseAPI::Function *func, ParseAPI::Block *block, bool push)

     Creates a :cpp:enumerator:`Absloc::Type::Heap` abstract region at address ``addr`` in block ``block``
     in function ``func``. If ``push`` is ``true``, the stack height is adjusted to allocate space
     for a word.

.. cpp:class:: AssignmentConverter

  Converts instructions to :ref:`Assignments <sec:dataflow-abstractions>`

  .. cpp:function:: AssignmentConverter(bool cache, bool stack)

    Creates a converter that can cache results can use stack analysis.

    When ``cache`` is ``true``, the conversion results are saved for reuse.
    When ``stack`` is ``true``, stack analysis is used to distinguish stack variables at different offsets.
    When ``stack`` is ``false``, the stack is treated as a single memory region.

  .. cpp:function:: void convert(InstructionAPI::Instruction const& insn, const Address &addr, \
                                 ParseAPI::Function *func, ParseAPI::Block *blk, \
                                 std::vector<Assignment::Ptr> &assign)

    Converts instruction ``insn`` at address ``addr`` contained in block ``blk`` from function ``func``
    to assignments and returns them in ``assign``.
