.. _`sec:dwarfResult.h`:

dwarfResult.h
#############

.. cpp:namespace:: Dyninst::DwarfDyninst


.. cpp:class:: DwarfResult

  **An interface for building representations of Dwarf expressions**

  In concrete mode, we have access to process state and can calculate a value. In symbolic mode, we
  lack this information and instead produce a representation.

  .. cpp:function:: DwarfResult(Architecture a)
  .. cpp:function:: DwarfResult(const DwarfResult &) = default
  .. cpp:function:: virtual ~DwarfResult() = default
  .. cpp:function:: virtual void pushReg(Dyninst::MachRegister reg) = 0
  .. cpp:function:: virtual void readReg(Dyninst::MachRegister reg) = 0
  .. cpp:function:: virtual void pushUnsignedVal(Dyninst::MachRegisterVal constant) = 0
  .. cpp:function:: virtual void pushSignedVal(Dyninst::MachRegisterVal constant) = 0
  .. cpp:function:: virtual void pushOp(Operator op) = 0
  .. cpp:function:: virtual void pushOp(Operator op, long long ref) = 0
  .. cpp:function:: virtual void pushFrameBase() = 0

    The frame base is the logical top of the stack as reported in the function's debug info

  .. cpp:function:: virtual void pushCFA() = 0

    And the CFA is the top of the stack as reported in call frame information.

  .. cpp:function:: bool err() const
  .. cpp:function:: virtual bool eval(MachRegisterVal &val) = 0

    The conditional branch needs an immediate eval mechanism

  .. cpp:member:: protected Architecture arch
  .. cpp:member:: protected bool error


.. cpp:enum:: DwarfResult::Operator

  .. cpp:enumerator:: Add
  .. cpp:enumerator:: Sub
  .. cpp:enumerator:: Mul
  .. cpp:enumerator:: Div
  .. cpp:enumerator:: Mod
  .. cpp:enumerator:: Deref
  .. cpp:enumerator:: Pick
  .. cpp:enumerator:: Drop
  .. cpp:enumerator:: And
  .. cpp:enumerator:: Or
  .. cpp:enumerator:: Not
  .. cpp:enumerator:: Xor
  .. cpp:enumerator:: Abs
  .. cpp:enumerator:: GE
  .. cpp:enumerator:: LE
  .. cpp:enumerator:: GT
  .. cpp:enumerator:: LT
  .. cpp:enumerator:: Eq
  .. cpp:enumerator:: Neq
  .. cpp:enumerator:: Shl
  .. cpp:enumerator:: Shr
  .. cpp:enumerator:: ShrArith


.. cpp:class:: SymbolicDwarfResult : public DwarfResult

  .. cpp:function:: SymbolicDwarfResult(VariableLocation &v, Architecture a)
  .. cpp:function:: virtual void pushReg(Dyninst::MachRegister reg)
  .. cpp:function:: virtual void readReg(Dyninst::MachRegister reg)
  .. cpp:function:: virtual void pushUnsignedVal(Dyninst::MachRegisterVal constant)
  .. cpp:function:: virtual void pushSignedVal(Dyninst::MachRegisterVal constant)
  .. cpp:function:: virtual void pushOp(Operator op)
  .. cpp:function:: virtual void pushOp(Operator op, long long ref)
  .. cpp:function:: virtual void pushFrameBase()

    DWARF logical "frame base", which may be the result of an expression in itself.

    TODO: figure out what info we need to carry around so we can compute it...

  .. cpp:function:: virtual void pushCFA()
  .. cpp:function:: VariableLocation &val()
  .. cpp:function:: virtual bool eval(MachRegisterVal &)
  .. cpp:member:: private std::stack<MachRegisterVal> operands
  .. cpp:member:: private VariableLocation &var


.. cpp:class:: ConcreteDwarfResult : public DwarfResult

  .. cpp:function:: ConcreteDwarfResult(ProcessReader *r, Architecture a, Address p, Dwarf *d, Elf *e)
  .. cpp:function:: ConcreteDwarfResult()
  .. cpp:function:: virtual ~ConcreteDwarfResult()
  .. cpp:function:: virtual void pushReg(Dyninst::MachRegister reg)
  .. cpp:function:: virtual void readReg(Dyninst::MachRegister reg)
  .. cpp:function:: virtual void pushUnsignedVal(Dyninst::MachRegisterVal constant)
  .. cpp:function:: virtual void pushSignedVal(Dyninst::MachRegisterVal constant)
  .. cpp:function:: virtual void pushOp(Operator op)
  .. cpp:function:: virtual void pushOp(Operator op, long long ref)
  .. cpp:function:: virtual void pushFrameBase()

    DWARF logical "frame base", which may be the result of an expression in itself.

    TODO: figure out what info we need to carry around so we can compute it...

  .. cpp:function:: virtual void pushCFA()
  .. cpp:function:: MachRegisterVal val()
  .. cpp:function:: bool eval(MachRegisterVal &v)
  .. cpp:member:: private ProcessReader *reader{}
  .. cpp:member:: private Address pc{}

    For getting access to other expressions

  .. cpp:member:: private Dwarf *dbg{}
  .. cpp:member:: private Elf *dbg_eh_frame{}
  .. cpp:member:: private std::vector<Dyninst::MachRegisterVal> operands

    Dwarf lets you access within the "stack", so we model it as a vector.

  .. cpp:function:: private MachRegisterVal peek(int index)
  .. cpp:function:: private void pop(int num)
  .. cpp:function:: private void popRange(int start, int end)
  .. cpp:function:: private void push(MachRegisterVal v)
