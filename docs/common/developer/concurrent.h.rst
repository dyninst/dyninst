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


Reader/Writer Lock
******************

A simple implementation of a condition variable-based
shared lock. The algorithm is described below.

Linearization is handled via two bitfields, rin and rout, which govern the
behavior and motion of readers through the lock. These are arranged as:

  - Bit 0: PHASE. Determines which wakeup signal waiting readers use.
  - Bit 1: WRITER. If 1, a writer has the lock or is waiting for it.
  - Remaining bits: TICKET. Unique identifier for a reader.

It is assumed the number of readers will never go above ``2^14 (16384)``

Incoming readers obtain a ticket by a fetch_add on rin, and if
``WRITER`` is set will immediately wait on a condition variable rcond.
The associated mutex inlock maintains consistent access with the two wakeup
booleans, ``rwakeup[2]``, and the choice of variable is determined by the
PHASE bit of the obtained ticket.

Outgoing readers indicate their disinterest by applying a fetch_add
on rout, which gives them a a secondary "outgoing" ticket. If ``WRITER``
is set the non-atomic field last contains the ticket of the final reader.
The final reader signals a condition variable wcond to wake up one of
the writers that may be waiting. The normal mutex outlock and boolean wwakeup
ensure no wakeups are lost on the writer side.

Writers are serialized amonst each other via a single normal mutex wlock.
All arriving writers choose a final reader via fetch_xor on rin, and
check for present readers via fetch_xor on rout. If readers are present,
it then waits for a wakeup via the condition variable wcond, resetting
wwakeup after this has occured successfully. All of this is done while wlock
is held, and wlock continues to be held through the critical section.

Leaving writers first ensure outgoing readers will not attempt to
(erroneously) wake up a writer via a fetch_xor (equiv. fetch_and)
on rout, permit incoming readers via the same on rin, and then wake up
any waiting readers via a notification on rcond.

Inspired by the previous implementation, which was based on the following:

  Björn B. Brandenburg and James H. Anderson. 2010. Spin-based reader-writer
  synchronization for multiprocessor real-time systems. Real-Time Systems
  46(1):25-87 (September 2010).  http://dx.doi.org/10.1007/s11241-010-9097-2

In addition to the above, the previous authors used a simplification where
the readers span on a single shared atomic boolean. The version below
replaces this with a condition variable, which is sufficient for the
as the readers only serialize at the transitions between phases.

