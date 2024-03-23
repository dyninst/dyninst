.. _`sec:BPatch_parRegion.h`:

BPatch_parRegion.h
##################

.. cpp:enum:: parRegType

  .. cpp:enumerator:: OMP_NONE
  .. cpp:enumerator:: OMP_PARALLEL
  .. cpp:enumerator:: OMP_DO_FOR
  .. cpp:enumerator:: OMP_DO_FOR_LOOP_BODY
  .. cpp:enumerator:: OMP_SECTIONS
  .. cpp:enumerator:: OMP_SINGLE
  .. cpp:enumerator:: OMP_PAR_DO
  .. cpp:enumerator:: OMP_PAR_SECTIONS
  .. cpp:enumerator:: OMP_MASTER
  .. cpp:enumerator:: OMP_CRITICAL
  .. cpp:enumerator:: OMP_BARRIER
  .. cpp:enumerator:: OMP_ATOMIC
  .. cpp:enumerator:: OMP_FLUSH
  .. cpp:enumerator:: OMP_ORDERED
  .. cpp:enumerator:: OMP_ANY


.. cpp:class:: BPatch_parRegion

  .. cpp:function:: BPatch_parRegion(int_parRegion * _parReg, BPatch_function * _func)
  .. cpp:function:: ~BPatch_parRegion()
  .. cpp:function:: int_parRegion *lowlevel_region() const
  .. cpp:function:: void printDetails()
  .. cpp:function:: int getClause(const char * key)
  .. cpp:function:: int replaceOMPParameter(const char * key, int value)

  .. cpp:function:: BPatch_Vector<BPatch_instruction *> * getInstructions()

    Returns the instructions that belong to the block.

  .. cpp:function:: bool getInstructions(std::vector<Dyninst::InstructionAPI::Instruction>& insns)

  .. cpp:function:: unsigned size() const

  .. cpp:function:: unsigned long getStartAddress() const

    Returns absolute starting address.

  .. cpp:function:: unsigned long getEndAddress() const

    Returns absolute ending address.
