.. _`sec:x86_process.h`:

x86_process.h
#############

.. cpp:class:: x86_process : virtual public int_process

  .. cpp:function:: x86_process(Dyninst::PID p, std::string e, std::vector<std::string> a,  std::vector<std::string> envp, std::map<int, int> f)
  .. cpp:function:: x86_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~x86_process()
  .. cpp:function:: virtual unsigned plat_breakpointSize()
  .. cpp:function:: virtual void plat_breakpointBytes(unsigned char *buffer)
  .. cpp:function:: virtual bool plat_breakpointAdvancesPC() const
  .. cpp:function:: virtual Address plat_findFreeMemory(size_t)


.. cpp:class:: x86_thread : virtual public int_thread

  .. cpp:function:: x86_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
  .. cpp:function:: virtual ~x86_thread()
  .. cpp:function:: virtual bool rmHWBreakpoint(hw_breakpoint *bp, bool suspend, std::set<response::ptr> &resps, bool &done)
  .. cpp:function:: virtual bool addHWBreakpoint(hw_breakpoint *bp, bool resume, std::set<response::ptr> &resps, bool &done)
  .. cpp:function:: virtual unsigned hwBPAvail(unsigned mode)
  .. cpp:function:: virtual EventBreakpoint::ptr decodeHWBreakpoint(response::ptr &resp, bool have_reg = false, Dyninst::MachRegisterVal regval = 0)
  .. cpp:function:: virtual bool bpNeedsClear(hw_breakpoint *hwbp)
