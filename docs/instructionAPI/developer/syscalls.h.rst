.. _`sec-dev:syscalls.h`:

syscalls.h
##########

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:namespace-push:: x86

.. cpp:struct:: tcb_syscall_visitor : Visitor

  Thread Control Block syscall visitor  Used to detect ``call gs:[0x10]`` syscalls in 32-bit code.

  .. cpp:member:: Result value
  .. cpp:member:: int num_imm{0}
  .. cpp:member:: bool valid{true}
  .. cpp:member:: bool found_deref{false}
  .. cpp:function:: void visit(BinaryFunction*) override
  .. cpp:function:: void visit(Immediate* imm) override
  .. cpp:function:: void visit(RegisterAST*) override
  .. cpp:function:: void visit(Dereference*) override


.. cpp:function:: bool isSystemCall(Instruction const& ins)

  Check for system call idioms

  Idioms checked:

    - ``syscall``
    - ``int <vector>`` (e.g., ``int 0x80``)
    - ``call DWORD PTR gs:[0x10]``

  Calls to Linux vdso functions (e.g., ``__vdso_clock_gettime``) are not considered system
  calls because they use the standard call mechanism to explicitly bypass the kernel.

  ``sysenter`` is checked by :cpp:func:`IA_IAPI::isSysEnter`.

