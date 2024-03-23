.. _`sec:Event.h`:

Event.h
#######

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:class:: Event : public boost::enable_shared_from_this<Event>

  **An action/response within ProcControlAPI**

  Each Event has an :cpp:class:`EventType` that describes the event. It also has
  pointers to the :cpp:class:`Process` and :cpp:class:`Thread` that the event
  occurred on. See :ref:`sec:proccontrol-intro-callback-events` for an overview.

  .. cpp:type:: boost::shared_ptr<Event> ptr
  .. cpp:type:: boost::shared_ptr<const Event> const_ptr

  .. cpp:function:: Event(EventType etype_, Thread::ptr thread_ = Thread::ptr())

  .. cpp:function:: virtual ~Event()

  .. cpp:function:: boost::shared_ptr<EventTerminate> getEventTerminate()

      Casts this event to an :cpp:class:`EventTerminate`.

      Returns ``nullptr`` if this event is not convertible to a ``EventTerminate``.

  .. cpp:function:: boost::shared_ptr<const EventTerminate> getEventTerminate() const

      Casts this event to an :cpp:class:`EventTerminate`.

      Returns ``nullptr`` if this event is not convertible to a ``EventTerminate``.

  .. cpp:function:: boost::shared_ptr<EventExit> getEventExit()

      Casts this event to an :cpp:class:`EventExit`.

      Returns ``nullptr`` if this event is not convertible to a ``EventExit``.

  .. cpp:function:: boost::shared_ptr<const EventExit> getEventExit() const

      Casts this event to an :cpp:class:`EventExit`.

      Returns ``nullptr`` if this event is not convertible to a ``EventExit``.

  .. cpp:function:: boost::shared_ptr<EventCrash> getEventCrash()

      Casts this event to an :cpp:class:`EventCrash`.

      Returns ``nullptr`` if this event is not convertible to a ``EventCrash``.

  .. cpp:function:: boost::shared_ptr<const EventCrash> getEventCrash() const

      Casts this event to an :cpp:class:`EventCrash`.

      Returns ``nullptr`` if this event is not convertible to a ``EventCrash``.

  .. cpp:function:: boost::shared_ptr<EventForceTerminate> getEventForceTerminate()

      Casts this event to an :cpp:class:`EventForceTerminate`.

      Returns ``nullptr`` if this event is not convertible to a ``EventForceTerminate``.

  .. cpp:function:: boost::shared_ptr<const EventForceTerminate> getEventForceTerminate() const

      Casts this event to an :cpp:class:`EventForceTerminate`.

      Returns ``nullptr`` if this event is not convertible to a ``EventForceTerminate``.

  .. cpp:function:: boost::shared_ptr<EventExec> getEventExec()

      Casts this event to an :cpp:class:`EventExec`.

      Returns ``nullptr`` if this event is not convertible to a ``EventExec``.

  .. cpp:function:: boost::shared_ptr<const EventExec> getEventExec() const

      Casts this event to an :cpp:class:`EventExec`.

      Returns ``nullptr`` if this event is not convertible to a ``EventExec``.

  .. cpp:function:: boost::shared_ptr<EventStop> getEventStop()

      Casts this event to an :cpp:class:`EventStop`.

      Returns ``nullptr`` if this event is not convertible to a ``EventStop``.

  .. cpp:function:: boost::shared_ptr<const EventStop> getEventStop() const

      Casts this event to an :cpp:class:`EventStop`.

      Returns ``nullptr`` if this event is not convertible to a ``EventStop``.

  .. cpp:function:: boost::shared_ptr<EventBreakpoint> getEventBreakpoint()

      Casts this event to an :cpp:class:`EventBreakpoint`.

      Returns ``nullptr`` if this event is not convertible to a ``EventBreakpoint``.

  .. cpp:function:: boost::shared_ptr<const EventBreakpoint> getEventBreakpoint() const

      Casts this event to an :cpp:class:`EventBreakpoint`.

      Returns ``nullptr`` if this event is not convertible to a ``EventBreakpoint``.

  .. cpp:function:: boost::shared_ptr<EventNewThread> getEventNewThread()

      Casts this event to an :cpp:class:`EventNewThread`.

      Returns ``nullptr`` if this event is not convertible to a ``EventNewThread``.

  .. cpp:function:: boost::shared_ptr<const EventNewThread> getEventNewThread() const

      Casts this event to an :cpp:class:`EventNewThread`.

      Returns ``nullptr`` if this event is not convertible to a ``EventNewThread``.

  .. cpp:function:: boost::shared_ptr<EventNewUserThread> getEventNewUserThread()

      Casts this event to an :cpp:class:`EventNewUserThread`.

      Returns ``nullptr`` if this event is not convertible to a ``EventNewUserThread``.

  .. cpp:function:: boost::shared_ptr<const EventNewUserThread> getEventNewUserThread() const

      Casts this event to an :cpp:class:`EventNewUserThread`.

      Returns ``nullptr`` if this event is not convertible to a ``EventNewUserThread``.

  .. cpp:function:: boost::shared_ptr<EventNewLWP> getEventNewLWP()

      Casts this event to an :cpp:class:`EventNewLWP`.

      Returns ``nullptr`` if this event is not convertible to a ``EventNewLWP``.

  .. cpp:function:: boost::shared_ptr<const EventNewLWP> getEventNewLWP() const

      Casts this event to an :cpp:class:`EventNewLWP`.

      Returns ``nullptr`` if this event is not convertible to a ``EventNewLWP``.

  .. cpp:function:: boost::shared_ptr<EventThreadDestroy> getEventThreadDestroy()

      Casts this event to an :cpp:class:`EventThreadDestroy`.

      Returns ``nullptr`` if this event is not convertible to a ``EventThreadDestroy``.

  .. cpp:function:: boost::shared_ptr<const EventThreadDestroy> getEventThreadDestroy() const

      Casts this event to an :cpp:class:`EventThreadDestroy`.

      Returns ``nullptr`` if this event is not convertible to a ``EventThreadDestroy``.

  .. cpp:function:: boost::shared_ptr<EventUserThreadDestroy> getEventUserThreadDestroy()

      Casts this event to an :cpp:class:`EventUserThreadDestroy`.

      Returns ``nullptr`` if this event is not convertible to a ``EventUserThreadDestroy``.

  .. cpp:function:: boost::shared_ptr<const EventUserThreadDestroy> getEventUserThreadDestroy() const

      Casts this event to an :cpp:class:`EventUserThreadDestroy`.

      Returns ``nullptr`` if this event is not convertible to a ``EventUserThreadDestroy``.

  .. cpp:function:: boost::shared_ptr<EventLWPDestroy> getEventLWPDestroy()

      Casts this event to an :cpp:class:`EventLWPDestroy`.

      Returns ``nullptr`` if this event is not convertible to a ``EventLWPDestroy``.

  .. cpp:function:: boost::shared_ptr<const EventLWPDestroy> getEventLWPDestroy() const

      Casts this event to an :cpp:class:`EventLWPDestroy`.

      Returns ``nullptr`` if this event is not convertible to a ``EventLWPDestroy``.

  .. cpp:function:: boost::shared_ptr<EventFork> getEventFork()

      Casts this event to an :cpp:class:`EventFork`.

      Returns ``nullptr`` if this event is not convertible to a ``EventFork``.

  .. cpp:function:: boost::shared_ptr<const EventFork> getEventFork() const

      Casts this event to an :cpp:class:`EventFork`.

      Returns ``nullptr`` if this event is not convertible to a ``EventFork``.

  .. cpp:function:: boost::shared_ptr<EventSignal> getEventSignal()

      Casts this event to an :cpp:class:`EventSignal`.

      Returns ``nullptr`` if this event is not convertible to a ``EventSignal``.

  .. cpp:function:: boost::shared_ptr<const EventSignal> getEventSignal() const

      Casts this event to an :cpp:class:`EventSignal`.

      Returns ``nullptr`` if this event is not convertible to a ``EventSignal``.

  .. cpp:function:: boost::shared_ptr<EventBootstrap> getEventBootstrap()

      Casts this event to an :cpp:class:`EventBootstrap`.

      Returns ``nullptr`` if this event is not convertible to a ``EventBootstrap``.

  .. cpp:function:: boost::shared_ptr<const EventBootstrap> getEventBootstrap() const

      Casts this event to an :cpp:class:`EventBootstrap`.

      Returns ``nullptr`` if this event is not convertible to a ``EventBootstrap``.

  .. cpp:function:: boost::shared_ptr<EventPreBootstrap> getEventPreBootstrap()

      Casts this event to an :cpp:class:`EventPreBootstrap`.

      Returns ``nullptr`` if this event is not convertible to a ``EventPreBootstrap``.

  .. cpp:function:: boost::shared_ptr<const EventPreBootstrap> getEventPreBootstrap() const

      Casts this event to an :cpp:class:`EventPreBootstrap`.

      Returns ``nullptr`` if this event is not convertible to a ``EventPreBootstrap``.

  .. cpp:function:: boost::shared_ptr<EventRPC> getEventRPC()

      Casts this event to an :cpp:class:`EventRPC`.

      Returns ``nullptr`` if this event is not convertible to a ``EventRPC``.

  .. cpp:function:: boost::shared_ptr<const EventRPC> getEventRPC() const

      Casts this event to an :cpp:class:`EventRPC`.

      Returns ``nullptr`` if this event is not convertible to a ``EventRPC``.

  .. cpp:function:: boost::shared_ptr<EventRPCLaunch> getEventRPCLaunch()

      Casts this event to an :cpp:class:`EventRPCLaunch`.

      Returns ``nullptr`` if this event is not convertible to a ``EventRPCLaunch``.

  .. cpp:function:: boost::shared_ptr<const EventRPCLaunch> getEventRPCLaunch() const

      Casts this event to an :cpp:class:`EventRPCLaunch`.

      Returns ``nullptr`` if this event is not convertible to a ``EventRPCLaunch``.

  .. cpp:function:: boost::shared_ptr<EventSingleStep> getEventSingleStep()

      Casts this event to an :cpp:class:`EventSingleStep`.

      Returns ``nullptr`` if this event is not convertible to a ``EventSingleStep``.

  .. cpp:function:: boost::shared_ptr<const EventSingleStep> getEventSingleStep() const

      Casts this event to an :cpp:class:`EventSingleStep`.

      Returns ``nullptr`` if this event is not convertible to a ``EventSingleStep``.

  .. cpp:function:: boost::shared_ptr<EventBreakpointClear> getEventBreakpointClear()

      Casts this event to an :cpp:class:`EventBreakpointClear`.

      Returns ``nullptr`` if this event is not convertible to a ``EventBreakpointClear``.

  .. cpp:function:: boost::shared_ptr<const EventBreakpointClear> getEventBreakpointClear() const

      Casts this event to an :cpp:class:`EventBreakpointClear`.

      Returns ``nullptr`` if this event is not convertible to a ``EventBreakpointClear``.

  .. cpp:function:: boost::shared_ptr<EventBreakpointRestore> getEventBreakpointRestore()

      Casts this event to an :cpp:class:`EventBreakpointRestore`.

      Returns ``nullptr`` if this event is not convertible to a ``EventBreakpointRestore``.

  .. cpp:function:: boost::shared_ptr<const EventBreakpointRestore> getEventBreakpointRestore() const

      Casts this event to an :cpp:class:`EventBreakpointRestore`.

      Returns ``nullptr`` if this event is not convertible to a ``EventBreakpointRestore``.

  .. cpp:function:: boost::shared_ptr<EventLibrary> getEventLibrary()

      Casts this event to an :cpp:class:`EventLibrary`.

      Returns ``nullptr`` if this event is not convertible to a ``EventLibrary``.

  .. cpp:function:: boost::shared_ptr<const EventLibrary> getEventLibrary() const

      Casts this event to an :cpp:class:`EventLibrary`.

      Returns ``nullptr`` if this event is not convertible to a ``EventLibrary``.

  .. cpp:function:: boost::shared_ptr<EventAsync> getEventAsync()

      Casts this event to an :cpp:class:`EventAsync`.

      Returns ``nullptr`` if this event is not convertible to a ``EventAsync``.

  .. cpp:function:: boost::shared_ptr<const EventAsync> getEventAsync() const

      Casts this event to an :cpp:class:`EventAsync`.

      Returns ``nullptr`` if this event is not convertible to a ``EventAsync``.

  .. cpp:function:: boost::shared_ptr<EventChangePCStop> getEventChangePCStop()

      Casts this event to an :cpp:class:`EventChangePCStop`.

      Returns ``nullptr`` if this event is not convertible to a ``EventChangePCStop``.

  .. cpp:function:: boost::shared_ptr<const EventChangePCStop> getEventChangePCStop() const

      Casts this event to an :cpp:class:`EventChangePCStop`.

      Returns ``nullptr`` if this event is not convertible to a ``EventChangePCStop``.

  .. cpp:function:: boost::shared_ptr<EventDetach> getEventDetach()

      Casts this event to an :cpp:class:`EventDetach`.

      Returns ``nullptr`` if this event is not convertible to a ``EventDetach``.

  .. cpp:function:: boost::shared_ptr<const EventDetach> getEventDetach() const

      Casts this event to an :cpp:class:`EventDetach`.

      Returns ``nullptr`` if this event is not convertible to a ``EventDetach``.

  .. cpp:function:: boost::shared_ptr<EventNop> getEventNop()

      Casts this event to an :cpp:class:`EventNop`.

      Returns ``nullptr`` if this event is not convertible to a ``EventNop``.

  .. cpp:function:: boost::shared_ptr<const EventNop> getEventNop() const

      Casts this event to an :cpp:class:`EventNop`.

      Returns ``nullptr`` if this event is not convertible to a ``EventNop``.

  .. cpp:function:: boost::shared_ptr<EventThreadDB> getEventThreadDB()

      Casts this event to an :cpp:class:`EventThreadDB`.

      Returns ``nullptr`` if this event is not convertible to a ``EventThreadDB``.

  .. cpp:function:: boost::shared_ptr<const EventThreadDB> getEventThreadDB() const

      Casts this event to an :cpp:class:`EventThreadDB`.

      Returns ``nullptr`` if this event is not convertible to a ``EventThreadDB``.

  .. cpp:function:: boost::shared_ptr<EventWinStopThreadDestroy> getEventWinStopThreadDestroy()

      Casts this event to an :cpp:class:`EventWinStopThreadDestroy`.

      Returns ``nullptr`` if this event is not convertible to a ``EventWinStopThreadDestroy``.

  .. cpp:function:: boost::shared_ptr<const EventWinStopThreadDestroy> getEventWinStopThreadDestroy() const

      Casts this event to an :cpp:class:`EventWinStopThreadDestroy`.

      Returns ``nullptr`` if this event is not convertible to a ``EventWinStopThreadDestroy``.

  .. cpp:function:: boost::shared_ptr<EventControlAuthority> getEventControlAuthority()

      Casts this event to an :cpp:class:`EventControlAuthority`.

      Returns ``nullptr`` if this event is not convertible to a ``EventControlAuthority``.

  .. cpp:function:: boost::shared_ptr<const EventControlAuthority> getEventControlAuthority() const

      Casts this event to an :cpp:class:`EventControlAuthority`.

      Returns ``nullptr`` if this event is not convertible to a ``EventControlAuthority``.

  .. cpp:function:: boost::shared_ptr<EventAsyncIO> getEventAsyncIO()

      Casts this event to an :cpp:class:`EventAsyncIO`.

      Returns ``nullptr`` if this event is not convertible to a ``EventAsyncIO``.

  .. cpp:function:: boost::shared_ptr<const EventAsyncIO> getEventAsyncIO() const

      Casts this event to an :cpp:class:`EventAsyncIO`.

      Returns ``nullptr`` if this event is not convertible to a ``EventAsyncIO``.

  .. cpp:function:: boost::shared_ptr<EventAsyncRead> getEventAsyncRead()

      Casts this event to an :cpp:class:`EventAsyncRead`.

      Returns ``nullptr`` if this event is not convertible to a ``EventAsyncRead``.

  .. cpp:function:: boost::shared_ptr<const EventAsyncRead> getEventAsyncRead() const

      Casts this event to an :cpp:class:`EventAsyncRead`.

      Returns ``nullptr`` if this event is not convertible to a ``EventAsyncRead``.

  .. cpp:function:: boost::shared_ptr<EventAsyncWrite> getEventAsyncWrite()

      Casts this event to an :cpp:class:`EventAsyncWrite`.

      Returns ``nullptr`` if this event is not convertible to a ``EventAsyncWrite``.

  .. cpp:function:: boost::shared_ptr<const EventAsyncWrite> getEventAsyncWrite() const

      Casts this event to an :cpp:class:`EventAsyncWrite`.

      Returns ``nullptr`` if this event is not convertible to a ``EventAsyncWrite``.

  .. cpp:function:: boost::shared_ptr<EventAsyncReadAllRegs> getEventAsyncReadAllRegs()

      Casts this event to an :cpp:class:`EventAsyncReadAllRegs`.

      Returns ``nullptr`` if this event is not convertible to a ``EventAsyncReadAllRegs``.

  .. cpp:function:: boost::shared_ptr<const EventAsyncReadAllRegs> getEventAsyncReadAllRegs() const

      Casts this event to an :cpp:class:`EventAsyncReadAllRegs`.

      Returns ``nullptr`` if this event is not convertible to a ``EventAsyncReadAllRegs``.

  .. cpp:function:: boost::shared_ptr<EventAsyncSetAllRegs> getEventAsyncSetAllRegs()

      Casts this event to an :cpp:class:`EventAsyncSetAllRegs`.

      Returns ``nullptr`` if this event is not convertible to a ``EventAsyncSetAllRegs``.

  .. cpp:function:: boost::shared_ptr<const EventAsyncSetAllRegs> getEventAsyncSetAllRegs() const

      Casts this event to an :cpp:class:`EventAsyncSetAllRegs`.

      Returns ``nullptr`` if this event is not convertible to a ``EventAsyncSetAllRegs``.

  .. cpp:function:: boost::shared_ptr<EventAsyncFileRead> getEventAsyncFileRead()

      Casts this event to an :cpp:class:`EventAsyncFileRead`.

      Returns ``nullptr`` if this event is not convertible to a ``EventAsyncFileRead``.

  .. cpp:function:: boost::shared_ptr<const EventAsyncFileRead> getEventAsyncFileRead() const

      Casts this event to an :cpp:class:`EventAsyncFileRead`.

      Returns ``nullptr`` if this event is not convertible to a ``EventAsyncFileRead``.

  .. cpp:function:: boost::shared_ptr<EventPostponedSyscall> getEventPostponedSyscall()

      Casts this event to an :cpp:class:`EventPostponedSyscall`.

      Returns ``nullptr`` if this event is not convertible to a ``EventPostponedSyscall``.

  .. cpp:function:: boost::shared_ptr<const EventPostponedSyscall> getEventPostponedSyscall() const

      Casts this event to an :cpp:class:`EventPostponedSyscall`.

      Returns ``nullptr`` if this event is not convertible to a ``EventPostponedSyscall``.

  .. cpp:function:: boost::shared_ptr<EventSyscall> getEventSyscall()

      Casts this event to an :cpp:class:`EventSyscall`.

      Returns ``nullptr`` if this event is not convertible to a ``EventSyscall``.

  .. cpp:function:: boost::shared_ptr<const EventSyscall> getEventSyscall() const

      Casts this event to an :cpp:class:`EventSyscall`.

      Returns ``nullptr`` if this event is not convertible to a ``EventSyscall``.

  .. cpp:function:: boost::shared_ptr<EventPreSyscall> getEventPreSyscall()

      Casts this event to an :cpp:class:`EventPreSyscall`.

      Returns ``nullptr`` if this event is not convertible to a ``EventPreSyscall``.

  .. cpp:function:: boost::shared_ptr<const EventPreSyscall> getEventPreSyscall() const

      Casts this event to an :cpp:class:`EventPreSyscall`.

      Returns ``nullptr`` if this event is not convertible to a ``EventPreSyscall``.

  .. cpp:function:: boost::shared_ptr<EventPostSyscall> getEventPostSyscall()

      Casts this event to an :cpp:class:`EventPostSyscall`.

      Returns ``nullptr`` if this event is not convertible to a ``EventPostSyscall``.

  .. cpp:function:: boost::shared_ptr<const EventPostSyscall> getEventPostSyscall() const

      Casts this event to an :cpp:class:`EventPostSyscall`.

      Returns ``nullptr`` if this event is not convertible to a ``EventPostSyscall``.

