.. _`sec:ProcessSet.h`:

ProcesssSet.h
=============

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:class:: AddressSet

  The AddressSet class is a set container of Process and Dyninst::Address
  tuples, with each set element a std::pair<Address, Process::ptr>.
  AddressSet is used by the ProcessSet and ThreadSet classes for
  performing group operations on large numbers of processes. An AddressSet
  might, for example, represent the location of a symbol across numerous
  processes, or the location of a buffer in each process where data can be
  written or read.
  
  The iteration interfaces of AddressSet resemble a C++ STL
  std::multimap<Address, Process::ptr>. When iterating all Addresses will
  appear in sequential order from smallest to largest.
  
  AddressSet Declared In:
  
     ProcessSet.h
  
  AddressSet Types:
  
     AddressSet::ptr
  
     AddressSet::const_ptr
  
  The AddressSet::ptr and AddressSet::const_ptr respectively represent a
  pointer and a const pointer to an AddressSet object. Both pointer types
  are reference counted and will cause the underlying AddressSet object to
  be cleaned when there are no more references.
  
     typedef std::pair<Dyninst::Address, Process::ptr> value_type
  
     class iterator {
  
     public:
  
     iterator();
  
     ~iterator();
  
     value_type operator*() const;
  
     bool operator==(const iterator &i);
  
     bool operator!=(const iterator &i);
  
     AddressSet::iterator operator++();
  
     AddressSet::iterator operator++(int);
  
     };
  
     class const_iterator {
  
     public:
  
     const_iterator();
  
     ~const_iterator();
  
     value_type operator*() const;
  
     bool operator==(const const_interator &i);
  
     bool operator!=(const const_iterator &i);
  
     AddressSet::const_iterator operator++();
  
     AddressSet::const_iterator operator++(int);
  
     };
  
  These are C++ iterators over the Address and Process pairs contained in
  the AddressSet. The behavior of operator*, operator==, operator!=,
  operator++, and operator++(int) match the standard behavior of C++
  iterators.
  
  AddressSet Static Member Functions:
  
     static AddressSet::ptr newAddressSet()
  
  This function returns a new AddressSet that is empty.
  
     | static AddressSet::ptr newAddressSet(ProcessSet::const_ptr ps,
     | Dyninst::Address addr)
  
  This function returns a new AddressSet initialized with the elements
  from ps paired with the Address addr.
  
     | static AddressSet::ptr newAddressSet(ProcessSet::const_ptr ps,
     | std::string library_name,
     | Dyninst::Offset off)
  
  This function returns a new AddressSet initialized with the elements
  from ps. The Address element for each process is calculated by looking
  up the load address of library_name in each Process and adding it to
  off.
  
  AddressSet Member Functions
  
     iterator begin()
  
     const_iterator begin() const
  
  These functions return an iterator that points to the first element in
  the AddressSet, or end() if the AddressSet is empty.
  
     iterator end()
  
     const_iterator end() const
  
  These functions return an iterator that points to the element that comes
  after the final element in the AddressSet.
  
     iterator find(Dyninst::Address addr)
  
     const_iterator find(Dyninst::Address addr) const
  
  These functions return an iterator that points to the first element in
  the AddressSet with an address of addr. They return end() if no element
  matches addr.
  
     iterator find(Dyninst::Address addr, Process::const_ptr proc)
  
     | const_iterator find(Dyninst::Address addr,
     | Process::const_ptr proc) const
  
  These functions return an iterator that points to any element that has a
  process and address of proc and addr. It returns end() if no element
  matches.
  
     size_t count(Dyninst::Address addr) const
  
  This function returns the number of elements with address addr.
  
     size_t size() const
  
  This function returns the number of elements in the AddressSet.
  
     bool empty() const
  
  This function returns true if the AddressSet has zero elements and false
  otherwise.
  
     | std::pair<iterator, bool> insert(Dyninst::Address addr,
     | Process::const_ptr proc)
  
  This function inserts a new element into the AddressSet with addr and
  proc as its values. If another element with those values already exists,
  then no new element will be inserted. It returns an iterator that points
  to the new or existing element and a boolean value that is true if a new
  element was inserted and false otherwise.
  
     size_t insert(Dyninst::Address addr, ProcessSet::const_ptr ps)
  
  For every element in ps, this function inserts it and addr into the
  AddressSet. It returns the number of new elements created.
  
     void erase(iterator pos)
  
  This function removes the element pointed to by pos from the AddressSet.
  
     size_t erase(Process::const_ptr proc)
  
  This function removes every element with a process of proc from the
  AddressSet. It returns the number of elements removed.
  
     size_t erase(Dyninst::Address addr, Process::const_ptr proc)
  
  This function removes any element that has and address and process of
  addr and proc from the AddressSet. It returns the number of elements
  removed.
  
     void clear()
  
  This function erases all elements from the AddressSet leaving an
  AddressSet of size zero.
  
     iterator lower_bound(Dyninst::Address addr)
  
  This function returns an iterator pointing to the first element in the
  AddressSet that has an address greater than or equal to addr.
  
     iterator upper_bound(Dyninst::Address addr)
  
  This function returns an iterator pointing to the first element in the
  AddressSet that has an address greater than addr.
  
     std::pair<iterator, iterator> equal_range(Address addr) const
  
  This function returns a pair of iterators. The first iterator has the
  same value as the return of lower_bound(addr) and the second iterator
  has the same value as the return of upper_bound(addr).
  
     AddressSet::ptr set_union(AddressSet::const_ptr aset)
  
  This function returns a new AddressSet whose elements are the set union
  of this AddressSet and aset.
  
     AddressSet::ptr set_intersection(AddressSet::const_ptr aset)
  
  This function returns a new AddressSet whose elements are the set
  intersection of this AddressSet and aset.
  
     AddressSet::ptr set_difference(AddressSet::const_ptr aset)
  
  This function returns a new AddressSet whose elements are the set
  difference of this AddressSet minus aset.

