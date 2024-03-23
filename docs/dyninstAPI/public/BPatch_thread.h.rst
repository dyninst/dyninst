.. _`sec:BPatch_thread.h`:

BPatch_thread.h
###############

.. cpp:class:: BPatch_thread
   
  **A thread of execution running in a process**

  .. cpp:function:: void getCallStack(std::vector<BPatch_frame>& stack)

    Returns in ``stack`` the current information about the call stack of the thread.

  .. cpp:function:: BPatch_process * getProcess()

    Returns a pointer to the process that owns this thread

  .. cpp:function:: dynthread_t getTid()

    Returns a platform-specific identifier for this thread.

    This is the identifier that is used by the threading library.

  .. cpp:function:: Dyninst::LWP getLWP()

    Returns a platform-specific identifier that the operating
    system uses to identify this thread.

  .. cpp:function:: unsigned getBPatchID()

    This function returns a Dyninst-specific identifier for this thread.
    These ID’s apply only to running threads, the BPatch ID of an already
    terminated thread my be repeated in a new thread.

  .. cpp:function:: BPatch_function * getInitialFunc()

    Return the function that was used by the application to start this
    thread. For example, on pthread applications this will return the
    initial function that was passed to pthread_create.

  .. cpp:function:: unsigned long getStackTopAddr()

    Returns the base address for this thread’s stack.

  .. cpp:function:: bool isDeadOnArrival()

    This function returns true if this thread terminated execution before
    Dyninst was able to attach to it. Since Dyninst performs new thread
    detection asynchronously, it is possible for a thread to be created and
    destroyed before Dyninst can attach to it. When this happens, a new
    BPatch_thread is created, but isDeadOnArrival always returns true for
    this thread. It is illegal to perform any thread-level operations on a
    dead on arrival thread.

  .. cpp:function:: ~BPatch_thread()

  .. cpp:function:: void * oneTimeCode(const BPatch_snippet &expr, bool *err = NULL)

    Cause the snippet expr to be evaluated by the process immediately. This
    is similar to the BPatch_process::oneTimeCode function, except that the
    snippet is guaranteed to run only on this thread. The process must be
    stopped to call this function. The behavior is synchronous; oneTimeCode
    will not return until after the snippet has been run in the application.

    Have mutatee execute specified code expr once.  Wait until done.

  .. cpp:function:: bool oneTimeCodeAsync(const BPatch_snippet &expr, void *userData = NULL,\
                                          BPatchOneTimeCodeCallback cb = NULL)

    This function sets up the snippet expr to be evaluated by this thread at
    the next available opportunity. When the snippet finishes running,
    Dyninst will callback any function registered through
    BPatch::registerOneTimeCodeCallback, with userData passed as a
    parameter. This function returns true if expr was posted or false
    otherwise.

    This is similar to the BPatch_process::oneTimeCodeAsync function, except
    that the snippet is guaranteed to run only on this thread. The process
    must be stopped to call this function. The behavior is asynchronous;
    oneTimeCodeAsync returns before the snippet is executed.
