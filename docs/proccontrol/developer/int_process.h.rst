.. _`sec:int_process.h`:

int_process.h
#############

.. cpp:type:: std::multimap<Dyninst::Address, Dyninst::ProcControlAPI::Process::ptr> int_addressSet
.. cpp:type:: std::set<Dyninst::ProcControlAPI::Process::ptr> int_processSet
.. cpp:type:: std::set<Dyninst::ProcControlAPI::Thread::ptr> int_threadSet
.. cpp:type:: boost::shared_ptr<int_iRPC> int_iRPC_ptr
.. cpp:type:: std::map<Dyninst::MachRegister, std::pair<unsigned int, unsigned int> > dynreg_to_user_t
.. cpp:type:: std::list<int_iRPC_ptr> rpc_list_t
.. cpp:type:: void* hwbp_key_t

.. cpp:class:: int_process

  This is the central class in PC, representing both a process and
  and the porting interface needed to move PC to a new platform.

  Most of the information about a process hangs off of here (except
  for memory related info, see mem_state above).  This includes
  PIDs, execution state, environment, etc.

  There are numerous pure virtual functions below, most beginning
  with plat_*.  To port PC to a new platform make a new class (ie.,
  linux_process) that inherits from int_process and fill-in these
  pure virtual functions.  Then have int_process::createProcess
  return your new class, but cast back to an int_process.

  There are existing child classes of int_process that you can
  use if your system shares certain things in common with other
  platforms.  For example, several systems (Linux, FreeBSD)
  use the System V interfaces for library loading.  Thus there
  exists a sysv_process class that inherits from int_process and
  fills in the library handling virtual functions of int_process.
  Other examples are x86_process, which fill things like how
  to build a breakpoint instruction.

  By having the new platforms inherit from these you leverage
  a lot of existing work.  Note that new ports will also have
  to implement their own decoders and generators.

  .. cpp:function:: protected int_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: protected int_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: static int_process *createProcess(Dyninst::PID p, std::string e)
  .. cpp:function:: static int_process *createProcess(std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: static int_process *createProcess(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~int_process()
  .. cpp:function:: protected static bool create(int_processSet *ps)
  .. cpp:function:: protected virtual bool plat_create() = 0
  .. cpp:function:: protected virtual async_ret_t post_create(std::set<response::ptr> &async_responses)
  .. cpp:function:: protected static bool attach(int_processSet *ps, bool reattach)
  .. cpp:function:: protected static bool reattach(int_processSet *pset)
  .. cpp:function:: protected virtual bool plat_attach(bool allStopped, bool &should_sync) = 0
  .. cpp:function:: protected bool attachThreads(bool &found_new_threads)
  .. cpp:function:: protected bool attachThreads()
  .. cpp:function:: protected virtual bool plat_attachThreadsSync()
  .. cpp:function:: protected virtual async_ret_t post_attach(bool wasDetached, std::set<response::ptr> &aresps)
  .. cpp:function:: protected async_ret_t initializeAddressSpace(std::set<response::ptr> &async_responses)
  .. cpp:function:: protected virtual bool plat_syncRunState() = 0
  .. cpp:function:: protected bool syncRunState()
  .. cpp:function:: creationMode_t getCreationMode() const
  .. cpp:function:: void setContSignal(int sig)
  .. cpp:function:: int getContSignal() const
  .. cpp:function:: virtual bool forked()
  .. cpp:function:: virtual OSType getOS() const = 0
  .. cpp:function:: protected virtual bool plat_forked() = 0
  .. cpp:function:: protected virtual bool post_forked()
  .. cpp:function:: bool execed()
  .. cpp:function:: virtual bool plat_detach(result_response::ptr resp, bool leave_stopped) = 0
  .. cpp:function:: virtual bool plat_detachDone()
  .. cpp:function:: protected virtual bool plat_execed()
  .. cpp:function:: protected virtual bool plat_terminate(bool &needs_sync) = 0
  .. cpp:function:: protected virtual bool needIndividualThreadAttach() = 0
  .. cpp:function:: protected virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps)
  .. cpp:function:: protected virtual void plat_threadAttachDone()
  .. cpp:function:: protected bool waitfor_startup()
  .. cpp:function:: protected void setPid(Dyninst::PID pid)
  .. cpp:function:: protected int_thread *findStoppedThread()
  .. cpp:function:: virtual bool plat_processGroupContinues()
  .. cpp:function:: State getState() const
  .. cpp:function:: void setState(State s)
  .. cpp:function:: Dyninst::PID getPid() const
  .. cpp:function:: int_threadPool *threadPool() const
  .. cpp:function:: Process::ptr proc() const
  .. cpp:function:: mem_state::ptr memory() const
  .. cpp:function:: err_t getLastError()
  .. cpp:function:: const char *getLastErrorMsg()
  .. cpp:function:: void clearLastError()
  .. cpp:function:: void setLastError(err_t err, const char *err_str)
  .. cpp:function:: void throwDetachEvent(bool temporary, bool leaveStopped)
  .. cpp:function:: virtual bool preTerminate()
  .. cpp:function:: bool terminate(bool &needs_sync)
  .. cpp:function:: void updateSyncState(Event::ptr ev, bool gen)
  .. cpp:function:: virtual void plat_adjustSyncType(Event::ptr, bool)
  .. cpp:function:: virtual Dyninst::Architecture getTargetArch() = 0
  .. cpp:function:: virtual unsigned getTargetPageSize() = 0
  .. cpp:function:: virtual unsigned plat_getRecommendedReadSize()
  .. cpp:function:: virtual Dyninst::Address mallocExecMemory(unsigned size)
  .. cpp:function:: virtual Dyninst::Address plat_mallocExecMemory(Dyninst::Address min, unsigned size) = 0
  .. cpp:function:: virtual void freeExecMemory(Dyninst::Address addr)
  .. cpp:function:: static bool waitAndHandleEvents(bool block)
  .. cpp:function:: static bool waitAndHandleForProc(bool block, int_process *proc, bool &proc_exited)
  .. cpp:function:: static bool waitForAsyncEvent(response::ptr resp)
  .. cpp:function:: static bool waitForAsyncEvent(std::set<response::ptr> resp)
  .. cpp:function:: virtual bool plat_waitAndHandleForProc()
  .. cpp:function:: Counter &asyncEventCount()
  .. cpp:function:: Counter &getForceGeneratorBlockCount()
  .. cpp:function:: Counter &getStartupTeardownProcs()
  .. cpp:function:: static const char *stateName(State s)
  .. cpp:function:: void initializeProcess(Process::ptr p)
  .. cpp:function:: virtual void instantiateRPCThread()
  .. cpp:function:: void setExitCode(int c)
  .. cpp:function:: void setCrashSignal(int s)
  .. cpp:function:: bool getExitCode(int &c)
  .. cpp:function:: bool getCrashSignal(int &s)
  .. cpp:function:: bool wasForcedTerminated() const
  .. cpp:function:: virtual bool plat_individualRegAccess() = 0
  .. cpp:function:: virtual bool plat_individualRegRead(Dyninst::MachRegister reg, int_thread *thr)
  .. cpp:function:: virtual bool plat_individualRegSet()
  .. cpp:function:: int getAddressWidth()
  .. cpp:function:: HandlerPool *handlerPool() const
  .. cpp:function:: bool addBreakpoint(Dyninst::Address addr, int_breakpoint *bp)
  .. cpp:function:: bool addBreakpoint_phase1(bp_install_state *is)
  .. cpp:function:: bool addBreakpoint_phase2(bp_install_state *is)
  .. cpp:function:: bool addBreakpoint_phase3(bp_install_state *is)
  .. cpp:function:: bool removeBreakpoint(Dyninst::Address addr, int_breakpoint *bp, std::set<response::ptr> &resps)
  .. cpp:function:: bool removeAllBreakpoints()
  .. cpp:function:: sw_breakpoint *getBreakpoint(Dyninst::Address addr)
  .. cpp:function:: virtual unsigned plat_breakpointSize() = 0
  .. cpp:function:: virtual void plat_breakpointBytes(unsigned char *buffer) = 0
  .. cpp:function:: virtual bool plat_breakpointAdvancesPC() const = 0
  .. cpp:function:: virtual bool plat_createDeallocationSnippet(Dyninst::Address addr, unsigned long size, void* &buffer, unsigned long &buffer_size, unsigned long &start_offset) = 0
  .. cpp:function:: virtual bool plat_createAllocationSnippet(Dyninst::Address addr, bool use_addr, unsigned long size, void* &buffer, unsigned long &buffer_size, unsigned long &start_offset) = 0
  .. cpp:function:: virtual bool plat_collectAllocationResult(int_thread *thr, reg_response::ptr resp) = 0
  .. cpp:function:: virtual bool plat_threadOpsNeedProcStop()
  .. cpp:function:: virtual SymbolReaderFactory *plat_defaultSymReader()
  .. cpp:function:: virtual SymbolReaderFactory *getSymReader()
  .. cpp:function:: virtual void setSymReader(SymbolReaderFactory *fact)
  .. cpp:function:: virtual Dyninst::Address direct_infMalloc(unsigned long size, bool use_addr = false, Dyninst::Address addr = 0x0)
  .. cpp:function:: virtual bool direct_infFree(Dyninst::Address addr)
  .. cpp:function:: Address infMalloc(unsigned long size, bool use_addr, Address addr)
  .. cpp:function:: bool infFree(Address addr)
  .. cpp:function:: static bool infMalloc(unsigned long size, int_addressSet *aset, bool use_addr)
  .. cpp:function:: static bool infFree(int_addressSet *aset)
  .. cpp:function:: static std::string plat_canonicalizeFileName(std::string s)
  .. cpp:function:: bool readMem(Dyninst::Address remote, mem_response::ptr result, int_thread *thr = NULL)
  .. cpp:function:: bool writeMem(const void *local, Dyninst::Address remote, size_t size, result_response::ptr result, int_thread *thr = NULL, bp_write_t bp_write = not_bp)
  .. cpp:function:: virtual bool plat_readMem(int_thread *thr, void *local, Dyninst::Address remote, size_t size) = 0
  .. cpp:function:: virtual bool plat_writeMem(int_thread *thr, const void *local, Dyninst::Address remote, size_t size, bp_write_t bp_write) = 0
  .. cpp:function:: virtual async_ret_t plat_calcTLSAddress(int_thread *thread, int_library *lib, Offset off, Address &outaddr, std::set<response::ptr> &resps)
  .. cpp:function:: virtual Address plat_findFreeMemory(size_t)
  .. cpp:function:: virtual bool plat_needsAsyncIO() const
  .. cpp:function:: virtual bool plat_readMemAsync(int_thread *thr, Dyninst::Address addr, mem_response::ptr result)

      If :cpp:func:`plat_needsAsyncIO` returns ``true``, then the async set of functions need to be implemented. By default they are not.

  .. cpp:function:: virtual bool plat_writeMemAsync(int_thread *thr, const void *local, Dyninst::Address addr, size_t size, result_response::ptr result, bp_write_t bp_write)

      If :cpp:func:`plat_needsAsyncIO` returns ``true``, then the async set of functions need to be implemented. By default they are not.

  .. cpp:function:: bool getMemoryAccessRights(Dyninst::Address addr, Process::mem_perm& rights)
  .. cpp:function:: bool setMemoryAccessRights(Dyninst::Address addr, size_t size, Process::mem_perm rights, Process::mem_perm& oldRights)
  .. cpp:function:: virtual bool plat_getMemoryAccessRights(Dyninst::Address addr, Process::mem_perm& rights)
  .. cpp:function:: virtual bool plat_setMemoryAccessRights(Dyninst::Address addr, size_t size, Process::mem_perm rights, Process::mem_perm& oldRights)
  .. cpp:function:: virtual bool plat_decodeMemoryRights(Process::mem_perm& rights_internal, unsigned long rights)
  .. cpp:function:: virtual bool plat_encodeMemoryRights(Process::mem_perm rights_internal, unsigned long& rights)
  .. cpp:function:: virtual bool findAllocatedRegionAround(Dyninst::Address addr, Process::MemoryRegion& memRegion)
  .. cpp:function:: virtual bool plat_findAllocatedRegionAround(Dyninst::Address addr, Process::MemoryRegion& memRegion)
  .. cpp:function:: memCache *getMemCache()
  .. cpp:function:: virtual bool plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates) = 0
  .. cpp:function:: virtual void* plat_getDummyThreadHandle() const
  .. cpp:function:: virtual void noteNewDequeuedEvent(Event::ptr ev)
  .. cpp:function:: static bool isInCB()
  .. cpp:function:: static void setInCB(bool b)
  .. cpp:function:: void throwNopEvent()
  .. cpp:function:: void throwRPCPostEvent()
  .. cpp:function:: virtual bool plat_supportFork()
  .. cpp:function:: virtual bool plat_supportExec()
  .. cpp:function:: virtual bool plat_supportDOTF()
  .. cpp:function:: virtual bool plat_supportThreadEvents()
  .. cpp:function:: virtual bool plat_supportLWPCreate()
  .. cpp:function:: virtual bool plat_supportLWPPreDestroy()
  .. cpp:function:: virtual bool plat_supportLWPPostDestroy()
  .. cpp:function:: virtual bool plat_preHandleEvent()
  .. cpp:function:: virtual bool plat_postHandleEvent()
  .. cpp:function:: virtual bool plat_preAsyncWait()
  .. cpp:function:: virtual bool plat_supportHWBreakpoint()
  .. cpp:function:: virtual bool plat_needsPCSaveBeforeSingleStep()
  .. cpp:function:: virtual async_ret_t plat_needsEmulatedSingleStep(int_thread *thr, std::vector<Dyninst::Address> &result)
  .. cpp:function:: virtual bool plat_convertToBreakpointAddress(Address &, int_thread *)
  .. cpp:function:: virtual void plat_getEmulatedSingleStepAsyncs(int_thread *thr, std::set<response::ptr> resps)
  .. cpp:function:: virtual bool plat_needsThreadForMemOps() const
  .. cpp:function:: virtual unsigned int plat_getCapabilities()
  .. cpp:function:: virtual Event::ptr plat_throwEventsBeforeContinue(int_thread *thr)
  .. cpp:function:: int_library *getLibraryByName(std::string s) const
  .. cpp:function:: size_t numLibs() const
  .. cpp:function:: virtual bool refresh_libraries(std::set<int_library *> &added_libs, std::set<int_library *> &rmd_libs, bool &waiting_for_async, std::set<response::ptr> &async_responses) = 0
  .. cpp:function:: virtual bool plat_isStaticBinary() = 0
  .. cpp:function:: virtual int_library *plat_getExecutable() = 0
  .. cpp:function:: virtual bool plat_supportDirectAllocation() const
  .. cpp:function:: bool forceGeneratorBlock() const
  .. cpp:function:: void setForceGeneratorBlock(bool b)
  .. cpp:function:: std::string getExecutable() const
  .. cpp:function:: static bool isInCallback()
  .. cpp:member:: static int_process *in_waitHandleProc
  .. cpp:function:: bool wasCreatedViaAttach() const
  .. cpp:function:: void wasCreatedViaAttach(bool val)
  .. cpp:function:: virtual bool addrInSystemLib(Address addr)

      Platform-specific; is this address in what we consider a system lib.

  .. cpp:function:: ProcStopEventManager &getProcStopManager()
  .. cpp:function:: std::map<int, int> &getProcDesyncdStates()
  .. cpp:function:: bool isRunningSilent()
  .. cpp:function:: void setRunningSilent(bool b)
  .. cpp:function:: virtual ExecFileInfo* plat_getExecutableInfo() const
  .. cpp:function:: int_libraryTracking *getLibraryTracking()
  .. cpp:function:: int_LWPTracking *getLWPTracking()
  .. cpp:function:: int_threadTracking *getThreadTracking()
  .. cpp:function:: int_followFork *getFollowFork()
  .. cpp:function:: int_multiToolControl *getMultiToolControl()
  .. cpp:function:: int_signalMask *getSignalMask()
  .. cpp:function:: int_memUsage *getMemUsage()
  .. cpp:function:: int_callStackUnwinding *getCallStackUnwinding()
  .. cpp:function:: int_remoteIO *getRemoteIO()
  .. cpp:member:: protected State state
  .. cpp:member:: protected Dyninst::PID pid
  .. cpp:member:: protected creationMode_t creation_mode
  .. cpp:member:: protected std::string executable
  .. cpp:member:: protected std::vector<std::string> argv
  .. cpp:member:: protected std::vector<std::string> env
  .. cpp:member:: protected std::map<int,int> fds
  .. cpp:member:: protected Dyninst::Architecture arch
  .. cpp:member:: protected int_threadPool *threadpool
  .. cpp:member:: protected Process::ptr up_proc
  .. cpp:member:: protected HandlerPool *handlerpool
  .. cpp:member:: protected LibraryPool libpool
  .. cpp:member:: protected bool hasCrashSignal
  .. cpp:member:: protected int crashSignal
  .. cpp:member:: protected bool hasExitCode
  .. cpp:member:: protected bool forcedTermination
  .. cpp:member:: protected bool silent_mode
  .. cpp:member:: protected int exitCode
  .. cpp:member:: protected static bool in_callback
  .. cpp:member:: protected mem_state::ptr mem
  .. cpp:member:: protected std::map<Dyninst::Address, unsigned> exec_mem_cache
  .. cpp:member:: protected int continueSig
  .. cpp:member:: protected bool createdViaAttach
  .. cpp:member:: protected memCache mem_cache
  .. cpp:member:: protected Counter async_event_count
  .. cpp:member:: protected Counter force_generator_block_count
  .. cpp:member:: protected Counter startupteardown_procs
  .. cpp:member:: protected ProcStopEventManager proc_stop_manager
  .. cpp:member:: protected std::map<int, int> proc_desyncd_states
  .. cpp:member:: protected void *user_data
  .. cpp:member:: protected err_t last_error
  .. cpp:member:: protected const char *last_error_string
  .. cpp:member:: protected SymbolReaderFactory *symbol_reader
  .. cpp:member:: protected static SymbolReaderFactory *user_set_symbol_reader
  .. cpp:member:: protected int_libraryTracking *pLibraryTracking
  .. cpp:member:: protected int_LWPTracking *pLWPTracking
  .. cpp:member:: protected int_threadTracking *pThreadTracking
  .. cpp:member:: protected int_followFork *pFollowFork
  .. cpp:member:: protected int_multiToolControl *pMultiToolControl
  .. cpp:member:: protected int_signalMask *pSignalMask
  .. cpp:member:: protected int_callStackUnwinding *pCallStackUnwinding
  .. cpp:member:: protected int_memUsage *pMemUsage
  .. cpp:member:: protected int_remoteIO *pRemoteIO
  .. cpp:member:: protected bool LibraryTracking_set
  .. cpp:member:: protected bool LWPTracking_set
  .. cpp:member:: protected bool ThreadTracking_set
  .. cpp:member:: protected bool FollowFork_set
  .. cpp:member:: protected bool MultiToolControl_set
  .. cpp:member:: protected bool SignalMask_set
  .. cpp:member:: protected bool CallStackUnwinding_set
  .. cpp:member:: protected bool MemUsage_set
  .. cpp:member:: protected bool remoteIO_set