.. cpp:class:: ProcessSet

  The ProcessSet class is a set container for multiple Process objects. It
  shares many of the same operations as the Process class, but when an
  operation is performed on a ProcessSet it is done on every Process in
  the ProcessSet. On some systems, such as Blue Gene/Q, a ProcessSet can
  achieve better performance when repeating an operation across many
  target processes.
  
  ProcessSet Declared In:
  
     ProcessSet.h
  
  ProcessSet Types
  
     ProcessSet::ptr
  
     ProcessSet::const_ptr
  
  The ptr and const_ptr types are smart pointers to a ProcessSet object.
  When the last smart pointer to the ProcessSet is cleaned, then the
  underlying ProcessSet is cleaned.
  
     ProcessSet::weak_ptr
  
     ProcessSet::const_weak_ptr
  
  The weak_ptr and const_weak_ptr are weak smart pointers to a ProcessSet
  object. Unlike regular smart pointers, weak pointers are not counted as
  references when determining whether to clean the ProcessSet object.
  
     struct CreateInfo {
  
     std::string executable;
  
     std::vector<std::string> argv;
  
     std::vector<std::string> envp;
  
     std::map<int, int> fds;
  
     ProcControlAPI::err_t error_ret;
  
     Process::ptr proc;
  
     }
  
     struct AttachInfo {
  
     Dyninst::PID pid;
  
     std::string executable;
  
     ProcControlAPI::err_t error_ret;
  
     Process::ptr proc;
  
     }
  
  The CreateInfo and AttachInfo types are used by the
  ProcessSet::createProcessSet and ProcessSet::attachProcessSet functions
  when creating groups of processes.
  
     class iterator {
  
     public:
  
     iterator()
  
     ~iterator()
  
     Process::ptr operator*() const
  
     bool operator==(const iterator &i) const
  
     bool operator!=(const iterator &i) const
  
     ProcessSet::iterator operator++();
  
     ProcessSet::iterator operator++(int);
  
     }
  
     class const_iterator {
  
     public:
  
     const_iterator()
  
     ~const_iterator()
  
     Process::const_ptr operator*() const
  
     bool operator==(const const_iterator &i) const
  
     bool operator!=(const const_iterator &i) const
  
     ProcessSet::const_iterator operator++();
  
     ProcessSet::const_iterator operator++(int);
  
     }
  
  These are C++ iterators over the Process pointers contained in the
  ProcessSet. The behavior of operator*, operator==, operator!=,
  operator++, and operator++(int) match the standard behavior of C++
  iterators.
  
     struct write_t {
  
     void \*buffer
  
     Dyninst::Address addr
  
     size_t size
  
     err_t err
  
     bool operator<(const write_t &w)
  
     }
  
     struct read_t {
  
     Dyninst::Address addr
  
     void \*buffer
  
     size_t size
  
     err_t err
  
     bool operator<(const read_t &r)
  
     }
  
  The write_t and read_t types are used by ProcessSet::readMemory and
  ProcessSet::writeMemory.
  
  ProcessSet Static Member Functions
  
     static ProcessSet::ptr newProcessSet()
  
  This function creates a new ProcessSet that is empty.
  
     static ProcessSet::ptr newProcessSet(Process::const_ptr proc)
  
  This function creates a new ProcessSet containing proc.
  
     static ProcessSet::ptr newProcessSet(ProcessSet::const_ptr pset)
  
  This function creates a new ProcessSet that is a copy of pset.
  
     | static ProcessSet::ptr newProcessSet(
     | const std::set<Process::const_ptr> &procs)
  
  This function creates a new ProcessSet containing every element from
  procs.
  
     | static ProcessSet newProcessSet(AddressSet::const_iterator ab,
     | AddressSet::const_iterator ae)
  
  This function creates a new ProcessSet containing the processes that are
  found within [ab, ae) of an AddressSet.
  
     | static ProcessSet::ptr createProcessSet(
     | std::vector<CreateInfo> &cinfo)
  
  This function creates a new ProcessSet by launching new processes. Each
  element in cinfo specifies an executable, arguments, environment and
  file descriptor mappings (with similar semantics to
  Process::createProcess), which are used to launch a new process.
  
  Every successfully created Process will be added to a new ProcessSet
  that is returned by this function.
  
  In addition, the cinfo vector will be updated so that each entry’s proc
  field points to the Process created by that entry, and the error_ret
  entry will contain an error code for any process launch that failed.
  
     | static ProcessSet::ptr attachProcessSet(
     | std::vector<AttachInfo> &ainfo)
  
  This function creates a new ProcessSet by attaching to existing
  processes. Each element in ainfo specifies a PID and executable (with
  similar semantics to Process::attachProcess), which are used to attach
  to the processes.
  
  Every successfully attached Process will be added to a new ProcessSet
  that is returned by this function.
  
  In addition, the ainfo vector will be updated so that each entry’s proc
  field points to the Process attached by that entry, and the error_ret
  entry will contain an error code any process attach that failed.
  
  ProcessSet Member Functions
  
     ProcessSet::ptr set_union(ProcessSet::ptr pset) const
  
  This function returns a new ProcessSet whose elements are a set union of
  this ProcessSet and pset.
  
     ProcessSet::ptr set_intersection(ProcessSet::ptr pset) const
  
  This function returns a new ProcessSet whose elements are a set
  intersection of this ProcessSet and pset.
  
     ProcessSet::ptr set_difference(ProcessSet::ptr pset) const
  
  This function returns a new ProcessSet whose elements are a set
  difference of this ProcessSet minus pset.
  
     iterator begin()
  
     const_iterator begin() const
  
  These functions return iterators to the first element in the ProcessSet.
  
     iterator end()
  
     const_iterator end() const
  
  These functions return iterators that come after the last element in the
  ProcessSet.
  
     iterator find(Process::const_ptr proc)
  
     const_iterator find(Process::const_ptr proc) const
  
  These functions search a ProcessSet for the Process pointed to by proc
  and returns an iterator that points to that element. It returns
  ProcessSet::end() if no element is found.
  
     iterator find(Dyninst::PID pid)
  
     const_iterator find(Dyninst::PID pid) const
  
  These functions search a ProcessSet for the Process pointed to by proc
  and returns an iterator that points to that element. It returns
  ProcessSet::end() if no element is found.
  
     bool empty() const
  
  This function returns true if the ProcessSet has zero elements, false
  otherwise.
  
     size_t size() const
  
  This function returns the number of elements in the ProcessSet.
  
     std::pair<iterator, bool> insert(Process::const_ptr proc)
  
  This function inserts proc into the ProcessSet. If proc already exists
  in the ProcessSet, then no change will occur. This function returns an
  iterator pointing to either the new or existing element and a boolean
  that is true if an insert happened and false otherwise.
  
     void erase(iterator pos)
  
  This function removes the element pointed to by pos from the ProcessSet.
  
     size_t erase(Process::const_ptr proc)
  
  This function searches the ProcessSet for proc, then erases that element
  from the ProcessSet. It returns 1 if it erased an element and 0
  otherwise.
  
     void clear()
  
  This function erases all elements in the ProcessSet.
  
     ProcessSet::ptr getErrorSubset() const
  
  This function returns a new ProcessSet containing every Process from
  this ProcessSet that has a non-zero error code. Error codes are reset
  upon every ProcessSet API call, so this function shows which Processes
  had an error on the last ProcessSet operation.
  
     void getErrorSubsets(std::map<ProcControlAPI::err_t, ProcessSet::ptr>
     &err_sets) const
  
  This function returns a set of new ProcessSets containing every Process
  from this ProcessSet that has non-zero error codes, and grouped by error
  code. For each error code generated by the last ProcessSet API operation
  an element will be added to err_sets, and every Process that has the
  same error code will be added to the new ProcessSet associated with that
  error code.
  
     bool anyTerminated() const;
  
     bool allTerminated() const;
  
  These functions respectively return true if any or all processes in this
  ProcessSet are terminated, and false otherwise.
  
     bool anyExited() const;
  
     bool allExited() const;
  
  These functions respectively return true if any or all processes in this
  ProcessSet have exited normally, and false otherwise.
  
     bool anyCrashed() const
  
     bool allCrashed() const
  
  These functions respectively return true if any or all processes in this
  ProcessSet have crashed normally, and false otherwise.
  
     bool anyDetached();
  
     bool allDetached();
  
  These functions respectively return true if any or all processes in this
  ProcessSet have been detached, and false otherwise.
  
     bool anyThreadStopped();
  
     bool allThreadStopped();
  
  These functions respectively return true if any or all threads in this
  ProcessSet are stopped, and false otherwise.
  
     bool anyThreadRunning();
  
     bool allThreadRunning();
  
  These functions respectively return true if any or all threads in this
  ProcessSet are running, and false otherwise.
  
     ProcessSet::ptr getTerminatedSubset() const
  
  This function returns a new ProcessSet, which is a subset of this
  ProcessSet, and contains every Process that is terminated.
  
     ProcessSet::ptr getExitedSubset() const
  
  This function returns a new ProcessSet, which is a subset of this
  ProcessSet, and contains every Process that has exited normally.
  
     ProcessSet::ptr getCrashedSubset() const
  
  This function returns a new ProcessSet, which is a subset of this
  ProcessSet, and contains every Process that has crashed.
  
     ProcessSet::ptr getDetachedSubset() const
  
  This function returns a new ProcessSet, which is a subset of this
  ProcessSet, and contains every Process that is detached.
  
     ProcessSet::ptr getAllThreadRunningSubset() const
  
     ProcessSet::ptr getAnyThreadRunningSubset() const
  
  This function returns a new ProcessSet, which is a subset of this
  ProcessSet, and contains every Process that respectively has any or all
  threads running.
  
     ProcessSet::ptr getAllThreadStoppedSubset() const
  
     ProcessSet::ptr getAnyThreadStoppedSubset() const
  
  This function returns a new ProcessSet, which is a subset of this
  ProcessSet, and contains every Process that respectively has any or all
  threads stopped.
  
     bool continueProcs()
  
  This function continues every thread in every process of this
  ProcessSet, similar to Process::continueProc. It returns true if every
  process was successfully continued and false otherwise.
  
     bool stopProcs()
  
  This function stops every thread in every process of this ProcessSet,
  similar to Process::stopProc. It returns true if every process was
  successfully stopped and false otherwise.
  
     bool detach(bool leaveStopped = true)
  
  This function detaches from every process in this ProcessSet, similar to
  Process::detach. It returns true if every process detach was successful
  and false otherwise.
  
  If the leaveStopped parameter is set to true, and the processes in this
  ProcessSet are stopped, then the processes will be left in a stopped
  state after the detach.
  
     bool terminate()
  
  This function terminates every process in this ProcessSet, similar to
  Process::terminate. It returns true if every process was successfully
  terminated and false otherwise.
  
     bool temporaryDetach()
  
  This function does a temporary detach from every process in this
  ProcessSet, similar to Process::temporaryDetach. It returns true if
  every process was successfully detached and false otherwise.
  
     bool reAttach()
  
  This function reattaches to every process in this ProcessSet, similar to
  Process::reAttach. It returns true if every process was successfully
  reAttached and false otherwise.
  
     AddressSet::ptr mallocMemory(size_t sz) const
  
  This function allocates a block of memory of size sz in each process in
  this ProcessSet. The addresses of the allocations are returned in a new
  AddressSet object.
  
     bool mallocMemory(size_t size, AddressSet::ptr location)
  
  This function allocates a block of memory of size sz in each process in
  this ProcessSet. The memory will be allocated in each process based on
  the Process/Address pairs in location.
  
  This function’s behavior is undefined if location contains processes not
  included in this ProcessSet.
  
  This function returns true if every allocation happened without error
  and false otherwise.
  
     bool freeMemory(AddressSet::ptr addrs) const
  
  This function frees memory allocated by Process::mallocMemory or
  ProcessSet::mallocMemory. The AddressSet addrs should contain a list of
  Process/Address pairs that point to the memory that should be freed.
  
  This function’s behavior is undefined if addrs contains processes not
  included in this ProcessSet.
  
  This function returns true if every free happened without error and
  false otherwise.
  
     | bool readMemory(AddressSet::ptr addrs,
     | std::multimap<Process::ptr, void \*> &result,
  
     size_t size) const
  
  This function reads memory from processes in this ProcessSet. addrs
  should contain the addresses to read memory from. size should be the
  amount of memory read from each process. The results of the memory reads
  will be returned by filling in the result multimap. Each process that is
  read from will have an entry in result along with a malloc allocated
  buffer containing the results of the read.
  
  It is the ProcControlAPI user’s responsibility to free the memory
  buffers returned by this function.
  
  This function’s behavior is undefined if addrs contains processes not
  included in this ProcessSet.
  
  This function returns true if every read happened without error, and
  false otherwise.
  
     | bool readMemory(AddressSet::ptr addrs,
     | std::map<void \*, ProcessSet::ptr> &result,
     | size_t size)
  
  This function reads memory from processes in this ProcessSet. addrs
  should contain the addresses to read memory from. size should be the
  amount of memory to read from each process. The results of the memory
  reads will be aggregated together into the result map. If any two
  processes read equivalent byte-for-byte data, then those processes are
  grouped together in a new ProcessSet associated with a common malloc
  allocated buffer containing their memory contents.
  
  It is the ProcControlAPI user’s responsibility to free the memory
  buffers returned by this function.
  
  This function’s behavior is undefined if addrs contains processes not
  included in this ProcessSet.
  
  This function returns true if every read happened without error, and
  false otherwise.
  
     bool readMemory(std::multimap<Process::const_ptr, read_t> &addr)
  
  This function reads memory from processes in this ProcessSet. The
  processes to read from are specified in the indexes of addr. The remote
  address, read size and local buffer are specified in the read_t elements
  of addr.
  
  This function’s behavior is undefined if addr contains processes not
  included in this ProcessSet.
  
  This function returns true if every read happened without error, and
  false otherwise. If any read results in an error, then the error_ret
  field of the associated addr element will be set.
  
     | bool writMemory(AddressSet::ptr addrs,
     | const void \*buffer,
     | size_t sz) const
  
  This function will write the contents of buffer of size sz into the
  memory of each process at addrs.
  
  This function’s behavior is undefined if addrs contains processes not
  included in this ProcessSet.
  
  This function returns true if every write happened without error, and
  false otherwise.
  
     | bool writeMemory(
     | std::multimap<Process::const_ptr, write_t> &addrs) const
  
  This function writes to the memory of each process in addrs. The
  processes to write to are specified as the indexes of addrs. The local
  memory buffer, buffer size, and target location are specified in the
  write_t element of addrs.
  
  This function’s behavior is undefined if addrs contains processes not
  included in this ProcessSet.
  
  This function returns true if every write happened without error, and
  false otherwise. If any write results in an error, then the error_ret
  field of the associated addr element will be set.
  
     bool addBreakpoint(AddressSet::ptr as, Breakpoint::ptr bp) const
  
  This function inserts the Breakpoint bp into every process and address
  specified by as. It is similar to Process::addBreakpoint.
  
  This function’s behavior is undefined if addrs contains processes not
  included in this ProcessSet.
  
  This function returns true if every breakpoint add happened without
  error, and false otherwise.
  
     bool rmBreakpoint(AddressSet::ptr as, Breakpoint::ptr bp) const
  
  The function removes the Breakpoint bp from each process at the
  locations specified in as. It is similar to Process::rmBreakpoint.
  
  This function’s behavior is undefined if as contains processes not
  included in this ProcessSet.
  
  This function returns true if every breakpoint remove happened without
  error, and false otherwise.
  
     bool postIRPC(const std::multimap<Process::const_ptr, IRPC::ptr>
     &rpcs) const
  
  This function posts the IRPC objects specified in rpcs to their
  associated processes in the multimap. It is similar to
  Process::postIRPC.
  
  This function’s behavior is undefined if rpcs contains processes not
  included in this ProcessSet.
  
  This function returns true if every post happened without error, and
  false otherwise.
  
     | bool postIRPC(IRPC::ptr irpc,
     | std::multimap<Process::ptr, IRPC::ptr> \*result = NULL)
  
  This function makes a copy of irpc for each Process in this ProcessSet
  and posts it to that Process. If result is non-NULL, then the multimap
  will be filled with each newly created IRPC and the Process to which it
  was posted. It is similar to Process::postIRPC.
  
  This function returns true if every post happened without error, and
  false otherwise.
  
     | bool postIRPC(IRPC::ptr irpc
     | AddressSet::ptr addrs,
     | std::multimap<Process::ptr, IRPC::ptr> \*result = NULL)
  
  This function makes a copy of irpc and posts it to each Process in addrs
  at the given Address. If result is non-NULL, then the multimap will be
  filled with each newly created IRPC and the Process to which it was
  posted. It is similar to Process::postIRPC.
  
  This function’s behavior is undefined if rpcs contains processes not
  included in this ProcessSet.
  
  This function returns true if every post happened without error, and
  false otherwise.
  
