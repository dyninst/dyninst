.. _`sec:int_thread_db.h`:

int_thread_db.h
###############

.. cpp:class:: thread_db_thread : virtual public int_thread

  .. cpp:function:: thread_db_thread(int_process *p, Dyninst::THR_ID t, Dyninst::LWP l)
  .. cpp:function:: virtual ~thread_db_thread()
  .. cpp:function:: virtual bool thrdb_getThreadArea(int val, Dyninst::Address &addr)
  .. cpp:function:: virtual bool haveUserThreadInfo()
  .. cpp:function:: virtual bool getTID(Dyninst::THR_ID &tid)
  .. cpp:function:: virtual bool getStartFuncAddress(Dyninst::Address &addr)
  .. cpp:function:: virtual bool getStackBase(Dyninst::Address &addr)
  .. cpp:function:: virtual bool getStackSize(unsigned long &size)
  .. cpp:function:: virtual bool getTLSPtr(Dyninst::Address &addr)
  .. cpp:function:: virtual bool plat_convertToSystemRegs(const int_registerPool &, unsigned char *, bool)


.. cpp:class:: thread_db_process : virtual public int_process

  .. cpp:function:: thread_db_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int, int> f)
  .. cpp:function:: thread_db_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~thread_db_process()
  .. cpp:function:: bool decodeTdbLWPExit(EventLWPDestroy::ptr lwp_ev)
  .. cpp:function:: async_ret_t decodeTdbBreakpoint(EventBreakpoint::ptr bp)
  .. cpp:function:: bool decodeThreadBP(EventBreakpoint::ptr bp)
  .. cpp:function:: static void addThreadDBHandlers(HandlerPool *hpool)
  .. cpp:function:: virtual async_ret_t post_attach(bool wasDetached, std::set<response::ptr> &)
  .. cpp:function:: virtual async_ret_t post_create(std::set<response::ptr> &)
  .. cpp:function:: virtual bool plat_getLWPInfo(lwpid_t, void *)
  .. cpp:function:: const char *getThreadLibName(const char *)
  .. cpp:function:: void freeThreadDBAgent()
  .. cpp:function:: async_ret_t getEventForThread(int_eventThreadDB *)
  .. cpp:function:: bool isSupportedThreadLib(std::string)
  .. cpp:function:: bool plat_supportThreadEvents()
  .. cpp:member:: bool threaddb_setTrackThreads(bool, std::set<std::pair<int_breakpoint *, Address> > &, bool &)
  .. cpp:function:: bool threaddb_isTrackingThreads()
  .. cpp:function:: ThreadTracking *threaddb_getThreadTracking()
  .. cpp:function:: bool setTrackThreads(bool b, std::set<std::pair<int_breakpoint *, Address> > &bps, bool &add_bp)
  .. cpp:function:: bool isTrackingThreads()
