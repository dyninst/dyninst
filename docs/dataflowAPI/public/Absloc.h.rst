.. _`sec:absloc.h`:

Absloc.h
########

.. cpp:namespace:: Dyninst

To represent locations in program memory and registers, a two-layered model
consisting of abstract locations (AbsLocs) and abstract regions (AbsRegions)
is used. An :cpp:class:`Absloc` defines a unique location in memory or a register. The stack
is considered to be a separate, indexed-from-zero memory location.
An :cpp:class:`AbsRegion` is a set of Abslocs.

The abbreviations Program Counter (PC), Stack Pointer (SP), and Frame Pointer (FP) are
used throughout.

.. cpp:class:: Absloc

  **Represents an** :ref:`Abstract Location <sec:dataflow-abstractions>`

  .. Note:: This class satisfies the `Compare <https://en.cppreference.com/w/cpp/named_req/Compare>`_ concept.

  .. cpp:function:: Absloc(MachRegister reg)

    Creates a :cpp:enumerator:`Type::Register` type abstract location representing register ``reg``.

  .. cpp:function:: Absloc(Address addr)

    Creates a :cpp:enumerator:`Type::Heap` abstract location representing a heap variable at
    address ``addr``.

  .. cpp:function:: Absloc(int o, int r, ParseAPI::Function *f)

    Creates a :cpp:enumerator:`Type::Stack` abstract location representing a stack variable in
    the frame of function ``f``, within abstract region ``r``, at offset
    ``o`` within the frame.

  .. cpp:function:: Absloc(MachRegister r, MachRegister p, bool c)

    Creates a :cpp:enumerator:`Type::PredicatedRegister` abstract location, representing a machine
    register ``p`` used as a predicate against the register ``r`` with state ``c``.

  .. cpp:enum:: Type

     .. cpp:enumerator:: Register

        A machine register

     .. cpp:enumerator:: Stack

        A variable located on the stack

     .. cpp:enumerator:: Heap

        A variable located on the heap

     .. cpp:enumerator:: PredicatedRegister

        A machine register used as a predicate

     .. cpp:enumerator:: Unknown

        The default type of abstract location

  .. cpp:function:: static Absloc makePC(Dyninst::Architecture arch)
  .. cpp:function:: static Absloc makeSP(Dyninst::Architecture arch)
  .. cpp:function:: static Absloc makeFP(Dyninst::Architecture arch)

    Shortcut interfaces for creating abstract locations.

  .. cpp:function:: bool isPC() const
  .. cpp:function:: bool isSP() const
  .. cpp:function:: bool isFP() const

    Checks if this abstract location represents PC, SP, or FP, respectively.

  .. cpp:function:: std::string format() const

    Returns a string representation of this abstract location.

  .. cpp:function:: const Type& type() const

    Returns the type of this abstract location.

  .. cpp:function:: bool isValid() const

    Checks if this abstract location is valid or not.

  .. cpp:function:: const MachRegister &reg() const

    Returns the register.

    .. Attention:: This should only be used if this abstract location represents a register.

  .. cpp:function:: int off() const

    Returns the offset of the stack variable.

    .. Attention:: This should only be used if this abstract location represents a stack variable.

  .. cpp:function:: int region() const

    Returns the region of the stack variable.

    .. Attention:: This should only be used if this abstract location represents a stack variable.

  .. cpp:function:: ParseAPI::Function *func() const

    Returns the function of the stack variable.

    .. Attention:: This should only be used if this abstract location represents a stack variable.

  .. cpp:function:: Address addr() const

    Returns the address of the heap variable.

    .. Attention:: This should only be used if this abstract location represents a heap variable.

  .. cpp:function:: const MachRegister &predReg() const

      Returns the predicate register.

    .. Attention:: This should only be used if this abstract location represents a predicate register.

  .. cpp:function:: bool isTrueCondition() const

    Returns ``true`` if state of the predicate register is ``true``.

    .. Attention:: This should only be used if this abstract location represents a predicate register.

  .. cpp:function:: void flipPredicateCondition()

    Inverts the state of the predicate register.

    .. Attention:: This should only be used if this abstract location represents a predicate register.

  .. cpp:function:: friend std::ostream &operator<<(std::ostream &os, const Absloc &a)

    Writes a representation of the abstract location to the stream ``os``.

    Implicitly calls :cpp:func:`format`.

