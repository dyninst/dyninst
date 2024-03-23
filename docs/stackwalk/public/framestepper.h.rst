.. _`sec:framestepper.h`:

framestepper.h
##############

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:enum:: gcframe_ret_t

  .. cpp:enumerator:: gcf_success
  .. cpp:enumerator:: gcf_stackbottom
  .. cpp:enumerator:: gcf_not_me
  .. cpp:enumerator:: gcf_error


.. cpp:class:: FrameStepper

  **Controls how to walk through a specific type of stack frame**

  There may be many different ways of walking through a stack frame on a platform, e.g, on
  Linux/x86 there are different mechanisms for walking through system calls, signal handlers,
  regular functions, and frameless functions.

  In addition to walking through individual stack frames, a stepper registers itself with a
  :cpp:class:`StepperGroup` to indicate when it can be used. The stepper registers the
  address ranges that cover objects in the target process' code space (such as functions).
  These address ranges contain the objects that will create stack frames through which
  the stepper can walk. If multiple steppers have overlapping address ranges, then a priority
  value is used to determine which stepper should be attempted first.

  .. cpp:member:: static const unsigned stackbottom_priority = 0x10000
  .. cpp:member:: static const unsigned dyninstr_priority = 0x10010
  .. cpp:member:: static const unsigned sighandler_priority = 0x10020
  .. cpp:member:: static const unsigned analysis_priority = 0x10058
  .. cpp:member:: static const unsigned debugstepper_priority = 0x10040
  .. cpp:member:: static const unsigned frame_priority = 0x10050
  .. cpp:member:: static const unsigned wanderer_priority = 0x10060

  .. cpp:function:: FrameStepper(Walker *w)

      Creates a stepper associated with the walker ``w``.

  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out) = 0

      Returns in ``out`` the frame that contains the call to the frame ``in``.

      Returns :cpp:enumerator:`gcframe_ret_t::gcf_stackbottom` if the bottom of the stack
      is reached. Returns :cpp:enumerator:`gcframe_ret_t::gcf_not_me` if the frame is
      not the correct type stepper and attempts to locate another stepper to handle
      ``in`` or abort the stackwalk. Returns :cpp:enumerator:`gcframe_ret_t::gcf_error`
      if there was an error and the stack walk should be aborted. Returns
      :cpp:enumerator:`gcframe_ret_t::gcf_success` on success.

  .. cpp:function:: virtual unsigned getPriority() const = 0

      Returns the priority of this stepper, the lower the number the higher the priority.

      This is used by the ``StepperGroup`` to decide which stepper to use if multiple
      have been registered for the same address range.

      .. important:: 

        If two ``FrameStepper`` objects have an overlapping address range and the same
        priority, then the order in which they are used is undefined.

  .. cpp:function:: virtual ProcessState *getProcessState()

      Return the ``ProcessState`` used by the ``FrameStepper``. Can be
      overridden if the user desires.

  .. cpp:function:: virtual Walker *getWalker()

      Return the ``Walker`` associated with the ``FrameStepper``. Can be
      overridden if the user desires.

  .. cpp:function:: virtual void newLibraryNotification(LibAddrPair *libaddr, lib_change_t change)

      This function is called when a new library is loaded by the process; it
      should be implemented if the ``FrameStepper`` requires such information.

  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)

  .. cpp:function:: virtual const char *getName() const = 0

      Returns a name for the ``FrameStepper``; must be implemented by the
      user.


.. cpp:class:: FrameFuncHelper

  .. cpp:type:: std::pair<frame_type, frame_state> alloc_frame_t

  .. cpp:function:: FrameFuncHelper(ProcessState *proc_)

  .. cpp:function:: virtual alloc_frame_t allocatesFrame(Address addr) = 0

      Walks through a single stack frame and generates the caller’s stack frame.

      Returns a description of the :cpp:enum:`frame_type` and :cpp:enum:`frame_state`
      of the function at ``addr`` when execution reached there.

      If ``addr`` is invalid or an error occurs, returns ``unknown_t`` and
      ``unknown_s``, respectively.

  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *steppergroup)

      Notifies a :cpp:class:`FrameStepper` when it is added to ``steppergroup``.

      This can be used to initialize the ``FrameStepper``.

  .. cpp:function:: virtual unsigned getPriority() const = 0


.. cpp:enum:: FrameFuncHelper::frame_type

  .. cpp:enumerator:: unknown_t
  .. cpp:enumerator:: no_frame
  .. cpp:enumerator:: standard_frame
  .. cpp:enumerator:: savefp_only_frame


.. cpp:enum:: FrameFuncHelper::frame_state

  **Determines the current state of function with a stack frame at some point of execution**

  A function may set up a standard stack frame and have a :cpp:enum:`frame_type` of
  :cpp:enumerator:`frame_type::standard_frame`, but execution may be at the first instruction in the
  function and the frame is not yet set up, in which case the ``frame_state`` will be
  :cpp:enumerator:`unset_frame`.

  If the function sets up a standard stack frame and the execution point is someplace where
  the frame is completely setup, then the ``frame_state`` should be :cpp:enumerator:`set_frame`. If the
  function sets up a standard frame and the execution point is at a point where the frame
  does not yet exist or has been torn down, then ``frame_state`` should be :cpp:enumerator:`unset_frame`.
  :cpp:enumerator:`halfset_frame` is currently only meaningful on the x86 family of architecture, and should
  be used if the function has saved the old frame pointer, but not yet set up a new frame pointer.

  .. cpp:enumerator:: unknown_s
  .. cpp:enumerator:: unset_frame
  .. cpp:enumerator:: halfset_frame
  .. cpp:enumerator:: set_frame