.. cpp:function:: template<typename OS> OS& operator<<(OS& str, Event& e)

.. cpp:enum:: Event::SyncType

  **Describes how a process or thread is stopped by an Event**

  .. cpp:enumerator:: unset
  .. cpp:enumerator:: async
  .. cpp:enumerator:: sync_thread
  .. cpp:enumerator:: sync_process


.. cpp:class:: ArchEvent

  .. cpp:function:: ArchEvent(std::string name_ = std::string(""))
  .. cpp:function:: std::string getName()


.. cpp:class:: EventTerminate : public Event

  **Parent class for EventExit and EventCrash**

  .. cpp:type:: boost::shared_ptr<EventTerminate> ptr
  .. cpp:type:: boost::shared_ptr<const EventTerminate> const_ptr

  .. cpp:function:: EventTerminate(EventType type_)
  .. cpp:function:: virtual ~EventTerminate()


.. cpp:class:: EventExit : public EventTerminate

  An EventExit triggers when a process performs a normal exit (e.g.,
  calling the exit function or returning from main). The process that
  exited can be retrieved via :cpp:func:`Event::getProcess`.

  An EventExit is a pre-exit or post-exit event. Pre-exit means the process
  has not yet cleaned up its address space, and thus memory can still be read
  or written. Post-exit means the process has cleaned up its address space,
  memory is no longer accessible.

  .. cpp:type:: boost::shared_ptr<EventExit> ptr
  .. cpp:type:: boost::shared_ptr<const EventExit> const_ptr

  .. cpp:function:: EventExit(EventType::Time eventtime, int exitcode_)
  .. cpp:function:: virtual ~EventExit()

  .. cpp:function:: int getExitCode() const

      Returns the process’ exit code.


