. _`sec:freebsd.h`:

freebsd.h
#########

.. cpp:class:: GeneratorFreeBSD : public GeneratorMT

  .. cpp:function:: GeneratorFreeBSD()
  .. cpp:function:: virtual ~GeneratorFreeBSD()
  .. cpp:function:: virtual bool initialize()
  .. cpp:function:: virtual bool canFastHandle()
  .. cpp:function:: virtual ArchEvent *getEvent(bool block)


.. cpp:class:: ArchEventFreeBSD : public ArchEvent

  .. cpp:member:: int status
  .. cpp:member:: pid_t pid
  .. cpp:member:: lwpid_t lwp
  .. cpp:member:: bool interrupted
  .. cpp:member:: int error
  .. cpp:function:: ArchEventFreeBSD(bool inter_)
  .. cpp:function:: ArchEventFreeBSD(pid_t p, lwpid_t l, int s)
  .. cpp:function:: ArchEventFreeBSD(int e)
  .. cpp:function:: virtual ~ArchEventFreeBSD()


.. cpp:class:: DecoderFreeBSD : public Decoder

  .. cpp:function:: DecoderFreeBSD()
  .. cpp:function:: virtual ~DecoderFreeBSD()
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual bool decode(ArchEvent *ae, std::vector<Event::ptr> &events)
  .. cpp:function:: Dyninst::Address adjustTrapAddr(Dyninst::Address address, Dyninst::Architecture arch)


.. cpp:class:: freebsd_process : virtual public sysv_process, virtual public unix_process, virtual public x86_process, virtual public thread_db_process, virtual public mmap_alloc_process, virtual public hybrid_lwp_control_process

  .. cpp:function:: freebsd_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int, int> f)
  .. cpp:function:: freebsd_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~freebsd_process()
  .. cpp:function:: virtual bool plat_create()
  .. cpp:function:: virtual bool plat_attach(bool allStopped, bool &)
  .. cpp:function:: virtual bool plat_forked()
  .. cpp:function:: virtual bool plat_execed()
  .. cpp:function:: virtual bool plat_detach(result_response::ptr resp, bool leave_stopped)
  .. cpp:function:: virtual bool plat_terminate(bool &needs_sync)
  .. cpp:function:: virtual bool plat_readMem(int_thread *thr, void *local, Dyninst::Address remote, size_t size)
  .. cpp:function:: virtual bool plat_writeMem(int_thread *thr, const void *local, Dyninst::Address remote, size_t size, bp_write_t bp_write)
  .. cpp:function:: virtual bool needIndividualThreadAttach()
  .. cpp:function:: virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps)
  .. cpp:function:: virtual Dyninst::Architecture getTargetArch()
  .. cpp:function:: virtual bool plat_individualRegAccess()
  .. cpp:function:: virtual bool plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates)
  .. cpp:function:: virtual OSType getOS() const
  .. cpp:function:: virtual async_ret_t post_attach(bool wasDetached, std::set<response::ptr> &async_responses)
  .. cpp:function:: virtual async_ret_t post_create(std::set<response::ptr> &async_responses)
  .. cpp:function:: virtual int getEventQueue()
  .. cpp:function:: virtual bool initKQueueEvents()
  .. cpp:function:: virtual SymbolReaderFactory *plat_defaultSymReader()
  .. cpp:function:: virtual bool plat_threadOpsNeedProcStop()
  .. cpp:function:: virtual bool forked()
  .. cpp:function:: virtual bool isForking() const
  .. cpp:function:: virtual void setForking(bool b)
  .. cpp:function:: virtual bool post_forked()
  .. cpp:function:: virtual freebsd_process *getParent()
  .. cpp:function:: virtual const char *getThreadLibName(const char *symName)
  .. cpp:function:: virtual bool isSupportedThreadLib(string libName)
  .. cpp:function:: virtual bool plat_getLWPInfo(lwpid_t lwp, void *lwpInfo)
  .. cpp:function:: virtual bool plat_suspendThread(int_thread *thr)
  .. cpp:function:: virtual bool plat_resumeThread(int_thread *thr)
  .. cpp:member:: protected string libThreadName
  .. cpp:member:: protected bool forking
  .. cpp:member:: protected freebsd_process *parent


