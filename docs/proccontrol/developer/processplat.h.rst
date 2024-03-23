.. _`sec:processplat.h`:

processplat.h
#############

.. cpp:class:: int_libraryTracking : virtual public int_process

  .. cpp:member:: static bool default_track_libs
  .. cpp:member:: LibraryTracking *up_ptr
  .. cpp:function:: int_libraryTracking(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: int_libraryTracking(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~int_libraryTracking()
  .. cpp:function:: virtual bool setTrackLibraries(bool b, int_breakpoint* &bp, Address &addr, bool &add_bp) = 0
  .. cpp:function:: virtual bool isTrackingLibraries() = 0


.. cpp:class:: int_LWPTracking : virtual public int_process

  .. cpp:member:: bool lwp_tracking
  .. cpp:member:: LWPTracking *up_ptr
  .. cpp:function:: int_LWPTracking(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: int_LWPTracking(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~int_LWPTracking()
  .. cpp:function:: virtual bool lwp_setTracking(bool b)
  .. cpp:function:: virtual bool plat_lwpChangeTracking(bool b)
  .. cpp:function:: virtual bool lwp_getTracking()
  .. cpp:function:: virtual bool lwp_refreshPost(result_response::ptr &resp)
  .. cpp:function:: virtual bool lwp_refreshCheck(bool &change)
  .. cpp:function:: virtual bool lwp_refresh()
  .. cpp:function:: virtual bool plat_lwpRefreshNoteNewThread(int_thread *thr)
  .. cpp:function:: virtual bool plat_lwpRefresh(result_response::ptr resp)


.. cpp:class:: int_threadTracking : virtual public int_process

  .. cpp:member:: ThreadTracking *up_ptr
  .. cpp:function:: int_threadTracking(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: int_threadTracking(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~int_threadTracking()
  .. cpp:function:: virtual bool setTrackThreads(bool b, std::set<std::pair<int_breakpoint *, Address> > &bps, bool &add_bp) = 0
  .. cpp:function:: virtual bool isTrackingThreads() = 0
  .. cpp:function:: virtual bool refreshThreads() = 0


.. cpp:class:: int_followFork : virtual public int_process

  .. cpp:member:: protected FollowFork::follow_t fork_tracking
  .. cpp:member:: FollowFork *up_ptr
  .. cpp:function:: int_followFork(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: int_followFork(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~int_followFork()
  .. cpp:function:: virtual bool fork_setTracking(FollowFork::follow_t b) = 0
  .. cpp:function:: virtual FollowFork::follow_t fork_isTracking() = 0


.. cpp:class:: int_callStackUnwinding : virtual public int_process

  .. cpp:function:: int_callStackUnwinding(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: int_callStackUnwinding(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~int_callStackUnwinding()
  .. cpp:function:: virtual bool plat_getStackInfo(int_thread *thr, stack_response::ptr stk_resp) = 0
  .. cpp:function:: virtual bool plat_handleStackInfo(stack_response::ptr stk_resp, CallStackCallback *cbs) = 0


.. cpp:class:: int_memUsage : virtual public resp_process

  .. cpp:member:: MemoryUsage *up_ptr
  .. cpp:function:: int_memUsage(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: int_memUsage(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~int_memUsage()
  .. cpp:function:: virtual bool plat_getStackUsage(MemUsageResp_t *resp) = 0
  .. cpp:function:: virtual bool plat_getHeapUsage(MemUsageResp_t *resp) = 0
  .. cpp:function:: virtual bool plat_getSharedUsage(MemUsageResp_t *resp) = 0
  .. cpp:function:: virtual bool plat_residentNeedsMemVals() = 0
  .. cpp:function:: virtual bool plat_getResidentUsage(unsigned long stacku, unsigned long heapu, unsigned long sharedu, MemUsageResp_t *resp) = 0


.. cpp:class:: int_multiToolControl : virtual public int_process

  .. cpp:member:: MultiToolControl *up_ptr
  .. cpp:function:: int_multiToolControl(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: int_multiToolControl(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~int_multiToolControl()
  .. cpp:function:: virtual std::string mtool_getName() = 0
  .. cpp:function:: virtual MultiToolControl::priority_t mtool_getPriority() = 0
  .. cpp:function:: virtual MultiToolControl *mtool_getMultiToolControl() = 0


.. cpp:class:: int_signalMask : virtual public int_process

  .. cpp:member:: protected dyn_sigset_t sigset
  .. cpp:member:: SignalMask *up_ptr
  .. cpp:function:: int_signalMask(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: int_signalMask(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~int_signalMask()
  .. cpp:function:: virtual bool allowSignal(int signal_no) = 0
  .. cpp:function:: dyn_sigset_t getSigMask()
  .. cpp:function:: void setSigMask(dyn_sigset_t msk)


.. cpp:class:: int_remoteIO : virtual public resp_process

  .. cpp:member:: RemoteIO *up_ptr
  .. cpp:function:: int_remoteIO(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: int_remoteIO(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~int_remoteIO()
  .. cpp:function:: bool getFileNames(FileSet *fset)
  .. cpp:function:: virtual bool plat_getFileNames(FileSetResp_t *resp) = 0
  .. cpp:function:: bool getFileStatData(FileSet &files)
  .. cpp:function:: virtual bool plat_getFileStatData(std::string filename, Dyninst::ProcControlAPI::stat64_ptr *stat_results, std::set<StatResp_t *> &resps) = 0
  .. cpp:function:: bool getFileDataAsync(const FileSet &files)
  .. cpp:function:: virtual bool plat_getFileDataAsync(int_eventAsyncFileRead *fileread) = 0
  .. cpp:function:: virtual int getMaxFileReadSize() = 0


.. cpp:class:: int_fileInfo

  .. cpp:function:: int_fileInfo()
  .. cpp:function:: ~int_fileInfo()
  .. cpp:member:: std::string filename
  .. cpp:member:: stat64_ptr stat_results
  .. cpp:member:: size_t cur_pos