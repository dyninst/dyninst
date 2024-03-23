.. _`sec-dev:codegen-x86.h`:

codegen-x86.h
#############

.. cpp:namespace:: dev::x86

.. cpp:type:: unsigned char codeBuf_t
.. cpp:type:: unsigned codeBufIndex_t

.. cpp:class:: insnCodeGen

  .. cpp:function:: static void generatePush64(codeGen &gen, Dyninst::Address val)

    Unified the 64-bit push between branch and call

  .. cpp:function:: static void generateBranch(codeGen &gen, Dyninst::Address from, Dyninst::Address to)

    Change the insn at ``addr`` to be a branch to ``newAddr``.

    Used to add multiple tramps to a point.

  .. cpp:function:: static void generateBranch(codeGen &gen, int disp)
  .. cpp:function:: static void generateBranch64(codeGen &gen, Dyninst::Address to)
  .. cpp:function:: static void generateBranch32(codeGen &gen, Dyninst::Address to)
  .. cpp:function:: static void generateCall(codeGen &gen, Dyninst::Address from, Dyninst::Address to)
  .. cpp:function:: static void generateNOOP(codeGen &gen, unsigned size = 1)

    We may want to generate an efficient set 'o nops

  .. cpp:function:: static void generateIllegal(codeGen &gen)
  .. cpp:function:: static void generateTrap(codeGen &gen)
  .. cpp:function:: static void generate(codeGen &gen, instruction &insn)
  .. cpp:function:: static bool generate(codeGen &gen, instruction &insn, AddressSpace *addrSpace, Dyninst::Address origAddr,\
                                         Dyninst::Address newAddr, patchTarget *fallthroughOverride = NULL,\
                                         patchTarget *targetOverride = NULL)

    And generate an equivalent stream somewhere else... fallthroughOverride and targetOverride are
    used for making the behavior of jumps change. It won't work for jumptables that should be cleared up sometime.

  .. cpp:function:: static bool generateMem(codeGen &gen, instruction &insn, Dyninst::Address origAddr, Dyninst::Address newAddr,\
                                            Dyninst::Register newLoadReg, Dyninst::Register newStoreReg)

    The comments and naming schemes in this function assume some familiarity with the IA32/IA32e
    instruction encoding.  If you don't understand this, I suggest you start with Chapter 2 of IA-32
    Intel Architecture Software Developer's Manual, Volume 2a and appendix A of: IA-32 Intel
    Architecture Software Developer's Manual, Volume 2b.

    This function takes an instruction
    that accesses memory, and emits a  copy of that instruction that has the load/store replaces with a
    load/store through a register.  For example, if this function were called with 'loadExpr = r12' on
    the instruction 'mov 12(%rax)->%rbx', we would emit 'mov (%r12)->%rbx'. Note that we strip off any
    displacements, indexs, etc...  The register is assumed to contain the final address that will be
    loaded/stored.

  .. cpp:function:: static bool modifyJump(Dyninst::Address target, NS_x86::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyJcc(Dyninst::Address target, NS_x86::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyCall(Dyninst::Address target, NS_x86::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyData(Dyninst::Address target, NS_x86::instruction &insn, codeGen &gen)
  .. cpp:function:: static bool modifyDisp(signed long newDisp, NS_x86::instruction &insn, codeGen &gen,\
                                           Dyninst::Architecture arch, Dyninst::Address addr)


.. cpp:function:: unsigned copy_prefixes_nosize(const unsigned char *&origInsn, unsigned char *&newInsn, unsigned insnType)

  Copy all prefixes but the Operand-Size and Dyninst::Address-Size prefixes (0x66 and 0x67)

.. cpp:function:: unsigned copy_prefixes_nosize_or_segments(const unsigned char *&origInsn, unsigned char *&newInsn, unsigned insnType) 

  Copy all prefixes but the Operand-Size and Dyninst::Address-Size prefixes (0x66 and 0x67)
  Returns the number of bytes copied
