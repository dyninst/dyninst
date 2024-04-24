.. _`sec:emit-power.h`:

emit-power.h
############


.. cpp:class:: EmitterPOWER : public Emitter

  .. cpp:function:: virtual ~EmitterPOWER()
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
    These implicitly use the stored originalnon-inst value

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
  .. cpp:function:: virtual bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen)
  .. cpp:function:: virtual Dyninst::Address emitMovePCToReg(Dyninst::Register, codeGen &gen)
  .. cpp:function:: virtual Dyninst::Register emitCall(opCode, codeGen &, const std::vector<AstNodePtr> &, bool, func_instance *)

    This one we actually use now.

  .. cpp:function:: virtual void emitGetRetVal(Dyninst::Register, bool, codeGen &)
  .. cpp:function:: virtual void emitGetRetAddr(Dyninst::Register, codeGen &)
  .. cpp:function:: virtual void emitGetParam(Dyninst::Register, Dyninst::Register, instPoint::Type, opCode, bool, codeGen &)
  .. cpp:function:: virtual void emitASload(int, int, int, long, Dyninst::Register, int, codeGen &)
  .. cpp:function:: virtual void emitCSload(int, int, int, long, Dyninst::Register, codeGen &)
  .. cpp:function:: virtual void emitPushFlags(codeGen &)
  .. cpp:function:: virtual void emitRestoreFlags(codeGen &, unsigned)
  .. cpp:function:: virtual void emitRestoreFlagsFromStackSlot(codeGen &)
  .. cpp:function:: virtual bool emitBTSaves(baseTramp *, codeGen &)
  .. cpp:function:: virtual bool emitBTRestores(baseTramp *, codeGen &)
  .. cpp:function:: virtual void emitStoreImm(Dyninst::Address, int, codeGen &, bool)
  .. cpp:function:: virtual void emitAddSignedImm(Dyninst::Address, int, codeGen &, bool)
  .. cpp:function:: virtual int Register_DWARFtoMachineEnc(int)
  .. cpp:function:: virtual bool emitPush(codeGen &, Dyninst::Register)
  .. cpp:function:: virtual bool emitPop(codeGen &, Dyninst::Register)
  .. cpp:function:: virtual bool emitAdjustStackPointer(int, codeGen &)
  .. cpp:function:: virtual bool clobberAllFuncCall(registerSpace *rs, func_instance *callee)
  .. cpp:function:: virtual Dyninst::Register emitCallReplacement(opCode, codeGen &, bool, func_instance *)
  .. cpp:function:: void emitCallWithSaves(codeGen &gen, Dyninst::Address dest, bool saveToc, bool saveLR, bool saveR12)
  .. cpp:function:: protected virtual bool emitCallInstruction(codeGen &, func_instance *, bool, Dyninst::Address)


.. cpp:class:: EmitterPOWER32Dyn : public EmitterPOWER

  .. cpp:function:: virtual ~EmitterPOWER32Dyn()


.. cpp:class:: EmitterPOWER32Stat : public EmitterPOWER

  .. cpp:function:: virtual ~EmitterPOWER32Stat()
  .. cpp:function:: virtual bool emitPLTCall(func_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitPLTJump(func_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitTOCCall(block_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitTOCJump(block_instance *dest, codeGen &gen)
  .. cpp:function:: protected virtual bool emitCallInstruction(codeGen &, func_instance *, bool, Dyninst::Address)
  .. cpp:function:: protected virtual Dyninst::Register emitCallReplacement(opCode, codeGen &, bool, func_instance *)
  .. cpp:function:: private bool emitPLTCommon(func_instance *dest, bool call, codeGen &gen)
  .. cpp:function:: private bool emitTOCCommon(block_instance *dest, bool call, codeGen &gen)


.. cpp:class:: EmitterPOWER64Dyn : public EmitterPOWER

  .. cpp:function:: virtual bool emitTOCCall(block_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitTOCJump(block_instance *dest, codeGen &gen)
  .. cpp:function:: virtual ~EmitterPOWER64Dyn()
  .. cpp:function:: private bool emitTOCCommon(block_instance *dest, bool call, codeGen &gen)


.. cpp:class:: EmitterPOWER64Stat : public EmitterPOWER

  .. cpp:function:: virtual ~EmitterPOWER64Stat()
  .. cpp:function:: virtual bool emitPLTCall(func_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitPLTJump(func_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitTOCCall(block_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitTOCJump(block_instance *dest, codeGen &gen)
  .. cpp:function:: protected virtual bool emitCallInstruction(codeGen &, func_instance *, bool, Dyninst::Address)
  .. cpp:function:: protected virtual Dyninst::Register emitCallReplacement(opCode, codeGen &, bool, func_instance *)
  .. cpp:function:: private bool emitPLTCommon(func_instance *dest, bool call, codeGen &gen)
  .. cpp:function:: private bool emitTOCCommon(block_instance *dest, bool call, codeGen &gen)
