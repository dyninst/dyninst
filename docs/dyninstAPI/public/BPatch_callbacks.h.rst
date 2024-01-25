.. _`sec:BPatch_callbacks.h`:

BPatch_callbacks.h
##################


The following functions are intended as a way for API users to be
informed when an error or significant event occurs. Each function allows
a user to register a handler for an event. The return code for all
callback registration functions is the address of the handler that was
previously registered (which may be NULL if no handler was previously
registered). For backwards compatibility reasons, some callbacks may
pass a BPatch_thread object when a BPatch_process may be more
appropriate. A BPatch_thread may be converted into a BPatch_process
using :cpp:func:`BPatch_thread::getProcess()`.


Asynchronous Callbacks
**********************

.. cpp:type:: void (*BPatchAsyncThreadEventCallback)(BPatch_process *proc, BPatch_thread *thread)
   
.. cpp:function:: bool registerThreadEventCallback(BPatch_asyncEventType type, BPatchAsyncThreadEventCallback cb)
   
.. cpp:function:: bool removeThreadEventCallback(BPatch_asyncEventType type, BPatch_AsyncThreadEventCallback cb)
   
  The type parameter can be either one of BPatch_threadCreateEvent or
  BPatch_threadDestroyEvent. Different callbacks can be registered for
  different values of type.

Code Discovery Callbacks
************************

.. cpp:type:: void (*BPatchCodeDiscoveryCallback)( \
      BPatch_Vector<BPatch_function*> &newFuncs, \
      BPatch_Vector<BPatch_function*> &modFuncs)
   
.. cpp:function:: bool registerCodeDiscoveryCallback(BPatchCodeDiscoveryCallback cb)
   
.. cpp:function:: bool removeCodeDiscoveryCallback(BPatchCodeDiscoveryCallback cb)
   
  This callback is invoked whenever previously un-analyzed code is
  discovered through runtime analysis, and delivers a vector of functions
  whose analysis have been modified and a vector of functions that are
  newly discovered.

Code Overwrite Callbacks
************************

.. cpp:type:: void (*BPatchCodeOverwriteBeginCallback)(BPatch_Vector<BPatch_basicBlock*> &overwriteLoopBlocks);
   
.. cpp:type void (*BPatchCodeOverwriteEndCallback)( \
      BPatch_Vector<std::pair<Dyninst::Address,int> > &deadBlocks, \
      BPatch_Vector<BPatch_function*> &owFuncs, \
      BPatch_Vector<BPatch_function*> &modFuncs, \
      BPatch_Vector<BPatch_function*> &newFuncs)
   
.. cpp:function:: bool registerCodeOverwriteCallbacks( \
      BPatchCodeOverwriteBeginCallback cbBegin, \
      BPatchCodeOverwriteEndCallback cbEnd)

  Register a callback at the beginning and end of overwrite events. Only
  invoke if Dyninst's hybrid analysis mode is set to BPatch_defensiveMode.

  The BPatchCodeOverwriteBeginCallback callback allows the user to remove
  any instrumentation when the program starts writing to a code page,
  which may be desirable as instrumentation cannot be removed during the
  overwrite loop's execution, and any breakpoint instrumentation will
  dramatically slow the loop's execution.

  The BPatchCodeOverwriteEndCallback callback delivers the effects of the
  overwrite loop when it is done executing. In many cases no code will
  have changed.

Dynamic calls
*************

.. cpp:type:: void (*BPatchDynamicCallSiteCallback)( \
         BPatch_point *at_point, BPatch_function *called_function);
   
.. cpp:function:: bool registerDynamicCallCallback(BPatchDynamicCallSiteCallback cb);
   
.. cpp:function:: bool removeDynamicCallCallback(BPatchDynamicCallSiteCallback cb);
   
  The registerDynamicCallCallback interface will not automatically
  instrument any dynamic call site. To make sure the call back function is
  called, the user needs to explicitly instrument dynamic call sites. One
  way to achieve this goal is to first get instrumentation points
  representing dynamic call sites and then call BPatch_point::monitorCalls
  with a NULL input parameter.

Dynamic libraries
*****************

.. cpp:type::  void (*BPatchDynLibraryCallback)(BPatch_thread *thr, \
         BPatch_object *obj, bool loaded);
   
.. cpp:function:: BPatchDynLibraryCallback registerDynLibraryCallback(BPatchDynLibraryCallback func)
   
  Note that in versions previous to 9.1, BPatchDynLibraryCallback’s
  signature took a BPatch_module instead of a BPatch_object.

Errors
******

.. cpp:enum:: BPatchErrorLevel

  .. cpp:enumerator:: BPatchFatal
  .. cpp:enumerator:: BPatchSerious
  .. cpp:enumerator:: BPatchWarning
  .. cpp:enumerator:: BPatchInfo

.. cpp:type:: void (*BPatchErrorCallback)(BPatchErrorLevel severity, int number, const char * const *params)
   
