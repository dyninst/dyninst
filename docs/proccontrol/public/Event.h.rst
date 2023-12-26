.. _`sec:Event.h`:

Event.h
=======

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:class:: ArchEvent

  .. cpp:function:: ArchEvent(std::string name_ = std::string(""));
  .. cpp:function:: virtual ~ArchEvent();
  .. cpp:function:: std::string getName();

.. cpp:class:: Event

  The Event class represents an instance of an event happening. Each Event
  has an EventType that describes the event and pointers to the Process
  and Thread that the event occurred on. See :ref:`sec:proccontrol-intro-callback-events`
  for an overview.

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

  .. cpp:enum:: SyncType

     .. cpp:enumerator:: SyncType::unset
     .. cpp:enumerator:: SyncType::async
     .. cpp:enumerator:: SyncType::sync_thread
     .. cpp:enumerator:: SyncType::sync_process

       The SyncType type is used to describe how a process or thread is stopped
       by an Event. See the above explanation for more details.

  .. cpp:function:: Thread::const_ptr getThread() const

    Returns a const pointer to the Thread object that
    represents the thread this event occurred on.

  .. cpp:function:: Process::const_ptr getProcess() const

    Returns a pointer to the process this event occurred on.

  .. cpp:function:: EventType getEventType() const

    Returns the EventType associated with this Event.

  .. cpp:function:: SyncType getSyncType() const

    Returns the SyncType associated with this Event.

  .. cpp:function:: std::string name() const

    Returns a human readable name for this Event.

  .. cpp:function:: EventTerminate::const_ptr getEventTerminate() const

  .. cpp:function:: EventExit::const_ptr getEventExit() const

  .. cpp:function:: EventCrash::const_ptr getEventCrash() const

  .. cpp:function:: EventForceTerminate::const_ptr getEventForceTerminate() const

  .. cpp:function:: EventExec::const_ptr getEventExec() const

  .. cpp:function:: EventStop::const_ptr getEventStop() const

  .. cpp:function:: EventBreakpoint::const_ptr getEventBreakpoint() const

  .. cpp:function:: EventNewThread::const_ptr getEventNewThread() const

  .. cpp:function:: EventNewUserThread::const_tr getEventNewUserThread() const

  .. cpp:function:: EventNewLWP::const_ptr getEventNewLWP() const

  .. cpp:function:: EventThreadDestroy::const_ptr getEventThreadDestroy() const

  .. cpp:function:: EventUserThreadDestroy::const_ptr getEventUserThreadDestroy() const

  .. cpp:function:: EventLWPDestroy::const_ptr getEventLWPDestroy() const

  .. cpp:function:: EventFork::const_ptr getEventFor() const

  .. cpp:function:: EventSignal::const_ptr getEventSignal() const

  .. cpp:function:: EventRPC::const_ptr getEventRPC() const

  .. cpp:function:: EventSingleStep::const_ptr getEventSingleStep() const

  .. cpp:function:: EventLibrary::const_ptr getEventLibrary() const

    These functions serve as a form of dynamic_cast. They cast the Event
    into a child type and return the result of that cast. If the Event
    object is not of the appropriate type for the given function, then they
    return a shared pointer NULL equivalent (ptr() or const_ptr()).

    For example, if an Event was an instance of an EventRPC, then the
    getEventRPC() function would cast it to EventRPC and return the
    resulting value.

.. cpp:class:: EventTerminate

  The EventTerminate class is a parent class for EventExit and EventCrash.
  It is never instantiated by ProcControlAPI and simply serves as a
  place-holder type for a user to deal with process termination without
  dealing with the specifics of whether a process exited properly or
  crashed.

  Associated EventType Codes:

     Exit, Crash and ForceTerminate

.. cpp:class:: EventExit

  An EventExit triggers when a process performs a normal exit (e.g.,
  calling the exit function or returning from main). The process that
  exited is referenced with Event’s getProcess function.

  An EventExit may be associated with an EventType of pre-exit or
  post-exit. Pre-exit means the process has not yet cleaned up its address
  space, and thus memory can still be read or written. Post-exit means the
  process has cleaned up its address space, memory is no longer
  accessible.

  Associated EventType Code:

     Exit

  .. cpp:function:: int getExitCode() const

    Returns the process’ exit code.

