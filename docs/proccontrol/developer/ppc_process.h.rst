.. _`sec:ppc_process.h`:

ppc_process.h
=============

.. cpp:class:: ppc_process : virtual public int_process

  .. cpp:function:: ppc_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int, int> f)
  .. cpp:function:: ppc_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~ppc_process()
  .. cpp:function:: virtual unsigned plat_breakpointSize()
  .. cpp:function:: virtual void plat_breakpointBytes(unsigned char *buffer)
  .. cpp:function:: virtual bool plat_breakpointAdvancesPC() const
  .. cpp:function:: virtual async_ret_t plat_needsEmulatedSingleStep(int_thread *thr, std::vector<Address> &addrResult)
  .. cpp:function:: virtual bool plat_convertToBreakpointAddress(Address &addr, int_thread *thr)
  .. cpp:function:: virtual bool plat_needsPCSaveBeforeSingleStep()
  .. cpp:function:: virtual void plat_getEmulatedSingleStepAsyncs(int_thread *thr, std::set<response::ptr> resps)
  .. cpp:function:: void cleanupSSOnContinue(int_thread *thr)


.. cpp:class:: ppc_thread : virtual public int_thread

  .. cpp:member:: protected bool have_cached_pc
  .. cpp:member:: protected Address cached_pc
  .. cpp:function:: ppc_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
  .. cpp:function:: virtual ~ppc_thread()
  .. cpp:function:: virtual bool rmHWBreakpoint(hw_breakpoint *bp, bool suspend, std::set<response::ptr> &resps, bool &done)
  .. cpp:function:: virtual bool addHWBreakpoint(hw_breakpoint *bp, bool resume, std::set<response::ptr> &resps, bool &done)
  .. cpp:function:: virtual unsigned hwBPAvail(unsigned mode)
  .. cpp:function:: virtual EventBreakpoint::ptr decodeHWBreakpoint(response::ptr &resp, bool have_reg = false, Dyninst::MachRegisterVal regval = 0)
  .. cpp:function:: virtual bool bpNeedsClear(hw_breakpoint *hwbp)
  .. cpp:function:: void setCachedPC(Address pc)
  .. cpp:function:: void clearCachedPC()
  .. cpp:function:: bool haveCachedPC(Address &pc)