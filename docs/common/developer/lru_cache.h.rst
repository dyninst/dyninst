.. _`sec:lru_cache.h`:

lru_cache.h
###########

.. cpp:class:: template<class K, class V> LRUCache

  **Least-recently used cache**

  .. cpp:type:: int (*lru_hash_func)(K key)
  .. cpp:function:: LRUCache(int initial_size, lru_hash_func f)
  .. cpp:function:: void insert(K key, V value)
  .. cpp:function:: bool lookup(K key, V &value)
