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

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitDiv(Dyninst::Register, Dyninst::Register, Dyninst::Register, codeGen &, bool)

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitTimesImm(Dyninst::Register, Dyninst::Register, Dyninst::RegValue, codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitDivImm(Dyninst::Register, Dyninst::Register, Dyninst::RegValue, codeGen &, bool)

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitLoad(Dyninst::Register, Dyninst::Address, int, codeGen &)
  .. cpp:function:: virtual void emitLoadConst(Dyninst::Register, Dyninst::Address, codeGen &)
  .. cpp:function:: virtual void emitLoadIndir(Dyninst::Register, Dyninst::Register, int, codeGen &)
  .. cpp:function:: virtual bool emitCallRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual bool emitLoadRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, int, codeGen &)
  .. cpp:function:: virtual void emitLoadShared(opCode op, Dyninst::Register dest, const image_variable *var, bool is_local, int size, codeGen &gen, Dyninst::Address offset)
  .. cpp:function:: virtual void emitLoadFrameAddr(Dyninst::Register, Dyninst::Address, codeGen &)

    .. warning:: Not implemented

  ......

  .. rubric::
      These implicitly use the stored original/non-inst value

  .. cpp:function:: virtual void emitLoadOrigFrameRelative(Dyninst::Register, Dyninst::Address, codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitLoadOrigRegRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, codeGen &, bool)

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitLoadOrigRegister(Dyninst::Address, Dyninst::Register, codeGen &)
  .. cpp:function:: virtual void emitStore(Dyninst::Address, Dyninst::Register, int, codeGen &)
  .. cpp:function:: virtual void emitStoreIndir(Dyninst::Register, Dyninst::Register, int, codeGen &)
  .. cpp:function:: virtual void emitStoreFrameRelative(Dyninst::Address, Dyninst::Register, Dyninst::Register, int, codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitStoreRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, int, codeGen &)
  .. cpp:function:: virtual void emitStoreShared(Dyninst::Register source, const image_variable *var, bool is_local, int size, codeGen &gen)
  .. cpp:function:: virtual void emitStoreOrigRegister(Dyninst::Address, Dyninst::Register, codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual bool emitMoveRegToReg(Dyninst::Register, Dyninst::Register, codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen)

    .. warning:: Not implemented

  .. cpp:function:: virtual Address emitMovePCToReg(Register, codeGen &gen)
  .. cpp:function:: virtual Register emitCall(opCode, codeGen &, const std::vector <AstNodePtr> &, bool, func_instance *)

    .. caution:: Experimental

  .. cpp:function:: virtual void emitGetRetVal(Register, bool, codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitGetRetAddr(Register, codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitGetParam(Register, Register, instPoint::Type, opCode, bool, codeGen &)
  .. cpp:function:: virtual void emitASload(int, int, int, long, Register, int, codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitCSload(int, int, int, long, Register, codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitPushFlags(codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitRestoreFlags(codeGen &, unsigned)

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitRestoreFlagsFromStackSlot(codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual bool emitBTSaves(baseTramp *, codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual bool emitBTRestores(baseTramp *, codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitStoreImm(Address, int, codeGen &, bool)

    .. warning:: Not implemented

  .. cpp:function:: virtual void emitAddSignedImm(Address, int, codeGen &, bool)

    .. warning:: Not implemented

  .. cpp:function:: virtual int Register_DWARFtoMachineEnc(int)

    .. warning:: Not implemented

  .. cpp:function:: virtual bool emitPush(codeGen &, Register)

    .. warning:: Not implemented

  .. cpp:function:: virtual bool emitPop(codeGen &, Register)

    .. warning:: Not implemented

  .. cpp:function:: virtual bool emitAdjustStackPointer(int, codeGen &)

    .. warning:: Not implemented

  .. cpp:function:: virtual bool clobberAllFuncCall(registerSpace *rs, func_instance *callee)

    Recursive function that goes to where our instrumentation is calling to figure out what registers
    are clobbered there, and in any function that it calls, to a certain depth ... at which point we
    clobber everything

    Update-12/06, njr, since we're going to a cached system we are just going to
    look at the first level and not do recursive, since we would have to also store and reexamine every
    call out instead of doing it on the fly like before

  .. cpp:function:: protected virtual bool emitCallInstruction(codeGen &, func_instance *, bool, Dyninst::Address)

    Generates call instruction sequence for all AARCH64-based systems under dynamic instrumentation.

    This should be able to stomp on the link register (LR) and TOC register (r2), as they were saved by
    Emitter::emitCall() as necessary.

    .. warning:: Not implemented

  .. cpp:function:: protected virtual Dyninst::Register emitCallReplacement(opCode, codeGen &, bool, func_instance *)

    .. warning:: Not implemented


.. cpp:class:: EmitterAARCH64Dyn : public EmitterAARCH64

  .. cpp:function:: virtual bool emitTOCCall(block_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitTOCJump(block_instance *dest, codeGen &gen)
  .. cpp:function:: virtual ~EmitterAARCH64Dyn()
  .. cpp:function:: private bool emitTOCCommon(block_instance *dest, bool call, codeGen &gen)

    .. warning:: Not implemented


.. cpp:class:: EmitterAARCH64Stat : public EmitterAARCH64

  .. cpp:function:: virtual ~EmitterAARCH64Stat()
  .. cpp:function:: virtual bool emitPLTCall(func_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitPLTJump(func_instance *dest, codeGen &gen)
  .. cpp:function:: virtual bool emitTOCCall(block_instance *dest, codeGen &gen)

    .. warning:: Not implemented

  .. cpp:function:: virtual bool emitTOCJump(block_instance *dest, codeGen &gen)

    .. warning:: Not implemented

  .. cpp:function:: protected virtual bool emitCallInstruction(codeGen &, func_instance *, bool, Dyninst::Address)

    .. warning:: Not implemented

  .. cpp:function:: protected virtual Dyninst::Register emitCallReplacement(opCode, codeGen &, bool, func_instance *)

    .. warning:: Not implemented

  .. cpp:function:: private bool emitPLTCommon(func_instance *dest, bool call, codeGen &gen)

    .. warning:: Not implemented

  .. cpp:function:: private bool emitTOCCommon(block_instance *dest, bool call, codeGen &gen)

    .. warning:: Not implemented

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
