.. _`sec-dev:procstate.h`:

procstate.h
###########

.. cpp:namespace:: Dyninst::Stackwalker::dev

.. note::

  | TODO: isRunning(), isStopped()...
  | TODO: Bug if trying to stop an already stopped process


.. cpp:class:: ProcessState

  .. cpp:member:: protected Dyninst::PID pid
  .. cpp:member:: protected std::string exec_path
  .. cpp:member:: protected LibraryState *library_tracker
  .. cpp:member:: protected Walker *walker
  .. cpp:member:: protected static std::map<Dyninst::PID, ProcessState *> proc_map
  .. cpp:member:: protected std::string executable_path

  .. cpp:function:: protected ProcessState(Dyninst::PID pid_ = 0, std::string executable_path_ = std::string(""))
  .. cpp:function:: protected void setPid(Dyninst::PID pid_)

  .. cpp:function:: void setDefaultLibraryTracker()
  .. cpp:function:: virtual LibraryState *getLibraryTracker()

  .. cpp:function:: virtual bool preStackwalk(Dyninst::THR_ID tid)

      Allow initialization

  .. cpp:function:: virtual bool postStackwalk(Dyninst::THR_ID tid)

      Allow uninitialization


.. cpp:class:: ProcDebug : public ProcessState

  Access to StackwalkerAPI’s debugger is through the ``ProcDebug`` class,
  which inherits from the ``ProcessState`` interface.

  .. cpp:member:: protected Dyninst::ProcControlAPI::Process::ptr proc
  .. cpp:member:: protected std::set<Dyninst::ProcControlAPI::Thread::ptr> needs_resume

  .. cpp:function:: protected ProcDebug(Dyninst::ProcControlAPI::Process::ptr p)


.. cpp:class:: LibraryState

  ``LibraryState`` is a helper class for ``ProcessState`` that provides
  information about the current DSOs (libraries and executables) that are
  loaded into a process’ address space. FrameSteppers frequently use the
  LibraryState to get the DSO through which they are attempting to stack
  walk.

  Each ``Library`` is represented using a ``LibAddrPair`` object, which is
  defined as follows:

  Class providing interfaces for library tracking. Only the public query
  interfaces below are user-facing; the other public methods are callbacks
  that allow StackwalkerAPI to update its internal state.

  .. cpp:member:: protected ProcessState *procstate
  .. cpp:member:: protected std::vector<std::pair<LibAddrPair, unsigned int> > arch_libs

  .. cpp:function:: LibraryState(ProcessState *parent)

  .. cpp:function:: virtual bool getLibraryAtAddr(Address addr, LibAddrPair &lib) = 0

      Given an address ``addr`` in the target process, returns ``true`` and
      sets ``lib`` to the name and base address of the library containing
      addr. Given an address outside the target process, returns ``false``.

  .. cpp:function:: virtual bool getLibraries(std::vector<LibAddrPair> &libs, bool allow_refresh = true) = 0

      Fills ``libs`` with the libraries loaded in the target process. If
      ``allow_refresh`` is true, this method will attempt to ensure that this
      list is freshly updated via inspection of the process; if it is false,
      it will return a cached list.

  .. cpp:function:: virtual void notifyOfUpdate() = 0

      This method is called by the ``ProcessState`` when it detects a change
      in the process’ list of loaded libraries. Implementations of
      ``LibraryStates`` should use this method to refresh their lists of
      loaded libraries.

  .. cpp:function:: virtual Address getLibTrapAddress() = 0

      Some platforms that implement the System/V standard (Linux) use a trap
      event to determine when a process loads a library. A trap instruction is
      inserted into a certain address, and that trap will execute whenever the
      list of loaded libraries change.

      On System/V platforms this method should return the address where a trap
      should be inserted to watch for libraries loading and unloading. The
      ProcessState object will insert a trap at this address and then call
      notifyOfUpdate when that trap triggers.

      On non-System/V platforms this method should return 0.

  .. cpp:function:: virtual bool getLibc(LibAddrPair &lc)

      Convenience function to find the name and base address of the standard C
      runtime, if present.

  .. cpp:function:: virtual bool getLibthread(LibAddrPair &lt)

      Convenience function to find the name and base address of the standard
      thread library, if present (e.g. pthreads).

  .. cpp:function:: virtual bool getAOut(LibAddrPair &ao) = 0

      Convenience function to find the name and base address of the
      executable.

  .. cpp:function:: virtual bool updateLibsArch(std::vector<std::pair<LibAddrPair, unsigned int> > &alibs)
  