.. cpp:function:: BPatchErrorCallback registerErrorCallback(BPatchErrorCallback func)
   
  This function registers the error callback function with the BPatch
  class. The return value is the address of the previous error callback
  function. Dyninst users can change the error callback during program
  execution (e.g., one error callback before a GUI is initialized, and a
  different one after). The severity field indicates how important the
  error is (from fatal to information/status). The number is a unique
  number that identifies this error message. Params are the parameters
  that describe the detail about an error, e.g., the process id where the
  error occurred. The number and meaning of params depends on the error.
  However, for a given error number the number of parameters returned will
  always be the same.

Exec
****

.. cpp:type:: void (*BPatchExecCallback)(BPatch_thread *thr)
   
.. cpp:function:: BPatchExecCallback registerExecCallback(BPatchExecCallback func)
   
  .. warning::
    Not implemented on Windows.
      
Exit
****

.. cpp:enum:: BPatch_exitType

  .. cpp:enumerator:: NoExit
  .. cpp:enumerator:: ExitedNormally
  .. cpp:enumerator:: ExitedViaSignal

.. cpp:type:: void (*BPatchExitCallback)(BPatch_thread *proc, BPatch_exitType exit_type);
   
.. cpp:function:: BPatchExitCallback registerExitCallback(BPatchExitCallback func)
   
  Register a function to be called when a process terminates. For a normal
  process exit, the callback will actually be called just before the
  process exits, but while its process state still exists. This allows
  final actions to be taken on the process before it actually exits. The
  function BPatch_thread::isTerminated() will return true in this context
  even though the process hasn’t yet actually exited. In the case of an
  exit due to a signal, the process will have already exited.

Fork
****

.. cpp:type:: void (*BPatchForkCallback)(BPatch_thread *parent, BPatch_thread* child);
   
  This is the prototype for the pre-fork and post-fork callbacks. The
  parent parameter is the parent thread, and the child parameter is a
  BPatch_thread in the newly created process. When invoked as a pre-fork
  callback, the child is NULL.

.. cpp:function:: BPatchForkCallback registerPreForkCallback(BPatchForkCallback func)
   
  .. warning::
    not implemented on Windows
   
.. cpp:function:: BPatchForkCallback registerPostForkCallback(BPatchForkCallback func)
   
  Register callbacks for pre-fork (before the child is created) and
  post-fork (immediately after the child is created). When a pre-fork
  callback is executed the child parameter will be NULL.

  .. warning::
    not implemented on Windows

   
One Time Code
*************

.. cpp:type:: void (*BPatchOneTimeCodeCallback)(Bpatch_thread *thr, void *userData, void *returnValue)
   
.. cpp:function:: BPatchOneTimeCodeCallback registerOneTimeCodeCallback(BPatchOneTimeCodeCallback func)
   
  The thr field contains the thread that executed the oneTimeCode (if
  thread-specific) or an unspecified thread in the process (if
  process-wide). The userData field contains the value passed to the
  oneTimeCode call. The returnValue field contains the return result of
  the oneTimeCode snippet.

Signal Handler
**************

.. cpp:type:: void (*BPatchSignalHandlerCallback)(BPatch_point *at_point, \
         long signum, std::vector<Dyninst::Address> *handlers)
   
.. cpp:function:: bool registerSignalHandlerCallback(BPatchSignalHandlerCallback cb, std::set<long> &signal_numbers)
   
.. cpp:function:: bool registerSignalHandlerCallback(BPatchSignalHandlerCallback cb, \
         BPatch_Set<long> *signal_numbers)
   
.. cpp:function:: bool removeSignalHandlerCallback(BPatchSignalHandlerCallback cb);
   
  This function registers the signal handler callback function with the
  BPatch class. The return value indicates success or failure. The
  signal_numbers set contains those signal numbers for which the callback
  will be invoked.

  The at_point parameter indicates the point at which the signal/exception
  was raised, signum is the number of the signal/exception that was
  raised, and the handlers vector contains any registered handler(s) for
  the signal/exception. In Windows this corresponds to the stack of
  Structured Exception Handlers, while for Unix systems there will be at
  most one registered exception handler. This functionality is only fully
  implemented for the Windows platform.

Stopped Threads
***************

.. cpp:type:: void (*BPatchStopThreadCallback)(BPatch_point *at_point, void *returnValue)
   
  This is the prototype for the callback that is associated with the
  stopThreadExpr snippet class (see Section 4.13). Unlike the other
  callbacks in this section, stopThreadExpr callbacks are registered
  during the creation of the stopThreadExpr snippet type. Whenever a
  stopThreadExpr snippet executes in a given thread, the snippet evaluates
  the calculation snippet that stopThreadExpr takes as a parameter, stops
  the thread’s execution and invokes this callback. The at_point parameter
  is the BPatch_point at which the stopThreadExpr snippet was inserted,
  and returnValue contains the computation made by the calculation
  snippet.

User-triggered callbacks
************************

.. cpp:type:: void (*BPatchUserEventCallback)(BPatch_process *proc, void *buf, unsigned int bufsize);
   
.. cpp:function:: bool registerUserEventCallback(BPatchUserEventCallback cb)
   
.. cpp:function:: bool removeUserEventCallback(BPatchUserEventCallback cb)
   
  Register a callback that is executed when the user sends a message from
  the mutatee using the DYNINSTuserMessage function in the runtime
  library.
