.. _`sec:ProcessSet.h`:

ProcessSet.h
============

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:class:: ProcessSet : public boost::enable_shared_from_this<ProcessSet>

  **A container for multiple processes**

  By grouping processes together, collective operations can be performed on the entire
  set in a single action which may be more efficient than the equivalent sequential operations.

  .. cpp:type:: boost::shared_ptr<ProcessSet> ptr
  .. cpp:type:: boost::shared_ptr<const ProcessSet> const_ptr

  .. cpp:function:: static ProcessSet::ptr newProcessSet()

      Creates an empty set.

  .. cpp:function:: static ProcessSet::ptr newProcessSet(Process::const_ptr p)

      Creates a set containing only ``p``.

  .. cpp:function:: static ProcessSet::ptr newProcessSet(ProcessSet::const_ptr pp)

      Creates a duplicate of ``pp``.

  .. cpp:function:: static ProcessSet::ptr newProcessSet(const std::set<Process::const_ptr> &procs)

      Creates a new set containing all processes in ``procs``.

  .. cpp:function:: static ProcessSet::ptr newProcessSet(AddressSet::const_iterator begin, AddressSet::const_iterator end)

      Creates a new set of processes from the open range ``[begin, end)``.

  .. cpp:function:: static ProcessSet::ptr newProcessSet(Process::ptr p)

      Creates a set containing only ``p``.

  .. cpp:function:: static ProcessSet::ptr newProcessSet(ProcessSet::ptr pp)

      Creates a duplicate of ``pp``.

  .. cpp:function:: static ProcessSet::ptr newProcessSet(const std::set<Process::ptr> &procs)

      Creates a new set containing all processes in ``procs``.

  .. cpp:function:: static ProcessSet::ptr createProcessSet(std::vector<CreateInfo> &cinfo)

      Creates a new set by launching new processes.

      Each element in ``cinfo`` specifies an executable, arguments, environment and
      file descriptor mappings (with similar semantics to
      :cpp:func:`Process::createProcess`) which are used to launch a new process.

      Every successfully created Process will be added to a new ProcessSet
      that is returned by this function. Each element in ``cinfo`` is updated
      so that each entry’s proc field points to the Process created by that entry,
      and the error_ret entry will contain an error code for any process launch that failed.

  .. cpp:function:: static ProcessSet::ptr attachProcessSet(std::vector<AttachInfo> &ainfo)

      Creates a new ProcessSet by attaching to the existing processes in ``ainfo``.

      Each element in ainfo specifies a PID and executable (with similar semantics to
      :cpp:func:`Process::attachProcess`), which are used to attach to the processes.

      Every successfully attached Process will be added to a new ProcessSet
      that is returned by this function. Each element in ``ainfo`` is updated so that each
      entry’s proc field points to the Process attached by that entry, and the error_ret
      entry will contain an error code any process attach that failed.

  .. cpp:function:: ProcessSet::ptr set_union(ProcessSet::ptr pp) const

      Creates a set containing the elements in this set unioned with the elements of ``pp``,
      including the elements in their intersection.

  .. cpp:function:: ProcessSet::ptr set_intersection(ProcessSet::ptr pp) const

      Creates a set containing the elements in this set intersected with the elements of ``pp``.

  .. cpp:function:: ProcessSet::ptr set_difference(ProcessSet::ptr pp) const

      Creates a set containing the elements in this set, but do not exist in ``pp``.

  .. cpp:function:: iterator begin()

      Models the C++ `Container <https://en.cppreference.com/w/cpp/named_req/Container>`_ concept.

  .. cpp:function:: iterator end()

      Models the C++ `Container <https://en.cppreference.com/w/cpp/named_req/Container>`_ concept.

  .. cpp:function:: const_iterator begin() const

      Models the C++ `Container <https://en.cppreference.com/w/cpp/named_req/Container>`_ concept.

  .. cpp:function:: const_iterator end() const

      Models the C++ `Container <https://en.cppreference.com/w/cpp/named_req/Container>`_ concept.

  .. cpp:function:: iterator find(Process::const_ptr p)

      Searches for the process ``p``.

  .. cpp:function:: iterator find(Dyninst::PID p)

      Searches for the process with system-dependent ID ``p``.

  .. cpp:function:: const_iterator find(Process::const_ptr p) const

      Searches for the process ``p``.

  .. cpp:function:: const_iterator find(Dyninst::PID p) const

      Searches for the process ``p``.

  .. cpp:function:: bool empty() const

      Checks if this container contains no elements.

  .. cpp:function:: size_t size() const

      Returns the number of elements in this set (it's cardinality).

  .. cpp:function:: std::pair<iterator, bool> insert(Process::const_ptr p)

      Inserts ``p`` into this container, if not already present.

      Returns a pair consisting of an iterator to the inserted element (or to the element
      that prevented the insertion) and a ``bool`` value set to true if and only if the
      insertion took place.

  .. cpp:function:: void erase(iterator pos)

      Removes the element in this set equal to the value pointed to by ``pos``, if present.

  .. cpp:function:: size_t erase(Process::const_ptr p)

      Removes the element in this set equal to the value pointed to by ``pos``, if present.

      Returns the number of elements removed (either 0 or 1).

  .. cpp:function:: void clear()

      Removes all elements from this container.

  .. cpp:function:: ProcessSet::ptr getErrorSubset() const

      Return the subset of processes that had any error on the last operation, or
      groups them into subsets based on unique error codes.

      Error codes are reset on every ``ProcessSet`` call, so this function shows which
      processes had an error on the last set operation.

  .. cpp:function:: void getErrorSubsets(std::map<ProcControlAPI::err_t, ProcessSet::ptr> &err_sets) const

      Returns all processes in this set that encountered an error, grouped by error code.

  .. cpp:function:: bool anyTerminated() const

      Checks if this set contains *any* process in the ``terminated`` state.

  .. cpp:function:: bool anyExited() const

      Checks if this set contains *any* process in the ``exited`` state.

  .. cpp:function:: bool anyCrashed() const

      Checks if this set contains *any* process in the ``crashed`` state.

  .. cpp:function:: bool anyDetached() const

      Checks if this set contains *any* process in the ``detached`` state.

  .. cpp:function:: bool anyThreadStopped() const

      Checks if this set contains *any* process with a thread in the ``stopped`` state.

  .. cpp:function:: bool anyThreadRunning() const

      Checks if this set contains *any* process with a thread in the ``running`` state.

  .. cpp:function:: bool allTerminated() const

      Checks if *all* processes in the this set are in the ``terminated`` state.

  .. cpp:function:: bool allExited() const

      Checks if *all* processes in the this set are in the ``exited`` state.

  .. cpp:function:: bool allCrashed() const

      Checks if *all* processes in the this set are in the ``crashed`` state.

  .. cpp:function:: bool allDetached() const

      Checks if *all* processes in the this set are in the ``detached`` state.

  .. cpp:function:: bool allThreadsStopped() const

      Checks if *any* process in the this set that has all of its threads in the ``stopped`` state.

  .. cpp:function:: bool allThreadsRunning() const

      Checks if *any* process in the this set that has all of its threads in the ``running`` state.

  .. cpp:function:: ProcessSet::ptr getTerminatedSubset() const

      Returns the subset of process with *any* process in the ``terminated`` state.

  .. cpp:function:: ProcessSet::ptr getExitedSubset() const

      Returns the subset of process with *any* process in the ``exited`` state.

  .. cpp:function:: ProcessSet::ptr getCrashedSubset() const

      Returns the subset of process with *any* process in the ``crashed`` state.

  .. cpp:function:: ProcessSet::ptr getDetachedSubset() const

      Returns the subset of process with *any* process in the ``detached`` state.

  .. cpp:function:: ProcessSet::ptr getAllThreadRunningSubset() const

      Returns the subset of process with *all* of its threads in the ``running`` state.

  .. cpp:function:: ProcessSet::ptr getAnyThreadRunningSubset() const

      Returns the subset of process with *any* of its threads in the ``running`` state.

  .. cpp:function:: ProcessSet::ptr getAllThreadStoppedSubset() const

      Returns the subset of process with *all* of its threads in the ``stopped`` state.

  .. cpp:function:: ProcessSet::ptr getAnyThreadStoppedSubset() const

      Returns the subset of process with *any* of its threads in the ``stopped`` state.

  .. cpp:function:: bool continueProcs() const

      Puts all processes in this set in the ``running`` state.

  .. cpp:function:: bool stopProcs() const

      Puts all processes in this set in the ``stopped`` state.

  .. cpp:function:: bool detach(bool leaveStopped = false) const

      Puts all processes in this set in the ``detached`` state.

      If ``leaveStopped`` is ``true`` and all processes in this set are stopped,
      then the processes will be left in a stopped state after the detach.

  .. cpp:function:: bool terminate() const

      Puts all processes in this set in the ``terminated`` state.

  .. cpp:function:: bool temporaryDetach() const

      Temporarily puts all processes in this set in the ``detached`` state.

  .. cpp:function:: bool reAttach() const

      Undoes :cpp:func:`detach`.

  .. cpp:function:: AddressSet::ptr mallocMemory(size_t size) const

      Allocates ``size`` bytes in all processes in this set.

      .. Attention:: It is the user’s responsibility to free the memory returned.

      .. Warning:: This behavior is undefined if ``addrs`` contains processes not included in this set.

      Returns the addresses of the allocations.

  .. cpp:function:: bool mallocMemory(size_t size, AddressSet::ptr location) const

      Allocates ``size`` bytes in all processes in ``location``.

      .. Warning:: This behavior is undefined if ``addrs`` contains processes not included in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool freeMemory(AddressSet::ptr addrs) const

      Frees memory allocated by :cpp:func:`Process::mallocMemory` or
      :cpp:func:`ProcessSet::mallocMemory` at the addresses in ``addrs``.

      .. Warning:: This behavior is undefined if ``addrs`` contains processes not included in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool readMemory(AddressSet::ptr addr, std::multimap<Process::ptr, void *> &result, size_t size) const

      Reads ``size`` bytes of memory at the address in ``addrs``.

      The memory read is returned ``result``.

      .. Attention:: It is the user’s responsibility to free the memory returned.

      .. Warning:: This behavior is undefined if ``addrs`` contains processes not included in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool readMemory(std::multimap<Process::const_ptr, read_t> &addr)

      Reads ``size`` bytes of memory from the processes in ``addr``. The remote address, read size,
      and local buffer are taken from each ``read_t``.

      .. Warning:: This behavior is undefined if ``addrs`` contains processes not included in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool writeMemory(AddressSet::ptr addr, const void *buffer, size_t size) const

      Writes ``size`` bytes of ``buffer`` into the memory of each process in ``addrs``.

      .. Warning:: This behavior is undefined if ``addrs`` contains processes not included in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool writeMemory(std::multimap<Process::const_ptr, write_t> &addrs) const

      Writes to the memory of each process in ``addrs``. The local memory buffer, buffer size, and target
      location are specified are taken from each ``write_t``.

      .. Warning:: This behavior is undefined if ``addrs`` contains processes not included in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool addBreakpoint(AddressSet::ptr as, Breakpoint::ptr bp) const

      Inserts the breakpoint ``bp`` into each process and at each address in ``as``.

      .. Warning:: This behavior is undefined if ``as`` contains processes not included in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool rmBreakpoint(AddressSet::ptr as, Breakpoint::ptr bp) const

      Removes the breakpoint ``bp`` into each process and at each address in ``as``.

      .. Warning:: This behavior is undefined if ``as`` contains processes not included in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool postIRPC(const std::multimap<Process::const_ptr, IRPC::ptr>& rpcs) const

      Posts the IRPCs in ``rpcs`` to their associated processes.

      It is similar to :cpp:func:`Process::postIRPC`.

      .. Warning:: This behavior is undefined if ``rpcs`` contains processes not included in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool postIRPC(IRPC::ptr irpc, std::multimap<Process::ptr, IRPC::ptr>* result = NULL)

      Copies ``irpc`` into each process in this set and posts it to that process.

      If ``result`` is provided, then it gets filled with each new IRPC and the Process to which it
      was posted.

      Returns ``false`` on error.

  .. cpp:function:: bool postIRPC(IRPC::ptr irpc, AddressSet::ptr addrs, std::multimap<Process::ptr, IRPC::ptr>* result = NULL)

      Copies ``irpc`` into each process in this set and posts it to that process at the corresponding address in ``addrs``.

      .. Warning:: This behavior is undefined if ``addrs`` contains processes not included in this set.

      Returns ``false`` on error.

  .. cpp:function:: LibraryTrackingSet *getLibraryTracking()
  .. cpp:function:: ThreadTrackingSet *getThreadTracking()
  .. cpp:function:: LWPTrackingSet *getLWPTracking()
  .. cpp:function:: FollowForkSet *getFollowFork()
  .. cpp:function:: RemoteIOSet *getRemoteIO()
  .. cpp:function:: MemoryUsageSet *getMemoryUsage()
  .. cpp:function:: const LibraryTrackingSet *getLibraryTracking() const
  .. cpp:function:: const ThreadTrackingSet *getThreadTracking() const
  .. cpp:function:: const LWPTrackingSet *getLWPTracking() const
  .. cpp:function:: const FollowForkSet *getFollowFork() const
  .. cpp:function:: const RemoteIOSet *getRemoteIO() const
  .. cpp:function:: const MemoryUsageSet *getMemoryUsage() const

.. cpp:struct:: ProcessSet::write_t

  Uses the :cpp:class:`AddressSet` forms to write from the same memory location in each process.
  Uses the write_t form to write from different memory locations/sizes in each process
  The :cpp:func:`ProcessSet::readMemory` that writes groups of processes based on having the same memory contents.

  .. cpp:member:: void *buffer
  .. cpp:member:: Dyninst::Address addr
  .. cpp:member:: size_t size
  .. cpp:member:: err_t err

  .. cpp:function:: bool operator<(const write_t &w)

.. cpp:struct:: ProcessSet::read_t

  Uses the :cpp:class:`AddressSet` forms to read from the same memory location in each process.
  Uses the read_t form to read from different memory locations/sizes in each process.
  The :cpp:class:`AddressSet` forms of :cpp:func:`ProcessSet::readMemory` need to have their memory free'd by the user.

  .. cpp:member:: Dyninst::Address addr
  .. cpp:member:: void *buffer
  .. cpp:member:: size_t size
  .. cpp:member:: err_t err

  .. cpp:function:: bool operator<(const read_t &w)

.. cpp:struct:: ProcessSet::iterator

  Models the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

.. cpp:struct:: ProcessSet::const_iterator

  Models the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

.. cpp:struct:: ProcessSet::CreateInfo

  **Creates new ProcessSets by attaching/creating new Process objects**

  .. cpp:member:: std::string executable
  .. cpp:member:: std::vector<std::string> argv
  .. cpp:member:: std::vector<std::string> envp
  .. cpp:member:: std::map<int, int> fds
  .. cpp:member:: ProcControlAPI::err_t error_ret
  .. cpp:member:: Process::ptr proc

.. cpp:struct:: ProcessSet::AttachInfo

  .. cpp:member:: Dyninst::PID pid
  .. cpp:member:: std::string executable
  .. cpp:member:: ProcControlAPI::err_t error_ret
  .. cpp:member:: Process::ptr proc


.. cpp:class:: AddressSet

  **A set of process/address pairs**

  It's is used by :cpp:class:`ProcessSet` and :cpp:class:`ThreadSet` for
  performing group operations on large numbers of processes. It
  might, for example, represent the location of a symbol across numerous
  processes, or the location of a buffer in each process where data can be
  written or read.

  It's essentially a ``std::multimap`` of ``Address -> Process::ptr`` plus some additional features:

    - Ability to create addresses based on ProcControlAPI objects, such as libraries.
    - Additional range features to make it easier to group addresses
    - No duplicates of ``Address``, ``Process:ptr`` pairs are allowed, though there are duplicate ``Address`` keys.

  .. cpp:type:: boost::shared_ptr<AddressSet> ptr
  .. cpp:type:: boost::shared_ptr<AddressSet> const_ptr

  .. cpp:function:: static AddressSet::ptr newAddressSet()

      Returns an empty set.

  .. cpp:function:: static AddressSet::ptr newAddressSet(ProcessSet::const_ptr ps, Dyninst::Address addr)

      Returns a set of processes from ``ps`` all at address ``addr``.

  .. cpp:function:: static AddressSet::ptr newAddressSet(ProcessSet::const_ptr ps, std::string library_name, Dyninst::Offset off)

      Returns a set of processes from ``ps`` with addresses calculated by looking up the load address of ``library_name``
      in each process relative to offset ``off``.

  .. cpp:function:: iterator begin()

      Models the C++ `Container <https://en.cppreference.com/w/cpp/named_req/Container>`_ concept.

  .. cpp:function:: const_iterator begin() const

      Models the C++ `Container <https://en.cppreference.com/w/cpp/named_req/Container>`_ concept.

  .. cpp:function:: iterator end()

      Models the C++ `Container <https://en.cppreference.com/w/cpp/named_req/Container>`_ concept.

  .. cpp:function:: const_iterator end() const

      Models the C++ `Container <https://en.cppreference.com/w/cpp/named_req/Container>`_ concept.

  .. cpp:function:: iterator find(Dyninst::Address addr)

  .. cpp:function:: const_iterator find(Dyninst::Address addr) const

      These functions return an iterator that points to the first element in
      the AddressSet with an address of addr. They return end() if no element
      matches addr.

  .. cpp:function:: iterator find(Dyninst::Address addr, Process::const_ptr proc)

      Searches the address/proc pair ``addr``/``proc``.

  .. cpp:function:: const_iterator find(Dyninst::Address addr, Process::const_ptr proc) const

      Searches the address/proc pair ``addr``/``proc``.

  .. cpp:function:: size_t count(Dyninst::Address addr) const

      Returns the number of elements with address ``addr``.

  .. cpp:function:: size_t size() const

      Returns the number of elements in the set (it's cardinality).

  .. cpp:function:: bool empty() const

      Returns true if the AddressSet has zero elements and false otherwise.

  .. cpp:function:: std::pair<iterator, bool> insert(Dyninst::Address addr, Process::const_ptr proc)

      This function inserts a new element into the AddressSet with addr and
      proc as its values. If another element with those values already exists,
      then no new element will be inserted. It returns an iterator that points
      to the new or existing element and a boolean value that is true if a new
      element was inserted and false otherwise.

  .. cpp:function:: size_t insert(Dyninst::Address addr, ProcessSet::const_ptr ps)

      For every element in ps, this function inserts it and addr into the
      AddressSet. It returns the number of new elements created.

  .. cpp:function:: void erase(iterator pos)

      This function removes the element pointed to by pos from the AddressSet.

  .. cpp:function:: size_t erase(Process::const_ptr proc)

      This function removes every element with a process of proc from the
      AddressSet. It returns the number of elements removed.

  .. cpp:function:: size_t erase(Dyninst::Address addr, Process::const_ptr proc)

      This function removes any element that has and address and process of
      addr and proc from the AddressSet. It returns the number of elements
      removed.

  .. cpp:function:: void clear()

      This function erases all elements from the AddressSet leaving an
      AddressSet of size zero.

  .. cpp:function:: iterator lower_bound(Dyninst::Address addr)

      Returns an iterator pointing to the first element in the
      AddressSet that has an address greater than or equal to addr.

  .. cpp:function:: iterator upper_bound(Dyninst::Address addr)

      Returns an iterator pointing to the first element in the
      AddressSet that has an address greater than addr.

  .. cpp:function:: std::pair<iterator, iterator> equal_range(Address addr) const

      Returns a pair of iterators. The first iterator has the
      same value as the return of lower_bound(addr) and the second iterator
      has the same value as the return of upper_bound(addr).

  .. cpp:function:: AddressSet::ptr set_union(AddressSet::const_ptr aset)

      Returns a new AddressSet whose elements are the set union
      of this AddressSet and aset.

  .. cpp:function:: AddressSet::ptr set_intersection(AddressSet::const_ptr aset)

      Returns a new AddressSet whose elements are the set
      intersection of this AddressSet and aset.

  .. cpp:function:: AddressSet::ptr set_difference(AddressSet::const_ptr aset)

      Returns a new AddressSet whose elements are the set
      difference of this AddressSet minus aset.


.. cpp:class:: ThreadSet : public boost::enable_shared_from_this<ThreadSet>
  
  **A set of threads**

  It has similar operations as :cpp:class:`Thread`, and operations done on a
  ThreadSet affect every thread in that set. On some systems, using a ThreadSet
  may be more efficient when doing the same operation across a large number of threads.

  .. cpp:type:: boost::shared_ptr<ThreadSet> ptr
  .. cpp:type:: boost::shared_ptr<const ThreadSet> const_ptr

  .. cpp:function:: static ThreadSet::ptr newThreadSet()

      Creates an empty set.

  .. cpp:function:: static ThreadSet::ptr newThreadSet(Thread::ptr thr)

      Creates a set that contains only ``thr``.

  .. cpp:function:: static ThreadSet::ptr newThreadSet(const ThreadPool &threadp)

      Creates a set that contains all of the threads in ``threadp``.

  .. cpp:function:: static ThreadSet::ptr newThreadSet(const std::set<Thread::const_ptr> &thrds)

      Creates a set that contains all of the threads in ``thrds``.

  .. cpp:function:: static ThreadSet::ptr newThreadSet(ProcessSet::ptr pset)

      Creates a set that contains every **live** thread in every process in ``pset``.

  .. cpp:function:: ThreadSet::ptr set_union(ThreadSet::ptr tset) const

      Creates a set containing the elements in this set unioned with the elements of ``tset``,
      including the elements in their intersection.

  .. cpp:function:: ThreadSet::ptr set_intersection(ThreadSet::ptr tset) const

      Creates a set containing the elements in this set intersected with the elements of ``tset``.

  .. cpp:function:: ThreadSet::ptr set_difference(ThreadSet::ptr tset) const

      Creates a set containing the elements in this set, but do not exist in ``tset``.

  .. cpp:function:: iterator begin()

      Models the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

  .. cpp:function:: const_iterator begin() const

      Models the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

  .. cpp:function:: iterator end()

      Models the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

  .. cpp:function:: const_iterator end() const

      Models the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

  .. cpp:function:: iterator find(Thread::const_ptr thr)

      Searches for the thread ``thr``.

  .. cpp:function:: const_iterator find(Thread::const_ptr thr) const

      Searches for the thread ``thr``.

  .. cpp:function:: bool empty() const

      Checks if this container contains no elements.

  .. cpp:function:: size_t size() const

      Returns the number of elements in this set.

  .. cpp:function:: std::pair<iterator, bool> insert(Thread::const_ptr thr)

      Inserts ``thr`` into this container, if not already present.

      Returns a pair consisting of an iterator to the inserted element (or to the element
      that prevented the insertion) and a ``bool`` value set to true if and only if the
      insertion took place.

  .. cpp:function:: void erase(iterator pos)

      Removes the element in this set equal to the value pointed to by ``pos``, if present.

  .. cpp:function:: size_t erase(Thread::const_ptr thr)

      Removes the element in this set equal to the value pointed to by ``pos``, if present.

      Returns the number of elements removed (either 0 or 1).

  .. cpp:function:: void clear()

      Removes all elements from this container.

  .. cpp:function:: ThreadSet::ptr getErrorSubset() const

      Returns the subset of threads that had any error on the last operation.

      Error codes are reset on ``ThreadSet`` call, so this function shows which
      threads had an error on the last set operation.

  .. cpp:function:: void getErrorSubsets(std::map<ProcControlAPI::err_t, ThreadSet::ptr> &err) const

      Returns all threads in this set that encountered an error, grouped by error code.

  .. cpp:function:: bool allStopped() const

      Checks if *all* threads in this set are in the ``stopped`` state.

  .. cpp:function:: bool anyStopped() const

      Checks if *any* threads in this set are in the ``stopped`` state.

  .. cpp:function:: bool allRunning() const

      Checks if *all* threads in this set are in the ``running`` state.

  .. cpp:function:: bool anyRunning() const

      Checks if *any* threads in this set are in the ``stopped`` state.

  .. cpp:function:: bool allTerminated() const

      Checks if *all* threads in this set are in the ``terminated`` state.

  .. cpp:function:: bool anyTerminated() const

      Checks if *any* threads in this set are in the ``terminated`` state.

  .. cpp:function:: bool allSingleStepMode() const

      Checks if *all* threads in this set are in single-step mode.

  .. cpp:function:: bool anySingleStepMode() const

      Checks if *any* threads in this set are in single-step mode.

  .. cpp:function:: bool allHaveUserThreadInfo() const

      Checks if *all* threads in this set have user thread information.

  .. cpp:function:: bool anyHaveUserThreadInfo() const

      Checks if *any* threads in this set have user thread information.

  .. cpp:function:: ThreadSet::ptr getStoppedSubset() const

     Checks if *all* threads in this set are in the ``stopped`` state.

  .. cpp:function:: ThreadSet::ptr getRunningSubset() const

      Returns the subset of threads in the ``running`` state.

  .. cpp:function:: ThreadSet::ptr getTerminatedSubset() const

      Returns the subset of threads in the ``terminated`` state.

  .. cpp:function:: ThreadSet::ptr getSingleStepSubset() const

      Returns the subset of threads in single-step mode.

  .. cpp:function:: ThreadSet::ptr getHaveUserThreadInfoSubset() const

      Returns the subset of threads that have user thread information.

  .. cpp:function:: bool getStartFunctions(AddressSet::ptr result) const

      Fills ``result`` with the addresses of every start function of each thread in this set.

      This information is only available on threads that have user thread information available.

      Returns ``false`` on error.

  .. cpp:function:: bool getStackBases(AddressSet::ptr result) const

      Fills ``result`` with the addresses of every stack base of each thread in this set.

      This information is only available on threads that have user thread information available.

      Returns ``false`` on error.

  .. cpp:function:: bool getTLSs(AddressSet::ptr result) const

      Fills ``result`` with the addresses of every thread-local storage region of each thread in this set.

      This information is only available on threads that have user thread information available.

      Returns ``false`` on error.

  .. cpp:function:: bool stopThreads() const

      Stops every thread in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool continueThreads() const

      Continues execution of every thread in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool setSingleStepMode(bool v) const

      Turns on and off single-stepping mode for every thread in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool getRegister(Dyninst::MachRegister reg, std::map<Thread::ptr, Dyninst::MachRegisterVal> &res) const

      Stores the value of the register ``reg`` in ``res`` for every thread in this set, grouped by thread.

      Returns ``false`` on error.

  .. cpp:function:: bool getRegister(Dyninst::MachRegister reg, std::map<Dyninst::MachRegisterVal, ThreadSet::ptr> &res) const

      Stores the value of the register ``reg`` in ``res`` for every thread in this set, grouped by the register's value.

      Returns ``false`` on error.

  .. cpp:function:: bool setRegister(Dyninst::MachRegister reg, const std::map<ThreadSet::const_ptr, Dyninst::MachRegisterVal> &vals) const

      Sets the value of register ``reg`` in each thread in this set. The value is looked up in ``vals``.

      .. Error:: This behavior is undefined if ``vals`` contains a thread not included in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val) const

      Sets the register ``reg`` to ``val`` for each thread in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool getAllRegisters(std::map<Thread::ptr, RegisterPool> &results) const

      Fills ``results`` with the values of every register in each thread in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool setAllRegisters(const std::map<Thread::const_ptr, RegisterPool> &vals) const

      Sets the values of every register in ``val`` for each thread in this set that is also in ``val``.

      .. Error:: This behavior is undefined if ``vals`` contains a thread not included in this set.

      Returns ``false`` on error.

  .. cpp:function:: bool postIRPC(const std::multimap<Thread::const_ptr, IRPC::ptr> &rpcs) const

      Posts each IRPC in ``rpcs`` to every thread in this set that is also in ``rpcs``.

      Returns ``false`` on error.

  .. cpp:function:: bool postIRPC(IRPC::ptr irpc, std::multimap<Thread::ptr, IRPC::ptr>* result = NULL)

      Posts a copy of ``irpc`` to every thread in this set.

      If ``result`` is provided, then the new IRPC objects are returned there.

      Returns ``false`` on error.

  .. cpp:function:: CallStackUnwindingSet* getCallStackUnwinding()
  .. cpp:function:: const CallStackUnwindingSet* getCallStackUnwinding() const

.. cpp:class:: ThreadSet::iterator

  Models the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

.. cpp:class:: ThreadSet::const_iterator

  Models the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.


Notes
*****

Use :cpp:func:`AddressSet::lower_bound`, :cpp:func:`AddressSet::upper_bound`, and
:cpp:func:`AddressSet::equal_range` to focus on an :cpp:type:`Dyninst::Address`.

For example:

.. code:: cpp

  pair<AddressSet::iterator, AddressSet::iterator> range = myset.equal_range(0x1000);
  for (AddressSet::iterator i = range.first; i != range.second; i++) {
    //Every Process::ptr with address equal to 0x1000 here
  }
