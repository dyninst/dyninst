InstructionDecoderImpl.h
========================

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:class:: InstructionDecoderImpl

  .. cpp:type:: boost::shared_ptr<InstructionDecoderImpl> Ptr
  .. cpp:function:: InstructionDecoderImpl(Architecture a)
  .. cpp:function:: virtual Instruction decode(InstructionDecoder::buffer& b)
  .. cpp:function:: virtual void doDelayedDecode(const Instruction* insn_to_complete) = 0
  .. cpp:function:: virtual void setMode(bool is64) = 0
  .. cpp:function:: static Ptr makeDecoderImpl(Architecture a)

  .. cpp:function:: protected virtual bool decodeOperands(const Instruction* insn_to_complete) = 0
  .. cpp:function:: protected virtual void decodeOpcode(InstructionDecoder::buffer&) = 0
  .. cpp:function:: protected virtual Expression::Ptr makeAddExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType)
  .. cpp:function:: protected virtual Expression::Ptr makeMultiplyExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType)
  .. cpp:function:: protected virtual Expression::Ptr makeLeftShiftExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType)
  .. cpp:function:: protected virtual Expression::Ptr makeRightArithmeticShiftExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType)
  .. cpp:function:: protected virtual Expression::Ptr makeRightLogicalShiftExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType)
  .. cpp:function:: protected virtual Expression::Ptr makeRightRotateExpression(Expression::Ptr lhs, Expression::Ptr rhs, Result_Type resultType)
  .. cpp:function:: protected virtual Expression::Ptr makeDereferenceExpression(Expression::Ptr addrToDereference, Result_Type resultType)
  .. cpp:function:: protected virtual Expression::Ptr makeRegisterExpression(MachRegister reg, uint32_t num_elements = 1)
  .. cpp:function:: protected virtual Expression::Ptr makeRegisterExpression(MachRegister reg, unsigned int start , unsigned int end)
  .. cpp:function:: protected virtual Expression::Ptr makeMaskRegisterExpression(MachRegister reg)
  .. cpp:function:: protected virtual Expression::Ptr makeRegisterExpression(MachRegister reg, Result_Type extendFrom)
  .. cpp:function:: protected virtual Result_Type makeSizeType(unsigned int opType) = 0
  .. cpp:function:: protected virtual Expression::Ptr makeTernaryExpression(Expression::Ptr cond, Expression::Ptr first, Expression::Ptr second, Result_Type resultType)
  .. cpp:function:: protected boost::shared_ptr<Instruction> makeInstruction(entryID opcode, const char* mnem, unsigned int decodedSize, const unsigned char* raw)

  .. cpp:member:: protected Operation m_Operation
  .. cpp:member:: protected Architecture m_Arch
