.. _`sec:InstructionCache.h`:

InstructionCache.h
##################

.. cpp:struct:: ReadWriteInfo

  .. cpp:member:: bitArray read
  .. cpp:member:: bitArray written
  .. cpp:member:: int insnSize

.. cpp:class:: InstructionCache

  .. cpp:function:; InstructionCache()
  
  Creates an empty cache.

  .. cpp:function:: bool getLivenessInfo(Address addr, ParseAPI::Function* func, ReadWriteInfo& rw)
  
  Returns the liveness information.
  
  .. danger:: ``rw`` is invalidated after calling :cpp:func:`clean`.
  
  .. cpp:function:: void insertInstructionInfo(Address addr, ReadWriteInfo rw, ParseAPI::Function* func)
  
  Store the liveness information in ``rw`` in the cache for the instruction in function ``func`` at address ``addr``.
  
  .. Warning:: Implicitly calls :cpp:func:`clean`.
  
  .. cpp:function:: void clean()
  
  Clears the cache.
  
  .. cpp:function:: ParseAPI::Function* getCurFunc()

  Returns the current function.
