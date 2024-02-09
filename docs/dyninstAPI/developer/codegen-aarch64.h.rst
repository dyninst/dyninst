.. _`sec:codegen-aarch64.h`:

codegen-aarch64.h
#################


.. cpp:class:: insnCodeGen

  .. cpp:function:: static instructUnion *insnPtr(codeGen &gen)
  .. cpp:function:: static void generateTrap(codeGen &gen)
  .. cpp:function:: static void generateIllegal(codeGen &gen)
  .. cpp:function:: static void generateBranch(codeGen &gen, long jump_off, bool link = false)
  .. cpp:function:: static void generateBranch(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool link = false)
  .. cpp:function:: static void generateCall(codeGen &gen, Dyninst::Address from, Dyninst::Address to)
  .. cpp:function:: static void generateLongBranch(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool isCall)
  .. cpp:function:: static void generateBranchViaTrap(codeGen &gen, Dyninst::Address from, Dyninst::Address to, bool isCall)

    Using the process trap mapping for a branch

  .. cpp:function:: static void generateConditionalBranch(codeGen &gen, Dyninst::Address to, unsigned opcode, bool s)

    Generate conditional branch

  .. cpp:function:: static void generateMemAccess(codeGen &gen, LoadStore accType, Dyninst::Register r1, Dyninst::Register r2, int immd, unsigned size, IndexMode im = Post)

    LDRSTR (immediate) immd in the range -256 to 255

  .. cpp:function:: static void generateMemAccessFP(codeGen &gen, LoadStore accType, Dyninst::Register rt, Dyninst::Register rn, int immd, int size, bool is128bit, IndexMode im = Offset)
  .. cpp:function:: static inline void loadImmIntoReg(codeGen &gen, Dyninst::Register rt, Dyninst::Address value)
  .. cpp:function:: static void saveRegister(codeGen &gen, Dyninst::Register r, int sp_offset, IndexMode im = Offset)
  .. cpp:function:: static void restoreRegister(codeGen &gen, Dyninst::Register r, int sp_offset, IndexMode im = Offset)
  .. cpp:function:: static void generateLoadReg(codeGen &gen, Dyninst::Register rt, Dyninst::Register ra, Dyninst::Register rb)
  .. cpp:function:: static void generateAddSubShifted(codeGen &gen, ArithOp op, int shift, int imm6, Dyninst::Register rm, Dyninst::Register rn, Dyninst::Register rd, bool is64bit)
  .. cpp:function:: static void generateAddSubImmediate(codeGen &gen, ArithOp op, int shift, int imm12, Dyninst::Register rn, Dyninst::Register rd, bool is64bit)
  .. cpp:function:: static void generateMul(codeGen &gen, Dyninst::Register rm, Dyninst::Register rn, Dyninst::Register rd, bool is64bit)
  .. cpp:function:: static void generateDiv(codeGen &gen, Dyninst::Register rm, Dyninst::Register rn, Dyninst::Register rd, bool is64bit, bool s)
  .. cpp:function:: static void generateBitwiseOpShifted(codeGen &gen, BitwiseOp op, int shift, Dyninst::Register rm, int imm6, Dyninst::Register rn, Dyninst::Register rd, bool is64bit)

  .. cpp:function:: static void generateMove(codeGen &gen, int imm16, int shift, Dyninst::Register rd, MoveOp movOp)

    This is for MOVK, MOVN, and MOVZ. For MOV use the other generateMove()

  .. cpp:function:: static void generateMove(codeGen &gen, Dyninst::Register rd, Dyninst::Register rm, bool is64bit = true)

    This is for MOV, which is an alias for ORR. See ARMv8 Documentation.

  .. cpp:function:: static void generateMoveSP(codeGen &gen, Dyninst::Register rn, Dyninst::Register rd, bool is64bit)
  .. cpp:function:: static Dyninst::Register moveValueToReg(codeGen &gen, long int val, std::vector<Dyninst::Register> *exclude = NULL)
  .. cpp:function:: static void generate(codeGen &gen, instruction &insn)
  .. cpp:function:: static void generate(codeGen &gen, instruction &insn, unsigned position)

    Copy instruction at position in codeGen buffer

  .. cpp:function:: static void write(codeGen &gen, instruction &insn)
  .. cpp:function:: static bool generate(codeGen &gen, instruction &insn, AddressSpace *proc, Dyninst::Address origAddr, Dyninst::Address newAddr, patchTarget *fallthroughOverride = NULL, patchTarget *targetOverride = NULL)
  .. cpp:function:: static void generateNOOP(codeGen &gen, unsigned size = 4)
  .. cpp:function:: static bool modifyJump(Dyninst::Address target, NS_aarch64::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyJcc(Dyninst::Address target, NS_aarch64::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyCall(Dyninst::Address target, NS_aarch64::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyData(Dyninst::Address target, NS_aarch64::instruction &insn, codeGen &gen)

  ......

  .. rubric:: TODO

  .. cpp:function:: static void generateStoreReg(codeGen &gen, Dyninst::Register rs, Dyninst::Register ra, Dyninst::Register rb)
  .. cpp:function:: static void generateLoadReg64(codeGen &gen, Dyninst::Register rt, Dyninst::Register ra, Dyninst::Register rb)
  .. cpp:function:: static void generateStoreReg64(codeGen &gen, Dyninst::Register rs, Dyninst::Register ra, Dyninst::Register rb)
  .. cpp:function:: static void generateLShift(codeGen &gen, Dyninst::Register rs, int shift, Dyninst::Register ra)
  .. cpp:function:: static void generateRShift(codeGen &gen, Dyninst::Register rs, int shift, Dyninst::Register ra)
  .. cpp:function:: static void generateLShift64(codeGen &gen, Dyninst::Register rs, int shift, Dyninst::Register ra)
  .. cpp:function:: static void generateRShift64(codeGen &gen, Dyninst::Register rs, int shift, Dyninst::Register ra)
  .. cpp:function:: static void generateRelOp(codeGen &gen, int cond, int mode, Dyninst::Register rs1, Dyninst::Register rs2, Dyninst::Register rd)
  .. cpp:function:: static void loadPartialImmIntoReg(codeGen &gen, Dyninst::Register rt, long value)
  .. cpp:function:: static void generateMoveFromLR(codeGen &gen, Dyninst::Register rt)
  .. cpp:function:: static void generateMoveToLR(codeGen &gen, Dyninst::Register rs)
  .. cpp:function:: static void generateMoveToCR(codeGen &gen, Dyninst::Register rs)
  .. cpp:function:: static bool generateMem(codeGen &gen, instruction &insn, Dyninst::Address origAddr, Dyninst::Address newAddr, Dyninst::Register newLoadReg, Dyninst::Register newStoreReg)

  .. cpp:function:: static int createStackFrame(codeGen &gen, int numRegs, std::vector<Dyninst::Register> &freeReg, std::vector<Dyninst::Register> &excludeReg)

    TODO  Routines to createremove a new stack frame for getting scratch registers

  .. cpp:function:: static void removeStackFrame(codeGen &gen)



.. cpp:enum:: MoveOp 

  .. cpp:enumerator:: MovOp_MOVK
  .. cpp:enumerator:: MovOp_MOVN
  .. cpp:enumerator:: MovOp_MOVZ


.. cpp:enum:: insnCodeGen::LoadStore

  .. cpp:enumerator:: Load
  .. cpp:enumerator:: Store


.. cpp:enum:: insnCodeGen::ArithOp 

  .. cpp:enumerator:: Add
  .. cpp:enumerator:: Sub


.. cpp:enum:: insnCodeGen::BitwiseOp 

  .. cpp:enumerator:: Or
  .. cpp:enumerator:: And
  .. cpp:enumerator:: Eor

.. cpp:enum:: insnCodeGen::IndexMode

  .. cpp:enumerator:: Post
  .. cpp:enumerator:: Pre
  .. cpp:enumerator:: Offset
