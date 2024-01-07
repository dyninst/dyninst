.. _`sec:Generator.h`:

Generator.h
===========

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:class:: Generator

  .. cpp:type:: void (*gen_cb_func_t)()

  .. cpp:member:: static bool startedAnyGenerator
  .. cpp:member:: state_t state

  .. cpp:function:: static Generator* getDefaultGenerator()
  .. cpp:function:: static void stopDefaultGenerator()
  .. cpp:function:: virtual bool getAndQueueEvent(bool block) = 0
  .. cpp:function:: virtual ~Generator()
  .. cpp:function:: static void registerNewEventCB(gen_cb_func_t func)
  .. cpp:function:: static void removeNewEventCB(gen_cb_func_t)
  .. cpp:function:: void forceEventBlock()
  .. cpp:function:: static const char* generatorStateStr(state_t)
  .. cpp:function:: virtual bool isExitingState()
  .. cpp:function:: virtual void setState(state_t newstate)
  .. cpp:function:: virtual state_t getState()

.. cpp:class:: GeneratorMT : public Generator

  .. cpp:function:: GeneratorMTInternals *getInternals()
  .. cpp:function:: GeneratorMT(std::string name_)
  .. cpp:function:: virtual ~GeneratorMT()
  .. cpp:function:: void launch()
  .. cpp:function:: void start()
  .. cpp:function:: virtual void plat_start()
  .. cpp:function:: virtual bool plat_continue(ArchEvent*)
  .. cpp:function:: virtual bool processWait(bool block)
  .. cpp:function:: virtual bool getAndQueueEvent(bool block)

  .. cpp:function:: protected void lock()
  .. cpp:function:: protected void unlock()

.. cpp:class:: GeneratorST : public Generator

  .. cpp:function:: GeneratorST(std::string name_)
  .. cpp:function:: virtual ~GeneratorST()
  .. cpp:function:: virtual bool processWait(bool block)
  .. cpp:function:: virtual bool getAndQueueEvent(bool block)

.. cpp:enum:: Generator::state_t

  .. cpp:enumerator:: none
  .. cpp:enumerator:: initializing
  .. cpp:enumerator:: process_blocked
  .. cpp:enumerator:: system_blocked
  .. cpp:enumerator:: decoding
  .. cpp:enumerator:: statesync
  .. cpp:enumerator:: handling
  .. cpp:enumerator:: queueing
  .. cpp:enumerator:: error
  .. cpp:enumerator:: exiting
