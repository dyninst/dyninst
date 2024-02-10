.. _`sec:inst-aarch64.h`:

inst-aarch64.h
##############

.. code:: cpp

  #define DEAD_REG              0
  #define LIVE_REG              1
  #define LIVE_UNCLOBBERED_REG  2
  #define LIVE_CLOBBERED_REG    3

  #define GPRSIZE_32            4
  #define GPRSIZE_64            8
  #define FPRSIZE_64           16

  #define REG_FP               29
  #define REG_LR               30
  #define REG_SP               31
  #define REG_TOC               2   /* TOC anchor                            */
  #define REG_GUARD_ADDR        5   /* Arbitrary                             */

  // REG_GUARD_OFFSET and REG_GUARD_VALUE could overlap.
  #define REG_GUARD_VALUE       6
  #define REG_GUARD_OFFSET      6

  #define REG_COST_ADDR         5
  #define REG_COST_VALUE        6

  #define REG_SCRATCH          10

  // #sasha This seemed to be copy and paste. Not sure if it all stands
  // for ARM.
  //
  // The stack grows down from high addresses toward low addresses.
  // There is a maximum number of bytes on the stack below the current
  // value of the stack frame pointer that a function can use without
  // first establishing a new stack frame.  When our instrumentation
  // needs to use the stack, we make sure not to write into this
  // potentially used area.  64-bit PowerPC ELF ABI Supplement,
  // Version 1.9, 2004-10-23, used by Linux, stated 288 bytes for this
  // area.
  #define STACKSKIP          288

  #define ALIGN_QUADWORD(x)  ( ((x) + 0xf) & ~0xf )  //x is positive or unsigned

  //TODO Fix for ARM
  #define GPRSAVE_64  (31*GPRSIZE_64)
  #define FPRSAVE_64  (32*FPRSIZE_64)
  #define SPRSAVE_64  (1*8+3*4)
  // #sasha Are these necessary?
  #define FUNCSAVE_64 (32*8)
  #define FUNCARGS_64 (16*8)
  #define LINKAREA_64 (6*8)

  // #sasha Why is PowerPC stuff here?
  #define PARAM_OFFSET(mutatee_address_width)                         \
          (                                                           \
              ((mutatee_address_width) == sizeof(uint64_t))           \
              ? (   /* 64-bit ELF PowerPC Linux                   */  \
                    sizeof(uint64_t) +  /* TOC save               */  \
                    sizeof(uint64_t) +  /* link editor doubleword */  \
                    sizeof(uint64_t) +  /* compiler doubleword    */  \
                    sizeof(uint64_t) +  /* LR save                */  \
                    sizeof(uint64_t) +  /* CR save                */  \
                    sizeof(uint64_t)    /* Stack frame back chain */  \
                )                                                     \
              : (   /* 32-bit ELF PowerPC Linux                   */  \
                    sizeof(uint32_t) +  /* LR save                */  \
                    sizeof(uint32_t)    /* Stack frame back chain */  \
                )                                                     \
          )

#define TRAMP_SPR_OFFSET_64 (0)
#define STK_LR       (              0)
#define STK_NZCV     (STK_SP_EL0  + 8)
#define STK_FPCR     (STK_NZCV    + 4)
#define STK_FPSR     (STK_FPCR    + 4)

#define TRAMP_FPR_OFFSET_64 (TRAMP_SPR_OFFSET_64 + SPRSAVE_64)
#define TRAMP_GPR_OFFSET_64 (TRAMP_FPR_OFFSET_64 + FPRSAVE_64)
#define FUNC_CALL_SAVE_64   (LINKAREA_64 + FUNCARGS_64)


.. cpp:function:: inline int TRAMP_GPR_OFFSET(int x)
.. cpp:function:: inline int TRAMP_FPR_OFFSET(int x)
.. cpp:function:: inline int TRAMP_SPR_OFFSET(int x)
.. cpp:function:: void pushStack(codeGen &gen)
.. cpp:function:: void popStack(codeGen &gen)

