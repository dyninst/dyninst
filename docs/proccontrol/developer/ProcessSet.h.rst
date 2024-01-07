.. _`sec-dev:ProcessSet.h`:

ProcesssSet.h
=============

.. cpp:namespace:: Dyninst::ProcControlAPI::dev

.. cpp:type:: std::multimap<Dyninst::Address, Dyninst::ProcControlAPI::Process::ptr> int_addressSet
.. cpp:type:: std::set<Dyninst::ProcControlAPI::Thread::ptr> int_threadSet
.. cpp:type:: std::set<Dyninst::ProcControlAPI::Process::ptr> int_processSet


.. cpp:class:: ProcessSet : public boost::enable_shared_from_this<ProcessSet>

  .. cpp:function:: int_processSet* getIntProcessSet()
  .. cpp:function:: int_addressSet* get_iaddrs()

.. cpp:class:: ThreadSet : public boost::enable_shared_from_this<ThreadSet>

  .. cpp:function:: int_threadSet* getIntThreadSet() const
