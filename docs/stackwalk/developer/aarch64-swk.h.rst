.. _`sec:aarch64-swk.h`:

aarch64-swk.h
#############

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:class:: aarch64_LookupFuncStart : public FrameFuncHelper

  .. cpp:member:: private static const unsigned int cache_size = 64

      We need some kind of re-entrant safe synchronization before we can
      globally turn this caching on, but it would sure help things.

  .. cpp:function:: static aarch64_LookupFuncStart *getLookupFuncStart(ProcessState *p)
  .. cpp:function:: void releaseMe()
  .. cpp:function:: virtual FrameFuncHelper::alloc_frame_t allocatesFrame(Address addr)
  .. cpp:function:: ~aarch64_LookupFuncStart()
  .. cpp:function:: static void clear_func_mapping(Dyninst::PID)