.. cpp:class:: EventCrash : public EventTerminate

  An EventCrash triggers when a process performs an abnormal exit (e.g.,
  crashing on a memory violation). The process that exited can be retrieved
  via :cpp:func:`Event::getProcess`.

  An EventCrash is a pre-crash or post-crash event. Pre-crash means the process
  has not yet cleaned up its address space, and thus memory can still be read or
  written. Post-crash means the process has cleaned up its address space, memory
  is no longer accessible.

  .. cpp:type:: boost::shared_ptr<EventCrash> ptr
  .. cpp:type:: boost::shared_ptr<const EventCrash> const_ptr

  .. cpp:function:: EventCrash(int termsig)
  .. cpp:function:: virtual ~EventCrash()

  .. cpp:function:: int getTermSignal() const

      Returns the signal that caused the process to crash.


.. cpp:class:: EventForceTerminate : public EventTerminate

  An EventForceTerminate triggers when a process is forcefully terminated
  via the Process::terminate function. When the callback is delivered for
  this event, the address space of the corresponding process will no
  longer be available.

  .. cpp:type:: boost::shared_ptr<EventForceTerminate> ptr
  .. cpp:type:: boost::shared_ptr<const EventForceTerminate> const_ptr

  .. cpp:function:: EventForceTerminate(int termsig)
  .. cpp:function:: virtual ~EventForceTerminate()

  .. cpp:function:: int getTermSignal() const

      Returns the signal that was used to terminate the process.

