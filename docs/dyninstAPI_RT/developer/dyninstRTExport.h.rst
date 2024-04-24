.. _`sec:dyninstRTExport.h`:

dyninstRTExport.h
#################

Function prototypes that may be useful for  dyninst users to directly
have access to from their own runtime libraries.

.. c::function:: int DYNINSTuserMessage(void *msg, unsigned int msg_size)

  May be used in conjunction with :c:func:`BPatch_process::registerUserMessageCallback`
  to implement a generic user-defined, asynchronous communications protocol from the mutatee
  (via this runtime library) to the mutator.

  Calls to ``DYNINSTuserMessage`` will result in ``msg`` (of ``msg_size`` bytes)
  being sent to the mutator, and then passed to the callback function
  provided by the API user via registerUserMessageCallback().

  Returns zero on success, nonzero on failure.

.. c::function:: int DYNINSTthreadCount()

  Returns the number of threads DYNINST currently knows about hich
  may differ at certain times from the number of threads actually present.

......

These function implement a locking mechanism that can be used by  a user's runtime library.

Be sure to always check for DYNINST_LIVE_LOCK and DYNINST_DEAD_LOCK.
When instrumenting multithread or signal-based application as these error
conditions can trigger even on simple synchronization mechanisms.


.. code:: c

  #define DYNINST_INITIAL_LOCK_PID ((void *)-129)

  /* Return values for 'dyninst_lock' */
  #define DYNINST_LIVE_LOCK      -1
  #define DYNINST_DEAD_LOCK      -2

  /* Declare a lock already initialized */
  #define DECLARE_DYNINST_LOCK(lck) dyninst_lock_t lck = {0, DYNINST_INITIAL_LOCK_PID}


.. c::type:: void * dyntid_t

.. c:struct:: dyninst_lock_t

  The contents of this structure are subject to change between dyninst
  versions.  Don't rely on it.

  .. c:member:: volatile int mutex
  .. c:member:: dyntid_t tid

.. c:function:: void dyninst_init_lock(dyninst_lock_t *lock)
.. c:function:: void dyninst_free_lock(dyninst_lock_t *lock)
.. c:function:: int dyninst_lock(dyninst_lock_t *lock)
.. c:function:: void dyninst_unlock(dyninst_lock_t *lock)

......

Internal functions that we export to ensure they show up.

.. c:function:: void DYNINSTsafeBreakPoint(void)
.. c:function:: void DYNINSTinit(void)
.. c:function:: void DYNINST_snippetBreakpoint(void)
.. c:function:: void DYNINST_stopThread(void *, void *, void *, void *)
.. c:function:: void DYNINST_stopInterProc(void *, void *, void *, void *, void *, void *)
.. c:function:: void *DYNINSTos_malloc(size_t, void *, void *) 
.. c:function:: int DYNINSTloadLibrary(char *)

......

Internal variables that we export to ensure they show up.

.. c:var:: extern dyntid_t (*DYNINST_pthread_self)(void)
.. c:var:: extern unsigned int DYNINSTobsCostLow
.. c:var:: extern int libdyninstAPI_RT_init_localCause
.. c:var:: extern int libdyninstAPI_RT_init_localPid
.. c:var:: extern int libdyninstAPI_RT_init_maxthreads
.. c:var:: extern int libdyninstAPI_RT_init_debug_flag
.. c:var:: extern struct DYNINST_bootstrapStruct DYNINST_bootstrap_info

