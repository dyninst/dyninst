.. _`sec:linux.h`:

linux.h
#######

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:function:: long do_ptrace(pt_req request_, pid_t pid_, void *addr_, void *data_)
.. cpp:function:: SymbolReaderFactory* getElfReader()

.. cpp:type:: enum __ptrace_request pt_req

.. cpp:class:: GeneratorLinux : public GeneratorMT

  .. cpp:function:: GeneratorLinux()
  .. cpp:function:: virtual ~GeneratorLinux()
  .. cpp:function:: virtual bool initialize()
  .. cpp:function:: virtual bool canFastHandle()
  .. cpp:function:: virtual ArchEvent *getEvent(bool block)
  .. cpp:function:: void evictFromWaitpid()


.. cpp:class:: ArchEventLinux : public ArchEvent

  .. cpp:member:: int status
  .. cpp:member:: pid_t pid
  .. cpp:member:: bool interrupted
  .. cpp:member:: int error
  .. cpp:member:: pid_t child_pid
  .. cpp:member:: int event_ext
  .. cpp:function:: bool findPairedEvent(ArchEventLinux* &parent, ArchEventLinux* &child)
  .. cpp:function:: void postponePairedEvent()
  .. cpp:function:: ArchEventLinux(bool inter_)
  .. cpp:function:: ArchEventLinux(pid_t p, int s)
  .. cpp:function:: ArchEventLinux(int e)
  .. cpp:function:: virtual ~ArchEventLinux()


.. cpp:class:: DecoderLinux : public Decoder

  .. cpp:function:: DecoderLinux()
  .. cpp:function:: virtual ~DecoderLinux()
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual bool decode(ArchEvent *ae, std::vector<Event::ptr> &events)
  .. cpp:function:: Dyninst::Address adjustTrapAddr(Dyninst::Address address, Dyninst::Architecture arch)


.. cpp:class:: linux_process : public sysv_process, public unix_process, public thread_db_process, public indep_lwp_control_process, public mmap_alloc_process, public int_followFork, public int_signalMask, public int_LWPTracking, public int_memUsage

  .. note::
  
    The async functions are only used if a linux debugging mode, ``debug_async_simulate`` is enabled, which tries to get Linux
    to simulate having async events for testing purposes.

  .. cpp:function:: linux_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: linux_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~linux_process()
  .. cpp:function:: virtual bool plat_create()
  .. cpp:function:: virtual bool plat_create_int()
  .. cpp:function:: virtual bool plat_attach(bool allStopped, bool &)
  .. cpp:function:: virtual bool plat_attachThreadsSync()
  .. cpp:function:: virtual bool plat_attachWillTriggerStop()
  .. cpp:function:: virtual bool plat_forked()
  .. cpp:function:: virtual bool plat_execed()
  .. cpp:function:: virtual bool plat_detach(result_response::ptr resp, bool leave_stopped)
  .. cpp:function:: virtual bool plat_terminate(bool &needs_sync)
  .. cpp:function:: virtual bool preTerminate()
  .. cpp:function:: virtual OSType getOS() const
  .. cpp:function:: virtual bool plat_needsAsyncIO() const
  .. cpp:function:: virtual bool plat_readMemAsync(int_thread *thr, Dyninst::Address addr, mem_response::ptr result)
  .. cpp:function:: virtual bool plat_writeMemAsync(int_thread *thr, const void *local, Dyninst::Address addr, size_t size, result_response::ptr result, bp_write_t bp_write)
  .. cpp:function:: virtual bool plat_readMem(int_thread *thr, void *local, Dyninst::Address remote, size_t size)
  .. cpp:function:: virtual bool plat_writeMem(int_thread *thr, const void *local, Dyninst::Address remote, size_t size, bp_write_t bp_write)
  .. cpp:function:: virtual SymbolReaderFactory *plat_defaultSymReader()
  .. cpp:function:: virtual bool needIndividualThreadAttach()
  .. cpp:function:: virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps)
  .. cpp:function:: virtual bool plat_individualRegAccess()
  .. cpp:function:: virtual Dyninst::Address plat_mallocExecMemory(Dyninst::Address min, unsigned size)
  .. cpp:function:: virtual bool plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates)
  .. cpp:function:: virtual bool plat_supportLWPCreate()
  .. cpp:function:: virtual bool plat_supportLWPPreDestroy()
  .. cpp:function:: virtual bool plat_supportLWPPostDestroy()
  .. cpp:function:: virtual void plat_adjustSyncType(Event::ptr ev, bool gen)
  .. cpp:function:: virtual bool fork_setTracking(FollowFork::follow_t b)
  .. cpp:function:: virtual FollowFork::follow_t fork_isTracking()
  .. cpp:function:: virtual bool plat_lwpChangeTracking(bool b)
  .. cpp:function:: virtual bool allowSignal(int signal_no)
  .. cpp:function:: bool readStatM(unsigned long &stk, unsigned long &heap, unsigned long &shrd)
  .. cpp:function:: virtual bool plat_getStackUsage(MemUsageResp_t *resp)
  .. cpp:function:: virtual bool plat_getHeapUsage(MemUsageResp_t *resp)
  .. cpp:function:: virtual bool plat_getSharedUsage(MemUsageResp_t *resp)
  .. cpp:function:: virtual bool plat_residentNeedsMemVals()
  .. cpp:function:: virtual bool plat_getResidentUsage(unsigned long stacku, unsigned long heapu, unsigned long sharedu, MemUsageResp_t *resp)
  .. cpp:function:: protected int computeAddrWidth()


