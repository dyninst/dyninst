.. _`sec:sysv.h`:

sysv.h
######

.. cpp:class:: PCProcReader : public ProcessReader

  .. cpp:function:: PCProcReader(sysv_process *proc_)
  .. cpp:function:: virtual ~PCProcReader()
  .. cpp:function:: virtual bool start()
  .. cpp:function:: virtual bool ReadMem(Dyninst::Address addr, void *buffer, unsigned size)
  .. cpp:function:: virtual bool GetReg(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
  .. cpp:function:: virtual bool done()
  .. cpp:function:: bool hasPendingAsync()
  .. cpp:function:: bool getNewAsyncs(std::set<response::ptr> &resps)


.. cpp:class:: sysv_process : public int_libraryTracking

  .. cpp:function:: sysv_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: sysv_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~sysv_process()
  .. cpp:function:: virtual bool refresh_libraries(std::set<int_library *> &added_libs, std::set<int_library *> &rmd_libs, bool &waiting_for_async, std::set<response::ptr> &async_responses)
  .. cpp:function:: virtual bool initLibraryMechanism()
  .. cpp:function:: Dyninst::Address getLibBreakpointAddr() const
  .. cpp:function:: bool isLibraryTrap(Dyninst::Address trap_addr)
  .. cpp:function:: static bool addSysVHandlers(HandlerPool *hpool)
  .. cpp:function:: virtual bool setTrackLibraries(bool b, int_breakpoint* &bp, Address &addr, bool &add_bp)
  .. cpp:function:: virtual bool isTrackingLibraries()
  .. cpp:function:: protected virtual bool plat_execed()
  .. cpp:function:: protected virtual bool plat_isStaticBinary()
  .. cpp:function:: protected virtual int_library *plat_getExecutable()
  .. cpp:function:: protected virtual bool plat_getInterpreterBase(Dyninst::Address &addr)
  .. cpp:function:: protected AddressTranslate *constructTranslator(Dyninst::PID pid)
  .. cpp:function:: protected AddressTranslate *translator()
  .. cpp:member:: protected static int_breakpoint *lib_trap
  .. cpp:member:: protected Address breakpoint_addr
  .. cpp:member:: protected bool lib_initialized
  .. cpp:member:: protected PCProcReader *procreader