.. cpp:enum:: int_process::creationMode_t

  .. cpp:enumerator:: ct_fork
  .. cpp:enumerator:: ct_launch
  .. cpp:enumerator:: ct_attach


.. cpp:enum:: int_process::State

  .. cpp:enumerator:: neonatal = 0
  .. cpp:enumerator:: neonatal_intermediate
  .. cpp:enumerator:: detached
  .. cpp:enumerator:: running
  .. cpp:enumerator:: exited
  .. cpp:enumerator:: errorstate


.. cpp:enum:: int_process::bp_write_t

  .. cpp:enumerator:: not_bp
  .. cpp:enumerator:: bp_install
  .. cpp:enumerator:: bp_clear


.. cpp:class:: indep_lwp_control_process : virtual public int_process

  These processes represent four common models of how to stop/continue threads.
  If a new platform follows one of these, then inherit them from the appropriate class.

  - indep_lwp_control_process

    - Each thread/lwp stops and continues independent from each other one.  (Linux)
  - unified_lwp_control_process

    - There is no thread-specific control, every thread stops/runs alongside its peers (BG/P)
  - hybrid_lwp_control_process

    - All threads in a process are run/stopped when a thread stops/runs, but threads can be overridden with a
      suspend state that can keep them stopped when others run (FreeBSD, Windows, BG/Q).

  .. cpp:function:: protected virtual bool plat_syncRunState()
  .. cpp:function:: indep_lwp_control_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: indep_lwp_control_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~indep_lwp_control_process()


