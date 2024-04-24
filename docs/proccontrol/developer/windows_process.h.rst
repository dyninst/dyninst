.. _`sec:windows_process.h`:

windows_process.h
=================

.. cpp:class:: windows_process : virtual public x86_process, virtual public hybrid_lwp_control_process

  .. cpp:function:: windows_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: windows_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~windows_process()
  .. cpp:function:: virtual bool plat_create()
  .. cpp:function:: virtual bool plat_create_int()
  .. cpp:function:: virtual bool plat_attach(bool allStopped, bool &)
  .. cpp:function:: virtual bool plat_attach_int()
  .. cpp:function:: virtual bool plat_attachWillTriggerStop()
  .. cpp:function:: virtual bool plat_forked()
  .. cpp:function:: virtual bool plat_execed()
  .. cpp:function:: virtual bool plat_detach(result_response::ptr resp, bool leave_stopped)
  .. cpp:function:: virtual bool plat_terminate(bool &needs_sync)
  .. cpp:function:: virtual bool plat_decodeMemoryRights(Process::mem_perm& perm, unsigned long rights)
  .. cpp:function:: virtual bool plat_encodeMemoryRights(Process::mem_perm perm, unsigned long& rights)
  .. cpp:function:: virtual bool plat_getMemoryAccessRights(Dyninst::Address addr, Process::mem_perm& rights)
  .. cpp:function:: virtual bool plat_setMemoryAccessRights(Dyninst::Address addr, size_t size, Process::mem_perm rights, Process::mem_perm& oldRights)
  .. cpp:function:: virtual bool plat_findAllocatedRegionAround(Dyninst::Address addr, Process::MemoryRegion& memRegion)
  .. cpp:function:: virtual bool plat_readMem(int_thread *thr, void *local, Dyninst::Address remote, size_t size)
  .. cpp:function:: virtual bool plat_writeMem(int_thread *thr, const void *local, Dyninst::Address remote, size_t size, bp_write_t bp_write)
  .. cpp:function:: virtual Address plat_findFreeMemory(size_t size)
  .. cpp:function:: virtual SymbolReaderFactory *plat_defaultSymReader()
  .. cpp:function:: virtual bool needIndividualThreadAttach()
  .. cpp:function:: virtual bool getThreadLWPs(std::vector<Dyninst::LWP> &lwps)
  .. cpp:function:: virtual Dyninst::Architecture getTargetArch()
  .. cpp:function:: virtual bool plat_individualRegAccess()
  .. cpp:function:: virtual bool plat_supportLWPCreate()
  .. cpp:function:: virtual bool plat_supportLWPPreDestroy()
  .. cpp:function:: virtual bool plat_supportLWPPostDestroy()
  .. cpp:function:: virtual bool plat_supportThreadEvents()
  .. cpp:function:: virtual Dyninst::Address plat_mallocExecMemory(Dyninst::Address min, unsigned size)
  .. cpp:function:: virtual bool plat_getOSRunningStates(std::map<Dyninst::LWP, bool> &runningStates)
  .. cpp:function:: virtual bool plat_convertToBreakpointAddress(psaddr_t &)
  .. cpp:function:: virtual unsigned int getTargetPageSize()
  .. cpp:function:: virtual bool plat_createAllocationSnippet(Dyninst::Address addr, bool use_addr, unsigned long size, void*& buffer, unsigned long& buffer_size, unsigned long& start_offset)
  .. cpp:function:: virtual bool plat_createDeallocationSnippet(Dyninst::Address addr, unsigned long size, void*& buffer, unsigned long& buffer_size, unsigned long& start_offset)
  .. cpp:function:: virtual bool plat_collectAllocationResult(int_thread* thr, reg_response::ptr resp)
  .. cpp:function:: virtual bool refresh_libraries(std::set<int_library *> &added_libs, std::set<int_library *> &rmd_libs, bool &waiting_forasync,  std::set<response::ptr> &async_responses)
  .. cpp:function:: virtual int_library *plat_getExecutable()
  .. cpp:function:: virtual bool initLibraryMechanism()
  .. cpp:function:: virtual bool plat_isStaticBinary()
  .. cpp:function:: HANDLE plat_getHandle()
  .. cpp:function:: void plat_setHandles(HANDLE hp, HANDLE hf, Address fb)
  .. cpp:function:: virtual bool hasPendingDetach() const
  .. cpp:function:: virtual void clearPendingDebugBreak()
  .. cpp:function:: virtual void setPendingDebugBreak()
  .. cpp:function:: virtual bool pendingDebugBreak() const
  .. cpp:function:: virtual Dyninst::Address direct_infMalloc(unsigned long size, bool use_addr = false, Dyninst::Address addr = 0x0)
  .. cpp:function:: virtual bool direct_infFree(Dyninst::Address addr)
  .. cpp:function:: virtual bool plat_suspendThread(int_thread *thr)
  .. cpp:function:: virtual bool plat_resumeThread(int_thread *thr)
  .. cpp:function:: virtual bool plat_needsThreadForMemOps() const
  .. cpp:function:: virtual bool addrInSystemLib(Address addr)

      Is this in ntdll or another lib we consider a system lib?

  .. cpp:function:: virtual int_thread *RPCThread()
  .. cpp:function:: virtual int_thread *createRPCThread(int_thread* best_candidate)
  .. cpp:function:: void destroyRPCThread()
  .. cpp:function:: virtual void* plat_getDummyThreadHandle() const
  .. cpp:function:: virtual void instantiateRPCThread()
  .. cpp:function:: virtual bool plat_supportDirectAllocation() const
  .. cpp:function:: virtual Dyninst::OSType getOS() const
  .. cpp:function:: virtual ExecFileInfo* plat_getExecutableInfo() const
  .. cpp:function:: void setStopThread(DWORD stopthr)
  .. cpp:function:: void clearStopThread()
  .. cpp:function:: DWORD getStopThread()
