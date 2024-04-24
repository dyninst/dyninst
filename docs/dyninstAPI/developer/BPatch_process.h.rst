.. _`sec-dev:BPatch_process.h`:

BPatch_process.h
################

.. cpp:namespace:: dev

.. cpp:class:: BPatch_process : public BPatch_addressSpace

  .. cpp:member:: private bool terminated

    BPatch-level once the callbacks are sent by the llproc, we're terminated Used because callbacks go (and can clean up user code) before the low-level process sets flags.

  .. cpp:function:: private void *oneTimeCodeInternal(const BPatch_snippet &expr, BPatch_thread *thread, void *userData, BPatchOneTimeCodeCallback cb = NULL, bool synchronous = true, bool *err = NULL, bool userRPC = true)

    thread == NULL if proc-wide

  .. cpp:function:: protected BPatch_process(const char *path, const char *argv[], BPatch_hybridMode mode, const char **envp = NULL,\
                                             int stdin_fd = 0, int stdout_fd = 1, int stderr_fd = 2)

    Starts a new process for the exectuable at ``path`` with optional file descriptors.

    ``argv`` andn ``envp`` are the usual arguments to the POSIX ``execvp``.

    The new process is placed into a stopped state before executing any code.

  .. cpp:function:: protected BPatch_process(const char *path, int pid, BPatch_hybridMode mode)

    Attaches to the existing process with ID ``pid`` that was launched from the exectuable at ``path``.

  .. cpp:function:: protected BPatch_process(PCProcess *proc)

    Forks a new process from ``proc``.

  .. cpp:function:: PCProcess *lowlevel_process() const

    DO NOT USE this function should go away as soon as Paradyn links against Dyninst

  .. cpp:function:: bool triggerStopThread(instPoint *intPoint, func_instance *intFunc, int cb_ID, void *retVal)

    Causes the execution of a callback in the mutator that was triggered for the evtStopThread event.

    ``intPoint`` is the location where the event occurred. It is wrapped in a :cpp:class:`BPatch_point`
    and sent to the callback as a parameter. ``intFunc`` is the function where the event occurred. It is
    wrapped in a :cpp:class:`BPatch_function` and sent to the callback as a parameter. ``cb_ID`` helps us
    identify the correct call. ``retVal`` is the return value from the stopThread snippet.

    As BPatch_stopThreadExpr snippets allow a different callback to be triggered for each snippet instance,
    the cb_ID is used to find the right callback to trigger. This code had to be in a BPatch-level class
    so that we could utilize the findOrCreateBPFunc and findOrCreateBPPoint functions.


  .. cpp:function:: bool isStopped()

    The state visible to the user is different than the state maintained by ProcControlAPI because
    processes remain in a stopped state while doing event handling -- the user  shouldn't see the
    process in a stopped state in this case.

    The user should see the process stopped if

      - BPatch_process::stopExecution is invoked
      - A snippet breakpoint occurs
      - The mutatee is delivered a stop signal

  .. cpp:function:: bool triggerSignalHandlerCB(instPoint *point, func_instance *func, long signum, BPatch_Vector<Dyninst::Address> *handlers)

    Grabs BPatch level objects for the instPoint and enclosing function and triggers any registered
    callbacks for this signal/exception.

    ``intPoint`` is the instPoint at which the event occurred, will be wrapped in a BPatch_point and
    sent to the callback as a parameter. ``intFunc`` is the function in which the event occurred,
    will be wrapped in a BPatch_function and sent to the callback as a parameter.

    Returns ``true`` if a matching callback was found and no error occurred.

  .. cpp:function:: bool triggerCodeOverwriteCB(instPoint * faultPoint, Dyninst::Address faultTarget)

    Grabs BPatch level objects for the instPoint and enclosing function and triggers a registered callback
    if there is one.

    ``intPoint`` is the instPoint at which the event occurred, will be wrapped in a BPatch_point and
    sent to the callback as a parameter.

    Returns ``true`` if a matching callback was found and no error occurred.

  .. cpp:function:: bool setMemoryAccessRights(Dyninst::Address start, size_t size, Dyninst::ProcControlAPI::Process::mem_perm rights)
  .. cpp:function:: unsigned char *makeShadowPage(Dyninst::Address pageAddress)
  .. cpp:function:: void overwriteAnalysisUpdate(std::map<Dyninst::Address,unsigned char*>& owPages, std::vector<std::pair<Dyninst::Address,int> >& deadBlocks, std::vector<BPatch_function*>& owFuncs, std::set<BPatch_function *> &monitorFuncs, bool &changedPages, bool &changedCode )
  .. cpp:function:: HybridAnalysis *getHybridAnalysis()
  .. cpp:function:: bool protectAnalyzedCode()

    Protect analyzed code without protecting relocated code in the runtime library and for now only
    protect code in the aOut, also don't protect code that hasn't been analyzed.

  .. cpp:function:: void debugSuicide()

    Continues a stopped process, letting it execute in single step mode, and printing the
    current instruction as it executes.

    DO NOT USE This is an internal debugging function

  .. cpp:function:: bool dumpCore(const char *file, bool terminate)

    Causes the process to dump its state to the file ``file``, and optionally terminates.

  .. cpp:function:: bool dumpImage(const char *file)

    Writes the contents of memory into the file ``file``.

  .. cpp:function:: bool finalizeInsertionSetWithCatchup(bool atomic, bool *modified, BPatch_Vector<BPatch_catchupInfo> &catchup_handles)

    Does nothing.

  .. cpp:function:: bool hideDebugger()

      This is a Windows only function that removes debugging artifacts that
      are added to user-space datastructures and the heap of the debugged
      process, by CreateProcess and DebugActiveProcess.  Removing the artifacts
      doesn't have any effect on the process, as the kernel still knows that
      the process is being debugged.  Three of the artifacts are flags that can
      be reached through the Process Environment Block of the debuggee (PEB):

      1. BeingDebugged, one byte at offset 2 in the PEB.
      2. NtGlobalFlags, at offset 0x68 in the PEB
      3. There are two consecutive words of heap flags which are at offset 0x0c
         from the beginning of the heap.  The heap base address is at offset
         0x18 from the beginning of the PEB.

      The other thing this method does is clear the 0xabababababababab value that
      it CreateProcess adds to the end of the heap section when creating a debugged
      process, in response to the heap flag: HEAP_TAIL_CHECKING_ENABLED, which it
      sets to true for debugged processes.  We are clearing that flag, but by the
      time we do, the value is already written to disk.

      Various system calls can still be used by the debuggee to recognize that
      it is being debugged, so this is not a complete solution.