.. cpp:class:: unified_lwp_control_process : virtual public int_process

  .. cpp:function:: protected virtual bool plat_syncRunState()
  .. cpp:function:: unified_lwp_control_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: unified_lwp_control_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~unified_lwp_control_process()
  .. cpp:function:: virtual bool plat_processGroupContinues()


.. cpp:class:: hybrid_lwp_control_process : virtual public int_process

  .. cpp:function:: protected virtual bool plat_syncRunState()
  .. cpp:function:: protected virtual bool plat_suspendThread(int_thread *thr) = 0
  .. cpp:function:: protected virtual bool plat_resumeThread(int_thread *thr) = 0
  .. cpp:member:: protected bool debugger_stopped
  .. cpp:function:: hybrid_lwp_control_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: hybrid_lwp_control_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~hybrid_lwp_control_process()
  .. cpp:function:: virtual bool suspendThread(int_thread *thr)
  .. cpp:function:: virtual bool resumeThread(int_thread *thr)
  .. cpp:function:: virtual void noteNewDequeuedEvent(Event::ptr ev)
  .. cpp:function:: virtual bool plat_debuggerSuspended()
  .. cpp:function:: virtual bool plat_processGroupContinues()


.. cpp:class:: int_registerPool

  A collection of registers from a thread.

  .. cpp:function:: int_registerPool()
  .. cpp:type:: std::map<Dyninst::MachRegister, Dyninst::MachRegisterVal> reg_map_t
  .. cpp:member:: reg_map_t regs
  .. cpp:member:: bool full
  .. cpp:member:: int_thread *thread
  .. cpp:type:: reg_map_t::iterator iterator
  .. cpp:type:: reg_map_t::const_iterator const_iterator


.. cpp:class:: thread_exitstate

  When a thread/process exits we delete the int_process/int_thread objects.  But,
  we leave the UI handles Process/Thread around until the user gets rid of their
  last pointer.  There are only a few operations that are legal on an exited process
  (such as getting the pid or exitcode).  thread_exitstate and proc_exitstate hold that
  information after a process exits.

  .. cpp:member:: Dyninst::LWP lwp
  .. cpp:member:: Dyninst::THR_ID thr_id
  .. cpp:member:: Process::ptr proc_ptr
  .. cpp:member:: void *user_data


