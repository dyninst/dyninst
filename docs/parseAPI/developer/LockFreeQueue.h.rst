.. _`sec-dev:LockFreeQueue.h`:

LockFreeQueue.h
###############

.. cpp:class:: template<typename T> LockFreeQueueItem

  .. cpp:function:: LockFreeQueueItem(T __value)
  .. cpp:function:: void setNext(item_type *__next)
  .. cpp:function:: void setNextPending()
  .. cpp:function:: item_type *next()
  .. cpp:function:: T value()

.. cpp:class:: template<typename T> LockFreeQueueIterator

  Designed for use in a context where where only insert_chain operations
  that have completed their exchange may be concurrent with iteration on
  the queue.

  .. note:: This class models the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

  .. cpp:type:: T value_type
  .. cpp:type:: T* pointer
  .. cpp:type:: T& reference
  .. cpp:type:: std::ptrdiff_t difference_type
  .. cpp:type:: std::forward_iterator_tag iterator_category

.. cpp:class:: template<typename T> LockFreeQueue

  .. cpp:type:: LockFreeQueueIterator<T> iterator
  .. cpp:type:: LockFreeQueueItem<T> item_type

  .. cpp:function:: LockFreeQueue(item_type *_head = 0)

  .. cpp:function:: void insert(T value)

      Inserts a singleton at the head of the queue.

      .. note:: This operation is wait-free unless the allocator blocks

  .. cpp:function:: void splice(LockFreeQueue<T> &q)

    Steal the linked list from q and insert it at the front of this queue.

    Wait-free for concurrent use.

  .. cpp:function:: item_type *peek()

    Inspects the head of the queue.

    Wait-free for concurrent use.

  .. cpp:function:: item_type *steal()

      Grabs the contents of the queue for your own private use.

  .. cpp:function:: item_type *pop()

      Designed for use in a context where where only insert_chain
      operations that have completed their exchange may be concurrent.

  .. cpp:function:: iterator begin()

      Designed for use in a context where where only insert_chain
      operations that have completed their exchange may be concurrent.

  .. cpp:function:: iterator end()

      Designed for use in a context where where only insert_chain
      operations that have completed their exchange may be concurrent.

  .. cpp:function:: void clear()

      Designed for use in a context where where only insert_chain
      operations that have completed their exchange may be concurrent.
