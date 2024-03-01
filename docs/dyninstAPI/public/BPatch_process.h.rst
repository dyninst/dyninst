.. _`sec:BPatch_process.h`:

BPatch_process.h
################

.. cpp:class:: BPatch_process : public BPatch_addressSpace
   
  **A running process**

  Includes one or more threads of execution and an address space.


  .. cpp:function:: int getPid()

    Return the system id for the mutatee process. On UNIX based systems this
    is a PID. On Windows this is the HANDLE object for a process.

  .. cpp:function:: int getAddressWidth()

    Get the address width (4 or 8) of the process

  .. cpp:function:: bool stopExecution()

    Puts the process into a stopped state.

    Depending on the operating system, stopping one process may stop all threads associated
    with a process.

  .. cpp:function:: bool continueExecution()

    Resume execution of mutatee process

  .. cpp:function:: bool terminateExecution()

    Terminates execution of the process and invokes the exit callback, if one is registered.

  .. cpp:function:: bool isStopped()

    Checks if mutatee process is currently stopped.

    If the process is stopped, then :cpp:func:`stopSignal` can be called to find out what
    signal caused the process to stop.

    May be called multiple times, and does not affect the state of the process.

  .. cpp:function:: int stopSignal()

    Returns signal number of signal that stopped mutatee process

  .. cpp:function:: bool isTerminated()

    Returns true if mutatee process is terminated

    May be called multiple times, and does not affect the state of the process.

    This may involve checking for thread events that may have recently changed this thread's status.

  .. cpp:function:: BPatch_exitType terminationStatus()

    Indicates how the program exited.

    If the process has exited, terminationStatus will indicate whether the
    process exited normally or because of a signal. If the process has not
    exited, NoExit will be returned. On AIX, the reason why a process exited
    will not be available if the process was not a child of the Dyninst
    mutator; in this case, ExitedNormally will be returned in both normal
    and signal exit cases.

  .. cpp:function:: int getExitCode()

    If the process exited in a normal way, getExitCode will return the
    associated exit code.

    Returns the actual exit code as provided by the debug
    interface and seen by the parent process. In particular, on Linux, this
    means that exit codes are normalized to the range 0-255.

  .. cpp:function:: int getExitSignal()

    If the process exited because of a received signal, returns the associated signal number.

  .. cpp:function:: bool wasRunningWhenAttached()

    Detach from the mutatee process, optionally leaving it running

  .. cpp:function:: void detach(bool cont)

    Detach from the process. The process must be stopped to call this
    function. Instrumentation and other changes to the process will remain
    active in the detached copy. The cont parameter is used to indicate if
    the process should be continued as a result of detaching.

    Linux does not support detaching from a process while leaving it
    stopped. All processes are continued after detach on Linux.

  .. cpp:function:: bool isDetached()

    Returns true if DyninstAPI is detached from this mutatee

  .. cpp:function:: void getThreads(BPatch_Vector<BPatch_thread *> &thrds)

    Get the list of threads in the process.

  .. cpp:function:: bool isMultithreaded()

    Checks if this process has more than one thread.

  .. cpp:function:: bool isMultithreadCapable()

    Checks if the process can create threads (e.g., it contains a threading library) even if it has not yet.

  .. cpp:function:: BPatch_thread* getThread(dynthread_t tid)

    Returns one of this process's threads, given a tid

  .. cpp:function:: BPatch_thread* getThreadByIndex(unsigned index)

    Returns one of this process's threads, given an index

  .. cpp:function:: BPatch_variableExpr* getInheritedVariable(BPatch_variableExpr &pVar)

    Retrieves an existing variable (such as one created by :cpp:func:`BPatch_process::malloc`) that
    was created in a parent process and now exists in a forked child process.

    When a process forks all existing BPatch_variableExprs are copied to the child process, but the Dyninst
    handles for these objects are not valid in the child BPatch_process.
    This function is invoked on the child process’ BPatch_process, parentVar
    is a variable from the parent process, and a handle to a variable in the
    child process is returned. If parentVar was not allocated in the parent
    process, then NULL is returned.

  .. cpp:function:: BPatchSnippetHandle* getInheritedSnippet(BPatchSnippetHandle &parentSnippet)

    Retrieves the snippet corresponding to ``parentSnippet`` that exists in this process and was
    inherited from and originally created in its parent process.

    This is similar to :cpp:func:`getInheritedVariable`, but operates on ``BPatchSnippetHandles``.

    Returns ``NULL`` if ``parentSnippet`` is not associated with the parent process.

  .. cpp:function:: void beginInsertionSet()

    Start the batch insertion of multiple points all calls to insertSnippet after this call will not
    actually instrument until :cpp:func:`finalizeInsertionSet` is called.

  .. cpp:function:: bool finalizeInsertionSet(bool atomic, bool *modified = NULL)

    Finalizes all instrumentation logically added since a call to beginInsertionSet.

    Arguments are ignored.

    Individual instrumentation can be manipulated via the :cpp:class:`BPatchSnippetHandles`
    returned from individual calls to :cpp:func:`insertSnippet`.

    Returns ``true`` if all instrumentation was successfully inserted.

  .. cpp:function:: void* oneTimeCode(const BPatch_snippet &expr, bool *err = NULL)

    Cause the snippet expr to be executed by the mutatee immediately.

    If the process is multithreaded, the snippet is run on a thread chosen by
    Dyninst. If the user requires the snippet to be run on a particular
    thread, use the BPatch_thread version of this function instead. The
    process must be stopped to call this function. The behavior is
    synchronous; oneTimeCode will not return until after the snippet has
    been run in the application.

  .. cpp:function:: bool oneTimeCodeAsync(const BPatch_snippet &expr, void *userData = NULL,\
                                          BPatchOneTimeCodeCallback cb = NULL)

    This function sets up a snippet to be evaluated by the process at the
    next available opportunity. When the snippet finishes running Dyninst
    will callback any function registered through
    BPatch::registerOneTimeCodeCallback, with userData passed as a
    parameter. This function return true on success and false if it could
    not post the oneTimeCode.

    If the process is multithreaded, the snippet is run on a thread chosen
    by Dyninst. If the user requires the snippet to be run on a particular
    thread, use the BPatch_thread version of this function instead. The
    behavior is asynchronous; oneTimeCodeAsync returns before the snippet is
    executed.

    If the process is running when oneTimeCodeAsync is called, expr will be
    run immediately. If the process is stopped, then expr will be run when
    the process is continued.

  .. cpp:function:: virtual BPatch_object* loadLibrary(const char *libname, bool reload = false)

    Load a shared library into the mutatee's address space  Returns true if successful

  .. cpp:function:: bool supportsUserThreadEvents()