.. cpp:class:: linux_x86_process : public linux_process, public x86_process

  .. cpp:function:: linux_x86_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: linux_x86_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~linux_x86_process()
  .. cpp:function:: virtual Dyninst::Architecture getTargetArch()
  .. cpp:function:: virtual bool plat_supportHWBreakpoint()


.. cpp:class:: linux_ppc_process : public linux_process, public ppc_process

  .. cpp:function:: linux_ppc_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: linux_ppc_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~linux_ppc_process()
  .. cpp:function:: virtual Dyninst::Architecture getTargetArch()


.. cpp:class:: linux_arm_process : public linux_process, public arm_process

  .. cpp:function:: linux_arm_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: linux_arm_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~linux_arm_process()
  .. cpp:function:: virtual Dyninst::Architecture getTargetArch()


.. cpp:class:: linux_thread : virtual public thread_db_thread

  .. cpp:function:: linux_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
  .. cpp:function:: virtual ~linux_thread()
  .. cpp:function:: virtual bool plat_cont()
  .. cpp:function:: virtual bool plat_stop()
  .. cpp:function:: virtual bool plat_getAllRegisters(int_registerPool &reg)
  .. cpp:function:: virtual bool plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
  .. cpp:function:: virtual bool plat_setAllRegisters(int_registerPool &reg)
  .. cpp:function:: virtual bool plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val)
  .. cpp:function:: virtual bool attach()
  .. cpp:function:: virtual bool plat_getAllRegistersAsync(allreg_response::ptr result)
  .. cpp:function:: virtual bool plat_getRegisterAsync(Dyninst::MachRegister reg, reg_response::ptr result)
  .. cpp:function:: virtual bool plat_setAllRegistersAsync(int_registerPool &pool, result_response::ptr result)
  .. cpp:function:: virtual bool plat_setRegisterAsync(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val, result_response::ptr result)
  .. cpp:function:: virtual bool thrdb_getThreadArea(int val, Dyninst::Address &addr)
  .. cpp:function:: virtual bool plat_convertToSystemRegs(const int_registerPool &pool, unsigned char *regs, bool gprs_only = false)
  .. cpp:function:: virtual bool plat_handle_ghost_thread()
  .. cpp:function:: void setOptions()
  .. cpp:function:: bool unsetOptions()
  .. cpp:function:: bool getSegmentBase(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
  .. cpp:function:: void postponeSyscallEvent(ArchEventLinux *event)
  .. cpp:function:: bool hasPostponedSyscallEvent()
  .. cpp:function:: ArchEventLinux *getPostponedSyscallEvent()
  .. cpp:function:: static void fake_async_main(void *)
  .. cpp:function:: virtual bool suppressSanityChecks()
  .. cpp:function:: void setGeneratorExiting()


.. cpp:class:: linux_x86_thread : virtual public linux_thread, virtual public x86_thread

  .. cpp:function:: linux_x86_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
  .. cpp:function:: virtual ~linux_x86_thread()


.. cpp:class:: linux_ppc_thread : virtual public linux_thread, virtual public ppc_thread

  .. cpp:function:: linux_ppc_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
  .. cpp:function:: virtual ~linux_ppc_thread()


.. cpp:class:: linux_arm_thread : virtual public linux_thread, virtual public arm_thread

  .. cpp:function:: linux_arm_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
  .. cpp:function:: virtual ~linux_arm_thread()

.. cpp:class:: LinuxPtrace

  .. cpp:function:: static LinuxPtrace *getPtracer()
  .. cpp:function:: LinuxPtrace()
  .. cpp:function:: ~LinuxPtrace()
  .. cpp:function:: void start()
  .. cpp:function:: void main()
  .. cpp:function:: long ptrace_int(pt_req request_, pid_t pid_, void *addr_, void *data_)
  .. cpp:function:: bool ptrace_read(Dyninst::Address inTrace, unsigned size_, void *inSelf, int pid_)
  .. cpp:function:: bool ptrace_write(Dyninst::Address inTrace, unsigned size_, const void *inSelf, int pid_)
  .. cpp:function:: bool plat_create(linux_process *p)


.. cpp:class:: LinuxHandleNewThr : public Handler

  .. cpp:function:: LinuxHandleNewThr()
  .. cpp:function:: virtual ~LinuxHandleNewThr()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: LinuxHandleLWPDestroy : public Handler

  .. cpp:function:: LinuxHandleLWPDestroy()
  .. cpp:function:: virtual ~LinuxHandleLWPDestroy()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: LinuxHandleForceTerminate : public Handler

  .. cpp:function:: LinuxHandleForceTerminate()
  .. cpp:function:: virtual ~LinuxHandleForceTerminate()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)




