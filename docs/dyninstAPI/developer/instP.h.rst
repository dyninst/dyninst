.. _`sec:instP.h`:

instP.h
#######

Functions that need to be provided by the inst-arch file.

.. cpp:function:: extern void initRegisters()
.. cpp:function:: extern void generateBranch(unsigned char *buffer, unsigned &offset, Dyninst::Address fromAddr, Dyninst::Address toAddr)
.. cpp:function:: extern unsigned generateAndWriteBranch(AddressSpace *proc, Dyninst::Address fromAddr, Dyninst::Address toAddr, unsigned fillSize)
.. cpp:function:: extern int flushPtrace()
.. cpp:function:: extern unsigned saveGPRegister(char *baseInsn, Dyninst::Address &base, Dyninst::Register reg)
.. cpp:function:: extern unsigned saveRestoreRegistersInBaseTramp(AddressSpace *proc, baseTramp *bt, registerSpace *rs)
.. cpp:function:: extern void generateNoopField(unsigned size, unsigned char *buffer)