.. cpp:class:: AbsRegion

  Represents an :ref:`Abstract Region <sec:dataflow-abstractions>`

  .. Note:: This class satisfies the `Compare <https://en.cppreference.com/w/cpp/named_req/Compare>`_ concept.

  .. cpp:function:: AbsRegion()

    Creates an abstract region containing a single :cpp:class:`Absloc` of :cpp:enumerator:`Absloc::Type::Unknown` type.

  .. cpp:function:: AbsRegion(Type t)

    Creates an abstract region representing all abstract locations with type ``t``.

  .. cpp:function:: AbsRegion(Absloc a)

    Creates an abstract region representing a single abstract location ``a``.

  .. cpp:function:: bool contains(const Type t) const
  .. cpp:function:: bool contains(const Absloc &abs) const
  .. cpp:function:: bool contains(const AbsRegion &rhs) const

    Checks if this abstract region contains abstract locations of
    type ``t``, contains abstract location ``abs``, or contains abstract
    region ``rhs``, respectively.

  .. cpp:function:: bool containsOfType(Type t) const

    Checks if this abstract region contains an abstract location of type ``t``.

  .. cpp:function:: const std::string format() const

    Returns the string representation of the abstract region.

  .. cpp:function:: Absloc absloc() const

    Returns the contained abstract location.

  .. cpp:function:: Type type() const

    Returns the type of this abstract region.

  .. cpp:function:: size_t size() const

    Returns the size of the region in bits.

  .. cpp:function:: AST::Ptr generator() const

    Returns the address calculation of the memory access.

    .. Note:: Only useful if this abstract region represents a memory location.

  .. cpp:function:: bool isImprecise() const

    Checks if this abstract region represents more than one abstract locations.

.. cpp:class:: Assignment

  Represents data dependencies between :ref:`Abstract Regions <sec:dataflow-abstractions>`.

  An output is an abstract region modified by an instruction. An input is an abstract region
  that is read by an instruction. An instruction may read or write several abstract regions,
  so an instruction can correspond to multiple assignments.

  .. cpp:function:: Assignment(const InstructionAPI::Instruction& i, const Address a, \
                             ParseAPI::Function *f, ParseAPI::Block *b, \
                             const std::vector<AbsRegion> &ins, const AbsRegion &o)

    Creates an Assignment for the instruction ``i``, at address ``a``, from the block ``b``,
    in function ``f`` with input regions ``ins`` and output regions ``o``.

  .. cpp:function:: Assignment(const InstructionAPI::Instruction& i, const Address a, \
                             ParseAPI::Function *f, ParseAPI::Block *b, \
                             const AbsRegion &o)

    Creates an Assignment for the instruction ``i``, at address ``a``, from the block ``b``,
    in function ``f`` with output regions ``o``.

  .. cpp:function:: static Assignment::Ptr makeAssignment(const InstructionAPI::Instruction& i, \
                             const Address a, ParseAPI::Function *f, ParseAPI::Block *b, \
                             const std::vector<AbsRegion> &ins, const AbsRegion &o)

  .. cpp:function:: static Assignment::Ptr makeAssignment(const InstructionAPI::Instruction& i, \
                             const Address a, ParseAPI::Function *f, ParseAPI::Block *b, \
                             const AbsRegion &o)

    Shortcut interfaces for creating Assignments.

  .. cpp:type:: boost::shared_ptr<Assignment> Ptr;

    A reference-counted pointer used to refer to an Assignment.

  .. cpp:function:: const std::vector<AbsRegion> &inputs() const
  .. cpp:function:: std::vector<AbsRegion> &inputs()

    Returns the input abstract regions.

  .. cpp:function:: const AbsRegion &out() const
  .. cpp:function:: AbsRegion &out()

    Returns the output abstract region.

  .. cpp:function:: InstructionAPI::Instruction const& insn() const

    Returns the instruction that contains this assignment.

  .. cpp:function:: Address addr() const

    Returns the address of this assignment.

  .. cpp:function:: ParseAPI::Function *func() const

    Returns the function that contains this assignment.

  .. cpp:function:: ParseAPI::Block *block() const

    Returns the block that contains this assignment.

  .. cpp:function:: const std::string format() const

    Returns a string representation of this assignment.

  .. cpp:function:: friend std::ostream &operator<<(std::ostream &os, const Assignment::Ptr &a)

    Writes a representation of the Assignment to the stream ``os``.

    Implicitly calls :cpp:func:`format`.
