.. _`sec:dyn_regs.h`:

dyn_regs.h
##########

.. cpp:namespace:: Dyninst

.. _`sec:register-categories`:

Register Categories
===================

Register categories are used to group registers with common functionality. For example,
general-purpose registers or vector registers associated with a specific ISA extensions.

As an example on x86_64, the categories are

  .. code:: cpp

    const signed int GPR    = 0x00010000;  // General-Purpose Registers
    const signed int SEG    = 0x00020000;  // Segment Registers
    const signed int FLAG   = 0x00030000;  // EFLAGS Register
    const signed int MISC   = 0x00040000;  // Internal ProcControlAPI Register
    const signed int CTL    = 0x00050000;  // Control Registers CR0-CR7
    const signed int DBG    = 0x00060000;  // Debug Registers DR0-DR7
    const signed int TST    = 0x00070000;  // Internal InstructionAPI Registers
    const signed int X87    = 0x00080000;  // x87 FPU Registers
    const signed int MMX    = 0x00090000;  // MM0-MM7 Registers
    const signed int XMM    = 0x000A0000;  // XMM0-XMM7 Registers from SSE
    const signed int YMM    = 0x000B0000;  // YMM0-YMM7 Registers from AVX2/FMA
    const signed int ZMM    = 0x000C0000;  // ZMM0-ZMM7 Registers from AVX-512
    const signed int KMASK  = 0x000D0000;  // K0-K7 opmask Registers from AVX-512
    const signed int FPCTL  = 0x000E0000;  // control/status Registers from x87, SSE, and AVX

By comparison on aarch64, they are far fewer categories:

  .. code:: cpp

    const signed int GPR    = 0x00010000;
    const signed int FPR    = 0x00020000;
    const signed int FLAG   = 0x00030000;
    const signed int FSR    = 0x00040000;
    const signed int SPR    = 0x00080000;
    const signed int SYSREG = 0x00100000;

  .. Warning:: Users should never rely on the values of these constants!


.. _`sec:abstract-registers`:

Abstract Registers
==================

Abstract registers represent general concepts that do not map to a specific register for
any given architecture.

  .. csv-table:: abstract registers

      InvalidReg, An invalid register. Used to indicate an error in :cpp:class:`MachRegister`.
      FrameBase, The base register used as a stack frame pointer.
      ReturnAddr, The location of the function return address.
      StackTop, The top of the stack.
