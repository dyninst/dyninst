.. _`sec:InstructionCategories.h`:

InstructionCategories.h
#######################

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:enum:: InsnCategory

  .. cpp:enumerator:: c_CallInsn
  .. cpp:enumerator:: c_ReturnInsn
  .. cpp:enumerator:: c_BranchInsn
  .. cpp:enumerator:: c_CompareInsn
  .. cpp:enumerator:: c_PrefetchInsn
  .. cpp:enumerator:: c_SysEnterInsn
  .. cpp:enumerator:: c_SyscallInsn
  .. cpp:enumerator:: c_VectorInsn
  .. cpp:enumerator:: c_GPUKernelExitInsn
  .. cpp:enumerator:: c_NoCategory

.. cpp:function:: InsnCategory entryToCategory(entryID e)