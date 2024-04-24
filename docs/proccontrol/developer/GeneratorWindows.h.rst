.. _`sec:GeneratorWindows.h`:

GeneratorWindows.h
##################

.. cpp:class:: GeneratorWindows : public GeneratorMT

  .. cpp:function:: GeneratorWindows()
  .. cpp:function:: virtual ~GeneratorWindows()
  .. cpp:function:: virtual bool initialize()
  .. cpp:function:: virtual bool canFastHandle()
  .. cpp:function:: virtual ArchEvent *getEvent(bool block)
  .. cpp:function:: virtual bool plat_continue(ArchEvent* evt)
  .. cpp:function:: virtual void plat_start()

.. cpp:enum:: GeneratorWindows::start_mode
  
  .. cpp:enumerator:: create
  .. cpp:enumerator:: attach


.. cpp:struct:: GeneratorWindows::StartInfo

  .. cpp:member:: start_mode mode
  .. cpp:member:: int_process* proc


.. cpp:struct:: GeneratorWindows::processData

  .. cpp:type:: boost::shared_ptr<processData> ptr
  .. cpp:member:: bool unhandled_exception
  .. cpp:member:: int_process* proc
  .. cpp:member:: state_t state
  .. cpp:member:: std::deque<StartInfo> procsToStart
  .. cpp:member:: std::map<int, processData::ptr> thread_to_proc
  .. cpp:member:: std::map<int, ArchEvent*> m_Events
  .. cpp:member:: std::map<Dyninst::PID, long long> alreadyHandled

  .. cpp:function:: processData()
  .. cpp:function:: void markUnhandledException(Dyninst::PID p)
  .. cpp:function:: void enqueue_event(start_mode m, int_process* p)
  .. cpp:function:: virtual bool isExitingState()
  .. cpp:function:: virtual void setState(state_t newstate)
  .. cpp:function:: virtual state_t getState()
  .. cpp:function:: virtual bool hasLiveProc()
  .. cpp:function:: void removeProcess(int_process* proc)
  .. cpp:function:: virtual ArchEvent* getCachedEvent()
  .. cpp:function:: virtual void setCachedEvent(ArchEvent* ae)
