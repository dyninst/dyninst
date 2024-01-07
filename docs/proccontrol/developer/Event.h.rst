.. _`sec-dev:Event.h`:

Event.h
#######

.. cpp:namespace:: Dyninst::ProcControlAPI::dev

.. cpp:class:: Event : public boost::enable_shared_from_this<Dyninst::ProcControlAPI::Event>

  .. cpp:function:: void setThread(Thread::const_ptr t)
  .. cpp:function:: void setProcess(Process::const_ptr p)
  .. cpp:function:: void setSyncType(SyncType t)
  .. cpp:function:: void setSuppressCB(bool b)
  .. cpp:function:: virtual bool suppressCB() const
  .. cpp:function:: virtual bool triggersCB() const
  .. cpp:function:: virtual bool canFastHandle() const
  .. cpp:function:: virtual bool userEvent() const
  .. cpp:function:: virtual void setUserEvent(bool b)
  .. cpp:function:: virtual bool procStopper() const
  .. cpp:function:: Event::weak_ptr subservientTo() const
  .. cpp:function:: void addSubservientEvent(Event::ptr ev)
  .. cpp:function:: void setLastError(err_t ec, const char *es)

  .. cpp:function:: boost::shared_ptr<EventIntBootstrap> getEventIntBootstrap()

      Casts this event to an :cpp:class:`EventIntBootstrap`.

      Returns ``nullptr`` if this event is not convertible to a ``EventIntBootstrap``.

  .. cpp:function:: boost::shared_ptr<const EventIntBootstrap> getEventIntBootstrap() const

      Casts this event to an :cpp:class:`EventIntBootstrap`.

      Returns ``nullptr`` if this event is not convertible to a ``EventIntBootstrap``.

  .. cpp:member:: protected EventType etype
  .. cpp:member:: protected Thread::const_ptr thread
  .. cpp:member:: protected Process::const_ptr proc
  .. cpp:member:: protected SyncType stype
  .. cpp:member:: protected std::vector<Event::ptr> subservient_events
  .. cpp:member:: protected Event::weak_ptr master_event
  .. cpp:member:: protected std::set<Handler *> handled_by
  .. cpp:member:: protected bool suppress_cb
  .. cpp:member:: protected bool user_event
  .. cpp:member:: protected bool handling_started
  .. cpp:member:: protected bool noted_event


.. cpp:class:: EventBootstrap : public Event

  .. cpp:type:: boost::shared_ptr<EventBootstrap> ptr
  .. cpp:type:: boost::shared_ptr<const EventBootstrap> const_ptr

  .. cpp:function:: EventBootstrap()
  .. cpp:function:: virtual ~EventBootstrap()

.. cpp:class:: EventPreBootstrap : public Event

  .. cpp:type:: boost::shared_ptr<EventPreBootstrap> ptr
  .. cpp:type:: boost::shared_ptr<const EventPreBootstrap> const_ptr

  .. cpp:function:: EventPreBootstrap()
  .. cpp:function:: virtual ~EventPreBootstrap()


.. cpp:class:: EventRPC : public Event

  An EventRPC triggers when a process or thread completes an iRPC.
  When a callback function receives an
  EventRPC, the memory and registers that were used by the iRPC can still
  be found in the address space and thread that the iRPC ran on. Once the
  callback function completes, the registers and memory are restored to
  their original state.

  .. cpp:type:: boost::shared_ptr<EventRPC> ptr
  .. cpp:type:: boost::shared_ptr<const EventRPC> const_ptr

  .. cpp:function:: EventRPC(rpc_wrapper *wrapper_)
  .. cpp:function:: virtual ~EventRPC()
  .. cpp:function:: virtual bool suppressCB() const
  .. cpp:function:: rpc_wrapper *getllRPC()
  .. cpp:function:: IRPC::const_ptr getIRPC() const
  .. cpp:function:: int_eventRPC *getInternal() const

.. cpp:class:: EventRPCLaunch : public Event

  .. cpp:type:: boost::shared_ptr<EventRPCLaunch> ptr
  .. cpp:type:: boost::shared_ptr<const EventRPCLaunch> const_ptr

  .. cpp:function:: virtual bool procStopper() const
  .. cpp:function:: EventRPCLaunch()
  .. cpp:function:: virtual ~EventRPCLaunch()