.. cpp:class:: proc_exitstate

  .. cpp:member:: Dyninst::PID pid
  .. cpp:member:: bool exited
  .. cpp:member:: bool crashed
  .. cpp:member:: int crash_signal
  .. cpp:member:: int exit_code
  .. cpp:member:: err_t last_error
  .. cpp:member:: const char *last_error_msg
  .. cpp:member:: void *user_data
  .. cpp:function:: void setLastError(err_t e_, const char *m)


.. cpp:class:: int_thread

  int_thread repesents a thread/lwp (PC assumes an ``M:N`` model of ``1:1``).

  An int_process also holds the stopped/running state of a process.  See the StateTracker
  comment for a longer discussion here.

  .. cpp:function:: protected int_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
  .. cpp:function:: protected static int_thread *createThreadPlat(int_process *proc, Dyninst::THR_ID thr_id, Dyninst::LWP lwp_id, bool initial_thrd)
  .. cpp:function:: static int_thread *createThread(int_process *proc, Dyninst::THR_ID thr_id, Dyninst::LWP lwp_id, bool initial_thrd, attach_status_t astatus = as_unknown)
  .. cpp:function:: static int_thread *createRPCThread(int_process *p)
  .. cpp:function:: Process::ptr proc() const
  .. cpp:function:: int_process *llproc() const
  .. cpp:function:: Dyninst::LWP getLWP() const
  .. cpp:function:: void changeLWP(Dyninst::LWP new_lwp)

  .. cpp:member:: static const int NumStateIDs       = 19
  .. cpp:member:: static const int NumTargetStateIDs = (NumStateIDs-2)

      Handler and Generator states aren't target states.

  .. cpp:member:: static const int AsyncStateID            = 0
  .. cpp:member:: static const int CallbackStateID         = 1
  .. cpp:member:: static const int PostponedSyscallStateID = 2
  .. cpp:member:: static const int PendingStopStateID      = 3
  .. cpp:member:: static const int IRPCStateID             = 4
  .. cpp:member:: static const int IRPCSetupStateID        = 5
  .. cpp:member:: static const int IRPCWaitStateID         = 6
  .. cpp:member:: static const int BreakpointStateID       = 7
  .. cpp:member:: static const int BreakpointHoldStateID   = 8
  .. cpp:member:: static const int BreakpointResumeStateID = 9
  .. cpp:member:: static const int ExitingStateID          = 10
  .. cpp:member:: static const int InternalStateID         = 11
  .. cpp:member:: static const int StartupStateID          = 12
  .. cpp:member:: static const int DetachStateID           = 13
  .. cpp:member:: static const int UserRPCStateID          = 14
  .. cpp:member:: static const int ControlAuthorityStateID = 15
  .. cpp:member:: static const int UserStateID             = 16
  .. cpp:member:: static const int HandlerStateID          = 17
  .. cpp:member:: static const int GeneratorStateID        = 18

  .. note::    The order of these IDs is very important. Lower-numbered states take precedence over higher numbered states.

  .. cpp:function:: static std::string stateIDToName(int id)

  .. cpp:function:: StateTracker &getPostponedSyscallState()
  .. cpp:function:: StateTracker &getExitingState()
  .. cpp:function:: StateTracker &getStartupState()
  .. cpp:function:: StateTracker &getBreakpointState()
  .. cpp:function:: StateTracker &getBreakpointResumeState()
  .. cpp:function:: StateTracker &getBreakpointHoldState()
  .. cpp:function:: StateTracker &getCallbackState()
  .. cpp:function:: StateTracker &getIRPCState()
  .. cpp:function:: StateTracker &getIRPCSetupState()
  .. cpp:function:: StateTracker &getIRPCWaitState()
  .. cpp:function:: StateTracker &getAsyncState()
  .. cpp:function:: StateTracker &getInternalState()
  .. cpp:function:: StateTracker &getDetachState()
  .. cpp:function:: StateTracker &getControlAuthorityState()
  .. cpp:function:: StateTracker &getUserRPCState()
  .. cpp:function:: StateTracker &getUserState()
  .. cpp:function:: StateTracker &getHandlerState()
  .. cpp:function:: StateTracker &getGeneratorState()
  .. cpp:function:: StateTracker &getPendingStopState()
  .. cpp:function:: StateTracker &getStateByID(int id)
  .. cpp:function:: StateTracker &getActiveState()
  .. cpp:function:: static char stateLetter(State s)
  .. cpp:function:: Counter &handlerRunningThreadsCount()
  .. cpp:function:: Counter &generatorRunningThreadsCount()
  .. cpp:function:: Counter &syncRPCCount()
  .. cpp:function:: Counter &runningSyncRPCThreadCount()
  .. cpp:function:: Counter &pendingStopsCount()
  .. cpp:function:: Counter &clearingBPCount()
  .. cpp:function:: Counter &procStopRPCCount()
  .. cpp:function:: Counter &getGeneratorNonExitedThreadCount()
  .. cpp:function:: Counter &neonatalThreadCount()
  .. cpp:function:: Counter &pendingStackwalkCount()
  .. cpp:function:: bool intStop()
  .. cpp:function:: bool intCont()
  .. cpp:function:: async_ret_t handleSingleStepContinue()
  .. cpp:function:: void terminate()
  .. cpp:function:: void setContSignal(int sig)
  .. cpp:function:: int getContSignal()
  .. cpp:function:: virtual bool plat_cont() = 0
  .. cpp:function:: virtual bool plat_stop() = 0
  .. cpp:function:: void setPendingStop(bool b)
  .. cpp:function:: bool hasPendingStop() const
  .. cpp:function:: bool wasRunningWhenAttached() const
  .. cpp:function:: void setRunningWhenAttached(bool b)
  .. cpp:function:: bool isStopped(int state_id)
  .. cpp:function:: virtual bool isRPCEphemeral() const

      Is this thread's lifetime only an IRPC and it gets discarded afterwards?

  .. cpp:function:: bool singleStepMode() const
  .. cpp:function:: void setSingleStepMode(bool s)
  .. cpp:function:: bool singleStepUserMode() const
  .. cpp:function:: void setSingleStepUserMode(bool s)
  .. cpp:function:: bool singleStep() const
  .. cpp:function:: void markClearingBreakpoint(bp_instance *bp)
  .. cpp:function:: bp_instance *isClearingBreakpoint()
  .. cpp:function:: void markStoppedOnBP(bp_instance *bp)
  .. cpp:function:: bp_instance *isStoppedOnBP()
  .. cpp:function:: bool syscallUserMode() const
  .. cpp:function:: void setSyscallUserMode(bool s)
  .. cpp:function:: bool syscallMode() const
  .. cpp:function:: bool preSyscall()
  .. cpp:function:: void addEmulatedSingleStep(emulated_singlestep *es)
  .. cpp:function:: void rmEmulatedSingleStep(emulated_singlestep *es)
  .. cpp:function:: emulated_singlestep *getEmulatedSingleStep()
  .. cpp:function:: void addPostedRPC(int_iRPC_ptr rpc_)
  .. cpp:function:: rpc_list_t *getPostedRPCs()
  .. cpp:function:: bool hasPostedRPCs()
  .. cpp:function:: void setRunningRPC(int_iRPC_ptr rpc_)
  .. cpp:function:: void clearRunningRPC()
  .. cpp:function:: int_iRPC_ptr runningRPC() const
  .. cpp:function:: bool saveRegsForRPC(allreg_response::ptr response)
  .. cpp:function:: bool restoreRegsForRPC(bool clear, result_response::ptr response)
  .. cpp:function:: bool hasSavedRPCRegs()
  .. cpp:function:: void incSyncRPCCount()
  .. cpp:function:: void decSyncRPCCount()
  .. cpp:function:: bool hasSyncRPC()
  .. cpp:function:: int_iRPC_ptr nextPostedIRPC() const
  .. cpp:function:: int_iRPC_ptr hasRunningProcStopperRPC() const
  .. cpp:function:: virtual bool notAvailableForRPC()
  .. cpp:function:: bool getAllRegisters(allreg_response::ptr result)
  .. cpp:function:: bool getRegister(Dyninst::MachRegister reg, reg_response::ptr result)
  .. cpp:function:: bool setAllRegisters(int_registerPool &pool, result_response::ptr result)
  .. cpp:function:: bool setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val, result_response::ptr result)
  .. cpp:function:: virtual bool plat_getAllRegisters(int_registerPool &pool) = 0
  .. cpp:function:: virtual bool plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val) = 0
  .. cpp:function:: virtual bool plat_setAllRegisters(int_registerPool &pool) = 0
  .. cpp:function:: virtual bool plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val) = 0
  .. cpp:function:: virtual bool plat_getAllRegistersAsync(allreg_response::ptr result)
  .. cpp:function:: virtual bool plat_getRegisterAsync(Dyninst::MachRegister reg, reg_response::ptr result)
  .. cpp:function:: virtual bool plat_setAllRegistersAsync(int_registerPool &pool, result_response::ptr result)
  .. cpp:function:: virtual bool plat_setRegisterAsync(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val, result_response::ptr result)
  .. cpp:function:: virtual bool plat_handle_ghost_thread()
  .. cpp:function:: virtual void plat_terminate()
  .. cpp:function:: void updateRegCache(int_registerPool &pool)
  .. cpp:function:: void updateRegCache(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val)
  .. cpp:function:: void clearRegCache()
  .. cpp:function:: bool isExiting() const

      The exiting property is separate from the main state because an exiting thread can either be running or stopped (depending
      on the desires of the user).

  .. cpp:function:: void setExiting(bool b)
  .. cpp:function:: bool isExitingInGenerator() const
  .. cpp:function:: void setExitingInGenerator(bool b)
  .. cpp:function:: static void cleanFromHandler(int_thread *thr, bool should_delete)
  .. cpp:function:: virtual bool attach() = 0
  .. cpp:function:: Thread::ptr thread()
  .. cpp:type:: void(*continue_cb_t)(int_thread *thrd)
  .. cpp:function:: static void addContinueCB(continue_cb_t cb)
  .. cpp:function:: void triggerContinueCBs()
  .. cpp:function:: void throwEventsBeforeContinue()
  .. cpp:function:: virtual bool suppressSanityChecks()
  .. cpp:function:: void setTID(Dyninst::THR_ID tid_)
  .. cpp:function:: virtual bool haveUserThreadInfo()
  .. cpp:function:: virtual bool getTID(Dyninst::THR_ID &tid)
  .. cpp:function:: virtual bool getStartFuncAddress(Dyninst::Address &addr)
  .. cpp:function:: virtual bool getStackBase(Dyninst::Address &addr)
  .. cpp:function:: virtual bool getStackSize(unsigned long &size)
  .. cpp:function:: virtual bool getTLSPtr(Dyninst::Address &addr)
  .. cpp:function:: virtual Dyninst::Address getThreadInfoBlockAddr()
  .. cpp:function:: virtual bool isUser() const

      Windows-only; default implementation is "yes, we're a user thread".

  .. cpp:function:: virtual unsigned hwBPAvail(unsigned mode)
  .. cpp:function:: virtual bool rmHWBreakpoint(hw_breakpoint *bp, bool suspend, std::set<response::ptr> &resps, bool &done)
  .. cpp:function:: virtual bool addHWBreakpoint(hw_breakpoint *bp, bool resume, std::set<response::ptr> &resps, bool &done)
  .. cpp:function:: virtual EventBreakpoint::ptr decodeHWBreakpoint(response::ptr &resp, bool have_reg = false, Dyninst::MachRegisterVal regval = 0)
  .. cpp:function:: virtual bool bpNeedsClear(hw_breakpoint *hwbp)
  .. cpp:function:: virtual ~int_thread()
  .. cpp:function:: static const char *stateStr(int_thread::State s)
  .. cpp:function:: State getTargetState() const
  .. cpp:function:: void setTargetState(State s)
  .. cpp:function:: void setSuspended(bool b)
  .. cpp:function:: bool isSuspended() const
  .. cpp:function:: void setLastError(err_t ec, const char *es)
  .. cpp:function:: hw_breakpoint *getHWBreakpoint(Address addr)
  .. cpp:member:: protected Dyninst::THR_ID tid
  .. cpp:member:: protected Dyninst::LWP lwp
  .. cpp:member:: protected int_process *proc_
  .. cpp:member:: protected Thread::ptr up_thread
  .. cpp:member:: protected int continueSig_
  .. cpp:member:: protected attach_status_t attach_status
  .. cpp:member:: protected Counter handler_running_thrd_count
  .. cpp:member:: protected Counter generator_running_thrd_count
  .. cpp:member:: protected Counter sync_rpc_count
  .. cpp:member:: protected Counter sync_rpc_running_thr_count
  .. cpp:member:: protected Counter pending_stop
  .. cpp:member:: protected Counter clearing_bp_count
  .. cpp:member:: protected Counter proc_stop_rpc_count
  .. cpp:member:: protected Counter generator_nonexited_thrd_count
  .. cpp:member:: protected Counter neonatal_threads
  .. cpp:member:: protected Counter pending_stackwalk_count
  .. cpp:member:: protected StateTracker postponed_syscall_state
  .. cpp:member:: protected StateTracker exiting_state
  .. cpp:member:: protected StateTracker startup_state
  .. cpp:member:: protected StateTracker pending_stop_state
  .. cpp:member:: protected StateTracker callback_state
  .. cpp:member:: protected StateTracker breakpoint_state
  .. cpp:member:: protected StateTracker breakpoint_hold_state
  .. cpp:member:: protected StateTracker breakpoint_resume_state
  .. cpp:member:: protected StateTracker irpc_setup_state
  .. cpp:member:: protected StateTracker irpc_wait_state
  .. cpp:member:: protected StateTracker irpc_state
  .. cpp:member:: protected StateTracker async_state
  .. cpp:member:: protected StateTracker internal_state
  .. cpp:member:: protected StateTracker detach_state
  .. cpp:member:: protected StateTracker user_irpc_state
  .. cpp:member:: protected StateTracker control_authority_state
  .. cpp:member:: protected StateTracker user_state
  .. cpp:member:: protected StateTracker handler_state
  .. cpp:member:: protected StateTracker generator_state
  .. cpp:member:: protected StateTracker *all_states[NumStateIDs]
  .. cpp:member:: protected State target_state
  .. cpp:member:: protected State saved_user_state
  .. cpp:member:: protected int_registerPool cached_regpool
  .. cpp:member:: protected Mutex<true> regpool_lock
  .. cpp:member:: protected int_iRPC_ptr running_rpc
  .. cpp:member:: protected int_iRPC_ptr writing_rpc
  .. cpp:member:: protected rpc_list_t posted_rpcs
  .. cpp:member:: protected int_registerPool rpc_regs
  .. cpp:member:: protected bool user_single_step
  .. cpp:member:: protected bool single_step
  .. cpp:member:: protected bool handler_exiting_state
  .. cpp:member:: protected bool generator_exiting_state
  .. cpp:member:: protected bool running_when_attached
  .. cpp:member:: protected bool suspended
  .. cpp:member:: protected bool user_syscall
  .. cpp:member:: protected bool next_syscall_is_exit
  .. cpp:member:: protected Address stopped_on_breakpoint_addr
  .. cpp:member:: protected Address postponed_stopped_on_breakpoint_addr
  .. cpp:member:: protected bp_instance *clearing_breakpoint
  .. cpp:member:: protected emulated_singlestep *em_singlestep
  .. cpp:member:: protected void *user_data
  .. cpp:member:: protected std::set<hw_breakpoint *> hw_breakpoints
  .. cpp:member:: protected static std::set<continue_cb_t> continue_cbs
  .. cpp:member:: protected CallStackUnwinding *unwinder
  .. cpp:member:: Address addr_fakeSyscallExitBp
  .. cpp:member:: bool isSet_fakeSyscallExitBp
  .. cpp:member:: Breakpoint::ptr BPptr_fakeSyscallExitBp