.. cpp:class:: EventExec : public Event

  An EventExec triggers when a process performs a UNIX-style exec
  operation. An EventType of post-Exec means the process has completed the
  exec and setup its new address space. An EventType of pre-Exec means the
  process has not yet torn down its old address space.

  .. cpp:type:: boost::shared_ptr<EventExec> ptr
  .. cpp:type:: boost::shared_ptr<const EventExec> const_ptr

  .. cpp:function:: EventExec(EventType::Time etime_, std::string path = std::string(""))
  .. cpp:function:: virtual ~EventExec()

  .. cpp:function:: std::string getExecPath() const

      Returns the file path to the process’ new executable.


.. cpp:class:: EventStop : public Event

  An EventStop is triggered when a process is stopped by a
  non-ProcControlAPI source. On UNIX based systems, this is triggered by
  receipt of a SIGSTOP signal.

  Unlike most other events, an EventStop will explicitly move the
  associated thread or process (see the Event’s SyncType to tell which) to
  a stopped state. Returning cbDefault from a callback function that has
  received EventStop will leave the target process in a stopped state
  rather than restore it to the pre-event state.

  .. cpp:type:: boost::shared_ptr<EventStop> ptr
  .. cpp:type:: boost::shared_ptr<const EventStop> const_ptr

  .. cpp:function:: EventStop()
  .. cpp:function:: virtual ~EventStop()


