.. _`sec:IA_x86.h`:

IA_x86.h
########

.. cpp:namespace:: Dyninst::InsnAdapter

.. cpp:class:: IA_x86 : public IA_IAPI

  .. cpp:function:: IA_x86(Dyninst::InstructionAPI::InstructionDecoder dec_, Address start_, Dyninst::ParseAPI::CodeObject* o, Dyninst::ParseAPI::CodeRegion* r, Dyninst::InstructionSource *isrc, Dyninst::ParseAPI::Block * curBlk_)
  .. cpp:function:: IA_x86(const IA_x86 &)
  .. cpp:function:: virtual IA_x86* clone() const
  .. cpp:function:: virtual bool isFrameSetupInsn(Dyninst::InstructionAPI::Instruction) const
  .. cpp:function:: virtual bool isNop() const
  .. cpp:function:: virtual bool isThunk() const
  .. cpp:function:: virtual bool isTailCall(const ParseAPI::Function* context, ParseAPI::EdgeTypeEnum type, unsigned int, const set<Address>& knownTargets) const
  .. cpp:function:: virtual bool savesFP() const
  .. cpp:function:: virtual bool isStackFramePreamble() const
  .. cpp:function:: virtual bool cleansStack() const
  .. cpp:function:: virtual bool sliceReturn(ParseAPI::Block* bit, Address ret_addr, ParseAPI::Function * func) const
  .. cpp:function:: virtual bool isReturnAddrSave(Address& retAddr) const
  .. cpp:function:: virtual bool isReturn(Dyninst::ParseAPI::Function * context, Dyninst::ParseAPI::Block* currBlk) const
  .. cpp:function:: virtual bool isFakeCall() const
  .. cpp:function:: virtual bool isIATcall(std::string &) const
  .. cpp:function:: virtual bool isLinkerStub() const
  .. cpp:function:: virtual bool isNopJump() const
