.. _`sec:dthread.h`:

dthread.h
#########

.. cpp:namespace:: Dyninst

.. cpp:class:: DThread

  .. cpp:type:: void (*initial_func_t)(void *)
  .. cpp:type:: void dthread_ret_t
  .. cpp:member:: pthread_t thrd
  .. cpp:member:: bool live
  .. cpp:function:: static long self()
  .. cpp:function:: bool spawn(initial_func_t func, void *param)
  .. cpp:function:: bool join()
  .. cpp:function:: long id()

.. cpp:struct:: template<bool isRecursive> boost_mutex_selector

  .. cpp:type:: boost::mutex mutex

.. cpp:struct:: template<> boost_mutex_selector<true>

  .. cpp:type:: boost::recursive_mutex mutex


.. cpp:class :: template <bool isRecursive = false> Mutex : public boost_mutex_selector<isRecursive>::mutex

  .. cpp:type:: Mutex<isRecursive> type

.. cpp:class:: template <typename mutex_t = Mutex<false> > CondVar

  .. cpp:function:: CondVar(mutex_t * m = NULL)
  .. cpp:function:: CondVar(CondVar const&) = delete
  .. cpp:function:: CondVar& operator=(CondVar const&) = delete
  .. cpp:function:: CondVar(CondVar &&) = delete
  .. cpp:function:: CondVar& operator=(CondVar &&rhs) = delete
  .. cpp:function:: void unlock()
  .. cpp:function:: bool trylock()
  .. cpp:function:: void lock()
  .. cpp:function:: void signal()
  .. cpp:function:: void broadcast()
  .. cpp:function:: void wait()

.. cpp:class:: template <class Mut = Mutex<false>> ScopeLock : public boost::interprocess::scoped_lock<Mut>

  .. cpp:function:: ScopeLock(Mut &mut)
