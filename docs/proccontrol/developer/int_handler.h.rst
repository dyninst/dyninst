.. _`sec:int_handler.h`:

int_handler.h
#############

.. cpp:class:: HandlerPool

  .. cpp:type:: std::set<Handler *, handler_cmp> HandlerSet_t
  .. cpp:type:: std::map<EventType, HandlerSet_t*, eventtype_cmp> HandlerMap_t
  .. cpp:function:: HandlerPool(int_process *owner_proc)
  .. cpp:function:: ~HandlerPool()
  .. cpp:function:: void addHandler(Handler *handler)
  .. cpp:function:: bool handleEvent(Event::ptr ev)
  .. cpp:function:: Event::ptr handleAsyncEvent(Event::ptr ev)
  .. cpp:function:: void notifyOfPendingAsyncs(const std::set<response::ptr> &asyncs, Event::ptr ev)
  .. cpp:function:: void notifyOfPendingAsyncs(response::ptr async, Event::ptr ev)
  .. cpp:function:: bool isEventAsyncPostponed(Event::ptr ev) const
  .. cpp:function:: bool hasAsyncEvent() const
  .. cpp:function:: static bool hasProcAsyncPending()
  .. cpp:function:: void markEventAsyncPending(Event::ptr ev)
  .. cpp:function:: void addLateEvent(Event::ptr ev)
  .. cpp:function:: Event::ptr curEvent()
  .. cpp:function:: void setNopAsCurEvent()
  .. cpp:function:: void clearNopAsCurEvent()


.. cpp:class:: HandlePreBootstrap : public Handler

  .. cpp:function:: HandlePreBootstrap()
  .. cpp:function:: virtual ~HandlePreBootstrap()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandleBootstrap : public Handler

  .. cpp:function:: HandleBootstrap()
  .. cpp:function:: virtual ~HandleBootstrap()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandleCrash : public Handler

  .. cpp:function:: HandleCrash()
  .. cpp:function:: virtual ~HandleCrash()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandleForceTerminate : public Handler

  .. cpp:function:: HandleForceTerminate()
  .. cpp:function:: virtual ~HandleForceTerminate()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const


.. cpp:class:: HandleSignal : public Handler

  .. cpp:function:: HandleSignal()
  .. cpp:function:: virtual ~HandleSignal()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandlePostExit : public Handler

  .. cpp:function:: HandlePostExit()
  .. cpp:function:: virtual ~HandlePostExit()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandlePostExitCleanup : public Handler

  .. cpp:function:: HandlePostExitCleanup()
  .. cpp:function:: virtual ~HandlePostExitCleanup()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const


.. cpp:class:: HandlePreExit : public Handler

  .. cpp:function:: HandlePreExit()
  .. cpp:function:: virtual ~HandlePreExit()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandleThreadCreate : public Handler

  .. cpp:function:: HandleThreadCreate()
  .. cpp:function:: virtual ~HandleThreadCreate()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandleThreadDestroy : public Handler

  .. cpp:function:: HandleThreadDestroy()
  .. cpp:function:: virtual ~HandleThreadDestroy()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const


.. cpp:class:: HandleThreadCleanup : public Handler

  .. cpp:function:: HandleThreadCleanup()
  .. cpp:function:: virtual ~HandleThreadCleanup()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const


.. cpp:class:: HandleThreadStop : public Handler

  .. cpp:function:: HandleThreadStop()
  .. cpp:function:: virtual ~HandleThreadStop()
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandlePostFork : public Handler

  .. cpp:function:: HandlePostFork()
  .. cpp:function:: virtual ~HandlePostFork()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandlePostForkCont : public Handler

  .. cpp:function:: HandlePostForkCont()
  .. cpp:function:: virtual ~HandlePostForkCont()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const


.. cpp:class:: HandlePostExec : public Handler

  .. cpp:function:: HandlePostExec()
  .. cpp:function:: virtual ~HandlePostExec()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandleSingleStep : public Handler

  .. cpp:function:: HandleSingleStep()
  .. cpp:function:: virtual ~HandleSingleStep()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandlePreSyscall : public Handler

  .. cpp:function:: HandlePreSyscall()
  .. cpp:function:: virtual ~HandlePreSyscall()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandlePostSyscall : public Handler

  .. cpp:function:: HandlePostSyscall()
  .. cpp:function:: virtual ~HandlePostSyscall()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandleBreakpoint : public Handler

  This handler is triggered when a breakpoint is first hit (e.g., on the SIGTRAP signal).
  It's main purpose is to prepare the thread state before the user callback.

  .. cpp:function:: HandleBreakpoint()
  .. cpp:function:: virtual ~HandleBreakpoint()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandleBreakpointContinue : public Handler

  .. cpp:function:: HandleBreakpointContinue()
  .. cpp:function:: virtual ~HandleBreakpointContinue()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const


.. cpp:class:: HandleBreakpointClear : public Handler

  .. cpp:function:: HandleBreakpointClear()
  .. cpp:function:: virtual ~HandleBreakpointClear()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)

    The handler triggers when a thread stopped on a breakpoint is continued.


.. cpp:class:: HandleBreakpointRestore : public Handler

  .. cpp:function:: HandleBreakpointRestore()
  .. cpp:function:: virtual ~HandleBreakpointRestore()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)


.. cpp:class:: HandleEmulatedSingleStep : public Handler

  .. cpp:function:: HandleEmulatedSingleStep()
  .. cpp:function:: ~HandleEmulatedSingleStep()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const


.. cpp:class:: HandleLibrary : public Handler

  .. cpp:function:: HandleLibrary()
  .. cpp:function:: virtual ~HandleLibrary()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: HandleDetach : public Handler

  .. cpp:function:: HandleDetach()
  .. cpp:function:: virtual ~HandleDetach()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: HandleAsync : public Handler

  .. cpp:function:: HandleAsync()
  .. cpp:function:: virtual ~HandleAsync()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: HandleAsyncIO : public Handler

  .. cpp:function:: HandleAsyncIO()
  .. cpp:function:: virtual ~HandleAsyncIO()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: HandleAsyncFileRead : public Handler

  .. cpp:function:: HandleAsyncFileRead()
  .. cpp:function:: virtual ~HandleAsyncFileRead()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: HandleNop : public Handler

  .. cpp:function:: HandleNop()
  .. cpp:function:: virtual ~HandleNop()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: HandleCallbacks : public Handler

  .. cpp:function:: HandleCallbacks()
  .. cpp:function:: virtual ~HandleCallbacks()
  .. cpp:function:: static HandleCallbacks *getCB()
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: bool hasCBs(Event::const_ptr ev)
  .. cpp:function:: bool hasCBs(EventType et)
  .. cpp:function:: bool registerCallback(EventType ev, Process::cb_func_t func)
  .. cpp:function:: bool removeCallback(EventType et, Process::cb_func_t func)
  .. cpp:function:: bool removeCallback(EventType et)
  .. cpp:function:: bool removeCallback(Process::cb_func_t func)
  .. cpp:function:: Handler::handler_ret_t deliverCallback(Event::ptr ev, const std::set<Process::cb_func_t> &cbset)
  .. cpp:function:: bool requiresCB(Event::const_ptr ev)
  .. cpp:function:: static void getRealEvents(EventType ev, std::vector<EventType> &out_evs)


.. cpp:class:: HandlePostponedSyscall : public Handler

  .. cpp:function:: HandlePostponedSyscall()
  .. cpp:function:: virtual ~HandlePostponedSyscall()
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