.. cpp:class:: EventSignal : public Event

  An EventSignal triggers when a process receives a UNIX-style signal.

  .. cpp:type:: boost::shared_ptr<EventSignal> ptr
  .. cpp:type:: boost::shared_ptr<const EventSignal> const_ptr

  .. cpp:member:: private Address addr

      Address that caused the signal (if any)

  .. cpp:member:: private Cause cause

      The cause of the signal.

  .. cpp:member:: private bool first

      Whether this is a first-cause exception (windows access violations).

  .. cpp:function:: EventSignal(int sig)
  .. cpp:function:: EventSignal(int s, Address a, Cause c, bool f)
  .. cpp:function:: virtual ~EventSignal()
  .. cpp:function:: int getSignal() const

      Returns the signal number that triggered the EventSignal.

  .. cpp:function:: void setThreadSignal(int newSignal) const
  .. cpp:function:: void clearThreadSignal() const
  .. cpp:function:: Address getAddress() const
  .. cpp:function:: Cause getCause() const
  .. cpp:function:: bool isFirst() const


.. cpp:enum:: EventSignal::Cause

  The causes of signals. ``Unknown`` refers to all non-access violations.

  .. cpp:enumerator:: Unknown
  .. cpp:enumerator:: ReadViolation
  .. cpp:enumerator:: WriteViolation
  .. cpp:enumerator:: ExecuteViolation


.. cpp:class:: EventExec : public Event

  .. cpp:function:: void setExecPath(std::string path_);

.. cpp:class:: EventNewUserThread : public EventNewThread

  An EventNewUserThread triggers when a process spawns a new user-level
  thread. The Event class’ getThread function returns the original Thread
  that performed the spawn operation. This thread may have already been
  created if the platform supports the EventNewLWP event. If not, the
  getNewThread function returns the newly created Thread.

  A callback function that receives an EventNewThread can use the two
  field form of :cpp:enum:`Process::cb_ret_t` to control the parent and child thread.

  .. cpp:type:: boost::shared_ptr<EventNewUserThread> ptr
  .. cpp:type:: boost::shared_ptr<const EventNewUserThread> const_ptr

  .. cpp:function:: EventNewUserThread()
  .. cpp:function:: virtual ~EventNewUserThread()
  .. cpp:function:: int_eventNewUserThread *getInternalEvent() const
  .. cpp:function:: virtual Dyninst::LWP getLWP() const
  .. cpp:function:: virtual Thread::const_ptr getNewThread() const


.. cpp:class:: EventNewLWP : public EventNewThread

  An EventNewLWP triggers when a process spawns a new LWP. The Event
  class’ getThread function returns the original Thread that performed the
  spawn operation, while EventNewThread’s getNewThread returns the newly
  created Thread.

  A callback function that receives an EventNewThread can use the two
  field form of :cpp:enum:`Process::cb_ret_t` to control the parent and
  child thread.

  .. cpp:type:: boost::shared_ptr<EventNewLWP> ptr
  .. cpp:type:: boost::shared_ptr<const EventNewLWP> const_ptr

  .. cpp:function:: EventNewLWP(Dyninst::LWP lwp_, int status = 0)
  .. cpp:function:: virtual ~EventNewLWP()
  .. cpp:function:: virtual Dyninst::LWP getLWP() const
  .. cpp:function:: virtual Thread::const_ptr getNewThread() const
  .. cpp:function:: int_eventNewLWP *getInternalEvent()


.. cpp:class:: EventBreakpoint : public Event

  Similar to :cpp:class:`EventStop`, :cpp:class:`EventBreakpoint` will explicitly move the thread or
  process to a stopped state. Returning cbDefault from a callback function
  that has received EventBreakpoint will leave the target process in a
  stopped state rather than restore it to the pre-event state.

  .. cpp:type:: boost::shared_ptr<EventBreakpoint> ptr
  .. cpp:type:: boost::shared_ptr<const EventBreakpoint> const_ptr

  .. cpp:function:: EventBreakpoint(int_eventBreakpoint *ibp)
  .. cpp:function:: virtual ~EventBreakpoint()

  .. cpp:function:: Dyninst::Address getAddress() const

    Returns the address at which the breakpoint was hit.

  .. cpp:function:: void getBreakpoints(std::vector<Breakpoint::const_ptr> &b) const

    Returns the breakpoints that were hit.

    Since it is possible to insert multiple breakpoints at the same
    location, it is possible for this function to return more than one
    breakpoint.

  .. cpp:function:: void getBreakpoints(std::vector<Breakpoint::ptr> &bps)

    Returns the breakpoints that were hit.

    Since it is possible to insert multiple breakpoints at the same
    location, it is possible for this function to return more than one
    breakpoint.

  .. cpp:function:: virtual bool suppressCB() const
  .. cpp:function:: virtual bool procStopper() const
  .. cpp:function:: int_eventBreakpoint *getInternal() const
  .. cpp:function:: void setExecPath(std::string path_)


