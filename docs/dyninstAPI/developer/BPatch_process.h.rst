.. _`sec-dev:BPatch_process.h`:

BPatch_process.h
################

.. cpp:namespace:: dev

.. cpp:class:: OneTimeCodeInfo

  This is used by the oneTimeCode (inferiorRPC) mechanism to keep per-RPC information.

  .. cpp:function:: OneTimeCodeInfo(bool _synchronous, void *_userData, BPatchOneTimeCodeCallback _cb, unsigned _thrID)
  .. cpp:function:: bool isSynchronous()
  .. cpp:function:: bool isCompleted() const
  .. cpp:function:: void setCompleted(bool _completed)
  .. cpp:function:: void *getUserData()
  .. cpp:function:: void setReturnValue(void *_returnValue)
  .. cpp:function:: void *getReturnValue()
  .. cpp:function:: unsigned getThreadID()
  .. cpp:function:: BPatchOneTimeCodeCallback getCallback()

.. cpp:class:: BPatch_process : public BPatch_addressSpace

  .. cpp:member:: private bool terminated

    BPatch-level once the callbacks are sent by the llproc, we're terminated Used because callbacks go (and can clean up user code) before the low-level process sets flags.

  .. cpp:function:: private void *oneTimeCodeInternal(const BPatch_snippet &expr, BPatch_thread *thread, void *userData, BPatchOneTimeCodeCallback cb = NULL, bool synchronous = true, bool *err = NULL, bool userRPC = true)

    thread == NULL if proc-wide

  .. cpp:function:: protected BPatch_process(const char *path, const char *argv[], BPatch_hybridMode mode, const char **envp = NULL, int stdin_fd = 0, int stdout_fd = 1, int stderr_fd = 2)

    for creating a process

  .. cpp:function:: protected BPatch_process(const char *path, int pid, BPatch_hybridMode mode)

    for attaching

  .. cpp:function:: protected BPatch_process(PCProcess *proc)

    for forking

  .. cpp:function:: PCProcess *lowlevel_process() const

    DO NOT USE this function should go away as soon as Paradyn links against Dyninst

  .. cpp:function:: bool triggerStopThread(instPoint *intPoint, func_instance *intFunc, int cb_ID, void *retVal)

    These internal funcs trigger callbacks registered to matching events

  .. cpp:function:: bool triggerSignalHandlerCB(instPoint *point, func_instance *func, long signum, BPatch_Vector<Dyninst::Address> *handlers)
  .. cpp:function:: bool triggerCodeOverwriteCB(instPoint * faultPoint, Dyninst::Address faultTarget)
  .. cpp:function:: bool setMemoryAccessRights(Dyninst::Address start, size_t size, Dyninst::ProcControlAPI::Process::mem_perm rights)
  .. cpp:function:: unsigned char *makeShadowPage(Dyninst::Address pageAddress)
  .. cpp:function:: void overwriteAnalysisUpdate(std::map<Dyninst::Address,unsigned char*>& owPages, std::vector<std::pair<Dyninst::Address,int> >& deadBlocks, std::vector<BPatch_function*>& owFuncs, std::set<BPatch_function *> &monitorFuncs, bool &changedPages, bool &changedCode )
  .. cpp:function:: HybridAnalysis *getHybridAnalysis()
  .. cpp:function:: bool protectAnalyzedCode()
  .. cpp:function:: void debugSuicide()

    DO NOT USE This is an internal debugging function