.. cpp:class:: ThreadSet
  
  The ThreadSet class is a set container for Thread pointers. It has
  similar operations as Thread, and operations done on a ThreadSet affect
  every Thread in that ThreadSet. One some system, such as Blue Gene Q,
  using a ThreadSet is more efficient when doing the same operation across
  a large number of Threads.
  
  ThreadSet Types:
  
     ThreadSet::ptr
  
     ThreadSet::const_ptr
  
  The ptr and const_ptr types are smart pointers to a ThreadSet object.
  When the last smart pointer to the ThreadSet is cleaned, then the
  underlying ThreadSet is cleaned. The const_ptr type is a const smart
  pointer.
  
     ThreadSet::weak_ptr
  
     ThreadSet::const_weak_ptr
  
  The weak_ptr and const_weak_ptr are weak smart pointers to a ThreadSet
  object. Unlike regular smart pointers, weak pointers are not counted as
  references when determining whether to clean the ThreadSet object. The
  const_weak_ptr type is a const weak smart pointer.
  
     class iterator {
  
     public:
  
     iterator()
  
     ~iterator()
  
     Thread::ptr operator*() const
  
     bool operator==(const iterator &i) const
  
     bool operator!=(const iterator &i) const
  
     ThreadSet::iterator operator++();
  
     ThreadSet::iterator operator++(int);
  
     }
  
     class const_iterator {
  
     public:
  
     const_iterator()
  
     ~const_iterator()
  
     Thread::const_ptr operator*() const
  
     bool operator==(const const_iterator &i) const
  
     bool operator!=(const const_iterator &i) const
  
     ThreadSet::const_iterator operator++();
  
     ThreadSet::const_iterator operator++(int);
  
     }
  
  These are C++ iterators over the Thread pointers contained in the
  ThreadSet. The behavior of operator*, operator==, operator!=,
  operator++, and operator++(int) match the standard behavior of C++
  iterators.
  
  ThreadSet Static Member Functions
  
     static ThreadSet::ptr newThreadSet()
  
  This function creates a new ThreadSet that is empty.
  
     static ThreadSet::ptr newThreadSet(Thread::ptr thr)
  
  This function creates a new ThreadSet that contains thr.
  
     static ThreadSet::ptr newThreadSet(const ThreadPool &threadp)
  
  This function creates a new ThreadSet that contains all of the Threads
  currently in threadp.
  
     | static ThreadSet::ptr newThreadSet (
     | const std::set<Thread::const_ptr> &thrds)
  
  This function creates a new ThreadSet that contains all of the threads
  in thrds.
  
     static ThreadSet::ptr newThreadSet(ProcessSet::ptr pset)
  
  This function creates a new ThreadSet that contains every live thread
  currently in every process in pset.
  
  ThreadSet Member Functions
  
     ThreadSet::ptr set_union(ThreadSet::ptr tset) const
  
  This function returns a new ThreadSet whose elements are a set union of
  this ThreadSet and tset.
  
     ThreadSet::ptr set_intersection(ThreadSet::ptr tset) const
  
  This function returns a new ThreadSet whose elements are a set
  intersection of this ThreadSet and tset.
  
     ThreadSet::ptr set_difference(ThreadSet::ptr tset) const
  
  This function returns a new ThreadSet whose elements are a set
  difference of this ThreadSet minus tset.
  
     iterator begin()
  
     const_iterator begin() const
  
  These functions return iterators to the first element in the ThreadSet.
  
     iterator end()
  
     const_iterator end() const
  
  These functions return iterators that come after the last element in the
  ThreadSet.
  
     iterator find(Thread::const_ptr thr)
  
     const_iterator find(Thread::const_ptr thr) const
  
  These functions search a ThreadSet for thr and returns an iterator
  pointing to that element. It returns ThreadSet::end() if no element is
  found
  
     bool empty() const
  
  This function returns true if the ThreadSet has zero elements and false
  otherwise.
  
     size_t size() const
  
  This function returns the number of elements in the ThreadSet.
  
     std::pair<iterator, bool> insert(Thread::const_ptr thr)
  
  This function inserts thr into the ThreadSet. If thr already exists in
  the ThreadSet, then no change will occur. This function returns an
  iterator pointing to either the new or existing element and a boolean
  that is true if an insert happened and false otherwise.
  
     void erase(iterator pos)
  
  This function removes the element pointed to by pos from the ThreadSet.
  
     size_t erase(Thread::const_ptr thr)
  
  This function searches the ThreadSet for thr, then erases that element
  from the ThreadSet. It returns 1 if it erased an element and 0
  otherwise.
  
     void clear()
  
  This function erases all elements in the ThreadSet.
  
     ThreadSet::ptr getErrorSubset() const
  
  This function returns a new ThreadSet containing every Thread from this
  ThreadSet that has a non-zero error code. Error codes are reset upon
  every ThreadSet API call, so this function shows which Threads had an
  error on the last ThreadSet operation.
  
     | void getErrorSubsets(
     | std::map<ProcControlAPI::err_t, ThreadSet::ptr> &err) const
  
  This function returns a set of new ThreadSets containing every Thread
  from this ThreadSet that has non-zero error codes, and grouped by error
  code. For each error code generated by the last ThreadSet API operation
  an element will be added to err, and every Thread that has that error
  code will be added to the new ThreadSet associated with that error code.
  
     bool allStopped() const
  
     bool anyStopped() const
  
  These functions respectively return true if any or all threads in this
  ThreadSet are stopped and false otherwise.
  
     bool allRunning() const
  
     bool anyRunning() const
  
  These functions respectively return true if any or all threads in this
  ThreadSet are running and false otherwise.
  
     bool allTerminated() const
  
     bool anyTerminated() const
  
  These functions respectively return true if any or all threads in this
  ThreadSet are terminated and false otherwise.
  
     bool allSingleStepMode() const
  
     bool anySingleStepMode() const
  
  These functions respectively return true if any or all threads in this
  ThreadSet are running in single step mode, and false otherwise.
  
     bool allHaveUserThreadInfo() const
  
     bool anyHaveUserThreadInfo() const
  
  These functions respectively return true if any or all threads in this
  ThreadSet have user thread information available and false otherwise.
  
     ThreadSet::ptr getStoppedSubset() const
  
  This function returns a new ThreadSet, which is a subset of this
  ThreadSet, and contains every Thread that is stopped.
  
     ThreadSet::ptr getRunningSubset() const
  
  This function returns a new ThreadSet, which is a subset of this
  ThreadSet, and contains every Thread that is running.
  
     ThreadSet::ptr getTerminatedSubset() const
  
  This function returns a new ThreadSet, which is a subset of this
  ThreadSet, and contains every Thread that is terminated.
  
     ThreadSet::ptr getSingleStepSubset() const
  
  This function returns a new ThreadSet, which is a subset of this
  ThreadSet, and contains every Thread that is in single step mode.
  
     ThreadSet::ptr getHaveUserThreadInfoSubset() const
  
  This function returns a new ThreadSet, which is a subset of this
  ThreadSet, and contains every Thread that has user thread information
  available.
  
     bool getStartFunctions(AddressSet::ptr result) const
  
  This function fills in the AddressSet pointed to by result with the
  address of every start function of each Thread in this ThreadSet. This
  information is only available on threads that have user thread
  information available.
  
  This function return true if it succeeded for every Thread, and false
  otherwise.
  
     bool getStackBases(AddressSet::ptr result) const
  
  This function fills in the AddressSet pointed to by result with the
  address of every stack base of each Thread in this ThreadSet. This
  information is only available on threads that have user thread
  information available.
  
  This function return true if it succeeded for every Thread, and false
  otherwise.
  
     bool getTLSs(AddressSet::ptr result) const
  
  This function fills in the AddressSet pointed to by result with the
  address of every thread-local storage region of each Thread in this
  ThreadSet. This information is only available on threads that have user
  thread information available.
  
  This function return true if it succeeded for every Thread, and false
  otherwise.
  
     bool stopThreads() const
  
  This function stops every Thread in this ThreadSet. It is similar to
  Thread::stopThread.
  
  This function return true if it succeeded for every Thread, and false
  otherwise.
  
     bool continueThreads() const
  
  This function stops every Thread in this ThreadSet. It is similar to
  Thread::continueThread.
  
  This function return true if it succeeded for every Thread, and false
  otherwise.
  
     bool setSingleStepMode(bool v) const
  
  This function puts every Thread in this ThreadSet into single step mode
  if v is true. It clears single step mode if v is false. It is similar to
  Thread::setSingleStepMode.
  
  This function return true if it succeeded for every Thread, and false
  otherwise.
  
     | bool getRegister(Dyninst::MachRegister reg,
     | std::map<Thread::ptr, Dyninst::MachRegisterVal> &res) const
  
  This function gets the value of register reg in every Thread in this
  ThreadSet. The collected values are returned in the res map, with each
  Thread mapped to the value of reg in that thread. It is similar to
  Thread::getRegister.
  
  This function return true if it succeeded for every Thread, and false
  otherwise.
  
     bool getRegister(Dyninst::MachRegister reg,
  
        | std::map<Dyninst::MachRegisterVal, ThreadSet::ptr> &res)
        | const
  
  This function gets the value of register reg in every Thread in this
  ThreadSet and then aggregates all identical values together. The res map
  will contain an entry for each unique register value, and map that value
  to a new ThreadSet that contains every Thread that produced that
  register value. It is similar to Thread::getRegister.
  
  This function return true if it succeeded for every Thread, and false
  otherwise.
  
     bool setRegister(Dyninst::MachRegister reg,
  
     | const std::map<ThreadSet::const_ptr,
     | Dyninst::MachRegisterVal> &vals) const
  
  This function sets the value of register reg in each Thread in this
  ThreadSet. The value set in each thread is looked up in the vals map. It
  is similar to Thread::setRegister.
  
  This function’s behavior is undefined if it is passed a Thread that is
  not in this ThreadSet.
  
  This function return true if it succeeded for every Thread, and false
  otherwise.
  
     | bool setRegister(Dyninst::MachRegister reg,
     | Dyninst::MachRegisterVal val) const
  
  This function sets the register reg to val in each Thread in this
  ThreadSet. It is similar to Thread::setRegister.
  
  This function return true if it succeeded for every Thread, and false
  otherwise.
  
     | bool getAllRegisters(
     | std::map<Thread::ptr, RegisterPool> &results) const
  
  This function gets the values of every register in each Thread in this
  ThreadSet. The register values are returned as RegisterPools in the
  results map, with each Thread mapped to its RegisterPool. It is similar
  to Thread::getAllRegisters.
  
  This function return true if it succeeded for every Thread, and false
  otherwise.
  
     | bool setAllRegisters(
     | const std::map<Thread::const_ptr, RegisterPool> &val) const
  
  This function sets the values of every register in each Thread in this
  ThreadSet. The register values are extracted from the val map, with each
  Thread specifying its register values via the map. This function is
  similar to Thread::setAllRegisters.
  
  This function’s behavior is undefined if it is passed a Thread that is
  not in this ThreadSet.
  
  This function return true if it succeeded for every Thread, and false
  otherwise.
  
     | bool postIRPC(const std::multimap<Thread::const_ptr,
     | IRPC::ptr> &rpcs) const
  
  This function posts an IRPC to every Thread in this ThreadSet. The IRPC
  to post to each Thread is specified in the rpcs multimap. This function
  is similar to Thread::postIRPC.
  
  This function return true if it succeeded for every Thread, and false
  otherwise.
  
     | bool postIRPC(IRPC::ptr irpc,
     | std::multimap<Thread::ptr, IRPC::ptr> \*result = NULL)
  
  This function posts a copy of irpc to every Thread in this ThreadSet. If
  result is non-NULL, then the new IRPC objects are returned in the result
  multimap, with the Thread mapped to the IRPC that was posted there. This
  function is similar to Thread::postIRPC.
  
  This function return true if it succeeded for every Thread, and false
  otherwise.