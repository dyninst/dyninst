.. _`sec:BPatch_frame.h`:

BPatch_frame.h
##############

.. cpp:class:: BPatch_frame
   
  **Frame information needed for stack walking**

  .. cpp:function:: BPatch_frame(BPatch_thread *_thread, void *_pc, void *_fp, bool isf = false, \
                                 bool istr = false, BPatch_point *point = NULL, bool isSynth = false)

  .. cpp:function:: BPatch_frame()
  .. cpp:function:: BPatch_frameType getFrameType()

    Returns the type of the stack frame.

  .. cpp:function:: bool isSynthesized()

    Returns true if this frame was artificially created, false otherwise.

    Per-frame method for determining  how the frame was created.

    .. warning:: Only call if you know what you are doing!

  .. cpp:function:: BPatch_thread * getThread()

    Returns the thread associated with the stack frame.

  .. cpp:function:: BPatch_point * getPoint()

    For stack frames corresponding to inserted instrumentation, returns the
    instrumentation point where that instrumentation was inserted. For other
    frames, returns NULL.

  .. cpp:function:: void *getPC()

    Returns the program counter associated with the stack frame.

  .. cpp:function:: void * getFP()

    Return the frame pointer for the stack frame.

  .. cpp:function:: BPatch_function * findFunction()

    Returns the function corresponding to this stack frame, NULL if there is none

  .. cpp:function:: BPatch_point * findPoint()


.. cpp:enum:: BPatch_frameType

  .. cpp:enumerator:: BPatch_frameNormal

    for a stack frame for a function

  .. cpp:enumerator:: BPatch_frameSignal

    for the stack frame created when a signal is delivered

  .. cpp:enumerator:: BPatch_frameTrampoline

    for a stack frame created by internal Dyninst instrumentation
