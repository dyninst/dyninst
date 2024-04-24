.. _`sec:pcrel.h`:

pcrel.h
#######


.. cpp:class:: pcRelRegion

  .. cpp:member:: codeGen *gen
  .. cpp:member:: instruction orig_instruc
  .. cpp:member:: unsigned cur_offset
  .. cpp:member:: unsigned cur_size
  .. cpp:function:: pcRelRegion(const instruction &i)
  .. cpp:function:: virtual unsigned apply(Dyninst::Address addr) = 0
  .. cpp:function:: virtual unsigned maxSize() = 0
  .. cpp:function:: virtual bool canPreApply()
  .. cpp:function:: virtual ~pcRelRegion()

.. cpp:class:: pcRelJump : public pcRelRegion

  .. cpp:member:: private Dyninst::Address addr_targ
  .. cpp:member:: private patchTarget *targ
  .. cpp:member:: private bool copy_prefixes_
  .. cpp:function:: private Dyninst::Address get_target()
  .. cpp:function:: pcRelJump(patchTarget *t, const instruction &i, bool copyPrefixes = true)
  .. cpp:function:: pcRelJump(Dyninst::Address target, const instruction &i, bool copyPrefixes = true)
  .. cpp:function:: virtual unsigned apply(Dyninst::Address addr)
  .. cpp:function:: virtual unsigned maxSize()
  .. cpp:function:: virtual bool canPreApply()
  .. cpp:function:: virtual ~pcRelJump()

.. cpp:class:: pcRelJCC : public pcRelRegion

  .. cpp:member:: private Dyninst::Address addr_targ
  .. cpp:member:: private patchTarget *targ
  .. cpp:function:: private Dyninst::Address get_target()
  .. cpp:function:: pcRelJCC(patchTarget *t, const instruction &i)
  .. cpp:function:: pcRelJCC(Dyninst::Address target, const instruction &i)
  .. cpp:function:: virtual unsigned apply(Dyninst::Address addr)
  .. cpp:function:: virtual unsigned maxSize()
  .. cpp:function:: virtual bool canPreApply()
  .. cpp:function:: virtual ~pcRelJCC()

.. cpp:class:: pcRelCall : public pcRelRegion

  .. cpp:member:: private Dyninst::Address targ_addr
  .. cpp:member:: private patchTarget *targ
  .. cpp:function:: private Dyninst::Address get_target()
  .. cpp:function:: pcRelCall(patchTarget *t, const instruction &i)
  .. cpp:function:: pcRelCall(Dyninst::Address targ_addr, const instruction &i)
  .. cpp:function:: virtual unsigned apply(Dyninst::Address addr)
  .. cpp:function:: virtual unsigned maxSize()
  .. cpp:function:: virtual bool canPreApply()
  .. cpp:function:: ~pcRelCall()

.. cpp:class:: pcRelData : public pcRelRegion

  .. cpp:member:: private Dyninst::Address data_addr
  .. cpp:function:: pcRelData(Dyninst::Address a, const instruction &i)
  .. cpp:function:: virtual unsigned apply(Dyninst::Address addr)
  .. cpp:function:: virtual unsigned maxSize()
  .. cpp:function:: virtual bool canPreApply()
