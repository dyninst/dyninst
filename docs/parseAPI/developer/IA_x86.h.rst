.. _`sec:IA_x86.h`:

IA_x86.h
########

.. cpp:namespace:: Dyninst::InsnAdapter

.. cpp:class:: IA_x86 : public IA_IAPI

  .. cpp:function:: IA_x86(Dyninst::InstructionAPI::InstructionDecoder dec_, Address start_,\
                           Dyninst::ParseAPI::CodeObject* o, Dyninst::ParseAPI::CodeRegion* r,\
                           Dyninst::InstructionSource *isrc, Dyninst::ParseAPI::Block * curBlk_)

  .. cpp:function:: IA_x86(const IA_x86 &)
  .. cpp:function:: virtual IA_x86* clone() const
  .. cpp:function:: virtual bool isFrameSetupInsn(Dyninst::InstructionAPI::Instruction) const
  .. cpp:function:: virtual bool isNop() const
  .. cpp:function:: virtual bool isThunk() const

    A thunk is a function composed of the following pair of instructions:

    .. code:: asm

      mov (%esp), REG
      ret

    It has the effect of putting the address following a call to thunk into
    the register, and is used in position independent code.


  .. cpp:function:: virtual bool isTailCall(const ParseAPI::Function* context, ParseAPI::EdgeTypeEnum type,\
                                            unsigned int, const set<Address>& knownTargets) const

  .. cpp:function:: virtual bool savesFP() const
  .. cpp:function:: virtual bool isStackFramePreamble() const
  .. cpp:function:: virtual bool cleansStack() const
  .. cpp:function:: virtual bool sliceReturn(ParseAPI::Block* bit, Address ret_addr, ParseAPI::Function * func) const
  .. cpp:function:: virtual bool isReturnAddrSave(Address& retAddr) const
  .. cpp:function:: virtual bool isReturn(Dyninst::ParseAPI::Function * context, Dyninst::ParseAPI::Block* currBlk) const
  .. cpp:function:: virtual bool isFakeCall() const

    Returns ``true`` if the call leads to:

    - an invalid instruction (or immediately branches/calls to an invalid insn)
    - a block not ending in a return instruction that pops the return address  off of the stack

  .. cpp:function:: virtual bool isIATcall(std::string &) const
  .. cpp:function:: virtual bool isLinkerStub() const
  .. cpp:function:: virtual bool isNopJump() const



.. cpp:class:: leaSimplifyVisitor : public InstructionAPI::Visitor

  Simplify an effective address calulation for 'lea' instruction.

  A common idiom for a multi-byte nop is to perform an effective address
  calculation that results in storing the value of a register into itself.

  Examples:

  .. code::

    lea rax, [rax]
    lea rax, [rax1 + 0]

  This visitor uses a stack and an RPN-style calculation to simplify instances of
  multiplicitive (1) and additive (0) identities. If a binary expression with an
  identity operand is encountered the result is the other value of the expression.
  All other expressions result in the original expression. The final result simplifies
  to either a register expression or some other expression. After applying the visitor
  to both operands, A NOP is then determined by testing if each visitor's result is a
  register and are identical.

  There are special cases that are handled implicitly:

  1. The pseudoregister ``riz`` in ``lea rsi, [rsi+riz1+0x0]`` is an assembly construct
     to indicate a SIB, is not present in the expression not considered to be read. This
     reduces the expression to ``rsi+0x0``.

  2. If the source and destination registers are different sizes, then the instruction is
     not considered a nop. For example, ``lea eax, [rax]``.

  .. cpp:function:: void visit(BinaryFunction *bf) override
  .. cpp:function:: void visit(Immediate *imm) override
  .. cpp:function:: void visit(RegisterAST *reg) override
  .. cpp:function:: void visit(Dereference *deref) override
  .. cpp:function:: const RegisterAST *getRegister() const

    return simplified result RegisterAST or nullptr if not a registerAST

  .. cpp:function:: void reset()
