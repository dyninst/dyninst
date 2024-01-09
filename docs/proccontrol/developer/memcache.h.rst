.. _`sec:memcache.h`:

memcache.h
##########

.. warning::

  DON'T USE THE MEMCACHE FOR ARBITRARY MEMORY OPERATIONS!

  This class is meant to assist the SysV parser and the
  thread_db parser--where we're wrapping another library
  that is not async aware.

  The semantics are such that this class expects that
  reads and writes may be restarted, and will cache
  and not redo operations it has already seen.  This
  makes a difference when you do operations such as:

  | write 'A' -> 0x1000
  | write 'B' -> 0x1000
  | write 'A' -> 0x1000

  Under this class 0x1000 will contain 'B' after these three
  operations.  The second 'A' write would be detected to be a
  duplicated of the first and dropped.  We really do want these
  semantics if we're restarting operations (we expect a write 'B'
  will follow the second write 'A'), but this is inappropriate
  for general purpose use.

  Update - The memcache can now store registers.  Just what
  every memcache needs.


.. cpp:enum:: async_ret_t

  .. cpp:enumerator:: aret_error = 0
  .. cpp:enumerator:: aret_success
  .. cpp:enumerator:: aret_async

.. cpp:enum:: token_t

  .. cpp:enumerator:: token_none = 0
  .. cpp:enumerator:: token_getmsg
  .. cpp:enumerator:: token_seteventreporting
  .. cpp:enumerator:: token_setevent
  .. cpp:enumerator:: token_init

.. cpp:class:: memEntry

  .. cpp:function:: memEntry()
  .. cpp:function:: memEntry(token_t token)
  .. cpp:function:: memEntry(Dyninst::Address remote, void *local, unsigned long size, bool is_read, memCache *cache)
  .. cpp:function:: memEntry(const memEntry *me, char* b)
  .. cpp:function:: memEntry& operator=(const memEntry&) = delete
  .. cpp:function:: memEntry(const memEntry&) = delete
  .. cpp:function:: ~memEntry()
  .. cpp:function:: Dyninst::Address getAddress() const
  .. cpp:function:: char *getBuffer() const
  .. cpp:function:: unsigned long getSize() const
  .. cpp:function:: bool isRead() const
  .. cpp:function:: bool isWrite() const
  .. cpp:function:: bool isToken() const
  .. cpp:function:: bool operator==(const memEntry &b) const


.. cpp:class:: memCache

  .. cpp:function:: memCache(int_process *p)
  .. cpp:function:: ~memCache()
  .. cpp:function:: async_ret_t readMemory(void *dest, Dyninst::Address src, unsigned long size,  std::set<mem_response::ptr> &resps, int_thread *thrd = NULL)
  .. cpp:function:: async_ret_t writeMemory(Dyninst::Address dest, void *src, unsigned long size,  std::set<result_response::ptr> &resps, int_thread *thrd = NULL)
  .. cpp:function:: async_ret_t getRegisters(int_thread *thr, int_registerPool &pool)
  .. cpp:function:: void startMemTrace(int &record)
  .. cpp:function:: void clear()
  .. cpp:function:: bool hasPendingAsync()
  .. cpp:function:: void getPendingAsyncs(std::set<response::ptr> &resps)
  .. cpp:function:: void setSyncHandling(bool b)
  .. cpp:function:: void markToken(token_t tk)
  .. cpp:function:: void condense()