.. _`sec:ABI.h`:

ABI.h
#####

.. cpp:namespace:: Dyninst

.. cpp:class:: ABI

  .. cpp:function:: const bitArray &getCallReadRegisters() const
  .. cpp:function:: const bitArray &getCallWrittenRegisters() const
  .. cpp:function:: const bitArray &getReturnReadRegisters() const
  .. cpp:function:: const bitArray &getReturnRegisters() const
  .. cpp:function:: const bitArray &getParameterRegisters() const
  .. cpp:function:: const bitArray &getSyscallReadRegisters() const
  .. cpp:function:: const bitArray &getSyscallWrittenRegisters() const
  .. cpp:function:: const bitArray &getAllRegs() const
  .. cpp:function:: int getIndex(MachRegister machReg)
  .. cpp:function:: std::map<MachRegister,int>* getIndexMap()
  .. cpp:function:: static void initialize32()
  .. cpp:function:: static void initialize64()
  .. cpp:function:: static ABI* getABI(int addr_width)
  .. cpp:function:: bitArray getBitArray()
