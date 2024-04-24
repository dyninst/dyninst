.. _`sec:legacy-instruction.h`:

legacy-instruction.h
####################

Legacy support for BPatch_instruction and BPatch_memoryAccess,
both of which hold a pointer to an opaque type containing the
platform-specific ``instruction`` type.


.. cpp:class:: internal_instruction

  .. cpp:function:: explicit internal_instruction(instruction * insn)
  .. cpp:function:: instruction * insn() const
  .. cpp:member:: private instruction * _insn
