.. _`sec-dev:ProcessSet.h`:

ProcesssSet.h
=============

.. cpp:namespace:: Dyninst::ProcControlAPI::dev

.. cpp:type:: pair<Address, Process::ptr> apair_t

.. cpp:type:: std::multimap<Dyninst::Address, Dyninst::ProcControlAPI::Process::ptr> int_addressSet
.. cpp:type:: std::set<Dyninst::ProcControlAPI::Thread::ptr> int_threadSet
.. cpp:type:: std::set<Dyninst::ProcControlAPI::Process::ptr> int_processSet


.. cpp:class:: ProcessSet : public boost::enable_shared_from_this<ProcessSet>

  .. cpp:function:: int_processSet* getIntProcessSet()
  .. cpp:function:: int_addressSet* get_iaddrs()

.. cpp:class:: ThreadSet : public boost::enable_shared_from_this<ThreadSet>

  .. cpp:function:: int_threadSet* getIntThreadSet() const


.. cpp:function:: static Process::const_ptr get_proc(const T &i, err_t *)

  This is a bundle of hackery is a wrapper around iterators that
  checks processes for common errors.  Many of the functions that implement
  ProcessSet and ThreadSet have do an operation where they iterate over
  a collection, pull some ``Process::ptr`` or ``Thread::ptr`` out of that collection,
  check it for common errors (e.g, operating on a dead process), then do some
  real work. These classes/templates are an attempt to bring all that error checking
  and iteration into a common place.

  Things are complicated by the fact that we use many different types of collections
  We might be operating over sets of ``Process::ptrs``, or multimaps from ``Thread::ptr`` to
  register values, etc.  The common iteration and error handling code is in the iter_t
  template, which takes the type of collection it's iterating over as template parameters.

  The three big operations iter_t needs to do is extract a process from an iterator and
  get the begin/end iterators from a collection.  These operations are done by a set
  of overloaded functions: get_proc, get_begin, and get_end.  We have an instance of these
  for each type of collection we deal with.
