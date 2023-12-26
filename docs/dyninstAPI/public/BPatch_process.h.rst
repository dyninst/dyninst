BPatch_process.h
================

.. cpp:class:: BPatch_process
   
   The **BPatch_process** class represents a running process, which
   includes one or more threads of execution and an address space.
   
   .. cpp:function:: bool stopExecution()
      
   .. cpp:function:: bool continueExecution()
      
   .. cpp:function:: bool terminateExecution()
      
      These three functions change the running state of the process.
      stopExecution puts the process into a stopped state. Depending on the
      operating system, stopping one process may stop all threads associated
      with a process. continueExecution continues execution of the process.
      terminateExecution terminates execution of the process and will invoke
      the exit callback if one is registered. Each function returns true on
      success, or false for failure. Stopping or continuing a terminated
      thread will fail and these functions will return false.
      
   .. cpp:function:: bool isStopped()
      
   .. cpp:function:: int stopSignal()
      
   .. cpp:function:: bool isTerminated()
      
      These three functions query the status of a process. isStopped returns
      true if the process is currently stopped. If the process is stopped (as
      indicated by isStopped), then stopSignal can be called to find out what
      signal caused the process to stop. isTerminated returns true if the
      process has exited. Any of these functions may be called multiple times,
      and calling them will not affect the state of the process.
      
   .. cpp:function:: BPatch_variableExpr *getInheritedVariable(BPatch_variableExpr &parentVar)
      
      Retrieve a new handle to an existing variable (such as one created by
      BPatch_process::malloc) that was created in a parent process and now
      exists in a forked child process. When a process forks all existing
      BPatch_variableExprs are copied to the child process, but the Dyninst
      handles for these objects are not valid in the child BPatch_process.
      This function is invoked on the child process’ BPatch_process, parentVar
      is a variable from the parent process, and a handle to a variable in the
      child process is returned. If parentVar was not allocated in the parent
      process, then NULL is returned.
      
   .. cpp:function:: BPatchSnippetHandle *getInheritedSnippet(BPatchSnippetHandle &parentSnippet)
      
      This function is similar to getInheritedVariable, but operates on
      BPatchSnippetHandles. Given a child process that was created via fork
      and a BPatch­SnippetHandle, parentSnippet, from the parent process, this
      function will return a handle to parentSnippet that is valid in the
      child process. If it is determined that parentSnippet is not associated
      with the parent process, then NULL is returned.
      
   .. cpp:function:: void detach(bool cont)
      
      Detach from the process. The process must be stopped to call this
      function. Instrumentation and other changes to the process will remain
      active in the detached copy. The cont parameter is used to indicate if
      the process should be continued as a result of detaching.
      
      Linux does not support detaching from a process while leaving it
      stopped. All processes are continued after detach on Linux.
      
   .. cpp:function:: int getPid()
      
      Return the system id for the mutatee process. On UNIX based systems this
      is a PID. On Windows this is the HANDLE object for a process.
      
   .. cpp:enum:: BPatch_exitType
   .. cpp:enumerator:: BPatch_exitType::NoExit
   .. cpp:enumerator:: BPatch_exitType::ExitedNormally
   .. cpp:enumerator:: BPatch_exitType::ExitedViaSignal
      
   .. cpp:function:: BPatch_exitType terminationStatus()
      
      If the process has exited, terminationStatus will indicate whether the
      process exited normally or because of a signal. If the process has not
      exited, NoExit will be returned. On AIX, the reason why a process exited
      will not be available if the process was not a child of the Dyninst
      mutator; in this case, ExitedNormally will be returned in both normal
      and signal exit cases.
      
   .. cpp:function:: int getExitCode()
      
      If the process exited in a normal way, getExitCode will return the
      associated exit code. Prior to Dyninst 8.2, getExitCode would return the
      argument passed to exit or the value returned by main; in Dyninst 8.2
      and later, it returns the actual exit code as provided by the debug
      interface and seen by the parent process. In particular, on Linux, this
      means that exit codes are normalized to the range 0-255.
      
   .. cpp:function:: int getExitSignal()
      
      If the process exited because of a received signal, getExitSignal will
      return the associated signal number.
      
   .. cpp:function:: void oneTimeCode(const BPatch_snippet &expr)
      
      Cause the snippet expr to be executed by the mutatee immediately. If the
      process is multithreaded, the snippet is run on a thread chosen by
      Dyninst. If the user requires the snippet to be run on a particular
      thread, use the BPatch_thread version of this function instead. The
      process must be stopped to call this function. The behavior is
      synchronous; oneTimeCode will not return until after the snippet has
      been run in the application.
      
   .. cpp:function:: bool oneTimeCodeAsync(const BPatch_snippet &expr, void *userData = NULL)
      
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
      
   .. cpp:function:: void getThreads(std::vector<BPatch_thread *> &thrds)
      
      Get the list of threads in the process.
      
   .. cpp:function:: bool isMultithreaded()
      
   .. cpp:function:: bool isMultithreadCapable()
      
      The former returns true if the process contains multiple threads; the
      latter returns true if the process can create threads (e.g., it contains
      a threading library) even if it has not yet.