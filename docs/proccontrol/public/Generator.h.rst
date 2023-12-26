.. _`sec:Generator.h`:

Generator.h
===========

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:class:: Generator

  .. cpp:function:: static Generator *getDefaultGenerator()
  .. cpp:function:: static void stopDefaultGenerator()
  .. cpp:function:: virtual bool getAndQueueEvent(bool block) = 0
  .. cpp:function:: virtual ~Generator()
  .. cpp:function:: static void registerNewEventCB(gen_cb_func_t func)
  .. cpp:function:: static void removeNewEventCB(gen_cb_func_t)
  .. cpp:function:: void forceEventBlock()

  .. cpp:type:: void (*gen_cb_func_t)()

  .. cpp:member:: static bool startedAnyGenerator

  .. cpp:enum:: state_t

     .. cpp:enumerator:: state_t::none
     .. cpp:enumerator:: state_t::initializing
     .. cpp:enumerator:: state_t::process_blocked
     .. cpp:enumerator:: state_t::system_blocked
     .. cpp:enumerator:: state_t::decoding
     .. cpp:enumerator:: state_t::statesync
     .. cpp:enumerator:: state_t::handling
     .. cpp:enumerator:: state_t::queueing
     .. cpp:enumerator:: state_t::error
     .. cpp:enumerator:: state_t::exiting

  .. cpp:member:: state_t state

  .. cpp:function:: static const char* generatorStateStr(state_t)
  .. cpp:function:: virtual bool isExitingState()
  .. cpp:function:: virtual void setState(state_t newstate)
  .. cpp:function:: virtual state_t getState()

.. cpp:class:: GeneratorMT : public Generator

  .. cpp:function:: void launch()
  .. cpp:function:: void start()
  .. cpp:function:: virtual void plat_start()
  .. cpp:function:: virtual bool plat_continue(ArchEvent*)
  .. cpp:function:: GeneratorMTInternals *getInternals();

  .. cpp:function:: GeneratorMT(std::string name_)
  .. cpp:function:: virtual ~GeneratorMT()
  .. cpp:function:: virtual bool processWait(bool block);
  .. cpp:function:: virtual bool getAndQueueEvent(bool block)

.. cpp:class:: GeneratorST : public Generator

  .. cpp:function:: GeneratorST(std::string name_)
  .. cpp:function:: virtual ~GeneratorST()
  .. cpp:function:: virtual bool processWait(bool block)
  .. cpp:function:: virtual bool getAndQueueEvent(bool block)
