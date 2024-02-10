.. _`sec:emitter.h`:

emitter.h
#########


.. cpp:class:: Emitter

  .. cpp:function:: virtual ~Emitter()
  .. cpp:function:: virtual codeBufIndex_t emitIf(Dyninst::Register expr_reg, Dyninst::Register target, RegControl rc, codeGen &gen) = 0
  .. cpp:function:: virtual void emitOp(unsigned opcode, Dyninst::Register dest, Dyninst::Register src1, Dyninst::Register src2, codeGen &gen) = 0
  .. cpp:function:: virtual void emitOpImm(unsigned opcode1, unsigned opcode2, Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src2imm, codeGen &gen) = 0
  .. cpp:function:: virtual void emitRelOp(unsigned op, Dyninst::Register dest, Dyninst::Register src1, Dyninst::Register src2, codeGen &gen, bool s) = 0
  .. cpp:function:: virtual void emitRelOpImm(unsigned op, Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src2imm, codeGen &gen, bool s) = 0
  .. cpp:function:: virtual void emitDiv(Dyninst::Register dest, Dyninst::Register src1, Dyninst::Register src2, codeGen &gen, bool s) = 0
  .. cpp:function:: virtual void emitTimesImm(Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src2imm, codeGen &gen) = 0
  .. cpp:function:: virtual void emitDivImm(Dyninst::Register dest, Dyninst::Register src1, Dyninst::RegValue src2imm, codeGen &gen, bool s) = 0
  .. cpp:function:: virtual void emitLoad(Dyninst::Register dest, Dyninst::Address addr, int size, codeGen &gen) = 0
  .. cpp:function:: virtual void emitLoadConst(Dyninst::Register dest, Dyninst::Address imm, codeGen &gen) = 0
  .. cpp:function:: virtual void emitLoadIndir(Dyninst::Register dest, Dyninst::Register addr_reg, int size, codeGen &gen) = 0
  .. cpp:function:: virtual bool emitCallRelative(Dyninst::Register, Dyninst::Address, Dyninst::Register, codeGen &) = 0
  .. cpp:function:: virtual bool emitLoadRelative(Dyninst::Register dest, Dyninst::Address offset, Dyninst::Register base, int size, codeGen &gen) = 0
  .. cpp:function:: virtual void emitLoadShared(opCode op, Dyninst::Register dest, const image_variable *var, bool is_local, int size, codeGen &gen, Dyninst::Address offset) = 0

  ......

  .. rubric::
    These implicitly use the stored originalnon-inst value

  .. cpp:function:: virtual void emitLoadFrameAddr(Dyninst::Register dest, Dyninst::Address offset, codeGen &gen) = 0
  .. cpp:function:: virtual void emitLoadOrigFrameRelative(Dyninst::Register dest, Dyninst::Address offset, codeGen &gen) = 0
  .. cpp:function:: virtual void emitLoadOrigRegRelative(Dyninst::Register dest, Dyninst::Address offset, Dyninst::Register base, codeGen &gen, bool store) = 0
  .. cpp:function:: virtual void emitLoadOrigRegister(Dyninst::Address register_num, Dyninst::Register dest, codeGen &gen) = 0
  .. cpp:function:: virtual void emitStoreOrigRegister(Dyninst::Address register_num, Dyninst::Register dest, codeGen &gen) = 0
  .. cpp:function:: virtual void emitStore(Dyninst::Address addr, Dyninst::Register src, int size, codeGen &gen) = 0
  .. cpp:function:: virtual void emitStoreIndir(Dyninst::Register addr_reg, Dyninst::Register src, int size, codeGen &gen) = 0
  .. cpp:function:: virtual void emitStoreFrameRelative(Dyninst::Address offset, Dyninst::Register src, Dyninst::Register scratch, int size, codeGen &gen) = 0
  .. cpp:function:: virtual void emitStoreRelative(Dyninst::Register source, Dyninst::Address offset, Dyninst::Register base, int size, codeGen &gen) = 0
  .. cpp:function:: virtual void emitStoreShared(Dyninst::Register source, const image_variable *var, bool is_local, int size, codeGen &gen) = 0
  .. cpp:function:: virtual bool emitMoveRegToReg(Dyninst::Register src, Dyninst::Register dest, codeGen &gen) = 0
  .. cpp:function:: virtual bool emitMoveRegToReg(registerSlot *src, registerSlot *dest, codeGen &gen) = 0
  .. cpp:function:: virtual Dyninst::Register emitCall(opCode op, codeGen &gen, const std::vector<AstNodePtr> &operands, bool noCost, func_instance *callee) = 0
  .. cpp:function:: virtual void emitGetRetVal(Dyninst::Register dest, bool addr_of, codeGen &gen) = 0
  .. cpp:function:: virtual void emitGetRetAddr(Dyninst::Register dest, codeGen &gen) = 0
  .. cpp:function:: virtual void emitGetParam(Dyninst::Register dest, Dyninst::Register param_num, instPoint::Type pt_type, opCode op, bool addr_of, codeGen &gen) = 0
  .. cpp:function:: virtual void emitASload(int ra, int rb, int sc, long imm, Dyninst::Register dest, int stackShift, codeGen &gen) = 0
  .. cpp:function:: virtual void emitCSload(int ra, int rb, int sc, long imm, Dyninst::Register dest, codeGen &gen) = 0
  .. cpp:function:: virtual void emitPushFlags(codeGen &gen) = 0
  .. cpp:function:: virtual void emitRestoreFlags(codeGen &gen, unsigned offset) = 0
  .. cpp:function:: virtual void emitRestoreFlagsFromStackSlot(codeGen &gen) = 0
  .. cpp:function:: virtual bool emitBTSaves(baseTramp *bt, codeGen &gen) = 0
  .. cpp:function:: virtual bool emitBTRestores(baseTramp *bt, codeGen &gen) = 0
  .. cpp:function:: virtual void emitStoreImm(Dyninst::Address addr, int imm, codeGen &gen, bool noCost) = 0
  .. cpp:function:: virtual void emitAddSignedImm(Dyninst::Address addr, int imm, codeGen &gen, bool noCost) = 0
  .. cpp:function:: virtual bool emitPush(codeGen &, Dyninst::Register) = 0
  .. cpp:function:: virtual bool emitPop(codeGen &, Dyninst::Register) = 0
  .. cpp:function:: virtual bool emitAdjustStackPointer(int index, codeGen &gen) = 0
  .. cpp:function:: virtual bool clobberAllFuncCall(registerSpace *rs, func_instance *callee) = 0
  .. cpp:function:: Dyninst::Address getInterModuleFuncAddr(func_instance *func, codeGen &gen)
  .. cpp:function:: Dyninst::Address getInterModuleVarAddr(const image_variable *var, codeGen &gen)
  .. cpp:function:: virtual bool emitPLTCall(func_instance *, codeGen &)
  .. cpp:function:: virtual bool emitPLTJump(func_instance *, codeGen &)
  .. cpp:function:: virtual bool emitTOCJump(block_instance *, codeGen &)
  .. cpp:function:: virtual bool emitTOCCall(block_instance *, codeGen &)
