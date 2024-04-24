.. _`sec:inst-power.h`:

inst-power.h
############

.. cpp:namespace:: dev::powerpc

Common definitions to the POWER specific instrumentation code.

"pseudo" instructions that are placed in the tramp code for the inst funcs
to patch up. This must be invalid instructions (any instruction with
its top 10 bits as 0 is invalid (technically UNIMP).

.. cpp:function:: void saveSPR(codeGen &gen, Dyninst::Register scratchReg, int sprnum, int stkOffset)

  Generates instructions to save a special purpose register onto the stack.

  NOTE: the bit layout of the mfspr instruction is as follows:
  ``opcode:6 ; RT: 5 ; SPR: 10 ; const 339:10 ; Rc: 1``. However, the two 5-bit halves of the SPR field are
  reversed so just using the xfxform will not work.

.. cpp:function:: void restoreSPR(codeGen &gen, Dyninst::Register scratchReg, int sprnum, int stkOffset)

  Generates instructions to restore a special purpose register from the stack.

.. cpp:function:: void saveLR(codeGen &gen, Dyninst::Register scratchReg, int stkOffset)

  Generates instructions to save link register onto stack.

.. cpp:function:: void restoreLR(codeGen &gen, Dyninst::Register scratchReg, int stkOffset)

  Generates instructions to restore link register from stack.

.. cpp:function:: void setBRL(codeGen &gen, Dyninst::Register scratchReg, long val, unsigned ti)

  Generates instructions to place a given value into link register. The entire instruction sequence
  consists of the generated instructions followed by a given (tail) instruction.

.. cpp:function:: void resetBRL(AddressSpace* p, Address loc, unsigned val)

  Writes out instructions in process ``p`` at address ``loc`` to place value ``val`` into the link register.

  If ``val == 0``, then the instruction sequence is followed by a ``nop``. If ``val != 0``, then the
  instruction sequence is followed by a ``brl``.

.. cpp:function:: void saveCR(codeGen &gen, Dyninst::Register scratchReg, int stkOffset)

  Generates instructions to save the condition codes register onto stack.

.. cpp:function:: void restoreCR(codeGen &gen, Dyninst::Register scratchReg, int stkOffset)
.. cpp:function:: void saveFPSCR(codeGen &gen, Dyninst::Register scratchReg, int stkOffset)
.. cpp:function:: void restoreFPSCR(codeGen &gen, Dyninst::Register scratchReg, int stkOffset)
.. cpp:function:: void saveRegister(codeGen &gen, Dyninst::Register reg, int save_off)
.. cpp:function:: void restoreRegister(codeGen &gen, Dyninst::Register source, Dyninst::Register dest, int save_off)

  We may want to restore a _logical_ register N (that is, the save slot for N) into a different reg. This
  avoids using a temporary.

.. cpp:function:: void restoreRegister(codeGen &gen, Dyninst::Register reg, int save_off)

  Much more common case

.. cpp:function:: void saveFPRegister(codeGen &gen, Dyninst::Register reg, int save_off)
.. cpp:function:: void restoreFPRegister(codeGen &gen, Dyninst::Register source, Dyninst::Register dest, int save_off)

  See above.

.. cpp:function:: void restoreFPRegister(codeGen &gen, Dyninst::Register reg, int save_off)
.. cpp:function:: void pushStack(codeGen &gen)
.. cpp:function:: void popStack(codeGen &gen)
.. cpp:function:: unsigned saveGPRegisters(codeGen &gen, registerSpace *theRegSpace, int save_off, int numReqGPRs = -1)
.. cpp:function:: unsigned restoreGPRegisters(codeGen &gen, registerSpace *theRegSpace, int save_off)
.. cpp:function:: unsigned saveFPRegisters(codeGen &gen, registerSpace *theRegSpace, int save_off)
.. cpp:function:: unsigned restoreFPRegisters(codeGen &gen, registerSpace *theRegSpace, int save_off)
.. cpp:function:: unsigned saveSPRegisters(codeGen &gen, registerSpace *, int save_off, int force_save)
.. cpp:function:: unsigned restoreSPRegisters(codeGen &gen, registerSpace *, int save_off, int force_save)


.. code:: cpp

  #define DEAD_REG              0
  #define LIVE_REG              1
  #define LIVE_UNCLOBBERED_REG  2
  #define LIVE_CLOBBERED_REG    3

  #define GPRSIZE_32            4
  #define GPRSIZE_64            8
  #define FPRSIZE               16

  #define REG_SP          1
  #define REG_TOC               2   /* TOC anchor */

  // REG_GUARD_OFFSET and REG_GUARD_VALUE could overlap.
  #define REG_GUARD_ADDR        5   /* Arbitrary */
  #define REG_GUARD_VALUE       6
  #define REG_GUARD_OFFSET      6

  #define REG_COST_ADDR         5
  #define REG_COST_VALUE        6

  #define REG_SCRATCH          10

  #define REG_MT_POS           12   /* Register to reserve for MT implementation */
  #define NUM_INSN_MT_PREAMBLE 26   /* number of instructions required for the MT preamble. */

  // The stack grows down from high addresses toward low addresses.
  // There is a maximum number of bytes on the stack below the current
  // value of the stack frame pointer that a function can use without
  // first establishing a new stack frame.  When our instrumentation
  // needs to use the stack, we make sure not to write into this
  // potentially used area.
  //
  // OpenPOWER ELF V2 ABI says user code can use 288 bytes underneath
  // the stack pointer and system code can further use 224 more bytes
  //
  // In case we are instrumenting signal handlers, we want to skip
  // skip more spaces, which is 288+224=512 bytes
  #define STACKSKIP          512

  // Both 32-bit and 64-bit PowerPC ELF ABI documents for Linux state
  // that the stack frame pointer value must always be 16-byte (quadword)
  // aligned.  Use the following macro on all quantities used to
  // increment or decrement the stack frame pointer.
  #define ALIGN_QUADWORD(x)  ( ((x) + 0xf) & ~0xf )  //x is positive or unsigned

  #define GPRSAVE_32  (32*4)
  #define GPRSAVE_64  (32*8)
  #define FPRSAVE     (14*8)
  #define VECSAVE     (33*16)

  #define SPRSAVE_32  (6*4+8)
  #define SPRSAVE_64  (6*8+8)
  #define FUNCSAVE_32 (32*4)
  #define FUNCSAVE_64 (32*8)
  #define FUNCARGS_32 (16*4)
  #define FUNCARGS_64 (16*8)
  #define LINKAREA_32 (6*4)
  #define LINKAREA_64 (6*8)

  #define PARAM_OFFSET(mutatee_address_width)                         \
          (                                                           \
              ((mutatee_address_width) == sizeof(uint64_t))           \
              ? (   /* 64-bit ELF PowerPC Linux                   */  \
                    sizeof(uint64_t) +  /* TOC save doubleword    */  \
                    sizeof(uint64_t) +  /* LR save doublewordd    */  \
                    sizeof(uint32_t) +  /* Reserved word          */  \
                    sizeof(uint32_t) +  /* CR save word           */  \
                    sizeof(uint64_t)    /* Stack frame back chain */  \
                )                                                     \
              : (   /* 32-bit ELF PowerPC Linux                   */  \
                    sizeof(uint32_t) +  /* LR save                */  \
                    sizeof(uint32_t)    /* Stack frame back chain */  \
                )                                                     \
          )

  // Okay, now that we have those defined, let's define the offsets upwards
  #define TRAMP_FRAME_SIZE_32 ALIGN_QUADWORD(STACKSKIP + GPRSAVE_32 + VECSAVE \
                                             + SPRSAVE_32 \
                                             + FUNCSAVE_32 + FUNCARGS_32 + LINKAREA_32)
  #define TRAMP_FRAME_SIZE_64 ALIGN_QUADWORD(STACKSKIP + GPRSAVE_64 + VECSAVE \
                                             + SPRSAVE_64 \
                                             + FUNCSAVE_64 + FUNCARGS_64 + LINKAREA_64)
  #define PDYN_RESERVED_32 (LINKAREA_32 + FUNCARGS_32 + FUNCSAVE_32)
  #define PDYN_RESERVED_64 (LINKAREA_64 + FUNCARGS_64 + FUNCSAVE_64)

  #define TRAMP_SPR_OFFSET_32 (PDYN_RESERVED_32) /* 4 for LR */
  #define STK_LR       (              0)
  #define STK_CR_32    (STK_LR      + 4)
  #define STK_CTR_32   (STK_CR_32   + 4)
  #define STK_XER_32   (STK_CTR_32  + 4)
  #define STK_FP_CR_32 (STK_XER_32  + 4)
  #define STK_SPR0_32  (STK_FP_CR_32+ 8)

  #define TRAMP_SPR_OFFSET_64 (PDYN_RESERVED_64)
  #define STK_CR_64    (STK_LR      + 8)
  #define STK_CTR_64   (STK_CR_64   + 8)
  #define STK_XER_64   (STK_CTR_64  + 8)
  #define STK_FP_CR_64 (STK_XER_64  + 8)
  #define STK_SPR0_64  (STK_FP_CR_64+ 8)

  #define TRAMP_SPR_OFFSET(x) (((x) == 8) ? TRAMP_SPR_OFFSET_64 : TRAMP_SPR_OFFSET_32)

  #define TRAMP_FPR_OFFSET_32 (TRAMP_SPR_OFFSET_32 + SPRSAVE_32)
  #define TRAMP_FPR_OFFSET_64 (TRAMP_SPR_OFFSET_64 + SPRSAVE_64)
  #define TRAMP_FPR_OFFSET(x) (((x) == 8) ? TRAMP_FPR_OFFSET_64 : TRAMP_FPR_OFFSET_32)

  #define TRAMP_GPR_OFFSET_32 (TRAMP_FPR_OFFSET_32 + VECSAVE)
  #define TRAMP_GPR_OFFSET_64 (TRAMP_FPR_OFFSET_64 + VECSAVE)
  #define TRAMP_GPR_OFFSET(x) (((x) == 8) ? TRAMP_GPR_OFFSET_64 : TRAMP_GPR_OFFSET_32)

  #define FUNC_CALL_SAVE_32 (LINKAREA_32 + FUNCARGS_32)
  #define FUNC_CALL_SAVE_64 (LINKAREA_64 + FUNCARGS_64)
  #define FUNC_CALL_SAVE(x) (((x) == 8) ? FUNC_CALL_SAVE_64 : FUNC_CALL_SAVE_32)


Saving and restoring registers
******************************

We create a new stack frame in the base tramp and save registers above it.
Currently, the plan is:

.. code::

   < 220 bytes as per system spec      > + 4 for 64-bit alignment
   < 14 GPR slots @ 4 bytes each       >
   < 14 FPR slots @ 8 bytes each       >
   < 6 SPR slots @ 4 bytes each        >
   < 1 FP SPR slot @ 8 bytes           >
   < Space to save live regs at func call >
   < Func call overflow area, 32 bytes >
   < Linkage area, 24 bytes            >

Of course, change all the 4's to 8's for 64-bit mode.
