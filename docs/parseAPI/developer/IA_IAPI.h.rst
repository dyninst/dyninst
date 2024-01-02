.. _`sec:IA_IAPI.h`:

IA_IAPI.h
#########

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:class:: IA_IAPI : public InstructionAdapter

  .. cpp:type:: std::vector<std::pair<Address, Dyninst::InstructionAPI::Instruction>> allInsns_t

  .. cpp:member:: protected Dyninst::InstructionAPI::InstructionDecoder dec

      Decoded instruction cache: contains the linear sequence of instructions decoded by the decoder underlying this adapter.

  .. cpp:member:: protected allInsns_t::iterator curInsnIter
  .. cpp:member:: protected mutable bool validCFT
  .. cpp:member:: protected mutable std::pair<bool, Address> cachedCFT
  .. cpp:member:: protected mutable bool validLinkerStubState
  .. cpp:member:: protected mutable bool cachedLinkerStubState
  .. cpp:member:: protected mutable std::pair<bool, bool> hascftstatus
  .. cpp:member:: protected mutable std::map<ParseAPI::EdgeTypeEnum, bool> tailCalls
  .. cpp:member:: protected static std::once_flag ptrInit
  .. cpp:member:: protected static std::map<Architecture, Dyninst::InstructionAPI::RegisterAST::Ptr> framePtr
  .. cpp:member:: protected static std::map<Architecture, Dyninst::InstructionAPI::RegisterAST::Ptr> stackPtr
  .. cpp:member:: protected static std::map<Architecture, Dyninst::InstructionAPI::RegisterAST::Ptr> thePC
  .. cpp:member:: protected static std::map<Address, bool> thunkAtTarget
  .. cpp:member:: protected allInsns_t allInsns

  .. cpp:function:: protected static void initASTs()
  .. cpp:function:: InstructionAPI::Instruction  current_instruction()
  .. cpp:function:: IA_IAPI(Dyninst::InstructionAPI::InstructionDecoder dec_, Address start_, Dyninst::ParseAPI::CodeObject* o, Dyninst::ParseAPI::CodeRegion* r, Dyninst::InstructionSource *isrc, Dyninst::ParseAPI::Block * curBlk_)
  .. cpp:function:: IA_IAPI(const IA_IAPI &)
  .. cpp:function:: IA_IAPI &operator=(const IA_IAPI &r)
  .. cpp:function:: static IA_IAPI* makePlatformIA_IAPI(Dyninst::Architecture arch, Dyninst::InstructionAPI::InstructionDecoder dec_, Address start_, Dyninst::ParseAPI::CodeObject* o, Dyninst::ParseAPI::CodeRegion* r, Dyninst::InstructionSource *isrc, Dyninst::ParseAPI::Block * curBlk_)
  .. cpp:function:: virtual IA_IAPI* clone() const = 0
  .. cpp:function:: virtual void reset(Dyninst::InstructionAPI::InstructionDecoder dec_, Address start, ParseAPI::CodeObject *o, ParseAPI::CodeRegion *r, InstructionSource *isrc, ParseAPI::Block *)
  .. cpp:function:: virtual const Dyninst::InstructionAPI::Instruction& getInstruction() const
  .. cpp:function:: virtual bool hasCFT() const
  .. cpp:function:: virtual size_t getSize() const
  .. cpp:function:: virtual bool isFrameSetupInsn() const
  .. cpp:function:: virtual bool isAbort() const
  .. cpp:function:: virtual bool isInvalidInsn() const
  .. cpp:function:: virtual bool isGarbageInsn() const

       ``true`` for insns indicative of bad parse, for defensive mode.

  .. cpp:function:: virtual void getNewEdges(std::vector<std::pair<Address, Dyninst::ParseAPI::EdgeTypeEnum>>& outEdges, \
                                             Dyninst::ParseAPI::Function * context, Dyninst::ParseAPI::Block * currBlk, \
                                             unsigned int num_insns, dyn_hash_map<Address, std::string> *pltFuncs, \
                                             const set<Address>& knownTargets) const
  .. cpp:function:: virtual InstrumentableLevel getInstLevel(Dyninst::ParseAPI::Function *, unsigned int num_insns ) const
  .. cpp:function:: virtual bool isDynamicCall() const
  .. cpp:function:: virtual bool isAbsoluteCall() const
  .. cpp:function:: virtual bool simulateJump() const
  .. cpp:function:: virtual void advance()
  .. cpp:function:: virtual bool retreat()
  .. cpp:function:: virtual bool isNop() const = 0
  .. cpp:function:: virtual bool isLeave() const
  .. cpp:function:: virtual bool isDelaySlot() const
  .. cpp:function:: virtual bool isRelocatable(InstrumentableLevel lvl) const
  .. cpp:function:: virtual bool isTailCall(const ParseAPI::Function *, Dyninst::ParseAPI::EdgeTypeEnum, unsigned int, const std::set<Address> &) const = 0
  .. cpp:function:: virtual std::pair<bool, Address> getCFT() const
  .. cpp:function:: virtual bool isStackFramePreamble() const = 0
  .. cpp:function:: virtual bool savesFP() const = 0
  .. cpp:function:: virtual bool cleansStack() const = 0
  .. cpp:function:: virtual bool isConditional() const
  .. cpp:function:: virtual bool isBranch() const
  .. cpp:function:: virtual bool isInterruptOrSyscall() const
  .. cpp:function:: virtual bool isSyscall() const
  .. cpp:function:: virtual bool isInterrupt() const
  .. cpp:function:: virtual bool isCall() const
  .. cpp:function:: virtual bool isReturnAddrSave(Address &ret_addr) const = 0
  .. cpp:function:: virtual bool isNopJump() const = 0
  .. cpp:function:: virtual bool sliceReturn(ParseAPI::Block* bit, Address ret_addr, ParseAPI::Function * func) const = 0
  .. cpp:function:: virtual bool isIATcall(std::string &calleeName) const = 0
  .. cpp:function:: virtual bool isThunk() const = 0
  .. cpp:function:: virtual bool isIndirectJump() const

  .. cpp:function:: protected virtual bool isRealCall() const
  .. cpp:function:: protected virtual bool parseJumpTable(Dyninst::ParseAPI::Function * currFunc, Dyninst::ParseAPI::Block* currBlk, std::vector<std::pair< Address, Dyninst::ParseAPI::EdgeTypeEnum > >& outEdges) const
  .. cpp:function:: protected virtual bool isIPRelativeBranch() const
  .. cpp:function:: protected virtual bool isFrameSetupInsn(Dyninst::InstructionAPI::Instruction i) const = 0
  .. cpp:function:: protected virtual bool isReturn(Dyninst::ParseAPI::Function *, Dyninst::ParseAPI::Block* currBlk) const = 0
  .. cpp:function:: protected virtual bool isFakeCall() const = 0
  .. cpp:function:: protected virtual bool isLinkerStub() const = 0
  .. cpp:function:: protected virtual bool isSysEnter() const
  .. cpp:function:: protected virtual void parseSyscall(std::vector<std::pair<Address, Dyninst::ParseAPI::EdgeTypeEnum> >& outEdges) const
  .. cpp:function:: protected virtual void parseSysEnter(std::vector<std::pair<Address, Dyninst::ParseAPI::EdgeTypeEnum> >& outEdges) const
  .. cpp:function:: protected std::pair<bool, Address> getFallthrough() const
  .. cpp:function:: protected const InstructionAPI::Instruction & curInsn() const;

