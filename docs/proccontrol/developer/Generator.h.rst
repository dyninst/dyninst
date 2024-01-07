.. _`sec-dev:Generator.h`:

Generator.h
===========

.. cpp:namespace:: Dyninst::ProcControlAPI::dev

.. cpp:class:: Generator

  .. cpp:type:: protected std::set<Decoder*, decoder_cmp> decoder_set_t

  .. cpp:member:: protected static std::map<Dyninst::PID, Process*> procs
  .. cpp:member:: protected decoder_set_t decoders
  .. cpp:member:: protected ArchEvent* m_Event
  .. cpp:member:: protected std::string name
  .. cpp:member:: protected static std::set<gen_cb_func_t> CBs
  .. cpp:member:: protected static Mutex<> *cb_lock

  .. cpp:function:: protected virtual ArchEvent* getCachedEvent()
  .. cpp:function:: protected virtual void setCachedEvent(ArchEvent* ae)
  .. cpp:function:: protected virtual bool hasLiveProc()
  .. cpp:function:: protected Generator(std::string name_)
  .. cpp:function:: protected bool getAndQueueEventInt(bool block)
  .. cpp:function:: protected static bool allStopped(int_process *proc, void *)
  .. cpp:function:: protected virtual bool initialize() = 0
  .. cpp:function:: protected virtual bool canFastHandle() = 0
  .. cpp:function:: protected virtual ArchEvent *getEvent(bool block) = 0
  .. cpp:function:: protected virtual bool plat_skipGeneratorBlock()
  .. cpp:function:: protected virtual bool processWait(bool block) = 0
  .. cpp:function:: protected virtual bool plat_continue(ArchEvent*)
  .. cpp:function:: protected virtual void wake(Dyninst::PID, long long)
  .. cpp:function:: protected virtual bool getMultiEvent(bool block, std::vector<ArchEvent *> &events)


.. cpp:class:: GeneratorMT : public Generator

  .. cpp:function:: protected void lock()
  .. cpp:function:: protected void unlock()