.. cpp:class:: EventNewThread : public Event

  An EventNewThread triggers when a process spawns a new thread. The Event
  class’ getThread function returns the original Thread that performed the
  spawn operation, while EventNewThread’s getNewThread returns the newly
  created Thread.

  This event is never instantiated by ProcControlAPI and simply serves as
  a place-holder type for a user to deal with thread creation without
  having to deal with the specifics of LWP and user thread creation.

  A callback function that receives an EventNewThread can use the two
  field form of :cpp:class:`Process::cb_ret_t` to control the parent and child thread.

  .. cpp:type:: boost::shared_ptr<EventNewThread> ptr
  .. cpp:type:: boost::shared_ptr<const EventNewThread> const_ptr

  .. cpp:function:: EventNewThread(EventType et)
  .. cpp:function:: virtual ~EventNewThread()
  .. cpp:function:: virtual Dyninst::LWP getLWP() const = 0
  .. cpp:function:: virtual Thread::const_ptr getNewThread() const = 0


.. cpp:class:: EventThreadDestroy : public Event

  An EventThreadDestroy triggers when a thread exits. Event’s getThread
  member function returns the thread that exited.

  This event is never instantiated by ProcControlAPI and simply serves as
  a place-holder type for a user to deal with thread destruction without
  having to deal with the specifics of LWP and user thread destruction.

  .. cpp:type:: boost::shared_ptr<EventThreadDestroy> ptr
  .. cpp:type:: boost::shared_ptr<const EventThreadDestroy> const_ptr

  .. cpp:function:: EventThreadDestroy(EventType et)
  .. cpp:function:: virtual ~EventThreadDestroy() = 0

