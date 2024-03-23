.. _`sec:pcEventHandler.h`:

pcEventHandler.h
################

The entry point for event and callback handling.

1:1 class with PCProcess that encapsulates all event handling, including waiting for events and callbacks.


.. cpp:class:: PCEventHandler

  .. cpp:type:: private Dyninst::ProcControlAPI::Event::const_ptr EventPtr
  .. cpp:function:: static PCEventHandler &handler()
  .. cpp:function:: static bool handle(EventPtr ev)
  .. cpp:function:: protected PCEventHandler()
  .. cpp:function:: protected bool handle_internal(EventPtr ev)
  .. cpp:function:: protected bool handleExit(Dyninst::ProcControlAPI::EventExit::const_ptr ev, PCProcess *evProc) const
  .. cpp:function:: protected bool handleFork(Dyninst::ProcControlAPI::EventFork::const_ptr ev, PCProcess *evProc) const
  .. cpp:function:: protected bool handleExec(Dyninst::ProcControlAPI::EventExec::const_ptr ev, PCProcess *&evProc) const
  .. cpp:function:: protected bool handleCrash(Dyninst::ProcControlAPI::EventCrash::const_ptr ev, PCProcess *evProc) const
  .. cpp:function:: protected bool handleForceTerminate(Dyninst::ProcControlAPI::EventForceTerminate::const_ptr ev, PCProcess *evProc) const
  .. cpp:function:: protected bool handleThreadCreate(Dyninst::ProcControlAPI::EventNewThread::const_ptr ev, PCProcess *evProc) const
  .. cpp:function:: protected bool handleThreadDestroy(Dyninst::ProcControlAPI::EventThreadDestroy::const_ptr ev, PCProcess *evProc) const
  .. cpp:function:: protected bool handleSignal(Dyninst::ProcControlAPI::EventSignal::const_ptr ev, PCProcess *evProc) const
  .. cpp:function:: protected bool handleLibrary(Dyninst::ProcControlAPI::EventLibrary::const_ptr ev, PCProcess *evProc) const
  .. cpp:function:: protected bool handleBreakpoint(Dyninst::ProcControlAPI::EventBreakpoint::const_ptr ev, PCProcess *evProc) const
  .. cpp:function:: protected bool handleRPC(Dyninst::ProcControlAPI::EventRPC::const_ptr ev, PCProcess *evProc) const
  .. cpp:function:: protected bool handleRTBreakpoint(Dyninst::ProcControlAPI::EventBreakpoint::const_ptr ev, PCProcess *evProc) const
  .. cpp:function:: protected bool handleStopThread(PCProcess *evProc, Dyninst::Address rt_arg) const
  .. cpp:function:: protected bool handleUserMessage(PCProcess *evProc, BPatch_process *bpProc, Dyninst::Address rt_arg) const
  .. cpp:function:: protected bool handleDynFuncCall(PCProcess *evProc, BPatch_process *bpProc, Dyninst::Address rt_arg) const

  ......

  .. rubric::
    platform-specific

  .. cpp:function:: protected static bool shouldStopForSignal(int signal)
  .. cpp:function:: protected static bool isValidRTSignal(int signal, RTBreakpointVal breakpointVal, Dyninst::Address arg1, int status)
  .. cpp:function:: protected static bool isCrashSignal(int signal)
  .. cpp:function:: protected static bool isKillSignal(int signal)
  .. cpp:member:: protected static PCEventHandler handler_


.. cpp:enum:: PCEventHandler::RTBreakpointVal

  .. cpp:enumerator:: NoRTBreakpoint
  .. cpp:enumerator:: NormalRTBreakpoint
  .. cpp:enumerator:: SoftRTBreakpoint



Notes
*****

Why syscallNotification is a friend:

It is a friend because it reaches in to determine whether to install breakpoints at specific system calls. I
didn't want to expose this to the rest of Dyninst.
