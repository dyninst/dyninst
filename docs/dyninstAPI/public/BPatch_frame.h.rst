BPatch_frame.h
==============

``BPatch_frame``
----------------
.. cpp:namespace:: BPatch_frame

.. cpp:class:: BPatch_frame
   
   A **BPatch_frame** object represents a stack frame. The getCallStack
   member function of BPatch_thread returns a vector of BPatch_frame
   objects representing the frames currently on the stack.
   
   .. cpp:function:: BPatch_frameType getFrameType()
      
      Return the type of the stack frame. Possible types are:
      
      +------------------------+------------------------------------+
      | **Frame Type**         | **Meaning**                        |
      +------------------------+------------------------------------+
      | BPatch_frameNormal     | A normal stack frame.              |
      +------------------------+------------------------------------+
      | BPatch_frameSignal     | A frame that represents a signal   |
      |                        | invocation.                        |
      +------------------------+------------------------------------+
      | BPatch_frameTrampoline | A frame the represents a call into |
      |                        | instrumentation code.              |
      +------------------------+------------------------------------+
      
   .. cpp:function:: void *getFP()
      
      Return the frame pointer for the stack frame.
      
   .. cpp:function:: void *getPC()
      
      Returns the program counter associated with the stack frame.
      
   .. cpp:function:: BPatch_function *findFunction()
      
      Returns the function associated with the stack frame.
      
   .. cpp:function:: BPatch_thread *getThread()
      
      Returns the thread associated with the stack frame.
      
   .. cpp:function:: BPatch_point *getPoint()
      
   .. cpp:function:: BPatch_point *findPoint()
      
      For stack frames corresponding to inserted instrumentation, returns the
      instrumentation point where that instrumentation was inserted. For other
      frames, returns NULL.
      
   .. cpp:function:: bool isSynthesized()
      
      Returns true if this frame was artificially created, false otherwise.
      