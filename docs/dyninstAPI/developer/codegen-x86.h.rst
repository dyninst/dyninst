.. _`sec:codegen-x86.h`:

codegen-x86.h
#############

.. cpp:namespace:: dev::x86

.. cpp:type:: unsigned char codeBuf_t
.. cpp:type:: unsigned codeBufIndex_t

.. cpp:class:: insnCodeGen

  .. cpp:function:: static void generatePush64(codeGen &gen, Dyninst::Address val)

    More code generation

  .. cpp:function:: static void generateBranch(codeGen &gen, Dyninst::Address from, Dyninst::Address to)

    Code generation

  .. cpp:function:: static void generateBranch(codeGen &gen, int disp)
  .. cpp:function:: static void generateBranch64(codeGen &gen, Dyninst::Address to)
  .. cpp:function:: static void generateBranch32(codeGen &gen, Dyninst::Address to)
  .. cpp:function:: static void generateCall(codeGen &gen, Dyninst::Address from, Dyninst::Address to)
  .. cpp:function:: static void generateNOOP(codeGen &gen, unsigned size = 1)

    We may want to generate an efficient set 'o nops

  .. cpp:function:: static void generateIllegal(codeGen &gen)
  .. cpp:function:: static void generateTrap(codeGen &gen)
  .. cpp:function:: static void generate(codeGen &gen, instruction &insn)
  .. cpp:function:: static bool generate(codeGen &gen, instruction &insn, AddressSpace *addrSpace, Dyninst::Address origAddr, Dyninst::Address newAddr, patchTarget *fallthroughOverride = NULL, patchTarget *targetOverride = NULL)

    And generate an equivalent stream somewhere else... fallthroughOverride and targetOverride are
    used for making the behavior of jumps change. It won't work for jumptables that should be cleared up sometime.

  .. cpp:function:: static bool generateMem(codeGen &gen, instruction &insn, Dyninst::Address origAddr, Dyninst::Address newAddr, Dyninst::Register newLoadReg, Dyninst::Register newStoreReg)
  .. cpp:function:: static bool modifyJump(Dyninst::Address target, NS_x86::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyJcc(Dyninst::Address target, NS_x86::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyCall(Dyninst::Address target, NS_x86::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyData(Dyninst::Address target, NS_x86::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyDisp(signed long newDisp, NS_x86::instruction &insn, codeGen &gen, Dyninst::Architecture arch, Dyninst::Address addr)
