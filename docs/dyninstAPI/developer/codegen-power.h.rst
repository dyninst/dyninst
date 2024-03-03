.. _`sec-dev:codegen-power.h`:

codegen-power.h
###############

.. cpp:namespace:: dev::power

.. cpp:class:: insnCodeGen

  .. cpp:function:: static instructUnion *insnPtr(codeGen &gen)
  .. cpp:function:: static void generateTrap(codeGen &gen)
  .. cpp:function:: static void generateIllegal(codeGen &gen)
  .. cpp:function:: static void generateBranch(codeGen &gen, long jump_off, bool link = false)
  .. cpp:function:: static void generateBranch(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool link = false)
  .. cpp:function:: static void generateCall(codeGen &gen, Dyninst::Address from, Dyninst::Address to)
  .. cpp:function:: static void generateLongBranch(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool isCall)

    This is a register-stomping, full-range branch. Uses one GPR and either LR or CTR.

    New addition: use liveness information to calculate which registers to use otherwise, trap.

  .. cpp:function:: static void generateBranchViaTrap(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool isCall)

    Using the process trap mapping for a branch

  .. cpp:function:: static void generateLoadReg(codeGen &gen, Dyninst::Register rt, Dyninst::Register ra, Dyninst::Register rb)
  .. cpp:function:: static void generateStoreReg(codeGen &gen, Dyninst::Register rs, Dyninst::Register ra, Dyninst::Register rb)
  .. cpp:function:: static void generateLoadReg64(codeGen &gen, Dyninst::Register rt, Dyninst::Register ra, Dyninst::Register rb)
  .. cpp:function:: static void generateStoreReg64(codeGen &gen, Dyninst::Register rs, Dyninst::Register ra, Dyninst::Register rb)
  .. cpp:function:: static void generateAddReg(codeGen &gen, int op, Dyninst::Register rt, Dyninst::Register ra, Dyninst::Register rb)
  .. cpp:function:: static void generateImm(codeGen &gen, int op, Dyninst::Register rt, Dyninst::Register ra, int immd)
  .. cpp:function:: static void generateMemAccess64(codeGen &gen, int op, int xop, Dyninst::Register r1, Dyninst::Register r2, int immd)
  .. cpp:function:: static void generateLShift(codeGen &gen, Dyninst::Register rs, int shift, Dyninst::Register ra)

    ``rlwinm ra,rs,n,0,31-n``

  .. cpp:function:: static void generateRShift(codeGen &gen, Dyninst::Register rs, int shift, Dyninst::Register ra, bool s)

    ``rlwinm ra,rs,32-n,n,31``

  .. cpp:function:: static void generateLShift64(codeGen &gen, Dyninst::Register rs, int shift, Dyninst::Register ra)

    ``sld ra, rs, rb``

  .. cpp:function:: static void generateRShift64(codeGen &gen, Dyninst::Register rs, int shift, Dyninst::Register ra, bool s)

    ``srd ra, rs, rb``

    This function uses rotate-left to implement right shift. Rotate left 64-n bits is rotating right n
    bits. However, rotation cannot correctly represent signed right shifting. So, this piece of code is
    wrong.

  .. cpp:function:: static void generateNOOP(codeGen &gen, unsigned size = 4)
  .. cpp:function:: static void generateRelOp(codeGen &gen, int cond, int mode, Dyninst::Register rs1, Dyninst::Register rs2, Dyninst::Register rd, bool s)
  .. cpp:function:: static void loadImmIntoReg(codeGen &gen, Dyninst::Register rt, long value)
  .. cpp:function:: static void loadPartialImmIntoReg(codeGen &gen, Dyninst::Register rt, long value)

    Helper method. Fills register with partial value to be completed by an operation with a 16-bit
    signed immediate. Such as loads and stores.

  .. cpp:function:: static void generateMoveFromLR(codeGen &gen, Dyninst::Register rt)
  .. cpp:function:: static void generateMoveToLR(codeGen &gen, Dyninst::Register rs)
  .. cpp:function:: static void generateMoveToCR(codeGen &gen, Dyninst::Register rs)
  .. cpp:function:: static void generate(codeGen &gen, instruction &insn)
  .. cpp:function:: static void write(codeGen &gen, instruction &insn)
  .. cpp:function:: static bool generate(codeGen &gen, instruction &insn, AddressSpace *proc, Dyninst::Address origAddr, Dyninst::Address newAddr, patchTarget *fallthroughOverride = NULL, patchTarget *targetOverride = NULL)
  .. cpp:function:: static bool generateMem(codeGen &gen, instruction &insn, Dyninst::Address origAddr, Dyninst::Address newAddr, Dyninst::Register newLoadReg, Dyninst::Register newStoreReg)

    .. warning:: not implemented

  .. cpp:function:: static int createStackFrame(codeGen &gen, int numRegs, std::vector<Dyninst::Register> &freeReg, std::vector<Dyninst::Register> &excludeReg)

    Routines to createremove a new stack frame for getting scratch registers

  .. cpp:function:: static void removeStackFrame(codeGen &gen)
  .. cpp:function:: static void generateVectorLoad(codeGen &gen, unsigned vectorReg, Dyninst::Register RegAddress)
  .. cpp:function:: static void generateVectorStore(codeGen &gen, unsigned vectorReg, Dyninst::Register RegAddress)
  .. cpp:function:: static bool modifyJump(Dyninst::Address target, NS_power::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyJumpCall(Dyninst::Address target, NS_power::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyJcc(Dyninst::Address target, NS_power::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyCall(Dyninst::Address target, NS_power::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyData(Dyninst::Address target, NS_power::instruction &insn, codeGen &gen)

    This function is used for PC-relative and hence may not be required for PPC. Consider for
    update/removal.

  .. cpp:function:: static void generateMoveToSPR(codeGen &gen, Dyninst::Register toSPR, unsigned sprReg)
  .. cpp:function:: static void generateMoveFromSPR(codeGen &gen, Dyninst::Register toSPR, unsigned sprReg)
  .. cpp:function:: static bool generateBranchTar(codeGen &gen, Dyninst::Register scratch, Dyninst::Address dest, bool isCall)
  .. cpp:function:: static bool generateBranchLR(codeGen &gen, Dyninst::Register scratch, Dyninst::Address dest, bool isCall)

    Generates a branch using LR to the address specified in dest.

    Returns true if this branch type was successfully used.

  .. cpp:function:: static bool generateBranchCTR(codeGen &gen, Dyninst::Register scratch, Dyninst::Address dest, bool isCall)

    Generates a branch using TAR to the address specified in dest.
    Returns true if this branch type was successfully used

  .. cpp:function:: static void saveVectors(codeGen &gen, int startStackOffset)
  .. cpp:function:: static void restoreVectors(codeGen &gen, int startStackOffset)


.. cpp:class:: TrackStacktraces

  .. cpp:member:: std::ofstream _out
  .. cpp:member:: std::set<std::string> _stacks
  .. cpp:function:: TrackStacktraces()
  .. cpp:function:: void Insert(std::string s)
  .. cpp:function:: ~TrackStacktraces()

.. cpp:var:: bool shouldAssertIfInLongBranch = true
.. cpp:var:: bool failedLongBranchLocal = false
.. cpp:var:: std::shared_ptr<TrackStacktraces> _global_stack_track
