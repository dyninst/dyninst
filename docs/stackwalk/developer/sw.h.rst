.. _`sec:sw.h`:

sw.h
####

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:type:: std::set<FrameStepper *, ltstepper> StepperSet



.. cpp:class:: AddrRangeStepper : public addrRange

  .. cpp:member:: Address start
  .. cpp:member:: Address end
  .. cpp:member:: StepperSet steppers
  .. cpp:function:: AddrRangeStepper()
  .. cpp:function:: AddrRangeStepper(Address s, Address e)
  .. cpp:function:: virtual Address get_address() const
  .. cpp:function:: virtual unsigned long get_size() const
  .. cpp:function:: virtual ~AddrRangeStepper()

.. cpp:struct:: ltstepper

  .. cpp:function:: bool operator()(const FrameStepper *a, const FrameStepper *b) const


.. cpp:class:: FrameFuncStepperImpl : public FrameStepper

  .. cpp:member:: private FrameStepper *parent
  .. cpp:member:: private FrameFuncHelper *helper
  .. cpp:function:: FrameFuncStepperImpl(Walker *w, FrameStepper *parent_, FrameFuncHelper *helper_)
  .. cpp:function:: virtual ~FrameFuncStepperImpl()
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: static gcframe_ret_t getBasicCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual const char *getName() const


.. cpp:class:: BottomOfStackStepperImpl : public FrameStepper

  .. cpp:member:: private BottomOfStackStepper *parent
  .. cpp:member:: private std::vector<std::pair<Address, Address> > ra_stack_tops
  .. cpp:member:: private std::vector<std::pair<Address, Address> > sp_stack_tops
  .. cpp:member:: private bool libc_init
  .. cpp:member:: private bool aout_init
  .. cpp:member:: private bool libthread_init
  .. cpp:function:: private void initialize()
  .. cpp:function:: BottomOfStackStepperImpl(Walker *w, BottomOfStackStepper *parent)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual void newLibraryNotification(LibAddrPair *la, lib_change_t change)
  .. cpp:function:: virtual ~BottomOfStackStepperImpl()
  .. cpp:function:: virtual const char *getName() const


.. cpp:class:: DyninstInstrStepperImpl : public FrameStepper

  .. cpp:member:: private static std::map<SymReader *, bool> isRewritten
  .. cpp:member:: private DyninstInstrStepper *parent
  .. cpp:function:: DyninstInstrStepperImpl(Walker *w, DyninstInstrStepper *p)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: gcframe_ret_t getCallerFrameArch(const Frame &in, Frame &out, Address base, Address lib_base, unsigned size, unsigned stack_height)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual const char *getName() const
  .. cpp:function:: virtual ~DyninstInstrStepperImpl()


.. cpp:class:: DyninstDynamicStepperImpl : public FrameStepper

  .. cpp:member:: private DyninstDynamicStepper *parent
  .. cpp:member:: private DyninstDynamicHelper *helper
  .. cpp:member:: private bool prevEntryExit

      remember if the previous frame was entry/exit instrumentation

  .. cpp:function:: DyninstDynamicStepperImpl(Walker *w, DyninstDynamicStepper *p, DyninstDynamicHelper *h)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: gcframe_ret_t getCallerFrameArch(const Frame &in, Frame &out, Address base, Address lib_base, unsigned size, unsigned stack_height, bool aligned, Address orig_ra, bool pEntryExit)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual const char *getName() const
  .. cpp:function:: virtual ~DyninstDynamicStepperImpl()


.. cpp:class:: DyninstInstFrameStepperImpl : public FrameStepper

  .. cpp:member:: private DyninstInstFrameStepper *parent
  .. cpp:function:: private bool getWord(Address &words, Address start)
  .. cpp:function:: DyninstInstFrameStepperImpl(Walker *w, DyninstInstFrameStepper *p = NULL)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual const char *getName() const
  .. cpp:function:: virtual ~DyninstInstFrameStepperImpl()


.. cpp:class:: CallChecker

  .. cpp:member:: private ProcessState * proc
  .. cpp:function:: CallChecker(ProcessState * proc_)
  .. cpp:function:: ~CallChecker()
  .. cpp:function:: bool isPrevInstrACall(Address addr, Address & target)


.. cpp:class:: int_walkerSet

  .. cpp:function:: int_walkerSet()
  .. cpp:function:: ~int_walkerSet()
  .. cpp:function:: pair<set<Walker *>::iterator, bool> insert(Walker *w)
  .. cpp:function:: void erase(set<Walker *>::iterator i)
  .. cpp:function:: private void addToProcSet(ProcDebug *)
  .. cpp:function:: private void eraseFromProcSet(ProcDebug *)
  .. cpp:function:: private void clearProcSet()
  .. cpp:function:: private void initProcSet()
  .. cpp:function:: private bool walkStacksProcSet(CallTree &tree, bool &bad_plat, bool walk_iniital_only)
  .. cpp:member:: private unsigned non_pd_walkers
  .. cpp:member:: private set<Walker *> walkers
  .. cpp:member:: private void *procset

      Opaque pointer, will refer to a ProcControl::ProcessSet in some situations