.. cpp:class:: EventUserThreadDestroy : public EventThreadDestroy

  An EventUserThreadDestroy triggers when a thread exits. Event’s
  getThread member function returns the thread that exited.

  If the platform also supports EventLWPDestroy events, this event will
  precede an EventLWPDestroy event.

  .. cpp:type:: boost::shared_ptr<EventUserThreadDestroy> ptr
  .. cpp:type:: boost::shared_ptr<const EventUserThreadDestroy> const_ptr

  .. cpp:function:: EventUserThreadDestroy(EventType::Time time_)
  .. cpp:function:: virtual ~EventUserThreadDestroy()

.. cpp:class:: EventLWPDestroy : public EventThreadDestroy

  An LWPThreadDestroy triggers when a thread exits. Event’s getThread
  member function returns the thread that exited.

  .. cpp:type:: boost::shared_ptr<EventLWPDestroy> ptr
  .. cpp:type:: boost::shared_ptr<const EventLWPDestroy> const_ptr

  .. cpp:function:: EventLWPDestroy(EventType::Time time_)
  .. cpp:function:: virtual ~EventLWPDestroy()

.. cpp:class:: EventFork : public Event

  An EventFork triggers when a process performs a UNIX-style fork
  operation. The process that performed the initial fork is returned via
  Event’s getProcess member function, while the newly created process can
  be found via EventFork’s getChildProcess member function.

  .. cpp:type:: boost::shared_ptr<EventFork> ptr
  .. cpp:type:: boost::shared_ptr<const EventFork> const_ptr

  .. cpp:function:: EventFork(EventType::Time time_, Dyninst::PID pid_)
  .. cpp:function:: virtual ~EventFork()
  .. cpp:function:: Dyninst::PID getPID() const
  .. cpp:function:: Process::const_ptr getChildProcess() const