.. cpp:enum:: int_thread::attach_status_t

  Threads found by getThreadLWPs come in as_needs_attach, others come int as_created_attached.
  Up to platforms to interpret this however they want.

  .. cpp:enumerator:: as_unknown
  .. cpp:enumerator:: as_created_attached
  .. cpp:enumerator:: as_needs_attach

.. cpp:enum:: int_thread::State

  .. cpp:enumerator:: none=0
  .. cpp:enumerator:: neonatal=1
  .. cpp:enumerator:: neonatal_intermediate=2
  .. cpp:enumerator:: running=3
  .. cpp:enumerator:: stopped=4
  .. cpp:enumerator:: dontcare=5
  .. cpp:enumerator:: ditto=6
  .. cpp:enumerator:: exited=7
  .. cpp:enumerator:: detached=8
  .. cpp:enumerator:: errorstate=9


.. cpp:class:: int_thread::StateTracker

  Making a decision on when to stop/continue a thread is complicated,
  especially when multiple events are happening on multiple threads.
  We have to consider cases like an iRPC running on one thread, while
  another thread handles a breakpoint and another thread is being
  stopped by the user.

  Each of these events might try to change the stop/running states of
  other threads, which can lead to conflicts over who's running.
  We resolve these conflicts by:

  - Giving each PC subsystem (e.g., iRPCs, breakpoints, user stops, ...) its
    variable indicating whether a thread should run.  These are the StateTrackers.

  - When we acutually decide whether a thread should stop/run, we use a priority-based
    projection to reduce the multiple StateTrackers from each subsystem into
    a single target state (this happens in int_process::syncRunState).

  As an example, if you look below you'll see that the IRPC subsystem's
  StateTrackers are a higher priority than the Breakpoint handling
  subsystem.  That means that if an IRPC and a breakpoint are happening
  at the same time, then we'll handle the stop/continues for the IRPC
  first.  When those are done (e.g., when the iRPC subsystem sets
  its StateTrackers to the dontcare value), then we'll move on to
  handle the breakpoints.

  The UserState is one of the lowest priority StateTrackers--meaning
  everything else takes precedence.  This state represents the wishes
  of the user.  You can override the user's wishes (i.e, impose a
  temporary stop while handling something) by setting a higher priority
  state, doing your work, then clearing that state.

  In general, most people will use StateTrackers from Handlers when
  dealing with an event that might have multiple stages.  In these cases,
  have the first stage of the event (or the code that starts the events)
  set the StateTracker to the desired state.  Have the last stage of the
  event clear the StateTracker.  Events that can be handled with a single
  stage (e.g, forwarding a signal) don't usually need a StateTracker.

  The HandlerState and GeneratorState are two special case StateTrackers.
  Instead of representing a goal state, they represent the actual
  stop/running state of a thread.  The GenratorState is the state
  as viewed by PC's generator thread, and similar HandlerState is for
  PC's handler thread.  These are seperate variables so that we don't
  get races if they both read/update that variable at once.  Most of the
  time, you'll want the HandlerState.

  .. cpp:member:: protected State state
  .. cpp:member:: protected int id
  .. cpp:member:: protected int sync_level
  .. cpp:member:: protected int_thread *up_thr
  .. cpp:function:: StateTracker(int_thread *t, int id, int_thread::State initial)
  .. cpp:function:: void desyncState(State ns = int_thread::none)
  .. cpp:function:: void desyncStateProc(State ns = int_thread::none)
  .. cpp:function:: bool setState(State ns = int_thread::none)
  .. cpp:function:: bool setStateProc(State ns = int_thread::none)
  .. cpp:function:: void restoreState()
  .. cpp:function:: void restoreStateProc()
  .. cpp:function:: State getState() const
  .. cpp:function:: bool isDesynced() const
  .. cpp:function:: std::string getName() const
  .. cpp:function:: int getID() const
  .. cpp:function:: int_thread *debugthr() const


