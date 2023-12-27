.. _`sec:stackanalysis.h`:

stackanalysis.h
###############

.. cpp:namespace:: Dyninst

Heights of abstract locations at any instruction in a function. Due to
there often being many paths through the CFG to reach a given
instruction, abstract locations may have different stack heights
depending on the path taken to reach that instruction. In other cases,
StackAnalysis is unable to adequately determine what is contained in an
abstract location. In both situations, StackAnalysis is conservative in
its reported stack heights. The table below explains what the reported
stack heights mean.

+-----------------------+---------------------------------------------+
| Reported stack height | Meaning                                     |
+=======================+=============================================+
| TOP                   | On all paths to this instruction, the       |
|                       | specified abstract location contains a      |
|                       | value that does not point to the stack.     |
+-----------------------+---------------------------------------------+
|                       |                                             |
+-----------------------+---------------------------------------------+
| *x* (some number)     | On at least one path to this instruction,   |
|                       | the specified abstract location has a stack |
|                       | height of *x*. On all other paths, the      |
|                       | abstract location either has a stack height |
|                       | of *x* or doesn’t point to the stack.       |
+-----------------------+---------------------------------------------+
|                       |                                             |
+-----------------------+---------------------------------------------+
| BOTTOM                | There are three possible meanings:          |
|                       |                                             |
|                       | #. On at least one path to this             |
|                       | instruction, StackAnalysis was unable to    |
|                       | determine whether or not the specified      |
|                       | abstract location points to the stack.      |
|                       |                                             |
|                       | #. On at least one path to this             |
|                       | instruction, StackAnalysis determined       |
|                       | that the specified abstract location        |
|                       | points to the stack but could not           |
|                       | determine the exact stack height.           |
|                       |                                             |
|                       | #. On at least two paths to this            |
|                       | instruction, the specified abstract         |
|                       | location pointed to different parts of      |
|                       | the stack.                                  |
+-----------------------+---------------------------------------------+

.. cpp:class:: StackAnalysis

  **An interface used to determine the possible stack**

  .. cpp:type:: boost::shared_ptr<InstructionAPI::Instruction> InstructionPtr

    An alias for :cpp:type:`InstructionAPI::Instruction::Ptr`.

  .. cpp:type:: boost::shared_ptr<InstructionAPI::Expression> ExpressionPtr

    An alias for :cpp:type:`InstructionAPI::Expression::Ptr`.

  .. cpp:function:: StackAnalysis(ParseAPI::Function *f)

    Creates a StackAnalysis object for function ``f``.

  .. cpp:function:: StackAnalysis(ParseAPI::Function *f, const std::map<Address, Address> &crm, const std::map<Address, TransferSet> &fs)

    Constructs a StackAnalysis object for function ``f`` with
    interprocedural analysis activated. A call resolution map is passed in
    ``crm`` mapping addresses of call sites to the resolved inter-module
    target address of the call. Generally the call resolution map is created
    with DyninstAPI where PLT resolution is done. Function summaries are
    passed in ``fs`` which maps function entry addresses to summaries. The
    function summaries are then used at all call sites to those functions.

  .. cpp:function:: Height find(ParseAPI::Block *b, Address addr, Absloc loc)

    Returns the stack height of abstract location ``loc`` before execution
    of the instruction with address ``addr`` contained in basic block ``b``.
    The address ``addr`` must be contained in block ``b``, and block ``b``
    must be contained in the function used to create this StackAnalysis
    object.

  .. cpp:function:: Height findSP(ParseAPI::Block *b, Address addr)
  .. cpp:function:: Height findFP(ParseAPI::Block *b, Address addr)

    Returns the stack height of the stack pointer and frame pointer,
    respectively, before execution of the instruction with address ``addr``
    contained in basic block ``b``. The address ``addr`` must be contained
    in block ``b``, and block ``b`` must be contained in the function used
    to create this StackAnalysis object.

  .. cpp:function:: void findDefinedHeights(ParseAPI::Block *b, Address addr, std::vector<std::pair<Absloc, Height>> &heights)

    Writes to the vector ``heights`` all defined <abstract location, stack
    height> pairs before execution of the instruction with address ``addr``
    contained in basic block ``b``. Note that abstract locations with stack
    heights of TOP (i.e. they do not point to the stack) are not written to
    ``heights``. The address ``addr`` must be contained in block ``b``, and
    block ``b`` must be contained in the function used to create this
    StackAnalysis object.

  .. cpp:function:: bool canGetFunctionSummary()

    Returns ``true`` if the function associated with this StackAnalysis object
    returns on some execution path.

  .. cpp:function:: bool getFunctionSummary(TransferSet &summary)

    Returns in ``summary`` a summary for the function associated with this
    StackAnalysis object. Function summaries can then be passed to the
    constructors for other StackAnalysis objects to enable interprocedural
    analysis.

    Returns ``true`` on success.

.. cpp:class:: StackAnalysis::Height

  **A representation of stack offsets**

  Every Height represents a stack height of either TOP, BOTTOM, or *x*, where *x* is some integral number.

  .. Note:: This class satisfies the `Compare <https://en.cppreference.com/w/cpp/named_req/Compare>`_ concept.

  .. cpp:type:: signed long Height_t

  .. cpp:enum:: Type

    .. cpp:enumerator:: TOP
    .. cpp:enumerator:: BOTTOM
    .. cpp:enumerator:: HEIGHT

  .. cpp:member:: static const Height_t uninitialized = MAXLONG
  .. cpp:member:: static const Height_t notUnique = MINLONG
  .. cpp:member:: static const Height bottom
  .. cpp:member:: static const Height top

  .. cpp:function:: Height(const Height_t h)

    Creates a Height object with stack height ``h``.

  .. cpp:function:: Height()

    Creates a Height object with stack height :cpp:enumerator::`TOP`.

  .. cpp:function:: Height_t height() const

    Returns the stack height as an integral value.

  .. cpp:function:: std::string format() const

    Returns a string representation of the stack height.

  .. cpp:function:: bool isTop() const

    Returns ``true`` if this stack height is :cpp:enumerator::`TOP`.

  .. cpp:function:: bool isBottom() const

    Returns ``true`` if this stack height is :cpp:enumerator::`BOTTOM`.

  .. cpp:function:: static Height meet(const Height &lhs, const Height &rhs)

    Selects the higher of the two heights.

  .. cpp:function:: static Height meet(std::set<Height> &ins)

    Selects the highest height.

  .. cpp:function:: Height &operator+=(const Height &rhs)
  .. cpp:function:: Height &operator+=(const signed long &rhs) const
  .. cpp:function:: Height operator+(const Height &rhs) const
  .. cpp:function:: const Height operator+(const signed long &rhs) const
  .. cpp:function:: const Height operator-(const Height &rhs) const

    Returns the result of basic arithmetic on Height objects.

  .. note:: For integral stack heights, *x* and *y*, and *S* any stack height

    .. math::

      TOP + TOP = TOP

      TOP + x = BOTTOM

      x + y = (x+y)

      BOTTOM + S = BOTTOM


    The subtraction rules can be obtained by replacing all + signs with - signs.

  .. cpp:function:: friend std::ostream& operator<<(std::ostream& stream, const Height& c)

    Writes a representation of a Height to ``stream``.

    .. Note:: Implicitly calls :cpp:func:`format`.