.. cpp:class:: EventSingleStep : public Event

  An EventSingleStep triggers when a thread, which was put in single-step
  mode by :cpp:func:`Thread::setSingleStep`, completes a single step operation. The
  Thread will remain in single-step mode after completion of this event
  (presuming it has not be explicitly disabled by ``Thread::setSingleStep``).

  .. cpp:type:: boost::shared_ptr<EventSingleStep> ptr
  .. cpp:type:: boost::shared_ptr<const EventSingleStep> const_ptr

  .. cpp:function:: EventSingleStep()
  .. cpp:function:: virtual ~EventSingleStep()

.. cpp:class:: EventSyscall : public Event

  .. cpp:type:: boost::shared_ptr<EventSyscall> ptr
  .. cpp:type:: boost::shared_ptr<const EventSyscall> const_ptr

  .. cpp:function:: EventSyscall(EventType type_)
  .. cpp:function:: virtual ~EventSyscall()
  .. cpp:function:: Dyninst::Address getAddress() const
  .. cpp:function:: MachSyscall getSyscall() const


.. cpp:class:: EventPreSyscall : public EventSyscall

  An EventPreSyscall is triggered when a thread enters a system call,
  provided that the thread is in system call tracing mode. It is a
  child of EventSyscall, which provides all the relevant methods.

  .. cpp:type:: boost::shared_ptr<EventPreSyscall> ptr
  .. cpp:type:: boost::shared_ptr<const EventPreSyscall> const_ptr

  .. cpp:function:: EventPreSyscall()
  .. cpp:function:: virtual ~EventPreSyscall()

  .. cpp:function:: Dyninst::Address getAddress() const

      Returns the address where the system call occurred.

  .. cpp:function:: MachSyscall getSyscall() const

      Returns information about the system call.


