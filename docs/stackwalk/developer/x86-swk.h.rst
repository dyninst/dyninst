.. _`sec:x86-swk.h`:

x86-swk.h
#########

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:class:: StepperWandererImpl : public FrameStepper

  .. cpp:member:: private WandererHelper *whelper
  .. cpp:member:: private FrameFuncHelper *fhelper
  .. cpp:member:: private StepperWanderer *parent
  .. cpp:function:: private bool getWord(Address &words, Address start)
  .. cpp:function:: StepperWandererImpl(Walker *walker_, StepperWanderer *parent_, WandererHelper *whelper_, FrameFuncHelper *fhelper_)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual const char *getName() const
  .. cpp:function:: virtual ~StepperWandererImpl()


.. cpp:class:: LookupFuncStart : public FrameFuncHelper

  .. cpp:member:: private static std::map<Dyninst::PID, LookupFuncStart*> all_func_starts
  .. cpp:function:: private LookupFuncStart(ProcessState *proc_)
  .. cpp:member:: private int ref_count
  .. cpp:function:: private void updateCache(Address addr, alloc_frame_t result)
  .. cpp:function:: private bool checkCache(Address addr, alloc_frame_t &result)
  .. cpp:member:: private static const unsigned int cache_size = 64

   We need some kind of re-entrant safe synhronization before we can
   globally turn this caching on, but it would sure help things.

  .. cpp:member:: private LRUCache<Address, alloc_frame_t> cache
  .. cpp:function:: static LookupFuncStart *getLookupFuncStart(ProcessState *p)
  .. cpp:function:: void releaseMe()
  .. cpp:function:: virtual alloc_frame_t allocatesFrame(Address addr)
  .. cpp:function:: ~LookupFuncStart()
  .. cpp:function:: static void clear_func_mapping(Dyninst::PID)
