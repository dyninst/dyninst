.. _`sec:concurrent.h`:

concurrent.h
############

.. cpp:namespace:: Dyninst::concurrent

.. cpp:class:: template<typename Key> hash_compare<false, Key>

  Old-style tbb_hash_compare concept

.. cpp:class:: template<typename Key> hash_compare<true, Key>

  New-style tbb_hash_compare concept (``TBB_VERSION_MAJOR >= 2021``)

.. cpp:var:: constexper auto v = TBB_VERSION_MAJOR >= 2021

.. cpp:class:: template<typename K, typename V> dyn_c_hash_map : \
               protected tbb::concurrent_hash_map<K, V, concurrent::detail::hash_compare<v, K>>

  .. cpp:type:: template<typename T> dyn_c_vector = tbb::concurrent_vector<T, std::allocator<T>>
  .. cpp:type:: template<typename T> dyn_c_queue = tbb::concurrent_queue<T, std::allocator<T>>

.. cpp:class:: dyn_mutex : public boost::mutex

.. cpp:class:: dyn_rwlock

  .. cpp:type:: unique_lock = boost::unique_lock<dyn_rwlock>

  .. cpp:type:: shared_lock = boost::shared_lock<dyn_rwlock>

  .. cpp:function:: void lock_shared()
  .. cpp:function:: void unlock_shared()
  .. cpp:function:: void lock()
  .. cpp:function:: void unlock()


.. cpp:class:: dyn_thread

  .. cpp:function:: dyn_thread()
  .. cpp:function:: unsigned int getId()
  .. cpp:function:: static unsigned int threads()
  .. cpp:function:: operator unsigned int()

  .. cpp:member:: static thread_local dyn_thread me

.. cpp:class:: template<typename T> dyn_threadlocal

  .. cpp:function:: dyn_threadlocal()
  .. cpp:function:: T get()
  .. cpp:function:: void set(const T& val)
