.. _`sec:BPatch_thread.h`:

BPatch_thread.h
###############

.. cpp:class:: BPatch_thread
   
  The **BPatch_thread** class represents and controls a thread of
  execution that is running in a process.

  .. cpp:function:: void getCallStack(std::vector<BPatch_frame>& stack)

    This function fills the given vector with current information about the
    call stack of the thread. Each stack frame is represented by a
    BPatch_frame (see section 4.24 for information about this class).

  .. cpp:function:: dynthread_t getTid()

    This function returns a platform-specific identifier for this thread.
    This is the identifier that is used by the threading library. For
    example, on pthread applications this function will return the thread’s
    pthread_t value.

  .. cpp:function:: Dyninst::LWP getLWP()

    This function returns a platform-specific identifier that the operating
    system uses to identify this thread. For example, on UNIX platforms this
    returns the LWP id. On Windows this returns a HANDLE object for the
    thread.

  .. cpp:function:: unsigned getBPatchID()

    This function returns a Dyninst-specific identifier for this thread.
    These ID’s apply only to running threads, the BPatch ID of an already
    terminated thread my be repeated in a new thread.

  .. cpp:function:: BPatch_function *getInitialFunc()

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

  .. cpp:function:: BPatch_process *getProcess()

    Return the BPatch_process that contains this thread.

  .. cpp:function:: void *oneTimeCode(const BPatch_snippet &expr, bool *err = NULL)

    Cause the snippet expr to be evaluated by the process immediately. This
    is similar to the BPatch_process::oneTimeCode function, except that the
    snippet is guaranteed to run only on this thread. The process must be
    stopped to call this function. The behavior is synchronous; oneTimeCode
    will not return until after the snippet has been run in the application.

  .. cpp:function:: bool oneTimeCodeAsync(const BPatch_snippet &expr, \
       void *userData = NULL, \
       BpatchOneTimeCodeCallback cb = NULL)

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
