.. _`sec:windows_thread.h`:

windows_thread.h
================

.. cpp:class:: windows_thread : public int_thread

  .. cpp:function:: windows_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
  .. cpp:function:: windows_thread()
  .. cpp:function:: virtual ~windows_thread()
  .. cpp:function:: virtual bool plat_cont()
  .. cpp:function:: virtual bool plat_stop()
  .. cpp:function:: virtual bool plat_getAllRegisters(int_registerPool &reg)
  .. cpp:function:: virtual bool plat_getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
  .. cpp:function:: virtual bool plat_setAllRegisters(int_registerPool &reg)
  .. cpp:function:: virtual bool plat_setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val)
  .. cpp:function:: virtual bool attach()
  .. cpp:function:: virtual bool plat_getThreadArea(int val, Dyninst::Address &addr)
  .. cpp:function:: virtual bool plat_convertToSystemRegs(const int_registerPool &pool, unsigned char *regs, bool gprs_only = false)
  .. cpp:function:: virtual bool plat_needsEmulatedSingleStep(std::vector<Dyninst::Address> &result)
  .. cpp:function:: virtual bool plat_needsPCSaveBeforeSingleStep()
  .. cpp:function:: virtual void plat_terminate()
  .. cpp:function:: virtual bool isRPCEphemeral() const
  .. cpp:function:: void setOptions()
  .. cpp:function:: bool getSegmentBase(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val)
  .. cpp:function:: bool plat_suspend()
  .. cpp:function:: bool plat_resume()
  .. cpp:function:: void plat_setSuspendCount(int count)
  .. cpp:function:: bool haveUserThreadInfo()
  .. cpp:function:: bool getTID(Dyninst::THR_ID& tid)
  .. cpp:function:: void setLWP(Dyninst::LWP L)
  .. cpp:function:: bool getStartFuncAddress(Dyninst::Address& start_addr)
  .. cpp:function:: bool getStackBase(Dyninst::Address& stack_base)
  .. cpp:function:: bool getStackSize(unsigned long& stack_size)
  .. cpp:function:: bool getTLSPtr(Dyninst::Address& tls_ptr)
  .. cpp:function:: void setHandle(HANDLE h)
  .. cpp:function:: std::string dumpThreadContext()
  .. cpp:function:: void setStartFuncAddress(Dyninst::Address addr)
  .. cpp:function:: void setTLSAddress(Dyninst::Address addr)
  .. cpp:function:: Address getThreadInfoBlockAddr()
  .. cpp:function:: virtual bool notAvailableForRPC()
  .. cpp:function:: bool isUser() const
  .. cpp:function:: void setUser(bool)
  .. cpp:function:: bool isRPCThread() const
  .. cpp:function:: void markRPCThread()
  .. cpp:function:: bool isRPCpreCreate() const
  .. cpp:function:: void markRPCRunning()
  .. cpp:function:: void setDummyRPCStart(Address)
  .. cpp:function:: Address getDummyRPCStart() const
  .. cpp:function:: void updateThreadHandle(Dyninst::THR_ID, Dyninst::LWP)
  .. cpp:function:: void* plat_getHandle() const