.. cpp:class:: int_threadPool

  Represents a collection of threads. Each int_process has one int_threadPool, which has multiple threads.

  .. cpp:var:: private mutable int_thread *initial_thread

      May be updated by side effect on Windows.

  .. cpp:function:: int_threadPool(int_process *p)
  .. cpp:function:: ~int_threadPool()
  .. cpp:function:: void setInitialThread(int_thread *thrd)
  .. cpp:function:: void addThread(int_thread *thrd)
  .. cpp:function:: void rmThread(int_thread *thrd)
  .. cpp:function:: void noteUpdatedLWP(int_thread *thrd)
  .. cpp:function:: void clear()
  .. cpp:function:: bool hadMultipleThreads() const
  .. cpp:type:: std::vector<int_thread *>::iterator iterator
  .. cpp:function:: iterator begin()
  .. cpp:function:: iterator end()
  .. cpp:function:: bool empty()
  .. cpp:function:: unsigned size() const
  .. cpp:function:: int_process *proc() const
  .. cpp:function:: ThreadPool *pool() const
  .. cpp:function:: int_thread *findThreadByLWP(Dyninst::LWP lwp)
  .. cpp:function:: int_thread *initialThread() const
  .. cpp:function:: bool allHandlerStopped()
  .. cpp:function:: bool allStopped(int state_id)
  .. cpp:function:: void saveUserState(Event::ptr ev)
  .. cpp:function:: void restoreUserState()


.. cpp:class:: int_library

  Represents a Dynamic Shared Object (aka DSO, aka .dll/.so) loaded by the process.
  Each DSO has a library name and load address. int_library doesn't hang directly from a
  process, but from its mem_state object.

  .. cpp:function:: int_library(std::string n, bool shared_lib, Dyninst::Address load_addr, Dyninst::Address dynamic_load_addr, Dyninst::Address data_load_addr = 0, bool has_data_load_addr = false)
  .. cpp:function:: int_library(int_library *l)
  .. cpp:function:: ~int_library()
  .. cpp:function:: std::string getName()
  .. cpp:function:: std::string getAbsName()
  .. cpp:function:: Dyninst::Address getAddr()
  .. cpp:function:: Dyninst::Address getDataAddr()
  .. cpp:function:: Dyninst::Address getDynamicAddr()
  .. cpp:function:: bool hasDataAddr()
  .. cpp:function:: void setMark(bool b)
  .. cpp:function:: bool isMarked() const
  .. cpp:function:: void setUserData(void *d)
  .. cpp:function:: void *getUserData()
  .. cpp:function:: bool isSharedLib() const
  .. cpp:function:: Library::ptr getUpPtr() const
  .. cpp:function:: void markAsCleanable()
  .. cpp:function:: void setLoadAddress(Address addr)
  .. cpp:function:: void setDynamicAddress(Address addr)
  .. cpp:function:: Address mapAddress()
  .. cpp:function:: void setMapAddress(Address a)
  .. cpp:function:: void markAOut()
  .. cpp:function:: bool inProcess(int_process *proc)


