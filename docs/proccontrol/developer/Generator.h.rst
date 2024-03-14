.. _`sec-dev:Generator.h`:

Generator.h
===========

.. cpp:namespace:: Dyninst::ProcControlAPI::dev

.. cpp:class:: Generator

  .. rubric:: Event handling

  .. cpp:member:: protected static std::map<Dyninst::PID, Process *> procs
  .. cpp:type:: protected std::set<Decoder *, decoder_cmp> decoder_set_t
  .. cpp:member:: protected decoder_set_t decoders
  .. cpp:member:: protected ArchEvent* m_Event
  .. cpp:function:: protected virtual ArchEvent* getCachedEvent()
  .. cpp:function:: protected virtual void setCachedEvent(ArchEvent* ae)

  ......

  .. cpp:function:: protected virtual bool hasLiveProc()
  .. cpp:member:: protected std::string name
  .. cpp:function:: protected Generator(std::string name_)
  .. cpp:function:: protected bool getAndQueueEventInt(bool block)
  .. cpp:function:: protected static bool allStopped(int_process *proc, void *)
  .. cpp:member:: protected static std::set<gen_cb_func_t> CBs
  .. cpp:member:: protected static Mutex<> *cb_lock

  ......

  .. rubric:: Implemented by architectures

  .. cpp:function:: protected virtual bool initialize() = 0
  .. cpp:function:: protected virtual bool canFastHandle() = 0
  .. cpp:function:: protected virtual ArchEvent *getEvent(bool block) = 0
  .. cpp:function:: protected virtual bool plat_skipGeneratorBlock()

  ......

  .. rubric:: Implemented by MT or ST

  .. cpp:function:: protected virtual bool processWait(bool block) = 0
  .. cpp:function:: protected virtual bool plat_continue(ArchEvent* evt)
  .. cpp:function:: protected virtual void wake(Dyninst::PID proc , long long  sequence )

  .. cpp:function:: protected virtual bool getMultiEvent(bool block, std::vector<ArchEvent *> &events)

    Optional interface for systems that want to return multiple events

  .. cpp:member:: private bool eventBlock_


.. cpp:class:: GeneratorMT : public Generator

  .. cpp:function:: protected void lock()
  .. cpp:function:: protected void unlock()


.. cpp:var:: static int_cleanup cleanup

  Library deinitialization

  It is crucial that this variable is located in the source file because it
  guarantees that the threads will be stopped before destructing the CBs collection and therefore
  avoiding a problem where the generator will continue to run but the CBs collection has already been
  destructed
