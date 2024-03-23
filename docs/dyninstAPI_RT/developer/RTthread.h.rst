.. _`sec:RTthread.h`:

RTthread.h
##########

.. cpp:function:: dyntid_t dyn_pthread_self(void)
.. cpp:function:: int dyn_lwp_self(void)

  LWP used by the kernel identifier

.. cpp:function:: int dyn_pid_self(void)

  PID identifier representing the containing process

.. cpp:var:: extern int DYNINST_multithread_capable

.. cpp:type:: dyninst_lock_t tc_lock_t

.. code:: cpp

  #define DECLARE_TC_LOCK(l)         tc_lock_t l={0 ,(dyntid_t)-1}

.. cpp:function:: int tc_lock_init(tc_lock_t*)
.. cpp:function:: int tc_lock_lock(tc_lock_t*)
.. cpp:function:: int tc_lock_unlock(tc_lock_t*)
.. cpp:function:: int tc_lock_destroy(tc_lock_t*)

.. cpp:function:: int _am_initial_thread(dyntid_t tid)