.. cpp:class:: FrameFuncStepper : public FrameStepper

  *Walks a stack through an architecture-standard call frame**

  For example, on x86 this will be used to walk through stack frames that are
  set up with a ``push %ebp/mov %esp,%ebp`` prologue.

  .. cpp:function:: FrameFuncStepper(Walker *w, FrameFuncHelper *helper = NULL)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual const char *getName() const


.. cpp:class:: DebugStepper : public FrameStepper

  **Walks a stack using debug information**

  It depends on :ref:`sec:symtab-intro` to read debug information from a
  binary, then uses that debug information to walk through a call frame.

  Most binaries must be built with debug information in order to include debug
  information used here. Some languages, such as C++, automatically include
  stackwalking debug information for use by exceptions. This walker can also make
  use of this kind of exception information.

  .. cpp:function:: DebugStepper(Walker *w)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual const char *getName() const


.. cpp:class:: WandererHelper

  .. cpp:function:: WandererHelper(ProcessState *proc_)
  .. cpp:function:: virtual bool isPrevInstrACall(Address addr, Address &target)
  .. cpp:function:: virtual pc_state isPCInFunc(Address func_entry, Address pc)
  .. cpp:function:: virtual bool requireExactMatch()


.. cpp:enum:: WandererHelper::pc_state

  .. cpp:enumerator:: unknown_s
  .. cpp:enumerator:: in_func
  .. cpp:enumerator:: outside_func


.. cpp:class:: StepperWanderer : public FrameStepper

  **Walks a stack using a heuristic approach**

  Heuristics are used to find possible return addresses in the stack frame. If a return address
  is found that matches a valid caller of the current function, it is assumed to be the actual return
  address and a matching stack frame is created. Since this approach is heuristic, it can make mistakes
  leading to incorrect stack information.

  .. cpp:function:: StepperWanderer(Walker *w, WandererHelper *whelper = NULL, FrameFuncHelper *fhelper = NULL)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual const char *getName() const


.. cpp:class:: SigHandlerStepper : public FrameStepper

  **Walks through UNIX signal handlers found on the call stack**

  On some systems a signal handler generates a special kind of stack frame that cannot be walked
  through using normal stack walking techniques.

  .. cpp:function:: SigHandlerStepper(Walker *w)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void newLibraryNotification(LibAddrPair *la, lib_change_t change)
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual const char *getName() const


.. cpp:class:: BottomOfStackStepper : public FrameStepper

  **Detect if the bottom of the call stack has been reached**

  This doesn't walk through any type of call frame. When the bottom of the stack
  is found, it reports :cpp:enumerator:`gcframe_ret_t::gcf_stackbottom` from its
  ``getCallerFrame`` method. Otherwise it will report :cpp:enumerator:`gcframe_ret_t::gcf_not_me`.

  This stepper runs with a higher priority than any other stepper.

  .. cpp:function:: BottomOfStackStepper(Walker *w)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void newLibraryNotification(LibAddrPair *la, lib_change_t change)
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual const char *getName() const


.. cpp:class:: DyninstInstrStepper : public FrameStepper

  .. cpp:function:: DyninstInstrStepper(Walker *w)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual ~DyninstInstrStepper()
  .. cpp:function:: virtual const char *getName() const


.. cpp:class:: AnalysisStepper : public FrameStepper

  **Walks a stack using dataflow analysis**

  Dataflow is used to determine possible stack sizes at all locations in a function as
  well as the location of the frame pointer. It is able to handle optimized code with
  omitted frame pointers and overlapping code sequences.

  .. cpp:function:: AnalysisStepper(Walker *w)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual const char *getName() const


.. cpp:class:: DyninstDynamicHelper

  .. cpp:function:: virtual bool isInstrumentation(Address ra, Address *orig_ra, unsigned *stack_height, bool *aligned, bool *entryExit) = 0
  .. cpp:function:: virtual ~DyninstDynamicHelper()


.. cpp:class:: DyninstDynamicStepper : public FrameStepper

  .. cpp:function:: DyninstDynamicStepper(Walker *w, DyninstDynamicHelper *dihelper = NULL)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual ~DyninstDynamicStepper()
  .. cpp:function:: virtual const char *getName() const


.. cpp:class:: DyninstInstFrameStepper : public FrameStepper

  .. cpp:function:: DyninstInstFrameStepper(Walker *w)
  .. cpp:function:: virtual gcframe_ret_t getCallerFrame(const Frame &in, Frame &out)
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual void registerStepperGroup(StepperGroup *group)
  .. cpp:function:: virtual ~DyninstInstFrameStepper()
  .. cpp:function:: virtual const char *getName() const


Notes
*****

There may be multiple ways of walking through a different types of stack
frames. Each ``FrameStepper`` class should be able to walk through a
type of stack frame. For example, on x86 one ``FrameStepper`` could be
used to walk through stack frames generated by ABI-compliant functions;
out’s FP and RA are found by reading from in’s FP, and out’s SP is set
to the word below in’s FP. A different ``FrameStepper`` might be used to
walk through stack frames created by functions that have optimized away
their FP. In this case, in may have a FP that does not point out’s FP
and RA. The ``FrameStepper`` will need to use other mechanisms to
discover out’s FP or RA; perhaps the ``FrameStepper`` searches through
the stack for the RA or performs analysis on the function that created
the stack frame.