.. cpp:class:: int_breakpoint

  There are five classes related to breakpoints:

  Breakpoint - The user interface to breakpoints.  The user will always
  use this when handling breakpoints.

  int_breakpoint - The internal handle for a breakpoint.  A Breakpoint
  will always have one int_breakpoint.  However, internal breakpoints
  (ie, the breakpoint used in System V library loading) don't necessary
  have the UI interface object of Breakpoint.

  int_breakpoint's aren't process specific (so they can be copied easily)
  upon fork.  A single int_breakpoint can be inserted into multiple
  processes, and multiple times into one int_process.

  An int_breakpoint can have properties, like a control transfer
  int_breakpoint will transfer control when it executes, a
  onetime breakpoint will clean itself after being hit, and
  a thread-specific breakpoint will only trigger if hit by
  certain threads.

  If internal code wants to keep a handle to a breakpoint, then
  it should use int_breakpoint.

  bp_instance - Each int_breakpoint/process/address triple is
  represented by a bp_instance.  This reprents an actual
  low-level breakpoint at some location.  Unless you're
  writing low-level BP code you can ignore this class.

  bp_instance is an abstract class, implemented by sw_breakpoint
  and hw_breakpoint.

  sw_breakpoint is a type of bp_instance, as implemented by a
  trap instruction.  A certain sequence of bytes is written
  into the process at a code location, which throws a SIGTRAP
  (or similar) when executed.

  hw_breakpoint is a type of bp_instance, as implemented by
  hardware debug registers.  These are usually used to implement
  things like watchpoints in debuggers.  They are usually
  thread-specific and can set to trigger when code executes
  or data is read or written.

  .. cpp:function:: int_breakpoint(Breakpoint::ptr up)
  .. cpp:function:: int_breakpoint(Dyninst::Address to, Breakpoint::ptr up, bool off)
  .. cpp:function:: int_breakpoint(unsigned int hw_prems_, unsigned int hw_size_, Breakpoint::ptr up)
  .. cpp:function:: ~int_breakpoint()
  .. cpp:function:: bool isCtrlTransfer() const
  .. cpp:function:: Dyninst::Address toAddr() const
  .. cpp:function:: Dyninst::Address getAddress(int_process *p) const
  .. cpp:function:: void *getData() const
  .. cpp:function:: void setData(void *v)
  .. cpp:function:: void setOneTimeBreakpoint(bool b)
  .. cpp:function:: void markOneTimeHit()
  .. cpp:function:: bool isOneTimeBreakpoint() const
  .. cpp:function:: bool isOneTimeBreakpointHit() const
  .. cpp:function:: void setThreadSpecific(Thread::const_ptr p)
  .. cpp:function:: bool isThreadSpecific() const
  .. cpp:function:: bool isThreadSpecificTo(Thread::const_ptr p) const
  .. cpp:function:: void setProcessStopper(bool b)
  .. cpp:function:: bool isProcessStopper() const
  .. cpp:function:: void setSuppressCallbacks(bool)
  .. cpp:function:: bool suppressCallbacks(void) const
  .. cpp:function:: bool isHW() const
  .. cpp:function:: unsigned getHWSize() const
  .. cpp:function:: unsigned getHWPerms() const
  .. cpp:function:: bool isOffsetTransfer() const
  .. cpp:function:: Breakpoint::weak_ptr upBreakpoint() const


.. cpp:class:: bp_instance

  .. cpp:member:: protected std::set<int_breakpoint *> bps
  .. cpp:member:: protected std::set<Breakpoint::ptr> hl_bps
  .. cpp:member:: protected Dyninst::Address addr
  .. cpp:member:: protected bool installed
  .. cpp:member:: protected int suspend_count
  .. cpp:member:: protected sw_breakpoint *swbp
  .. cpp:member:: protected hw_breakpoint *hwbp
  .. cpp:function:: protected bool suspend_common()
  .. cpp:function:: protected bool resume_common()
  .. cpp:function:: virtual bool checkBreakpoint(int_breakpoint *bp, int_process *proc)
  .. cpp:function:: virtual bool rmBreakpoint(int_process *proc, int_breakpoint *bp, bool &empty, std::set<response::ptr> &resps)
  .. cpp:function:: virtual async_ret_t uninstall(int_process *proc, std::set<response::ptr> &resps) = 0
  .. cpp:function:: Address getAddr() const
  .. cpp:function:: bp_instance(Address addr)
  .. cpp:function:: bp_instance(const bp_instance *ip)
  .. cpp:type:: std::set<int_breakpoint *>::iterator iterator
  .. cpp:function:: iterator begin()
  .. cpp:function:: iterator end()
  .. cpp:function:: bool containsIntBreakpoint(int_breakpoint *bp)
  .. cpp:function:: int_breakpoint *getCtrlTransferBP(int_thread *thread)
  .. cpp:function:: bool isInstalled() const
  .. cpp:function:: virtual bool needsClear() = 0
  .. cpp:function:: virtual async_ret_t suspend(int_process *proc, std::set<response::ptr> &resps) = 0
  .. cpp:function:: virtual async_ret_t resume(int_process *proc, std::set<response::ptr> &resps) = 0
  .. cpp:function:: sw_breakpoint *swBP()
  .. cpp:function:: hw_breakpoint *hwBP()
  .. cpp:function:: virtual ~bp_instance()


.. code:: cpp

  #define BP_BUFFER_SIZE 8

    At least as large as the trap instruction of any architecture.

  #define BP_LONG_SIZE 4

    Long breakpoints can be used to artifically increase the size of the BP write,
    which fools the BG breakpoint interception code that looks for 4 byte writes.

.. cpp:class:: sw_breakpoint : public bp_instance

  .. cpp:function:: sw_breakpoint(mem_state::ptr memory_, const sw_breakpoint *ip)
  .. cpp:function:: ~sw_breakpoint()
  .. cpp:function:: static sw_breakpoint *create(int_process *proc, int_breakpoint *bp, Dyninst::Address addr_)
  .. cpp:function:: bool prepBreakpoint(int_process *proc, mem_response::ptr mem_resp)
  .. cpp:function:: bool insertBreakpoint(int_process *proc, result_response::ptr res_resp)
  .. cpp:function:: bool addToIntBreakpoint(int_breakpoint *bp, int_process *proc)
  .. cpp:function:: virtual async_ret_t uninstall(int_process *proc, std::set<response::ptr> &resps)
  .. cpp:function:: virtual async_ret_t suspend(int_process *proc, std::set<response::ptr> &resps)
  .. cpp:function:: virtual async_ret_t resume(int_process *proc, std::set<response::ptr> &resps)
  .. cpp:function:: unsigned getNumIntBreakpoints() const
  .. cpp:function:: virtual bool needsClear()


.. cpp:class:: hw_breakpoint : public bp_instance

  .. cpp:function:: virtual async_ret_t uninstall(int_process *proc, std::set<response::ptr> &resps)
  .. cpp:function:: static hw_breakpoint *create(int_process *proc, int_breakpoint *bp, Dyninst::Address addr_)
  .. cpp:function:: ~hw_breakpoint()
  .. cpp:function:: bool install(bool &done, std::set<response::ptr> &resps)
  .. cpp:function:: unsigned int getPerms() const
  .. cpp:function:: unsigned int getSize() const
  .. cpp:function:: bool procWide() const
  .. cpp:function:: int_thread *getThread() const
  .. cpp:function:: virtual bool needsClear()
  .. cpp:function:: virtual async_ret_t suspend(int_process *proc, std::set<response::ptr> &resps)
  .. cpp:function:: virtual async_ret_t resume(int_process *proc, std::set<response::ptr> &resps)


.. cpp:class:: emulated_singlestep

  On PPC64 certain synchronization instructions can mis-behave if we
  try to single-step across them.  This class recognizes these situations
  and replaces a single-step operation with a breakpoint insertion/run over
  the offending code.

  Breakpoints that are added and removed in a group to emulate a single step with breakpoints.

  .. cpp:function:: emulated_singlestep(int_thread *thr)
  .. cpp:function:: ~emulated_singlestep()
  .. cpp:function:: bool containsBreakpoint(Address addr) const
  .. cpp:function:: async_ret_t add(Address addr)
  .. cpp:function:: async_ret_t clear()
  .. cpp:function:: void restoreSSMode()
  .. cpp:member:: std::set<response::ptr> clear_resps