.. cpp:class:: EventPostSyscall : public EventSyscall

  An EventPostSyscall is triggered when a system call returns. It is a
  child of EventSyscall, which provides all the relevant methods.

  .. cpp:type:: boost::shared_ptr<EventPostSyscall> ptr
  .. cpp:type:: boost::shared_ptr<const EventPostSyscall> const_ptr

  .. cpp:function:: EventPostSyscall()
  .. cpp:function:: virtual ~EventPostSyscall()
  .. cpp:function:: long getReturnValue() const

  .. cpp:function:: Dyninst::Address getAddress() const

      Returns the address where the system call occurred.

  .. cpp:function:: MachSyscall getSyscall() const

      Returns information about the system call.


.. cpp:class:: EventLibrary : public Event

  An EventLibrary triggers when the process either loads or unloads a
  shared library. ProcControlAPI will not trigger an EventLibrary for
  library unloads associated with a Process being terminated, and it will
  not trigger EventLibrary for library loads that happened before a
  ProcControlAPI attach operation.

  It is possible for multiple libraries to be loaded or unloaded at the
  same time. In this case, an EventLibrary will contain multiple libraries
  in its load and unload sets.

  .. cpp:type:: boost::shared_ptr<EventLibrary> ptr
  .. cpp:type:: boost::shared_ptr<const EventLibrary> const_ptr

  .. cpp:function:: EventLibrary()
  .. cpp:function:: EventLibrary(const std::set<Library::ptr> &added_libs_, const std::set<Library::ptr> &rmd_libs_)
  .. cpp:function:: virtual ~EventLibrary()
  .. cpp:function:: void setLibs(const std::set<Library::ptr> &added_libs_, const std::set<Library::ptr> &rmd_libs_)

  .. cpp:function:: const std::set<Library::ptr> &libsAdded() const

      Returns the libraries loaded into the target process’ address space.

  .. cpp:function:: const std::set<Library::ptr> &libsRemoved() const

      Returns the libraries unloaded from the target process’ address space.


.. cpp:class:: EventChangePCStop : public Event

  .. cpp:type:: boost::shared_ptr<EventChangePCStop> ptr
  .. cpp:type:: boost::shared_ptr<const EventChangePCStop> const_ptr

  .. cpp:function:: EventChangePCStop()
  .. cpp:function:: virtual ~EventChangePCStop()


.. cpp:class:: EventNop : public Event

  .. cpp:type:: boost::shared_ptr<EventNop> ptr
  .. cpp:type:: boost::shared_ptr<const EventNop> const_ptr

  .. cpp:function:: EventNop()
  .. cpp:function:: virtual ~EventNop()


.. cpp:class:: EventPostponedSyscall : public Event

  .. cpp:type:: boost::shared_ptr<EventPostponedSyscall> ptr
  .. cpp:type:: boost::shared_ptr<const EventPostponedSyscall> const_ptr

  .. cpp:function:: EventPostponedSyscall()
  .. cpp:function:: virtual ~EventPostponedSyscall()



Notes
=====

The Event class is an abstract class that is never instantiated.
Instead, ProcControlAPI will instantiate children of the Event class,
each of which add information specific to the EventType. For example, an
Event representing a thread creation will have an EventType of
ThreadCreate and can be cast into an EventNewThread for specific
information about the new thread. The specific events that are
instantiated from Event are described in the Section 3.15..

An event that occurs on a running thread may cause the process, thread,
or neither to stop running until the event has been handled. The
specifics of what is stopped can change between different event types
and operating systems. Each Event describes whether it stopped the
associated process or thread with a SyncType field. The values of this
field can be async (the event stopped neither the process nor thread),
sync_thread (the event stopped its thread), or sync_process (the event
stopped all threads in the process). A callback function can choose how
to resume or stop a process or thread using its return value.