.. cpp:enum:: BPatch_asyncEventType

  .. cpp:enumerator:: BPatch_nullEvent
  .. cpp:enumerator:: BPatch_newConnectionEvent
  .. cpp:enumerator:: BPatch_internalShutDownEvent
  .. cpp:enumerator:: BPatch_threadCreateEvent
  .. cpp:enumerator:: BPatch_threadDestroyEvent
  .. cpp:enumerator:: BPatch_dynamicCallEvent
  .. cpp:enumerator:: BPatch_userEvent
  .. cpp:enumerator:: BPatch_errorEvent
  .. cpp:enumerator:: BPatch_dynLibraryEvent
  .. cpp:enumerator:: BPatch_preForkEvent
  .. cpp:enumerator:: BPatch_postForkEvent
  .. cpp:enumerator:: BPatch_execEvent
  .. cpp:enumerator:: BPatch_exitEvent
  .. cpp:enumerator:: BPatch_signalEvent
  .. cpp:enumerator:: BPatch_oneTimeCodeEvent

.. cpp:type:: Dyninst::THR_ID dynthread_t


.. cpp:class:: BPatch_catchupInfo

  .. cpp:member:: BPatch_snippet snip
  .. cpp:member:: BPatchSnippetHandle *sh
  .. cpp:member:: BPatch_thread *thread

