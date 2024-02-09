.. _`sec:emit-aarch64.h`:

emit-aarch64.h
##############


.. cpp:class:: EmitterAARCH64 : public Emitter

  .. cpp:function:: virtual ~EmitterAARCH64()
  .. cpp:function:: virtual codeBufIndex_t emitIf(Dyninst::Register, Dyninst::Register, RegControl, codeGen &)
  .. cpp:function:: virtual void emitOp(unsigned, Dyninst::Register, Dyninst::Register, Dyninst::Register, codeGen &)
  .. cpp:function:: virtual void emitOpImm(unsigned, unsigned, Dyninst::Register, Dyninst::Register, Dyninst::RegValue, codeGen &)
  .. cpp:function:: virtual void emitRelOp(unsigned, Dyninst::Register, Dyninst::Register, Dyninst::Register, codeGen &, bool)
  .. cpp:function:: virtual void emitRelOpImm(unsigned, Dyninst::Register, Dyninst::Register, Dyninst::RegValue, codeGen &, bool)
  .. cpp:function:: virtual void emitDiv(Dyninst::Register, Dyninst::Register, Dyninst::Register, codeGen &, bool)
  .. cpp:function:: virtual void emitTimesImm(Dyninst::Register, Dyninst::Register, Dyninst::RegValue, codeGen &)
  .. cpp:function:: virtual void emitDivImm(Dyninst::Register, Dyninst::Register, Dyninst::RegValue, codeGen &, bool)
  .. cpp:function:: virtual void emitLoad(Dyninst::Register, Dyninst::Address, int, codeGen &)
  .. cpp:function:: virtual void emitLoadConst(Dyninst::Register, Dyninst::Address, codeGen &)
  .. cpp:function:: virtual void emitLoadIndir(Dyninst::Register, Dyninst::Register, int, codeGen &)
  .. cpp:function:: virtual bool emitCallRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, codeGen &)
  .. cpp:function:: virtual bool emitLoadRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, int, codeGen &)
  .. cpp:function:: virtual void emitLoadShared(opCode op, Dyninst::Register dest, const image_variable *var, bool is_local, int size, codeGen &gen, Dyninst::Address offset)
  .. cpp:function:: virtual void emitLoadFrameAddr(Dyninst::Register, Dyninst::Address, codeGen &)

  ......

  .. rubric::
      These implicitly use the stored original/non-inst value

  .. cpp:function:: virtual void emitLoadOrigFrameRelative(Dyninst::Register, Dyninst::Address, codeGen &)
  .. cpp:function:: virtual void emitLoadOrigRegRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, codeGen &, bool)
  .. cpp:function:: virtual void emitLoadOrigRegister(Dyninst::Address, Dyninst::Register, codeGen &)
  .. cpp:function:: virtual void emitStore(Dyninst::Address, Dyninst::Register, int, codeGen &)
  .. cpp:function:: virtual void emitStoreIndir(Dyninst::Register, Dyninst::Register, int, codeGen &)
  .. cpp:function:: virtual void emitStoreFrameRelative(Dyninst::Address, Dyninst::Register, Dyninst::Register, int, codeGen &)
  .. cpp:function:: virtual void emitStoreRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, int, codeGen &)
  .. cpp:function:: virtual void emitStoreShared(Dyninst::Register source, const image_variable *var, bool is_local, int size, codeGen &gen)
  .. cpp:function:: virtual void emitStoreOrigRegister(Dyninst::Address, Dyninst::Register, codeGen &)
  .. cpp:function:: virtual bool emitMoveRegToReg(Dyninst::Register, Dyninst::Register, codeGen &)
  .. cpp:function:: protected virtual bool emitCallInstruction(codeGen &, func_instance *, bool, Dyninst::Address)

    This one we actually use now.

  .. cpp:function:: protected virtual Dyninst::Register emitCallReplacement(opCode, codeGen &, bool, func_instance *)

.. cpp:class:: EmitterAARCH64Dyn : public EmitterAARCH64

  .. cpp:function:: virtual bool emitTOCCall(block_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitTOCJump(block_instance *dest, codeGen &gen)
  .. cpp:function:: virtual ~EmitterAARCH64Dyn()
  .. cpp:function:: private bool emitTOCCommon(block_instance *dest, bool call, codeGen &gen)

.. cpp:class:: EmitterAARCH64Stat : public EmitterAARCH64

  .. cpp:function:: virtual ~EmitterAARCH64Stat()
  .. cpp:function:: virtual bool emitPLTCall(func_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitPLTJump(func_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitTOCCall(block_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitTOCJump(block_instance *dest, codeGen &gen)
  .. cpp:function:: protected virtual bool emitCallInstruction(codeGen &, func_instance *, bool, Dyninst::Address)
  .. cpp:function:: protected virtual Dyninst::Register emitCallReplacement(opCode, codeGen &, bool, func_instance *)
  .. cpp:function:: private bool emitPLTCommon(func_instance *dest, bool call, codeGen &gen)
  .. cpp:function:: private bool emitTOCCommon(block_instance *dest, bool call, codeGen &gen)

.. cpp:class:: EmitterAARCH64SaveRegs

  .. cpp:function:: virtual ~EmitterAARCH64SaveRegs()
  .. cpp:function:: unsigned saveGPRegisters(codeGen &gen, registerSpace *theRegSpace, int offset, int numReqGPRs = -1)
  .. cpp:function:: unsigned saveFPRegisters(codeGen &gen, registerSpace *theRegSpace, int offset)
  .. cpp:function:: unsigned saveSPRegisters(codeGen &gen, registerSpace *, int offset, bool force_save)
  .. cpp:function:: void createFrame(codeGen &gen)
  .. cpp:function:: private void saveSPR(codeGen &gen, Dyninst::Register scratchReg, int sprnum, int stkOffset)
  .. cpp:function:: private void saveFPRegister(codeGen &gen, Dyninst::Register reg, int save_off)

.. cpp:class:: EmitterAARCH64RestoreRegs

  .. cpp:function:: virtual ~EmitterAARCH64RestoreRegs()
  .. cpp:function:: unsigned restoreGPRegisters(codeGen &gen, registerSpace *theRegSpace, int offset)
  .. cpp:function:: unsigned restoreFPRegisters(codeGen &gen, registerSpace *theRegSpace, int offset)
  .. cpp:function:: unsigned restoreSPRegisters(codeGen &gen, registerSpace *, int offset, int force_save)
  .. cpp:function:: void tearFrame(codeGen &gen)
  .. cpp:function:: void restoreSPR(codeGen &gen, Dyninst::Register scratchReg, int sprnum, int stkOffset)
  .. cpp:function:: void restoreFPRegister(codeGen &gen, Dyninst::Register reg, int save_off)
