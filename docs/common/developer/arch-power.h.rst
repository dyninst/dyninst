.. _`sec:arch-power.h`:

arch-power.h
############

.. cpp:namespace:: NS_power

.. cpp:union:: instructUnion
 
  .. cpp:member:: unsigned char byte[4]
  .. cpp:member:: unsigned int raw

  .. cpp:type:: instructUnion codeBuf_t
  .. cpp:type:: unsigned codeBufIndex_t

.. cpp:class:: instruction

  .. cpp:function:: instruction()
  .. cpp:function:: instruction(unsigned int raw)
  .. cpp:function:: instruction(const void *ptr)
  .. cpp:function:: instruction(const void *ptr, bool)
  .. cpp:function:: instruction *copy() const
  .. cpp:function:: void clear()
  .. cpp:function:: void setInstruction(codeBuf_t *ptr, Dyninst::Address = 0)
  .. cpp:function:: void setBits(unsigned int pos, unsigned int len, unsigned int value)
  .. cpp:function:: unsigned int asInt() const
  .. cpp:function:: void setInstruction(unsigned char *ptr, Dyninst::Address = 0)
  .. cpp:function:: static int signExtend(unsigned int i, unsigned int pos)
  .. cpp:function:: static instructUnion &swapBytes(instructUnion &i)
  .. cpp:function:: static unsigned size()
  .. cpp:function:: Dyninst::Address getBranchOffset() const
  .. cpp:function:: void setBranchOffset(Dyninst::Address newOffset)

    TODO: argument *needs* to be an int, or ``ABS()`` doesn't work.

  .. cpp:function:: static unsigned jumpSize(Dyninst::Address from, Dyninst::Address to, unsigned addr_width)
  .. cpp:function:: static unsigned jumpSize(Dyninst::Address disp, unsigned addr_width)
  .. cpp:function:: static unsigned maxJumpSize(unsigned addr_width)
  .. cpp:function:: static unsigned maxInterFunctionJumpSize(unsigned addr_width)
  .. cpp:function:: unsigned type() const
  .. cpp:function:: const unsigned char *ptr() const
  .. cpp:function:: unsigned opcode() const
  .. cpp:function:: bool isInsnType(const unsigned mask, const unsigned match) const
  .. cpp:function:: Dyninst::Address getTarget(Dyninst::Address insnAddr) const
  .. cpp:function:: unsigned spaceToRelocate() const
  .. cpp:function:: bool getUsedRegs(std::vector<int> &regs)
  .. cpp:function:: bool valid() const
  .. cpp:function:: bool isCall() const
  .. cpp:function:: static bool isAligned(Dyninst::Address addr)
  .. cpp:function:: bool isCondBranch() const
  .. cpp:function:: bool isUncondBranch() const
  .. cpp:function:: bool isThunk() const
  .. cpp:function:: bool isCleaningRet() const
