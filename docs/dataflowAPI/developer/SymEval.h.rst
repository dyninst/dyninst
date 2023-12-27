.. _`sec:dev-SymEval.h`:

SymEval.h
=========

.. cpp:namespace:: Dyninst::dev::DataflowAPI

.. cpp:class:: SymEval

  .. cpp:function:: static Retval_t expand(Dyninst::Graph::Ptr slice, DataflowAPI::Result_t &res)

    An AST is used to represent the symbolic expressions of an assignment. A
    symbolic expression AST contains internal node type ``RoseAST``, which
    abstracts the operations performed with its child nodes, and two leaf
    node types: ``VariableAST`` and ``ConstantAST``.

  .. cpp:function:: static bool expandInsn(const InstructionAPI::Instruction& insn, const uint64_t addr, Result_t& res)

    Symbolically evaluate an instruction and assign an AST representation to every written absloc.

.. cpp:class:: Variable

  **An abstract region at a particular address**

  .. cpp:function:: Variable()
  .. cpp:function:: Variable(AbsRegion r)
  .. cpp:function:: Variable(AbsRegion r, Address a)

  .. cpp:function:: const std::string format() const

    Return the string representation of the Variable.

  .. cpp:function:: AbsRegion reg()

    The abstraction region of this Variable.

  .. cpp:function:: Address addr()

    The address of this Variable.

  .. cpp:function:: friend std::ostream& operator<<(std::ostream& stream, const Variable& c)

    Writes the representation of ``c`` to ``stream``.

    Implicitly calls :cpp:function::`format`.

.. cpp:class::  Constant

  **A constant value in code**

  .. cpp:function:: Constant()
  .. cpp:function:: Constant(uint64_t v)
  .. cpp:function:: Constant(uint64_t v, size_t s)

  .. cpp:function:: const std::string format() const

    Returns the string representation of the Constant object.

  .. cpp:function:: uint64_t val()

    Returns the numerical value of this Constant.

  .. cpp:function:: size_t size()

    Returns the size in bits of this Constant.

  .. cpp:function:: Op op()

  .. cpp:function:: friend std::ostream& operator<<(std::ostream& stream, const ROSEOperation& c)

    Writes the representation of ``c`` to ``stream``.

    Implicitly calls :cpp:function::`format`.

.. cpp:class:: BottomAST : public AST

  .. cpp:type:: boost::shared_ptr<bool> Ptr
  .. cpp:function:: BottomAST(bool)
  .. cpp:function:: static Ptr create(bool)
  .. cpp:function:: bool& val() const

.. cpp:class:: ConstantAST : public AST

  .. cpp:type:: boost::shared_ptr<ConstantAST> Ptr
  .. cpp:function:: ConstantAST(Constant)
  .. cpp:function:: static Ptr create(Constant)
  .. cpp:function:: Constant& val() const

.. cpp:class:: VariableAST : public AST

  .. cpp:type:: boost::shared_ptr<VariableAST> Ptr
  .. cpp:function:: VariableAST(Variable)
  .. cpp:function:: static Ptr create(Variable)
  .. cpp:function:: Variable& val() const

.. cpp:class:: RoseAST : public AST

  .. cpp:type:: boost::shared_ptr<RoseAST> Ptr
  .. cpp:function:: RoseAST(ROSEOperation)
  .. cpp:function:: static Ptr create(ROSEOperation)
  .. cpp:function:: ROSEOperation& val() const

.. cpp:struct:: ROSEOperation

  ``ROSEOperation`` defines the following operations and represents the
  semantics of all instructions with these operations.

  .. cpp:enum:: Op

    .. cpp:enumerator:: nullOp

      No operation

    .. cpp:enumerator:: extractOp

      Extract bit ranges from a value

    .. cpp:enumerator:: invertOp

      Flip every bit

    .. cpp:enumerator:: negateOp

      Negate the value

    .. cpp:enumerator:: signExtendOp

      Sign-extend the value

    .. cpp:enumerator:: equalToZeroOp

      Check whether the value is zero or not

    .. cpp:enumerator:: generateMaskOp

      Generate mask

    .. cpp:enumerator:: LSBSetOp

      LSB set op

    .. cpp:enumerator:: MSBSetOp

      MSB set op

    .. cpp:enumerator:: concatOp

      Concatenate two values to form a new value

    .. cpp:enumerator:: andOp

      Bit-wise and operation

    .. cpp:enumerator:: orOp

      Bit-wise or operation

    .. cpp:enumerator:: xorOp

      Bit-wise xor operation

    .. cpp:enumerator:: addOp

      Add operation

    .. cpp:enumerator:: rotateLOp

      Rotate to left operation

    .. cpp:enumerator:: rotateROp

      Rotate to right operation

    .. cpp:enumerator:: shiftLOp

      Shift to left operation

    .. cpp:enumerator:: shiftROp

      Shift to right operation

    .. cpp:enumerator:: shiftRArithOp

      Arithmetic shift to right operation

    .. cpp:enumerator:: derefOp

      Dereference memory operation

    .. cpp:enumerator:: writeRepOp

      Write rep operation

    .. cpp:enumerator:: writeOp

      Write operation

    .. cpp:enumerator:: ifOp

      If operation

    .. cpp:enumerator:: sMultOp

      Signed multiplication operation

    .. cpp:enumerator:: uMultOp

      Unsigned multiplication operation

    .. cpp:enumerator:: sDivOp

      Signed division operation

    .. cpp:enumerator:: sModOp

      Signed modular operation

    .. cpp:enumerator:: uDivOp

      Unsigned division operation

    .. cpp:enumerator:: uModOp

      Unsigned modular operation

    .. cpp:enumerator:: extendOp

      Zero extend operation

    .. cpp:enumerator:: extendMSBOp

      Extend the most significant bit operation
