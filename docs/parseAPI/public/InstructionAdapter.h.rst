.. _`sec:InstructionAdapter.h`:

InstructionAdapter.h
####################

.. cpp:namespace:: Dyninst::InsnAdapter

.. cpp:enum:: InstrumentableLevel

  .. cpp:enumerator:: NORMAL

    The function can be instrumented normally with no problems (normal case)

  .. cpp:enumerator:: HAS_BR_INDIR

    The function contains unresolved indirect branches; we have to assume
    these can go anywhere in the function to be safe, so we must instrument
    safely (e.g., with traps)

  .. cpp:enumerator:: UNINSTRUMENTABLE

    The function is flatly uninstrumentable and must not be touched.

.. cpp:class:: InstructionAdapter

  .. cpp:function:: InstructionAdapter(Address start, ParseAPI::CodeObject *o , ParseAPI::CodeRegion* r, InstructionSource * isrc,  ParseAPI::Block *)
  .. cpp:function:: InstructionAdapter(const InstructionAdapter&) = default
  .. cpp:function:: void reset(Address start, ParseAPI::CodeObject *o, ParseAPI::CodeRegion *r, InstructionSource *isrc, ParseAPI::Block *)
  .. cpp:function:: virtual const InstructionAPI::Instruction& getInstruction() const = 0
  .. cpp:function:: virtual bool hasCFT() const = 0
  .. cpp:function:: virtual size_t getSize() const = 0
  .. cpp:function:: virtual bool isFrameSetupInsn() const = 0
  .. cpp:function:: virtual bool isInvalidInsn() const = 0
  .. cpp:function:: virtual bool isAbort() const = 0
  .. cpp:function:: virtual bool isGarbageInsn() const = 0
  .. cpp:function:: virtual void getNewEdges(std::vector<std::pair<Address,ParseAPI::EdgeTypeEnum> >& outEdges, ParseAPI::Function* context, ParseAPI::Block* currBlk, unsigned int num_insns, dyn_hash_map<Address, std::string> *pltFuncs, const std::set<Address> &) const = 0
  .. cpp:function:: virtual bool isDynamicCall() const = 0
  .. cpp:function:: virtual bool isAbsoluteCall() const = 0
  .. cpp:function:: virtual InstrumentableLevel getInstLevel(ParseAPI::Function* context, unsigned int num_insns) const
  .. cpp:function:: virtual ParseAPI::FuncReturnStatus getReturnStatus(ParseAPI::Function* context, unsigned int num_insns) const
  .. cpp:function:: virtual bool hasUnresolvedControlFlow(ParseAPI::Function* context, unsigned int num_insns) const
  .. cpp:function:: virtual bool isNopJump() const
  .. cpp:function:: virtual bool simulateJump() const= 0
  .. cpp:function:: virtual void advance() = 0
  .. cpp:function:: virtual bool retreat() = 0
  .. cpp:function:: virtual bool isNop() const = 0
  .. cpp:function:: virtual bool isLeave() const = 0
  .. cpp:function:: virtual bool isDelaySlot() const = 0
  .. cpp:function:: virtual bool isRelocatable(InstrumentableLevel lvl) const = 0
  .. cpp:function:: virtual Address getAddr() const
  .. cpp:function:: virtual Address getPrevAddr() const
  .. cpp:function:: virtual Address getNextAddr() const
  .. cpp:function:: virtual std::pair<bool, Address>  getCFT() const = 0
  .. cpp:function:: virtual bool isStackFramePreamble() const = 0
  .. cpp:function:: virtual bool savesFP() const = 0
  .. cpp:function:: virtual bool cleansStack() const = 0
  .. cpp:function:: virtual bool isConditional() const = 0
  .. cpp:function:: virtual bool isBranch() const = 0
  .. cpp:function:: virtual bool isInterruptOrSyscall() const = 0
  .. cpp:function:: virtual bool isCall() const = 0
  .. cpp:function:: virtual bool isReturnAddrSave(Address &ret_addr) const = 0
  .. cpp:function:: virtual bool isTailCall(const ParseAPI::Function *, ParseAPI::EdgeTypeEnum type, unsigned int num_insns, const std::set<Address> &) const = 0
