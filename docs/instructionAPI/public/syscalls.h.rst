.. _`sec:syscalls.h`:

syscalls.h
##########

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:function:: bool isSystemCall(Instruction const& ins)

  Checks if ``ins`` is a system call.

  System calls can be implemented by the hardware as instructions as well as by
  idioms. This checks for both for all supported platforms.