.. cpp:class:: EventCrash

  An EventCrash triggers when a process performs an abnormal exit (e.g.,
  crashing on a memory violation). The process that crashed is referenced
  with Event’s getProcess function.

  An EventCrash may be associated with an EventType of pre-crash or
  post-crash. Pre-crash means the process has not yet cleaned up its
  address space, and thus memory can still be read or written. Post-crash
  means the process has cleaned up its address space, memory is no longer
  accessible.

  Associated EventType Code:

     Crash

  .. cpp:function:: int getTermSignal() const

    Returns the signal that caused the process to crash.

.. cpp:class:: EventForceTerminate

  An EventForceTerminate triggers when a process is forcefully terminated
  via the Process::terminate function. When the callback is delivered for
  this event, the address space of the corresponding process will no
  longer be available.

  Associated EventType Code:

     ForceTerminate

  .. cpp:function:: int getTermSignal() const

    Returns the signal that was used to terminate the process.

.. cpp:class:: EventExec

  An EventExec triggers when a process performs a UNIX-style exec
  operation. An EventType of post-Exec means the process has completed the
  exec and setup its new address space. An EventType of pre-Exec means the
  process has not yet torn down its old address space.

  Associated EventType Code:

     Exec

  .. cpp:function:: std::string getExecPath() const

    Returns the file path to the process’ new executable.

.. cpp:class:: EventStop

  An EventStop is triggered when a process is stopped by a
  non-ProcControlAPI source. On UNIX based systems, this is triggered by
  receipt of a SIGSTOP signal.

  Unlike most other events, an EventStop will explicitly move the
  associated thread or process (see the Event’s SyncType to tell which) to
  a stopped state. Returning cbDefault from a callback function that has
  received EventStop will leave the target process in a stopped state
  rather than restore it to the pre-event state.

  Associated EventType Code:

     Stop

.. cpp:class:: EventBreakpoint

  An EventBreakpoint triggers when the target process encounters a
  breakpoint inserted by the ProcControlAPI (see Section 3.4.).

  Similar to EventStop, EventBreakpoint will explicitly move the thread or
  process to a stopped state. Returning cbDefault from a callback function
  that has received EventBreakpoint will leave the target process in a
  stopped state rather than restore it to the pre-event state.

  Associated EventType Code:

     Breakpoint

  .. cpp:function:: Dyninst::Address getAddress() const

    Returns the address at which the breakpoint was hit.

  .. cpp:function:: void getBreakpoints(std::vector<Breakpoint::const_ptr> &b) const

    Returns the breakpoints that were hit.

    Since it is possible to insert multiple breakpoints at the same
    location, it is possible for this function to return more than one
    breakpoint.

.. cpp:class:: EventNewThread

  An EventNewThread triggers when a process spawns a new thread. The Event
  class’ getThread function returns the original Thread that performed the
  spawn operation, while EventNewThread’s getNewThread returns the newly
  created Thread.

  This event is never instantiated by ProcControlAPI and simply serves as
  a place-holder type for a user to deal with thread creation without
  having to deal with the specifics of LWP and user thread creation.

  A callback function that receives an EventNewThread can use the two
  field form of :cpp:enum:`Process::cb_ret_t` to control the parent and child thread.

  Associated EventType Codes:

     ThreadCreate, UserThreadCreate, LWPCreate

  .. cpp:function:: Thread::const_ptr getNewThread() const

    Creates a new thread.

.. cpp:class:: EventNewUserThread

  An EventNewUserThread triggers when a process spawns a new user-level
  thread. The Event class’ getThread function returns the original Thread
  that performed the spawn operation. This thread may have already been
  created if the platform supports the EventNewLWP event. If not, the
  getNewThread function returns the newly created Thread.

  A callback function that receives an EventNewThread can use the two
  field form of :cpp:enum:`Process::cb_ret_t` to control the parent and child thread.

  Associated EventType Code:

     UserThreadCreate

  .. cpp:function:: Thread::const_ptr getNewThread() const

    Creates a new thread.

.. cpp:class:: EventNewLWP

  An EventNewLWP triggers when a process spawns a new LWP. The Event
  class’ getThread function returns the original Thread that performed the
  spawn operation, while EventNewThread’s getNewThread returns the newly
  created Thread.

  A callback function that receives an EventNewThread can use the two
  field form of ``Process::cb_ret_t`` to control the parent and child thread.

  Associated EventType Code:

     LWPCreate

  .. cpp:function:: Thread::const_ptr getNewThread() const

    Creates a new thread.