.. cpp:function:: static bool hasWeirdEntryBytes(func_instance *func)

  Is the first instruction ``[00 00] add byte ptr ds:[eax],al``?


.. cpp:class:: AstNullNode : public AstNode

  .. cpp:member:: private PCProcess *llproc
  .. cpp:member:: private BPatch_Vector<BPatch_thread *> threads
  .. cpp:member:: private int lastSignal
  .. cpp:member:: private int exitCode
  .. cpp:member:: private int exitSignal
  .. cpp:member:: private bool exitedNormally
  .. cpp:member:: private bool exitedViaSignal
  .. cpp:member:: private bool mutationsActive
  .. cpp:member:: private bool createdViaAttach
  .. cpp:member:: private bool detached
  .. cpp:member:: private bool terminated
  .. cpp:member:: private bool reportedExit
  .. cpp:member:: private HybridAnalysis *hybridAnalysis_

  .. cpp:function:: private void setExitedNormally()
  .. cpp:function:: private void setExitedViaSignal(int signalnumber)
  .. cpp:function:: private void setExitCode(int exitcode)
  .. cpp:function:: private void setExitSignal(int exitsignal)
  .. cpp:function:: private bool statusIsTerminated()

    Checks if the process has terminated.

  .. cpp:function:: private void setLastSignal(int signal)
  .. cpp:function:: private processType getType()
  .. cpp:function:: private bool getTerminated()
  .. cpp:function:: private bool getMutationsActive()

  .. cpp:function:: private static int oneTimeCodeCallbackDispatch(PCProcess *theProc, unsigned rpcid, void *userData, void *returnValue)

      ``theProc``: The process in which the RPC completed. ``userData``: This is a value that can be set when we invoke an inferior RPC
      ``returnValue``:  The value returned by the RPC.

  .. cpp:function:: private void* oneTimeCodeInternal(const BPatch_snippet &expr, BPatch_thread *thread, void *userData, \
                                                      BPatchOneTimeCodeCallback cb = NULL, bool synchronous = true, bool *err = NULL,\
                                                      bool userRPC = true)

    Causes ``expr`` to be evaluated once in the mutatee at the next available opportunity.

    If present, ``cb`` is invoked with ``userData`` when the snippet has executed in the mutatee. If ``synchronous`` is
    ``true``, then execution waits until the snippet has finished.

  .. cpp:function:: protected void triggerThreadCreate(PCThread *thread)
  .. cpp:function:: protected void triggerInitialThreadEvents()

    Events and callbacks shouldn't be delivered from a constructor so after a BPatch_process is
    constructed, this should be called.

  .. cpp:function:: protected void deleteBPThread(BPatch_thread *thrd)

    Removes ``thrd`` from this process' collection of threads.


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