.. cpp:class:: EventIntBootstrap : public Event

  .. cpp:type:: boost::shared_ptr<EventIntBootstrap> ptr
  .. cpp:type:: boost::shared_ptr<const EventIntBootstrap> const_ptr

  .. cpp:function:: EventIntBootstrap(void *d = NULL)
  .. cpp:function:: virtual ~EventIntBootstrap()
  .. cpp:function:: void *getData() const
  .. cpp:function:: void setData(void *v)


.. cpp:class:: EventBreakpointClear : public Event

  .. cpp:type:: boost::shared_ptr<EventBreakpointClear> ptr
  .. cpp:type:: boost::shared_ptr<const EventBreakpointClear> const_ptr

  .. cpp:function:: EventBreakpointClear()
  .. cpp:function:: virtual ~EventBreakpointClear()
  .. cpp:function:: int_eventBreakpointClear *getInternal() const
  .. cpp:function:: virtual bool procStopper() const
  .. cpp:member:: int_eventBreakpointRestore *int_bpr
  .. cpp:function:: EventBreakpointRestore(int_eventBreakpointRestore *iebpr)


.. cpp:class:: EventBreakpointRestore : public Event

  .. cpp:type:: boost::shared_ptr<EventBreakpointRestore> ptr
  .. cpp:type:: boost::shared_ptr<const EventBreakpointRestore> const_ptr

  .. cpp:function:: EventBreakpointRestore(int_eventBreakpointRestore *iebpr)
  .. cpp:function:: virtual ~EventBreakpointRestore()
  .. cpp:function:: int_eventBreakpointRestore *getInternal() const


.. cpp:class:: EventAsync : public Event

  .. cpp:type:: boost::shared_ptr<EventAsync> ptr
  .. cpp:type:: boost::shared_ptr<const EventAsync> const_ptr

  .. cpp:function:: EventAsync(int_eventAsync *ievent)
  .. cpp:function:: virtual ~EventAsync()
  .. cpp:function:: int_eventAsync *getInternal() const


.. cpp:class:: EventDetach : public Event

  .. cpp:type:: boost::shared_ptr<EventDetach> ptr
  .. cpp:type:: boost::shared_ptr<const EventDetach> const_ptr

  .. cpp:function:: EventDetach()
  .. cpp:function:: virtual ~EventDetach()
  .. cpp:function:: int_eventDetach *getInternal() const
  .. cpp:function:: virtual bool procStopper() const


.. cpp:class:: EventThreadDB : public Event

  .. cpp:type:: boost::shared_ptr<EventThreadDB> ptr
  .. cpp:type:: boost::shared_ptr<const EventThreadDB> const_ptr

  .. cpp:function:: int_eventThreadDB *getInternal() const
  .. cpp:function:: EventThreadDB()
  .. cpp:function:: virtual ~EventThreadDB()
  .. cpp:function:: virtual bool triggersCB() const

.. cpp:class:: EventWinStopThreadDestroy : public EventThreadDestroy

  .. cpp:type:: boost::shared_ptr<EventWinStopThreadDestroy> ptr
  .. cpp:type:: boost::shared_ptr<const EventWinStopThreadDestroy> const_ptr

  .. cpp:function:: EventWinStopThreadDestroy(EventType::Time time_)
  .. cpp:function:: virtual ~EventWinStopThreadDestroy()