.. cpp:class:: EventThreadDestroy

  An EventThreadDestroy triggers when a thread exits. Event’s getThread
  member function returns the thread that exited.

  This event is never instantiated by ProcControlAPI and simply serves as
  a place-holder type for a user to deal with thread destruction without
  having to deal with the specifics of LWP and user thread destruction.

  Associated EventType Codes:

     ThreadDestroy, UserThreadDestroy, LWPDestroy

.. cpp:class:: EventUserThreadDestroy

  An EventUserThreadDestroy triggers when a thread exits. Event’s
  getThread member function returns the thread that exited.

  If the platform also supports EventLWPDestroy events, this event will
  precede an EventLWPDestroy event.

  Associated EventType Code:

     UserThreadDestroy

.. cpp:class:: EventLWPDestroy

  An LWPThreadDestroy triggers when a thread exits. Event’s getThread
  member function returns the thread that exited.

  Associated EventType Code:

     LWPDestroy

.. cpp:class:: EventFork

  An EventFork triggers when a process performs a UNIX-style fork
  operation. The process that performed the initial fork is returned via
  Event’s getProcess member function, while the newly created process can
  be found via EventFork’s getChildProcess member function.

  Associated EventType Code:

     Fork

  .. cpp:function:: Process::const_ptr getChildProcess() const

    Creates the newly-created child process.

.. cpp:class:: EventSignal

  An EventSignal triggers when a process receives a UNIX style signal.

  Associated EventType Code:

     Signal

  .. cpp:function:: int getSignal() const

    Returns the signal number that triggered the EventSignal.

.. cpp:class:: EventRPC

  An EventRPC triggers when a process or thread completes a ProcControlAPI
  iRPC (see Sections 2.3. and 3.5.). When a callback function receives an
  EventRPC, the memory and registers that were used by the iRPC can still
  be found in the address space and thread that the iRPC ran on. Once the
  callback function completes, the registers and memory are restored to
  their original state.

  Associated EventType Code:

     RPC

  .. cpp:function:: IRPC::const_ptr getIRPC() const

    Returns the IRPC that completed.

.. cpp:class:: EventSingleStep

  An EventSingleStep triggers when a thread, which was put in single-step
  mode by ``Thread::setSingleStep``, completes a single step operation. The
  Thread will remain in single-step mode after completion of this event
  (presuming it has not be explicitly disabled by ``Thread::setSingleStep``).

  Associated EventType Code:

     SingleStep

.. cpp:class:: EventLibrary

  An EventLibrary triggers when the process either loads or unloads a
  shared library. ProcControlAPI will not trigger an EventLibrary for
  library unloads associated with a Process being terminated, and it will
  not trigger EventLibrary for library loads that happened before a
  ProcControlAPI attach operation.

  It is possible for multiple libraries to be loaded or unloaded at the
  same time. In this case, an EventLibrary will contain multiple libraries
  in its load and unload sets.

  Associated EventType Code:

     Library

  .. cpp:function:: const std::set<Library::ptr> &libsAdded() const

    Returns the libraries loaded into the target process’ address space.

  .. cpp:function:: const std::set<Library::ptr> &libsRemoved() const

    Returns the libraries unloaded from the target process’ address space.

.. cpp:class:: EventPreSyscall

  An EventPreSyscall is triggered when a thread enters a system call,
  provided that the thread is in system call tracing mode. It is a
  child of EventSyscall, which provides all the relevant methods.

  Associated EventType Code:

     Syscall

  .. cpp:function:: Dyninst::Address getAddress() const

    Returns the address where the system call occurred.

  .. cpp:function:: MachSyscall getSyscall() const

    Returns information about the system call.

.. cpp:class:: EventPostSyscall

  An EventPostSyscall is triggered when a system call returns. It is a
  child of EventSyscall, which provides all the relevant methods.

  Associated EventType Code:

     Syscall

  .. cpp:function:: Dyninst::Address getAddress() const

    Returns the address where the system call occurred.

  .. cpp:function:: MachSyscall getSyscall() const

    Returns information about the system call.
