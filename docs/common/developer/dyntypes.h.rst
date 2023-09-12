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
.. cpp:type:: unsigned long Offset
.. cpp:type:: int PID
.. cpp:type:: int PROC_HANDLE
.. cpp:type:: int LWP
.. cpp:type:: long THR_ID

.. cpp:function:: int ThrIDToTid(Dyninst::THR_ID id)

.. cpp:enum:: OSType

  .. cpp:enumerator:: OSType::OSNone
  .. cpp:enumerator:: OSType::Linux
  .. cpp:enumerator:: OSType::FreeBSD
  .. cpp:enumerator:: OSType::Windows
