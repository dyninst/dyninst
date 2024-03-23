.. _`sec:windows_handler.h`:

windows_handler.h
#################

.. cpp:class:: ArchEventWindows : public ArchEvent

  .. cpp:function:: bool findPairedEvent(ArchEventWindows* &parent, ArchEventWindows* &child)
  .. cpp:function:: void postponePairedEvent()
  .. cpp:function:: ArchEventWindows(DEBUG_EVENT e)
  .. cpp:function:: virtual ~ArchEventWindows()
  .. cpp:member:: DEBUG_EVENT evt


.. cpp:class:: WinEventNewThread : public EventNewLWP

  .. cpp:type:: boost::shared_ptr<WinEventNewThread> ptr
  .. cpp:type:: boost::shared_ptr<const WinEventNewThread> const_ptr
  .. cpp:function:: WinEventNewThread(Dyninst::LWP l, HANDLE ht, LPTHREAD_START_ROUTINE ts, LPVOID base)
  .. cpp:function:: virtual ~WinEventNewThread()
  .. cpp:function:: HANDLE getHandle() const
  .. cpp:function:: LPTHREAD_START_ROUTINE getThreadStart() const
  .. cpp:function:: LPVOID getTLSBase() const


.. cpp:class:: WinEventThreadInfo : public Event

  .. cpp:type:: boost::shared_ptr<WinEventThreadInfo> ptr
  .. cpp:type:: boost::shared_ptr<const WinEventThreadInfo> const_ptr
  .. cpp:function:: WinEventThreadInfo(Dyninst::LWP l, HANDLE ht, LPTHREAD_START_ROUTINE ts, LPVOID base)
  .. cpp:function:: virtual ~WinEventThreadInfo()
  .. cpp:function:: HANDLE getHandle() const
  .. cpp:function:: LPTHREAD_START_ROUTINE getThreadStart() const
  .. cpp:function:: LPVOID getTLSBase() const
  .. cpp:function:: Dyninst::LWP getLWP() const


.. cpp:class:: WindowsHandleSetThreadInfo : public Handler

  .. cpp:function:: WindowsHandleSetThreadInfo()
  .. cpp:function:: virtual ~WindowsHandleSetThreadInfo()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: WindowsHandleNewThr : public Handler

  .. cpp:function:: WindowsHandleNewThr()
  .. cpp:function:: virtual ~WindowsHandleNewThr()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: WindowsHandleLWPDestroy : public Handler

  .. cpp:function:: WindowsHandleLWPDestroy()
  .. cpp:function:: virtual ~WindowsHandleLWPDestroy()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: WindowsHandleProcessExit : public Handler

  .. cpp:function:: WindowsHandleProcessExit()
  .. cpp:function:: virtual ~WindowsHandleProcessExit()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: WinHandleSingleStep : public Handler

  .. cpp:function:: WinHandleSingleStep()
  .. cpp:function:: virtual ~WinHandleSingleStep()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: WinHandleBootstrap : public Handler

  .. cpp:function:: WinHandleBootstrap()
  .. cpp:function:: virtual ~WinHandleBootstrap()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: WinHandleContinue : public Handler

  .. cpp:function:: WinHandleContinue()
  .. cpp:function:: virtual ~WinHandleContinue()
  .. cpp:function:: virtual handler_ret_t handleEvent(Event::ptr ev)
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: void getEventTypesHandled(std::vector<EventType> &etypes)


.. cpp:class:: WindowsHandleThreadStop : public HandleThreadStop

  .. cpp:function:: WindowsHandleThreadStop()
  .. cpp:function:: virtual ~WindowsHandleThreadStop()
  .. cpp:function:: virtual int getPriority() const
  .. cpp:function:: virtual void getEventTypesHandled(std::vector<EventType> &etypes)
