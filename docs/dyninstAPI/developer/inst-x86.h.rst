.. _`sec:inst-x86.h`:

inst-x86.h
##########

.. cpp:namespace:: dev::x86

We don't use the machine registers to store temporaries,
but "virtual registers" that are located on the stack.

The stack frame for a tramp is::

  ebp->    saved ebp (4 bytes)
  ebp-4:   128-byte space for 32 virtual registers (32*4 bytes)
  ebp-132: saved registers (8*4 bytes)
  ebp-164: saved flags registers (4 bytes)
  ebp-168: (MT only) thread index

  The temporaries are assigned numbers from 1 so that it is easier
  to refer to them: -(reg*4)[ebp]. So the first reg is -4[ebp].

  We are using a fixed number of temporaries now (32), but we could
  change to using an arbitrary number.


.. cpp:function:: void emitAddressingMode(unsigned base, unsigned index, unsigned int scale, Dyninst::RegValue disp, int reg_opcode, codeGen &gen)
.. cpp:function:: void emitAddressingMode(unsigned base, Dyninst::RegValue disp, unsigned reg_opcode, codeGen &gen)
.. cpp:function:: void emitOpRegReg(unsigned opcode, RealRegister dest, RealRegister src, codeGen &gen)

  low-level code generation functions

.. cpp:function:: void emitOpExtReg(unsigned opcode, unsigned char ext, RealRegister reg, codeGen &gen)
.. cpp:function:: void emitOpRegRM(unsigned opcode, RealRegister dest, RealRegister base, int disp, codeGen &gen)
.. cpp:function:: void emitOpExtRegImm8(int opcode, char ext, RealRegister dest, unsigned char imm, codeGen &gen)
.. cpp:function:: void emitOpExtRegImm(int opcode, int opcode2, RealRegister dest, int imm, codeGen &gen)
.. cpp:function:: void emitOpRMReg(unsigned opcode, RealRegister base, int disp, RealRegister src, codeGen &gen)
.. cpp:function:: void emitOpRegRegImm(unsigned opcode, RealRegister dest, RealRegister src, unsigned imm, codeGen &gen)
.. cpp:function:: void emitOpRegImm(int opcode, RealRegister dest, int imm, codeGen &gen)
.. cpp:function:: void emitOpSegRMReg(unsigned opcode, RealRegister dest, RealRegister src, int disp, codeGen &gen)
.. cpp:function:: void emitMovRegToReg(RealRegister dest, RealRegister src, codeGen &gen)
.. cpp:function:: void emitMovIRegToReg(RealRegister dest, RealRegister src, codeGen &gen)
.. cpp:function:: void emitMovPCRMToReg(RealRegister dest, int offset, codeGen &gen, bool deref_result = true)
.. cpp:function:: void emitMovMToReg(RealRegister dest, int disp, codeGen &gen)
.. cpp:function:: void emitMovMBToReg(RealRegister dest, int disp, codeGen &gen)
.. cpp:function:: void emitMovMWToReg(RealRegister dest, int disp, codeGen &gen)
.. cpp:function:: void emitMovRegToM(int disp, RealRegister src, codeGen &gen)
.. cpp:function:: void emitMovRegToMB(int disp, RealRegister dest, codeGen &gen)
.. cpp:function:: void emitMovRegToMW(int disp, RealRegister dest, codeGen &gen)
.. cpp:function:: void emitMovImmToReg(RealRegister dest, int imm, codeGen &gen)
.. cpp:function:: void emitMovImmToRM(RealRegister base, int disp, int imm, codeGen &gen)
.. cpp:function:: void emitMovRegToRM(RealRegister base, int disp, RealRegister src, codeGen &gen)
.. cpp:function:: void emitMovRMToReg(RealRegister dest, RealRegister base, int disp, codeGen &gen)
.. cpp:function:: void emitMovImmToMem(Dyninst::Address maddr, int imm, codeGen &gen)
.. cpp:function:: void emitPushImm(unsigned int imm, codeGen &gen)
.. cpp:function:: void emitSaveO(codeGen &gen)
.. cpp:function:: void emitRestoreO(codeGen &gen)
.. cpp:function:: void emitSimpleInsn(unsigned opcode, codeGen &gen)
.. cpp:function:: void emitAddRegImm32(RealRegister dest, int imm, codeGen &gen)
.. cpp:function:: void emitSubRegReg(RealRegister dest, RealRegister src, codeGen &gen)
.. cpp:function:: void emitSHL(RealRegister dest, unsigned char pos, codeGen &gen)
.. cpp:function:: void restoreGPRtoGPR(RealRegister reg, RealRegister dest, codeGen &gen)
.. cpp:function:: Dyninst::Register restoreGPRtoReg(RealRegister reg, codeGen &gen, RealRegister *dest_to_use = NULL)
.. cpp:function:: void emitLEA(RealRegister base, RealRegister index, unsigned int scale, Dyninst::RegValue disp, RealRegister dest, codeGen &gen)
.. cpp:function:: bool emitPush(RealRegister reg, codeGen &gen)
.. cpp:function:: bool emitPop(RealRegister reg, codeGen &gen)
.. cpp:function:: void emitJump(unsigned disp32, codeGen &gen)
.. cpp:function:: void emitJccR8(int condition_code, char jump_offset, codeGen &gen)
.. cpp:function:: void emitJcc(int condition, int offset, codeGen &gen, bool willRegen = true)
.. cpp:function:: void emitAddMemImm32(Dyninst::Address dest, int imm, codeGen &gen)
.. cpp:function:: void emitCallRel32(unsigned disp32, codeGen &gen)
.. cpp:function:: void emitJmpMC(int condition, int offset, codeGen &gen)
.. cpp:function:: unsigned char cmovOpcodeFromRelOp(unsigned op, bool s)

  helper functions for emitters

