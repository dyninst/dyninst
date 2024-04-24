.. _`sec:arch-aarch64.h`:

arch-aarch64.h
##############

.. cpp:namespace:: NS_aarch64

.. cpp:type:: const unsigned int insn_mask

.. cpp:class:: ATOMIC_t

  .. cpp:member:: static insn_mask LD_MASK
  .. cpp:member:: static insn_mask ST_MASK
  .. cpp:member:: static insn_mask LD
  .. cpp:member:: static insn_mask ST

.. cpp:class:: UNCOND_BR_t

  .. cpp:member:: static insn_mask IMM_MASK
  .. cpp:member:: static insn_mask IMM
  .. cpp:member:: static insn_mask IMM_OFFSET_MASK
  .. cpp:member:: static insn_mask IMM_OFFSHIFT
  .. cpp:member:: static insn_mask REG_MASK
  .. cpp:member:: static insn_mask REG
  .. cpp:member:: static insn_mask REG_OFFSET_MASK
  .. cpp:member:: static insn_mask REG_OFFSHIFT

.. cpp:class:: COND_BR_t

  .. cpp:member:: static insn_mask BR_MASK
  .. cpp:member:: static insn_mask CB_MASK
  .. cpp:member:: static insn_mask TB_MASK
  .. cpp:member:: static insn_mask BR
  .. cpp:member:: static insn_mask CB
  .. cpp:member:: static insn_mask TB
  .. cpp:member:: static insn_mask CB_OFFSET_MASK
  .. cpp:member:: static insn_mask TB_OFFSET_MASK
  .. cpp:member:: static insn_mask BR_OFFSET_MASK
  .. cpp:member:: static insn_mask CB_OFFSHIFT
  .. cpp:member:: static insn_mask TB_OFFSHIFT
  .. cpp:member:: static insn_mask BR_OFFSHIFT

.. cpp:union:: instructionUnion

  .. cpp:member:: unsigned char byte[4]
  .. cpp:member:: unsigned int  raw

.. cpp:type:: instructUnion codeBuf_t
.. cpp:type:: unsigned codeBufIndex_t

.. cpp:function:: unsigned int swapBytesIfNeeded(unsigned int i)

  .. warning:: unimplemented

.. cpp:class:: instruction

  .. cpp:function:: instruction()
  .. cpp:function:: instruction(unsigned int raw)
  .. cpp:function:: instruction(const void *ptr)
  .. cpp:function:: instruction(const void *ptr, bool)
  .. cpp:function:: instruction(const instruction &insn)
  .. cpp:function:: instruction(instructUnion &insn)
  .. cpp:function:: instruction *copy() const

  .. warning:: unimplemented

  .. cpp:function:: void clear()
  .. cpp:function:: void setInstruction(codeBuf_t *ptr, Dyninst::Address = 0)

    .. warning:: unimplemented

  .. cpp:function:: void setBits(unsigned int pos, unsigned int len, unsigned int value)
  .. cpp:function:: unsigned int asInt() const
  .. cpp:function:: void setInstruction(unsigned char *ptr, Dyninst::Address = 0)
  .. cpp:function:: static int signExtend(unsigned int i, unsigned int pos)

    i = signed int value to be extended
    pos = the total length of signed value to be extended

  .. cpp:function:: static instructUnion &swapBytes(instructUnion &i)

  .. warning:: unimplemented

  .. cpp:function:: static unsigned size()
  .. cpp:function:: Dyninst::Address getBranchOffset() const
  .. cpp:function:: Dyninst::Address getBranchTargetAddress() const
  .. cpp:function:: void setBranchOffset(Dyninst::Address newOffset)

  .. warning:: unimplemented

  .. cpp:function:: static unsigned jumpSize(Dyninst::Address from, Dyninst::Address to, unsigned addr_width)

    .. warning:: unimplemented

  .. cpp:function:: static unsigned jumpSize(Dyninst::Address disp, unsigned addr_width)

    Returns -1 if we can't do a branch due to architecture limitations

  .. cpp:function:: static unsigned maxJumpSize(unsigned addr_width)

    .. warning:: unimplemented

  .. cpp:function:: static unsigned maxInterFunctionJumpSize(unsigned addr_width)

    .. warning:: unimplemented

  .. cpp:function:: unsigned type() const
  .. cpp:function:: const unsigned char *ptr() const
  .. cpp:function:: unsigned opcode() const

    .. warning:: unimplemented

  .. cpp:function:: bool isInsnType(const unsigned mask, const unsigned match) const
  .. cpp:function:: Dyninst::Address getTarget(Dyninst::Address insnAddr) const
  .. cpp:function:: unsigned spaceToRelocate() const

    .. warning:: unimplemented

  .. cpp:function:: bool getUsedRegs(std::vector<int> &regs)
  .. cpp:function:: bool valid() const
  .. cpp:function:: bool isCall() const

  .. warning:: unimplemented

  .. cpp:function:: static bool isAligned(Dyninst::Address addr)
  .. cpp:function:: bool isBranchReg() const
  .. cpp:function:: bool isCondBranch() const
  .. cpp:function:: bool isUncondBranch() const
  .. cpp:function:: bool isThunk() const
  .. cpp:function:: bool isCleaningRet() const
  .. cpp:function:: bool isAtomicLoad() const
  .. cpp:function:: bool isAtomicStore() const
  .. cpp:function:: unsigned getTargetReg() const
  .. cpp:function:: unsigned getBranchTargetReg() const

