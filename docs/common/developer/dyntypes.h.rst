.. _`sec:dyntypes.h`:

dyntypes.h
##########

.. cpp:namespace:: Dyninst

.. cpp:type:: \
  template <typename Key, \
        typename Value, \
        typename Hash = std::hash<Key>, \
        typename Comp = std::equal_to<Key>, \
        typename Alloc = std::allocator<std::pair<const Key, Value>>> \
  dyn_hash_map = std::unordered_map<Key, Value, Hash, Comp, Alloc>

.. cpp:type:: \
  template <typename Key, \
        typename Hash = std::hash<Key>, \
        typename Comp = std::equal_to<Key>, \
        typename Alloc = std::allocator<Key>> \
  dyn_hash_set = std::unordered_set<Key, Hash, Comp, Alloc>

Various values used throughout the codebase. These can have different underlying types on non-Unix platforms.

.. c:macro:: NULL_PID
.. c:macro:: NULL_LWP
.. c:macro:: NULL_THR_ID
.. c:macro:: DYNINST_SINGLETHREADED
.. cpp:type:: unsigned long Address

  An integer value capable of holding an address in the target process.
  Address variables should not, and in many cases cannot, be used directly
  as a pointer. It may refer to an address in a different process, and it
  may not directly match the target process’ pointer representation.
  Address is guaranteed to be at least large enough to hold an address in
  a target process, but may be larger.


.. cpp:type:: unsigned long Offset

.. cpp:type:: int PID

  A handle for identifying a process. On UNIX systems this will be an
  integer representing a PID. On Windows this will be a HANDLE object.

.. cpp:type:: int PROC_HANDLE
.. cpp:type:: int LWP

.. cpp:type:: long THR_ID

  A handle for identifying a thread. On Linux platforms this is an integer
  referring to a TID (Thread Identifier). On Windows it is a HANDLE object.

.. cpp:function:: int ThrIDToTid(Dyninst::THR_ID id)

.. cpp:enum:: OSType

  .. cpp:enumerator:: OSNone
  .. cpp:enumerator:: Linux
  .. cpp:enumerator:: FreeBSD
  .. cpp:enumerator:: Windows