.. cpp:class:: EventControlAuthority : public Event

  .. cpp:type:: boost::shared_ptr<EventControlAuthority> ptr
  .. cpp:type:: boost::shared_ptr<const EventControlAuthority> const_ptr

  .. cpp:function:: int_eventControlAuthority *getInternalEvent() const
  .. cpp:function:: EventControlAuthority(EventType::Time t, int_eventControlAuthority *iev_)
  .. cpp:function:: virtual ~EventControlAuthority()
  .. cpp:function:: virtual bool procStopper() const
  .. cpp:function:: std::string otherToolName() const
  .. cpp:function:: unsigned int otherToolID() const
  .. cpp:function:: int otherToolPriority() const
  .. cpp:function:: Trigger eventTrigger() const


.. cpp:enum:: EventControlAuthority::Trigger

  .. cpp:enumerator:: ControlUnset
  .. cpp:enumerator:: ControlLost
  .. cpp:enumerator:: ControlGained
  .. cpp:enumerator:: ControlNoChange


.. cpp:class:: EventAsyncIO : public Event

  .. cpp:type:: boost::shared_ptr<EventAsyncIO> ptr
  .. cpp:type:: boost::shared_ptr<const EventAsyncIO> const_ptr

  .. cpp:function:: int_eventAsyncIO *getInternalEvent() const
  .. cpp:function:: EventAsyncIO(EventType et, int_eventAsyncIO *iev_)
  .. cpp:function:: ~EventAsyncIO()
  .. cpp:function:: bool hadError() const
  .. cpp:function:: void *getOpaqueVal() const

.. cpp:class:: EventAsyncRead : public EventAsyncIO

  .. cpp:type:: boost::shared_ptr<EventAsyncRead> ptr
  .. cpp:type:: boost::shared_ptr<const EventAsyncRead> const_ptr

  .. cpp:function:: EventAsyncRead(int_eventAsyncIO *iev_)
  .. cpp:function:: ~EventAsyncRead()
  .. cpp:function:: void *getMemory() const
  .. cpp:function:: size_t getSize() const
  .. cpp:function:: Dyninst::Address getAddress() const


.. cpp:class:: EventAsyncWrite : public EventAsyncIO

  .. cpp:type:: boost::shared_ptr<EventAsyncWrite> ptr
  .. cpp:type:: boost::shared_ptr<const EventAsyncWrite> const_ptr

  .. cpp:function:: EventAsyncWrite(int_eventAsyncIO *iev_)
  .. cpp:function:: ~EventAsyncWrite()
  .. cpp:function:: size_t getSize() const
  .. cpp:function:: Dyninst::Address getAddress() const


.. cpp:class:: EventAsyncReadAllRegs : public EventAsyncIO

  .. cpp:type:: boost::shared_ptr<EventAsyncReadAllRegs> ptr
  .. cpp:type:: boost::shared_ptr<const EventAsyncReadAllRegs> const_ptr

  .. cpp:function:: EventAsyncReadAllRegs(int_eventAsyncIO *iev_)
  .. cpp:function:: ~EventAsyncReadAllRegs()
  .. cpp:function:: const RegisterPool &getRegisters() const


.. cpp:class:: EventAsyncSetAllRegs : public EventAsyncIO

  .. cpp:type:: boost::shared_ptr<EventAsyncSetAllRegs> ptr
  .. cpp:type:: boost::shared_ptr<const EventAsyncSetAllRegs> const_ptr

  .. cpp:function:: EventAsyncSetAllRegs(int_eventAsyncIO *iev_)
  .. cpp:function:: ~EventAsyncSetAllRegs()


.. cpp:class:: EventAsyncFileRead : public Event

  .. cpp:type:: boost::shared_ptr<EventAsyncFileRead> ptr
  .. cpp:type:: boost::shared_ptr<const EventAsyncFileRead> const_ptr

  .. cpp:function:: int_eventAsyncFileRead *getInternal()
  .. cpp:function:: EventAsyncFileRead(int_eventAsyncFileRead *iev_)
  .. cpp:function:: ~EventAsyncFileRead()
  .. cpp:function:: std::string getFilename() const
  .. cpp:function:: size_t getReadSize() const
  .. cpp:function:: Dyninst::Offset getReadOffset() const
  .. cpp:function:: void *getBuffer() const
  .. cpp:function:: size_t getBufferSize() const
  .. cpp:function:: bool isEOF() const
  .. cpp:function:: int errorCode() const
