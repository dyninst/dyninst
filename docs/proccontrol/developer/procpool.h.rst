.. _`sec:procpool.h`:

procpool.h
##########

.. cpp:class:: ProcessPool

  .. cpp:member:: protected std::set<Dyninst::LWP> deadThreads
  .. cpp:member:: protected std::map<Dyninst::PID, int_process *> procs
  .. cpp:member:: protected std::map<Dyninst::LWP, int_thread *> lwps
  .. cpp:function:: protected ProcessPool()
  .. cpp:member:: protected CondVar<> var
  .. cpp:function:: ~ProcessPool()
  .. cpp:type:: bool(*ifunc)(int_process *, void *data)
  .. cpp:function:: int_process *findProcByPid(Dyninst::PID pid)
  .. cpp:function:: void addProcess(int_process *proc)
  .. cpp:function:: void addThread(int_process *proc, int_thread *thr)
  .. cpp:function:: void rmProcess(int_process *proc)
  .. cpp:function:: void rmThread(int_thread *thr)
  .. cpp:function:: int_thread *findThread(Dyninst::LWP lwp)
  .. cpp:function:: bool deadThread(Dyninst::LWP lwp)

    On Linux, we can get notifications for dead threads.

  .. cpp:function:: void addDeadThread(Dyninst::LWP lwp)
  .. cpp:function:: void removeDeadThread(Dyninst::LWP lwp)
  .. cpp:function:: unsigned numProcs()
  .. cpp:function:: bool LWPIDsAreUnique()
  .. cpp:function:: bool for_each(ifunc f, void *data = NULL)
  .. cpp:function:: CondVar<> *condvar()