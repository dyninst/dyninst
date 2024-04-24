RoseInsnFactory.h
#################

.. cpp:namespace:: Dyninst::DataflowAPI


.. cpp:class:: RoseInsnFactory

  .. cpp:type:: protected boost::shared_ptr<InstructionAPI::Expression> ExpressionPtr
  .. cpp:type:: protected boost::shared_ptr<InstructionAPI::Instruction> InstructionPtr
  .. cpp:member:: protected uint64_t _addr = 0
  .. cpp:function:: RoseInsnFactory(void)
  .. cpp:function:: virtual ~RoseInsnFactory(void)
  .. cpp:function:: virtual SgAsmInstruction *convert(const InstructionAPI::Instruction &insn, uint64_t addr)
  .. cpp:function:: protected virtual SgAsmInstruction *createInsn() = 0
  .. cpp:function:: protected virtual void setOpcode(SgAsmInstruction *insn, Dyninst::entryID opcode, Dyninst::prefixEntryID prefix, std::string mnem) = 0
  .. cpp:function:: protected virtual void setSizes(SgAsmInstruction *insn) = 0
  .. cpp:function:: protected virtual bool handleSpecialCases(Dyninst::entryID opcode, SgAsmInstruction *rinsn, SgAsmOperandList *roperands) = 0
  .. cpp:function:: protected virtual void massageOperands(const InstructionAPI::Instruction &insn, std::vector<InstructionAPI::Operand> &operands) = 0
  .. cpp:function:: protected virtual SgAsmExpression *convertOperand(const ExpressionPtr expression, int64_t addr, size_t insnSize)
  .. cpp:function:: protected virtual Architecture arch()

.. cpp:class:: RoseInsnX86Factory : public RoseInsnFactory

  .. cpp:function:: RoseInsnX86Factory(Architecture arch)
  .. cpp:function:: virtual ~RoseInsnX86Factory()
  .. cpp:member:: private Architecture a
  .. cpp:function:: private virtual SgAsmInstruction *createInsn()
  .. cpp:function:: private virtual void setOpcode(SgAsmInstruction *insn, Dyninst::entryID opcode, Dyninst::prefixEntryID prefix, std::string mnem)
  .. cpp:function:: private virtual void setSizes(SgAsmInstruction *insn)
  .. cpp:function:: private virtual bool handleSpecialCases(Dyninst::entryID opcode, SgAsmInstruction *rinsn, SgAsmOperandList *roperands)
  .. cpp:function:: private virtual void massageOperands(const InstructionAPI::Instruction &insn, std::vector<InstructionAPI::Operand> &operands)
  .. cpp:function:: private X86InstructionKind convertKind(Dyninst::entryID opcode, Dyninst::prefixEntryID prefix)
  .. cpp:function:: private virtual Architecture arch()

.. cpp:class:: RoseInsnPPCFactory : public RoseInsnFactory

  .. cpp:function:: RoseInsnPPCFactory(void)
  .. cpp:function:: virtual ~RoseInsnPPCFactory(void)
  .. cpp:function:: private virtual SgAsmInstruction *createInsn()
  .. cpp:function:: private virtual void setOpcode(SgAsmInstruction *insn, Dyninst::entryID opcode, Dyninst::prefixEntryID prefix, std::string mnem)
  .. cpp:function:: private virtual void setSizes(SgAsmInstruction *insn)
  .. cpp:function:: private virtual bool handleSpecialCases(Dyninst::entryID opcode, SgAsmInstruction *rinsn, SgAsmOperandList *roperands)
  .. cpp:function:: private virtual void massageOperands(const InstructionAPI::Instruction &insn, std::vector<InstructionAPI::Operand> &operands)
  .. cpp:function:: private PowerpcInstructionKind convertKind(Dyninst::entryID opcode, std::string mnem)
  .. cpp:function:: private PowerpcInstructionKind makeRoseBranchOpcode(Dyninst::entryID iapi_opcode, bool isAbsolute, bool isLink)
  .. cpp:function:: private virtual Architecture arch()
  .. cpp:member:: private PowerpcInstructionKind kind

.. cpp:class:: RoseInsnArmv8Factory : public RoseInsnFactory

  .. cpp:function:: RoseInsnArmv8Factory(Architecture arch)
  .. cpp:function:: virtual ~RoseInsnArmv8Factory()
  .. cpp:member:: private Architecture a
  .. cpp:function:: private virtual SgAsmInstruction *createInsn()
  .. cpp:function:: private virtual void setOpcode(SgAsmInstruction *insn, Dyninst::entryID opcode, Dyninst::prefixEntryID prefix, std::string mnem)
  .. cpp:function:: private virtual bool handleSpecialCases(Dyninst::entryID opcode, SgAsmInstruction *rinsn, SgAsmOperandList *roperands)
  .. cpp:function:: private virtual void massageOperands(const InstructionAPI::Instruction &insn, std::vector<InstructionAPI::Operand> &operands)
  .. cpp:function:: private virtual void setSizes(SgAsmInstruction *insn)
  .. cpp:function:: private ARMv8InstructionKind convertKind(Dyninst::entryID opcode)
  .. cpp:function:: private virtual Architecture arch()

.. cpp:class:: RoseInsnAMDGPUFactory : public RoseInsnFactory

  .. cpp:function:: RoseInsnAMDGPUFactory(Architecture arch)
  .. cpp:function:: virtual ~RoseInsnAMDGPUFactory()
  .. cpp:member:: private Architecture a
  .. cpp:function:: private virtual SgAsmInstruction *createInsn()
  .. cpp:function:: private virtual void setOpcode(SgAsmInstruction *insn, Dyninst::entryID opcode, Dyninst::prefixEntryID prefix, std::string mnem)
  .. cpp:function:: private virtual bool handleSpecialCases(Dyninst::entryID opcode, SgAsmInstruction *rinsn, SgAsmOperandList *roperands)
  .. cpp:function:: private virtual void massageOperands(const InstructionAPI::Instruction &insn, std::vector<InstructionAPI::Operand> &operands)
  .. cpp:function:: private virtual void setSizes(SgAsmInstruction *insn)
  .. cpp:function:: private AMDGPUInstructionKind convertKind(Dyninst::entryID opcode)

    This function should just translate each instructionAPI opcode to a rose equivalent version

  .. cpp:function:: private virtual Architecture arch()
