.. _`sec-dev:BPatch.h`:

BPatch.h
########

.. cpp:namespace:: dev

Keep old versions defined, that way someone can test if we're more
at or more recent than version 5.1 with '#if defined(DYNINST_5_1)'
If they want to get the current version, they should use DYNINST_MAJOR,
DYNINST_MINOR, and DYNINST_SUBMINOR.

.. note:: These are no longer supported.

.. code:: cpp

  #define DYNINST_5_1
  #define DYNINST_5_2
  #define DYNINST_6_0
  #define DYNINST_6_1
  #define DYNINST_7_0
  #define DYNINST_8_0
  #define DYNINST_8_1
  #define DYNINST_8_1_1
  #define DYNINST_8_1_2
  #define DYNINST_8_2
  #define DYNINST_9_0
  #define DYNINST_9_1

  #define DYNINST_MAJOR DYNINST_MAJOR_VERSION
  #define DYNINST_MINOR DYNINST_MINOR_VERSION
  #define DYNINST_SUBMINOR DYNINST_PATCH_VERSION


.. cpp:class:: BPatch

  .. cpp:member:: private BPatch_libInfo *info
  .. cpp:member:: private bool typeCheckOn
  .. cpp:member:: private int lastError
  .. cpp:member:: private bool debugParseOn
  .. cpp:member:: private bool baseTrampDeletionOn
  .. cpp:member:: private bool trampRecursiveOn

    If true, trampolines can recurse to their heart's content. Defaults to false

  .. cpp:member:: private bool forceRelocation_NP
  .. cpp:member:: private bool autoRelocation_NP

    If true,allows automatic relocation of functions if dyninst deems it necessary.  Defaults to true

  .. cpp:member:: private bool saveFloatingPointsOn

    If true, we save FPRs in situations we normally would  Defaults to true

  .. cpp:member:: private bool forceSaveFloatingPointsOn
  .. cpp:member:: private bool livenessAnalysisOn_

    If true, we will use liveness calculations to avoid saving registers on platforms that support it. Defaults to true.

  .. cpp:member:: private int livenessAnalysisDepth_

    How far through the CFG do we follow calls?

  .. cpp:member:: private bool asyncActive

    If true, override requests to block while waiting for events, polling instead

  .. cpp:member:: private bool delayedParsing_

    If true, deep parsing (anything beyond symtab info) is delayed until accessed  Note: several bpatch constructs have "access everything" behavior, which will trigger full parsing. This should be looked into.

  .. cpp:member:: private bool instrFrames
  .. cpp:member:: private BPatch_stats stats
  .. cpp:function:: private void updateStats()
  .. cpp:member:: private char *systemPrelinkCommand

    this is used to denote the fully qualified name of the prelink command on linux

  .. cpp:function:: private void continueIfExists(int pid)

    Wrapper - start process running if it was not deleted.  We use this at the end of callbacks to user code, since those callbacks may delete BPatch objects.

  .. cpp:member:: private int notificationFDOutput_

    Internal notification file descriptor - a pipe

  .. cpp:member:: private int notificationFDInput_
  .. cpp:member:: private bool FDneedsPolling_

    Easier than non-blocking reads... there is either 1 byte in the pipe or 0.

  .. cpp:member:: private BPatchErrorCallback errorCallback

    Callbacks

  .. cpp:member:: private BPatchForkCallback preForkCallback
  .. cpp:member:: private BPatchForkCallback postForkCallback
  .. cpp:member:: private BPatchExecCallback execCallback
  .. cpp:member:: private BPatchExitCallback exitCallback
  .. cpp:member:: private BPatchOneTimeCodeCallback oneTimeCodeCallback
  .. cpp:member:: private BPatchDynLibraryCallback dynLibraryCallback
  .. cpp:member:: private BPatchAsyncThreadEventCallback threadCreateCallback
  .. cpp:member:: private BPatchAsyncThreadEventCallback threadDestroyCallback
  .. cpp:member:: private BPatchDynamicCallSiteCallback dynamicCallSiteCallback
  .. cpp:member:: private InternalSignalHandlerCallback signalHandlerCallback
  .. cpp:member:: private std::set<long> callbackSignals
  .. cpp:member:: private InternalCodeOverwriteCallback codeOverwriteCallback
  .. cpp:member:: private BPatch_Vector<BPatchUserEventCallback> userEventCallbacks
  .. cpp:member:: private BPatch_Vector<BPatchStopThreadCallback> stopThreadCallbacks
  .. cpp:member:: private bool inDestructor

    If we're destroying everything, skip cleaning up some intermediate data structures

  .. cpp:function:: void registerProvisionalThread(int pid)
  .. cpp:function:: void registerForkedProcess(PCProcess *parentProc, PCProcess *childProc)
  .. cpp:function:: void registerForkingProcess(int forkingPid, PCProcess *proc)
  .. cpp:function:: void registerExecExit(PCProcess *proc)
  .. cpp:function:: void registerExecCleanup(PCProcess *proc, char *arg0)
  .. cpp:function:: void registerNormalExit(PCProcess *proc, int exitcode)
  .. cpp:function:: void registerSignalExit(PCProcess *proc, int signalnum)
  .. cpp:function:: void registerThreadExit(PCProcess *llproc, PCThread *llthread)
  .. cpp:function:: bool registerThreadCreate(BPatch_process *proc, BPatch_thread *newthr)
  .. cpp:function:: void registerProcess(BPatch_process *process, int pid=0)
  .. cpp:function:: void unRegisterProcess(int pid, BPatch_process *proc)
  .. cpp:function:: void registerUserEvent(BPatch_process *process, void *buffer, unsigned int bufsize)
  .. cpp:function:: void registerDynamicCallsiteEvent(BPatch_process *process, Dyninst::Address callTarget, Dyninst::Address callAddr)
  .. cpp:function:: void registerStopThreadCallback(BPatchStopThreadCallback stopCB)
  .. cpp:function:: int getStopThreadCallbackID(BPatchStopThreadCallback stopCB)
  .. cpp:function:: void registerLoadedModule(PCProcess *process, mapped_object *obj)
  .. cpp:function:: void registerUnloadedModule(PCProcess *process, mapped_object *obj)
  .. cpp:function:: BPatch_thread *getThreadByPid(int pid, bool *exists = NULL)
  .. cpp:function:: BPatch_process *getProcessByPid(int pid, bool *exists = NULL)
  .. cpp:function:: static void reportError(BPatchErrorLevel severity, int number, const char *str)
  .. cpp:function:: void clearError()
  .. cpp:function:: int getLastError()

  .. cpp:function:: void getBPatchVersion(int &major, int &minor, int &subminor)

    Returns the version number for Dyninst.

    The major version number will be stored
    in ``major``, the minor version number in ``minor``, and the subminor version in
    ``subminor``. For example, under Dyninst 5.1.0, this function will return 5
    in ``major``, 1 in ``minor``, and 0 in ``subminor``.

------

This is a purposefully undocumented prototype of a "remote debugging"
interface.  Meant to generalize debuggers like remote gdb and wtx.

.. cpp:enum:: BPatch_remote_t

  .. cpp:enumerator:: BPATCH_REMOTE_DEBUG_WTX
  .. cpp:enumerator:: BPATCH_REMOTE_DEBUG_END


.. cpp:struct:: BPatch_remoteWtxInfo

  .. cpp:member:: char *target
  .. cpp:member:: char *tool
  .. cpp:member:: char *host


.. cpp:struct:: BPatch_remoteHost

  .. cpp:member:: BPatch_remote_t type
  .. cpp:member:: void *info
