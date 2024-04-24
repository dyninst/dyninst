.. _`sec-dev:BPatch_instruction.h`:

BPatch_instruction.h
####################

.. cpp:namespace:: dev

.. cpp:class:: BPatch_instruction

  .. cpp:member:: protected unsigned int nacc
  .. cpp:member:: protected internal_instruction *insn_
  .. cpp:member:: protected bool *isLoad
  .. cpp:member:: protected bool *isStore
  .. cpp:member:: protected int *preFcn

    prefetch function(-1 = none)

  .. cpp:member:: protected int *condition

      -1 means no condition, all other values are machine specific conditions, currently(8/13/02) the tttn field on x86

  .. cpp:member:: protected bool *nonTemporal

    non-temporal(cache non-polluting) write on x86

  .. cpp:member:: protected BPatch_basicBlock *parent
  .. cpp:member:: protected long unsigned int addr

  .. cpp:function:: internal_instruction *insn()
  .. cpp:function:: char *getMnemonic() const


.. cpp:class:: BPatch_branchInstruction : public BPatch_instruction

  .. cpp:function:: BPatch_branchInstruction(internal_instruction *insn, long unsigned int _addr, void *target)
  .. cpp:function:: ~BPatch_branchInstruction()
  .. cpp:function:: void *getTarget()
  .. cpp:member:: protected void *target_
