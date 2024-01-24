.. _`sec:dbgstepper-impl.h`:

dbgstepper-impl.h
#################

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:class:: DebugStepperImpl : public FrameStepper, public Dyninst::ProcessReader

  .. cpp:member:: private dyn_hash_map<Address, cache_t> cache_
  .. cpp:member:: private Dyninst::Address last_addr_read
  .. cpp:member:: private unsigned long last_val_read
  .. cpp:member:: private unsigned addr_width
  .. cpp:member:: private DebugStepper *parent_stepper
  .. cpp:member:: private const Frame *cur_frame

      TODO: Thread safety

  .. cpp:member:: private const Frame *depth_frame

      Current position in the stackwalk

  .. cpp:function:: private void addToCache(const Frame &cur, const Frame &caller)
  .. cpp:function:: private bool lookupInCache(const Frame &cur, Frame &caller)
  .. cpp:function:: private location_t getLastComputedLocation(unsigned long val)

  .. cpp:function:: DebugStepperImpl(Walker *w, DebugStepper *parent)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual bool ReadMem(Address addr, void *buffer, unsigned size)
  .. cpp:function:: virtual bool GetReg(MachRegister reg, MachRegisterVal &val)
  .. cpp:function:: virtual ~DebugStepperImpl()
  .. cpp:function:: virtual bool start()
  .. cpp:function:: virtual bool done()
  .. cpp:function:: virtual const char *getName() const

  .. cpp:function:: protected gcframe_ret_t getCallerFrameArch(Address pc, const Frame &in, Frame &out, DwarfDyninst::DwarfFrameParserPtr dinfo, bool isVsyscallPage)
  .. cpp:function:: protected bool isFrameRegister(MachRegister reg)
  .. cpp:function:: protected bool isStackRegister(MachRegister reg)

.. cpp:struct:: DebugStepperImpl::cache_t

  ra and fp are differences in address, sp is difference in value.

  .. cpp:member:: unsigned ra_delta
  .. cpp:member:: unsigned fp_delta
  .. cpp:member:: unsigned sp_delta
  .. cpp:function:: cache_t()
  .. cpp:function:: cache_t(unsigned a, unsigned b, unsigned c)
