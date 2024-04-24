.. _`sec:BPatch_instruction.h`:

BPatch_instruction.h
####################

.. cpp:class:: BPatch_instruction

  .. cpp:member:: static const unsigned int nmaxacc_NP

    maximum number of memory accesses per instruction platform dependent

  .. cpp:function:: BPatch_instruction(internal_instruction *insn, Dyninst::Address _addr)
  .. cpp:function:: virtual ~BPatch_instruction()
  .. cpp:function:: void getInstruction(const unsigned char *&_buffer, unsigned char &_length)
  .. cpp:function:: BPatch_point * getInstPoint()
  .. cpp:function:: BPatch_basicBlock * getParent()
  .. cpp:function:: void * getAddress()
  .. cpp:function:: bool equals(const BPatch_instruction* mp) const
  .. cpp:function:: bool equals(const BPatch_instruction& rp) const
  .. cpp:function:: bool hasALoad() const
  .. cpp:function:: bool hasAStore() const
  .. cpp:function:: bool hasAPrefetch_NP() const
  .. cpp:function:: unsigned int getNumberOfAccesses() const

  .. cpp:function:: bool isALoad(int which = 0) const

    Checks if the memory access is a load (memory is read into a register).

  .. cpp:function:: bool isAStore(int which = 0) const

    Return true if the memory access is write. Some machine instructions may
    both load and store.

  .. cpp:function:: bool isAPrefetch_NP(int which = 0) const

    Return true if memory access is a prefetch (i.e, it has no observable
    effect on user registers). It this returns true, the instruction is
    considered neither load nor store. Prefetches are detected only on IA32.


  .. cpp:function:: bool isConditional_NP(int which = 0) const
  .. cpp:function:: bool isNonTemporal_NP(int which = 0) const

  .. cpp:function:: int prefetchType_NP(int which = 0) const

    If the memory access is a prefetch, this method returns a platform
    specific prefetch type.

  .. cpp:function:: int conditionCode_NP(int which = 0) const



.. cpp:class:: BPatch_register
   
  A **BPatch_register** represents a single register of the mutatee. The
  list of BPatch_registers can be retrieved with the
  BPatch_addressSpace::getRegisters method.

  .. cpp:function:: std::string name() const

    This function returns the canonical name of the register.