.. cpp:class:: freebsd_thread : public thread_db_thread

  .. cpp:function:: freebsd_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
  .. cpp:function:: freebsd_thread()
  .. cpp:function:: virtual ~freebsd_thread()
  .. cpp:function:: virtual bool plat_cont()
  .. cpp:function:: virtual bool plat_stop()
  .. cpp:function:: virtual bool plat_getAllRegisters(int_registerPool &reg)
  .. cpp:function:: virtual bool plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
  .. cpp:function:: virtual bool plat_setAllRegisters(int_registerPool &reg)
  .. cpp:function:: virtual bool plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val)
  .. cpp:function:: virtual bool attach()
  .. cpp:function:: virtual bool plat_suspend()
  .. cpp:function:: virtual bool plat_resume()
  .. cpp:function:: virtual bool plat_setStep()
  .. cpp:function:: void setBootstrapStop(bool b)
  .. cpp:function:: bool hasBootstrapStop() const
  .. cpp:function:: void setPCBugCondition(bool b)
  .. cpp:function:: bool hasPCBugCondition() const
  .. cpp:function:: void setPendingPCBugSignal(bool b)
  .. cpp:function:: bool hasPendingPCBugSignal() const
  .. cpp:function:: void setSignalStopped(bool b)
  .. cpp:function:: bool isSignalStopped() const
  .. cpp:function:: bool isAlive()
  .. cpp:member:: protected bool bootstrapStop
  .. cpp:member:: protected bool pcBugCondition
  .. cpp:member:: protected bool pendingPCBugSignal
  .. cpp:member:: protected bool signalStopped
  .. cpp:member:: protected bool is_pt_setstep
  .. cpp:member:: protected bool is_exited


.. cpp:class:: FreeBSDPollLWPDeathHandler : public Handler

  .. cpp:function:: FreeBSDPollLWPDeathHandler()
  .. cpp:function:: virtual ~FreeBSDPollLWPDeathHandler()
  .. cpp:function:: virtual Handler::handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: FreeBSDPostThreadDeathBreakpointHandler : public Handler

  .. cpp:function:: FreeBSDPostThreadDeathBreakpointHandler()
  .. cpp:function:: virtual ~FreeBSDPostThreadDeathBreakpointHandler()
  .. cpp:function:: virtual Handler::handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)

.. cpp:class:: FreeBSDPreStopHandler : public Handler

  Only available when ``bug_freebsd_mt_suspend`` is defined.

  .. cpp:function:: FreeBSDPreStopHandler()
  .. cpp:function:: virtual ~FreeBSDPreStopHandler()
  .. cpp:function:: virtual Handler::handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: FreeBSDBootstrapHandler : public Handler

  Only available when ``bug_freebsd_mt_suspend`` is defined.

  .. cpp:function:: FreeBSDBootstrapHandler()
  .. cpp:function:: virtual ~FreeBSDBootstrapHandler()
  .. cpp:function:: virtual Handler::handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)

.. cpp:class:: FreeBSDChangePCHandler : public Handler

  Only available when ``bug_freebsd_change_pc`` is defined.

  .. cpp:function:: FreeBSDChangePCHandler()
  .. cpp:function:: virtual ~FreeBSDChangePCHandler()
  .. cpp:function:: virtual Handler::handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)

.. cpp:class:: FreeBSDPreForkHandler : public Handler

  .. cpp:function:: FreeBSDPreForkHandler()
  .. cpp:function:: ~FreeBSDPreForkHandler()
  .. cpp:function:: virtual Handler::handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)