.. cpp:function:: unsigned char jccOpcodeFromRelOp(unsigned op, bool s)
.. cpp:function:: bool xmmCapable()

  function that uses cpuid instruction to figure out whether the processor uses XMM registers

.. cpp:function:: void emitBTRegRestores32(baseTramp *bti, codeGen &gen)


.. cpp:struct:: stackItem

  .. cpp:member:: stackItem_t item{}
  .. cpp:member:: RealRegister reg
  .. cpp:function:: stackItem(stackItem_t i)
  .. cpp:function:: stackItem(RealRegister r)
  .. cpp:function:: stackItem()


.. cpp:struct:: stackItemLocation

  .. cpp:member:: RealRegister reg
  .. cpp:member:: int offset
  .. cpp:function:: stackItemLocation(RealRegister r, int o)



.. cpp:enum:: stackItem::stackItem_t

  .. cpp:enumerator:: reg_item
  .. cpp:enumerator:: stacktop
  .. cpp:enumerator:: framebase


.. cpp:function:: stackItemLocation getHeightOf(stackItem sitem, codeGen &gen)


.. code:: cpp

  #define NUM_VIRTUAL_REGISTERS (32)   /* number of virtual registers */
  #define NUM_FPR_REGISTERS (1)
  #define IA32_FPR_VIRTUAL_REGISTER (NUM_VIRTUAL_REGISTERS + 1)
  #define IA32_FLAG_VIRTUAL_REGISTER (IA32_FPR_VIRTUAL_REGISTER + 1)

  /* Add one for the REG_MT_POS 'reserved' reg */
  #define TRAMP_FRAME_SIZE ((NUM_VIRTUAL_REGISTERS+1)*4)

  // offset from EBP of the saved EAX for a tramp
  #define SAVED_EAX_OFFSET (10*4-4)
  #define SAVED_EFLAGS_OFFSET (SAVED_EAX_OFFSET+4)

  // Undefine REG_MT_POS, basically
  #define REG_MT_POS NUM_VIRTUAL_REGISTERS

  #define IA32_STACK_ALIGNMENT     16
  #define AMD64_STACK_ALIGNMENT    32  // This is extremely conservative, 16 may be enough.
  #define AMD64_RED_ZONE         0x80

  /*
     Function arguments are in the stack and are addressed with a displacement
     from EBP. EBP points to the saved EBP, EBP+4 is the saved return address,
     EBP+8 is the first parameter.
     TODO: what about far calls?
   */
  #define FUNC_PARAM_OFFSET (8+(10*4)+STACK_PAD_CONSTANT)
  #define CALLSITE_PARAM_OFFSET (4+(10*4)+STACK_PAD_CONSTANT)

  // Macro for single x86/x86_64 register access
  // Register names for use with ptrace calls, not instruction generation.
  #if defined(__x86_64__) && __WORDSIZE == 64
    #define PTRACE_REG_15   r15
    #define PTRACE_REG_14   r14
    #define PTRACE_REG_13   r13
    #define PTRACE_REG_12   r12
    #define PTRACE_REG_BP   rbp
    #define PTRACE_REG_BX   rbx
    #define PTRACE_REG_11   r11
    #define PTRACE_REG_10   r10
    #define PTRACE_REG_9    r9
    #define PTRACE_REG_8    r8
    #define PTRACE_REG_AX   rax
    #define PTRACE_REG_CX   rcx
    #define PTRACE_REG_DX   rdx
    #define PTRACE_REG_SI   rsi
    #define PTRACE_REG_DI   rdi
    #define PTRACE_REG_ORIG_AX  orig_rax
    #define PTRACE_REG_IP   rip
    #define PTRACE_REG_CS   cs
    #define PTRACE_REG_FLAGS  eflags
    #define PTRACE_REG_SP   rsp
    #define PTRACE_REG_SS   ss
    #define PTRACE_REG_FS_BASE  fs_base
    #define PTRACE_REG_GS_BASE  gs_base
    #define PTRACE_REG_DS   ds
    #define PTRACE_REG_ES   es
    #define PTRACE_REG_FS   fs
    #define PTRACE_REG_GS   gs
  #else
    #define PTRACE_REG_BX   ebx
    #define PTRACE_REG_CX   ecx
    #define PTRACE_REG_DX   edx
    #define PTRACE_REG_SI   esi
    #define PTRACE_REG_DI   edi
    #define PTRACE_REG_BP   ebp
    #define PTRACE_REG_AX   eax
    #define PTRACE_REG_DS   xds
    #define PTRACE_REG_ES   xes
    #define PTRACE_REG_FS   xfs
    #define PTRACE_REG_GS   xgs
    #define PTRACE_REG_ORIG_AX  orig_eax
    #define PTRACE_REG_IP   eip
    #define PTRACE_REG_CS   xcs
    #define PTRACE_REG_FLAGS  eflags
    #define PTRACE_REG_SP   esp
    #define PTRACE_REG_SS   xss
  #endif

  // Define access method for saved register (GPR)
  #define GET_GPR(x, insn) emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, SAVED_EAX_OFFSET-(x*4), insn)

  // Define access method for virtual registers (stack-based)
  #define LOAD_VIRTUAL32(x, insn) emitMovRMToReg(REGNUM_EAX, REGNUM_EBP, -1*(x*4), insn)
  #define SAVE_VIRTUAL32(x, insn) emitMovRegToRM(REGNUM_EBP, -1*(x*4), REGNUM_EAX, insn)
  #define LOAD_VIRTUAL64(x, insn) emitMovRMToReg(REGNUM_RAX, REGNUM_RBP, -1*(x*8), insn)
  #define SAVE_VIRTUAL64(x, insn) emitMovRegToRM(REGNUM_RBP, -1*(x*8), REGNUM_RAX, insn)


