.. _`sec:pool_allocators.h`:

pool_allocators.h
#################

.. cpp:class:: template <typename T> unlocked_fast_alloc

  .. cpp:type:: boost::fast_pool_allocator<T, boost::default_user_allocator_new_delete, boost::details::pool::null_mutex> type

.. cpp:class:: template <typename T> unlocked_pool_alloc

  .. cpp:type:: boost::pool_allocator<T, boost::default_user_allocator_new_delete, boost::details::pool::null_mutex> type

.. cpp:class:: template <typename T> pooled_set

  .. cpp:type:: std::set<T, typename std::less<T>, typename unlocked_fast_alloc<T>::type> type

.. cpp:class:: template <typename T> pooled_vector

  .. cpp:type:: std::vector<T, typename unlocked_pool_alloc<T>::type> type