.. cpp:struct:: clearError

  .. cpp:function:: void operator()(Process::ptr p)
  .. cpp:function:: template <class T> void operator()(const std::pair<T, Process::const_ptr> &v)
  .. cpp:function:: template <class T> void operator()(const std::pair<Process::const_ptr, T> &v)
  .. cpp:function:: template <class T> void operator()(const std::pair<T, Process::ptr> &v)
  .. cpp:function:: template <class T> void operator()(const std::pair<Process::ptr, T> &v)


.. cpp:struct:: setError

  .. cpp:function:: setError(err_t e, const char *s)
  .. cpp:function:: void operator()(Process::ptr p)
  .. cpp:function:: void operator()(const std::pair<Address, Process::ptr> &v)


.. cpp:class:: int_notify

  The notify class is the internal interface to th UI Notify class.
  It is used to signal the user (via platform-specific interfaces)
  that an event is ready to be handled.

  .. cpp:type:: details_t::wait_object_t wait_object_t
  .. cpp:function:: int_notify *notify()
  .. cpp:function:: EventNotify *Dyninst::ProcControlAPI::evNotify()
  .. cpp:function:: int_notify()
  .. cpp:function:: void noteEvent()
  .. cpp:function:: void clearEvent()
  .. cpp:function:: void registerCB(EventNotify::notify_cb_t cb)
  .. cpp:function:: void removeCB(EventNotify::notify_cb_t cb)
  .. cpp:function:: bool hasEvents()
  .. cpp:function:: details_t::wait_object_t getWaitable()


.. cpp:class:: int_notify::windows_details

  Only available if ``os_windows`` is defined.

  .. cpp:type:: HANDLE wait_object_t
  .. cpp:function:: windows_details()
  .. cpp:function:: void noteEvent()
  .. cpp:function:: void clearEvent()
  .. cpp:function:: bool createInternals()
  .. cpp:function:: bool internalsValid()
  .. cpp:function:: wait_object_t getWaitObject()

  .. cpp:type:: windows_details details_t

.. cpp:class:: int_notify::unix_details

  Only available if ``os_windows`` is not defined.

  .. cpp:type:: unix_details details_t
  .. cpp:function:: unix_details()
  .. cpp:type:: int wait_object_t
  .. cpp:function:: void noteEvent()
  .. cpp:function:: void clearEvent()
  .. cpp:function:: bool createInternals()
  .. cpp:function:: bool internalsValid()
  .. cpp:function:: wait_object_t getWaitObject()

  .. cpp:function:: int_notify *notify()
  .. cpp:function:: extern void setGeneratorThread(long t)
  .. cpp:function:: void setHandlerThread(long t)
  .. cpp:function:: bool isGeneratorThread()
  .. cpp:function:: bool isHandlerThread()
  .. cpp:function:: bool isUserThread()
  .. cpp:function:: HandlerPool *createDefaultHandlerPool(int_process *p)
  .. cpp:function:: HandlerPool *plat_createDefaultHandlerPool(HandlerPool *hpool)

.. cpp:class:: MTManager

  .. cpp:member:: static const Process::thread_mode_t default_thread_mode = Process::HandlerThreading
  .. cpp:function:: MTManager()
  .. cpp:function:: ~MTManager()
  .. cpp:function:: void run()
  .. cpp:function:: void stop()
  .. cpp:function:: void startWork()
  .. cpp:function:: void endWork()
  .. cpp:function:: bool handlerThreading()
  .. cpp:function:: Process::thread_mode_t getThreadMode()
  .. cpp:function:: bool setThreadMode(Process::thread_mode_t tm, bool init = false)
  .. cpp:function:: static void eventqueue_cb_wrapper()

.. cpp:function:: inline MTManager* mt()

.. cpp:class:: MTLock

  .. cpp:function:: MTLock(initialize, callbacks c = nocb)
  .. cpp:function:: MTLock(generator)
  .. cpp:function:: MTLock(callbacks)
  .. cpp:function:: MTLock()
  .. cpp:function:: ~MTLock()


.. cpp:enum:: MTLock::initialize

  .. cpp:enumerator:: allow_init

.. cpp:enum:: MTLock::generator

  .. cpp:enumerator:: allow_generator

.. cpp:enum:: MTLock::callbacks

  .. cpp:enumerator:: nocb
  .. cpp:enumerator:: deliver_callbacks


.. cpp:class:: int_cleanup

  .. cpp:function:: ~int_cleanup()


.. code:: cpp

  #define PROC_EXIT_TEST(STR, RET)                      \
     if (!llproc_) {                                    \
       perr_printf(STR " on exited process\n");         \
       setLastError(err_exited, "Process is exited");   \
       return RET;                                      \
     }

  #define PROC_DETACH_TEST(STR, RET)                       \
     if (llproc_->getState() == int_process::detached) {   \
       perr_printf(STR " on detached process\n");          \
       setLastError(err_detached, "Process is detached");  \
       return RET;                                         \
     }

  #define PROC_CB_TEST(STR, RET)                                          \
     if (int_process::isInCB()) {                                         \
       perr_printf(STR " while in callback\n");                           \
       setLastError(err_incallback, "Cannot do operation from callback"); \
       return RET;                                                        \
     }

  #define PROC_EXIT_DETACH_TEST(STR, RET)         \
     PROC_EXIT_TEST(STR, RET)                     \
     PROC_DETACH_TEST(STR, RET)

  #define PROC_EXIT_DETACH_CB_TEST(STR, RET)      \
     PROC_EXIT_TEST(STR, RET)                     \
     PROC_DETACH_TEST(STR, RET)                   \
     PROC_CB_TEST(STR, RET)

  #define THREAD_EXIT_TEST(STR, RET)                    \
     if (!llthread_) {                                  \
       perr_printf(STR " on exited thread\n");          \
       setLastError(err_exited, "Thread is exited");    \
       return RET;                                      \
     }                                                  \
     if (!llthread_->llproc()) {                        \
       perr_printf(STR " on exited process\n");         \
       setLastError(err_exited, "Process is exited");   \
       return RET;                                      \
     }

  #define THREAD_DETACH_TEST(STR, RET)                                    \
     if (llthread_->llproc()->getState() == int_process::detached) {      \
       perr_printf(STR " on detached process\n");                         \
       setLastError(err_detached, "Process is detached");                 \
       return RET;                                                        \
     }                                                                    \

  #define THREAD_STOP_TEST(STR, RET)                                     \
     if (llthread_->getUserState().getState() != int_thread::stopped) {  \
        setLastError(err_notstopped, "Thread not stopped");              \
        perr_printf(STR " on running thread %d\n", llthread_->getLWP()); \
        return RET;                                                      \
     }

  #define THREAD_EXIT_DETACH_TEST(STR, RET)         \
     THREAD_EXIT_TEST(STR, RET)                     \
     THREAD_DETACH_TEST(STR, RET)

  #define THREAD_EXIT_DETACH_CB_TEST(STR, RET)      \
     THREAD_EXIT_TEST(STR, RET)                     \
     THREAD_DETACH_TEST(STR, RET)                   \
     PROC_CB_TEST(STR, RET)

  #define THREAD_EXIT_DETACH_STOP_TEST(STR, RET)    \
     THREAD_EXIT_TEST(STR, RET)                     \
     THREAD_DETACH_TEST(STR, RET)                   \
     THREAD_STOP_TEST(STR, RET)

  #define PTR_EXIT_TEST(P, STR, RET)                       \
     if (!P || !P->llproc()) {                             \
        perr_printf(STR " on exited process\n");           \
        P->setLastError(err_exited, "Process is exited");  \
        return RET;                                        \
     }

  #define TRUTH_TEST(P, STR, RET)                                 \
     if (!(P)) {                                                  \
        perr_printf(STR " parameter is invalid\n");               \
        setLastError(err_badparam, STR " paramter is invalid\n"); \
        return RET;                                               \
     }
