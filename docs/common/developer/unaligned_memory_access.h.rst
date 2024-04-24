.. _`sec:unaligned_memory_access.h`:

unaligned_memory_access.h
#########################

Routines to access data at an arbitrary memory location (that is possibly
unaligned for the data type) via ``char*``\-like pointer with no alignment
restrictions.  The set of functions range from safe read and write of memory
cast to a type; to functions that cast to a pointer with more restrictive
alignment (caller is responsible for addr's alignment correctness).

C++
===

.. cpp:namespace:: Dyninst

.. cpp:function:: template <typename ResultType> ResultType read_memory_as(const void *addr)

  Returns an object of ``TYPE`` created by reading ``sizeof(TYPE)`` bytes at ``addr``.

  The address ``addr`` does not have to be properly aligned for the type.

.. cpp:function:: template <typename DataType> void write_memory_as(void *addr, const DataType &data)

  Writes ``sizeof(TYPE)`` bytes to the memory pointed to by ``addr`` by copying the
  bytes of the supplied parameter.

  The address ``addr`` does not have to be properly aligned for the type.

.. cpp:function:: template <typename PointerType, typename DataType> void append_memory_as(PointerType *&addr, const DataType &data)

  Writes ``sizeof(TYPE)`` bytes to the memory pointed to by ``addr`` by copying the
  bytes of the supplied parameter, and adjusts ``addr`` by ``sizeof(TYPE)``.

  The address ``addr`` does not have to be properly aligned for the type.

.. cpp:function:: template <typename DataType> DataType* alignas_cast(const void *addr)

  Converts the ``addr`` pointer to ``TYPE*`` in a way that avoids triggering a warning that the
  alignment of the new pointer type is larger than the current pointer type.

  .. WARNING::
  
    The caller is responsible to ensure that the alignment is correct
    if the pointer is dereferenced.  Uncomment the assert to audit proper
    alignment of addr.
  
    Use :cpp:func:`read_memory_as` or :cpp:func:`write_memory_as` if possible and definitely if the
    alignment of addr can not be guaranteed.

.. cpp:function template <typename DataType> DataType* alignas_cast(void *addr)

.. cpp:namespace-pop::

C
=

.. c:macro:: CAST_WITHOUT_ALIGNMENT_WARNING(toType, addr)

  C language macro that casts the expression addr to type toType
  without producing a warning that alignment of the new pointer type is
  larger than the current pointer type.

  .. warning::
  
    The caller is responsible to ensure that the alignment is correct or use memcpy if the pointer is dereferenced.
