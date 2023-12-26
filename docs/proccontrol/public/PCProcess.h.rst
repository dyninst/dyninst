.. _`sec:PCProcess.h`:

PCProcess.h
===========

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:class:: Breakpoint

  For an overview of breakpoints, see :ref:`sec:proccontrol-intro-breakpoints`. 

  .. cpp:type:: Breakpoint* ptr
  .. cpp:type:: Breakpoint const* const_ptr

    Convenience typedefs used in interfaces.

  .. cpp:var:: static const int BP_X = 1
  .. cpp:var:: static const int BP_W = 2
  .. cpp:var:: static const int BP_R = 4

    Constants are used to set execute, write and read bits on hardware breakpoints.

  .. cpp:function:: Breakpoint::ptr newBreakpoint()

    Creates a new software Breakpoint object.

    The Breakpoint is not inserted into a process until it is passed to :cpp:func:`Process::addBreakpoint()`.

  .. cpp:function:: Breakpoint::ptr newTransferBreakpoint(Dyninst::Address ctrl_to)

    Creates a new control transfer software breakpoint.

    Upon resumption after executing this Breakpoint,control will resume at the address specified by the ctrl_to parameter.

  .. cpp:function:: Breakpoint::ptr newHardwareBreakpoint(unsigned int mode, unsigned int size)

    Creates a new hardware breakpoint.

    The ``mode`` parameter is a bitfield that contains an OR combination the values :cpp:member:`BP_X`,
    :cpp:member:`BP_W`, and :cpp:member:`BP_R`. These control whether the breakpoint will trigger when its target
    address is executed, written or read. The size parameter specifies a range of memory that this breakpoint
    monitors. If memory is accessed between the target address and target
    address + size, then the breakpoint will trigger.

  .. cpp:function:: bool isCtrlTransfer() const

    Returns ``true`` if the Breakpoint is a control transfer breakpoint (see :cpp:func:`newTransferBreakpoint`),
    and ``false`` if it is a regular Breakpoint.

  .. cpp:function:: Dyninst::Address getToAddress() const

    If this Breakpoint is a control transfer breakpoint, then returns the address to which
    it transfers control. If this Breakpoint is not a control transfer breakpoint, then returns 0.

  .. cpp:function:: void setData(void *data) const

    Sets the value of an opaque handle that is associated with each Breakpoint.

    The opaque handle can point to any content, and is retrieved with :cpp:func:`getData`.

  .. cpp:function:: void* getData() const

    Returns the value of the opaque handled that is associated with this Breakpoint.

  .. cpp:function:: void setSuppressCallbacks(bool val)

    Toggle suppression of callbacks stemming from this breakpoint.

    All other effects from this breakpoint will still occur, but it will not generate a callback.
    By default, callbacks occur from every breakpoint.

    See :ref:`sec:proccontrol-intro-callbacks` for an overview of callbacks.

  .. cpp:function:: bool suppressCallbacks() const

    Returns ``true`` if callbacks have been suppressed for this breakpoint (see :cpp:func:`setSuppressCallbacks`).

    See :ref:`sec:proccontrol-intro-callbacks` for an overview of callbacks.

.. cpp:class:: EventNotify

  Notifies the user when ProcControlAPI is ready to deliver a callback function.

  See :ref:`sec:proccontrol-intro-callback-events` for an overview of callback events.

  .. cpp:type:: void (*notify_cb_t)()

    Function signature is used for light-weight notification callback.

  .. cpp:function:: EventNotify *evNotify()

    Returns the singleton instance of the EventNotify class.

  .. cpp:function:: int getFD()

    Returns a file descriptor.

    ProcControlAPI will write a byte that will be available for reading on this file descriptor when a
    callback function is ready to be invoked. Upon seeing that a byte has
    been written to this file descriptor (likely via select or poll) the
    user should call the :cpp:func:`Process::handleEvents` function. The user should
    never actually read the byte from this file descriptor; ProcControlAPI
    will handle clearing the byte after the callback function is invoked.

    Returns -1 on error. The specific error can be retrieved from :cpp:func:`getLastError`.

  .. cpp:function:: void registerCB(notify_cb_t cb)

    Registers a light-weight callback function that will be invoked when ProcControlAPI notifies
    the user when a callback function is ready to be invoked.

    This light-weight callback may be called by a ProcControlAPI internal thread or from a signal handler; the
    user is encouraged to keep its implementation appropriately safe for these circumstances.

  .. cpp:function:: void removeCB(notify_cb_t cb)

    Unregisters the light-weight callback previously registered with :cpp:func:`EventNotify::registerCB`.

.. cpp:class:: ExecFileInfo

  .. cpp:member:: void* fileHandle
  .. cpp:member:: void* processHandle
  .. cpp:member:: Dyninst::Address fileBase

.. cpp:class:: IRPC

  An Inferior Remote Procedure Call that can be run in a target process.

  See :ref:`sec:proccontrol-intro-irpcs` for details on iRPCs.

  .. cpp:type:: IRPC* ptr
  .. cpp:type:: IRPC const* const_ptr

    Convenience typedefs used in interfaces

  .. cpp:function:: IRPC::ptr createIRPC(void *binary_blob, unsigned int size, bool non_blocking = false)

    Create an IRPC that executes machine code in `binary_blob` of `size` bytes.

    If ``non_blocking`` is ``true``, then calls to :cpp:func:`Process::handleEvents`
    will block until this IRPC is completed.

    ProcControlAPI will maintain its own copy of the binary_blob buffer. Users can free the buffer.

  .. cpp:function:: IRPC::ptr createIRPC(void *binary_blob, unsigned int size, Dyninst::Address addr, bool non_blocking = false)

    Create an IRPC that executes machine code in `binary_blob` of `size` bytes starting at address `addr`.

    If ``non_blocking`` is ``true``, then calls to :cpp:func:`Process::handleEvents`
    will block until this IRPC is completed.

    ProcControlAPI will maintain its own copy of the binary_blob buffer. Users can free the buffer.

  .. cpp:function:: Dyninst::Address getAddress() const

    Returns the address where the IRPC will be run.

    If the IRPC was not given an address at construction and has not yet started running, then returns 0.

  .. cpp:function:: void *getBinaryCodeBlob() const

    Returns a pointer to memory that contains the binary code for this IRPC.

  .. cpp:function:: unsigned int getBinaryCodeSize() const

    Returns the size of the binary code blob buffer.

  .. cpp:function:: unsigned long getID() const

    Returns an integer identifier that uniquely identifies this IRPC.

  .. cpp:function:: void setStartOffset(unsigned long off)

    Sets the starting position of execution for the iRPC.

    By default an IRPC will start executing its code at the beginning
    of the blob (see :cpp:func:`createIRPC`). This function can be used to
    tell ProcControlAPI to start execution of the code blob at a specific offset.

    .. warning:: This function should be called before the IRPC is posted.

  .. cpp:function:: unsigned long getStartOffset() const

    Returns the start offset, if specified. Otherwise, returns 0.

.. cpp:class:: Library

  A ``Library`` represents a single shared library (frequently referred to as
  a DLL or DSO, depending on the OS) that has been loaded into the target
  process. In addition, it will be used to represent the process’
  executable. Process’ with statically linked executables will only
  contain the single ``Library`` that represents the executable.

  Each Library contains a load address and a file name. The load address
  is the address at which the OS loaded the library, and the file name is
  the path to the library’s file. Note that on some operating systems
  (e.g., Linux) the load address does not necessarily represent the beginning
  of the library in memory. Instead, it is a value that can be added to
  a library’s symbol offsets to compute the dynamic address of a symbol.

  Libraries may be loaded and unloaded by the process during execution. A
  library load or unload can trigger a callback with an EventLibrary
  parameter. The current list of libraries loaded into a process can be
  accessed via a Process’ :cpp:class:`LibraryPool`.

  .. cpp:type:: boost::shared_ptr<Library> ptr
  .. cpp:type:: boost::shared_ptr<const Library> const_ptr

    Convenience typedefs used in interfaces

  .. cpp:function:: std::string getName() const

    The file name for this library.

  .. cpp:function:: std::string getAbsoluteName() const

    Returns a file name for this Library that does not contain symlinks or a relative path.

  .. cpp:function:: Dyninst::Address getLoadAddress() const

    Returns the load address for this Library.

    The AIX operating system can have two load addresses for a library: one
    for the code region and one for the data region. On non-AIX systems, returns 0.

  .. cpp:function:: Dyninst::Address getDataLoadAddress() const

    Returns the load address of the code region.

    The AIX operating system can have two load addresses for a library: one
    for the code region and one for the data region. On non-AIX systems, returns 0.

  .. cpp:function:: Dyninst::Address getDynamicAddress() const

    Returns the address of the dynamic section.

    Returns 0 on non ELF-based systems (e.g., Windows).

  .. cpp:function:: bool isSharedLib() const

    Returns ``true`` if this is a shared library.

  .. cpp:function:: void setData(void *p) const

    Associate an opaque data object with the library.

    ProcControlAPI does not try to interpret this value, but it can be retrieved via :cpp:func:`getData`.

  .. cpp:function:: void* getData() const

    Returns an opaque data object that user code can associate with this library.

    Use :cpp:func:`setData` to set this opaque value.

.. cpp:class:: LibraryPool

  A container representing the executable and set of shared libraries (e.g., .dll and .so libraries) loaded into the target process’
  address space. A statically linked target process will only have a
  single executable, while a dynamically linked target process will have
  an executable and zero or more shared libraries.

    .. cpp:class:: iterator

      Helper class modelling the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

      The underlying ``value_type`` is :cpp:type:`Library::ptr` or :cpp:type:`Library::const_ptr`.

  .. cpp:class:: const_iterator

    A ``const`` version of :cpp:class:`iterator`.

  .. cpp:function:: iterator begin()

    Returns a ``const`` iterator pointing to the beginning of the pool.

  .. cpp:function:: const_iterator begin() const

    Returns an iterator pointing to the beginning of the pool.

  .. cpp:function:: iterator end()

    Returns an iterator marking the end of the pool.

  .. cpp:function:: const_iterator end() const

    Returns a ``const`` iterator marking the end of the pool.

  .. cpp:function:: size_t size() const

    Returns the number of elements in the library set

  .. cpp:function:: ptr getExecutable()

    Returns a ``const`` pointer to the :cpp:class:`Library` object that represents the target process’ executable.

  .. cpp:function:: const_ptr getExecutable() const

    Returns a pointer to the :cpp:class:`Library` object that represents the target process’ executable.

  .. cpp:function:: Library::ptr getLibraryByName(std::string name)

    Returns a pointer to the :cpp:class:`Library` object that with a file name equal to name.

    If no library is found, a value equivalent to :cpp:func:`end()` is returned.

  .. cpp:function:: Library::const_ptr getLibraryByName(std::string name) const

    Returns a ``const`` pointer to the :cpp:class:`Library` object that with a file name equal to name.

    If no library is found, a value equivalent to :cpp:func:`end()` is returned.

.. cpp:class:: Process

  The primary handle for operating on a single target process. ``Process`` objects may be
  created by calls to :cpp:func:`Process::createProcess` or :cpp:func:`Process::attachProcess`,
  or in response to certain types of events (e.g, fork on UNIX systems).

  .. cpp:type:: boost::shared_ptr<Process> ptr
  .. cpp:type:: boost::shared_ptr<const Process> const_ptr

    Convenience typedefs used in interfaces

  .. cpp:enum:: cb_action_t

    .. cpp:enumerator:: cbDefault
    .. cpp:enumerator:: cbThreadContinue
    .. cpp:enumerator:: cbThreadStop
    .. cpp:enumerator:: cbProcContinue
    .. cpp:enumerator:: cbProcStop

      The return type for callback functions registered through :cpp:func:`registerEventCallback`.
      A callback function can specify whether the thread or process associated with its event
      should be stopped or continued by respectively returning ``cbThreadContinue``,
      ``cbThreadStop``, ``cbProcContinue``, or ``cbProcStop``. ``cbDefault`` returns a Process and
      Thread to the original state before the event occurred.

    .. cpp:struct:: cb_ret_t

      .. cpp:member:: cb_action_t parent
      .. cpp:member:: cb_action_t child

        Some events, such as process spawn or thread create involve two
        processes or threads. In this case the ProcControlAPI user can specify a
        cb_action_t value for both the parent and child.

    .. cpp:type:: cb_ret_t(*cb_func_t)(Event::const_ptr)

      A function pointer type for functions that can handle event callbacks.
      The parameter is the :cpp:class:`Event` that triggered the callback. The
      return value indicates what action to take after handling the event.

  .. cpp:type:: std::pair<Dyninst::Address, Dyninst::Address> MemoryRegion

    The start and end addresses of a region of allocated memory.

  .. cpp:function:: static Process::ptr createProcess(std::string executable,const std::vector<std::string> &argv,const std::vector<std::string> &envp = emptyEnv,const std::map<int,int> &fds = emptyFDs)

    Creates a new process by launching an executable file
    specified by ``executable`` with the arguments in ``argv``, the
    environment in ``envp``, and returns a pointer to the new
    :cpp:class:`Process` object upon success.

    The new process will be created with its initial thread in the stopped state.

    If ``fds`` map is not empty, then the new process will be created with
    the file descriptors from its keys ``dup2`` mapped to the
    file descriptors in its values. If ``envp`` is empty, the environment will
    be inherited from the calling process.

    .. attention:: ProcControlAPI may deliver callbacks when this function is called.

    Returns an empty :cpp:type:`Process::ptr` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function from within a callback function

  .. cpp:function:: static Process::ptr attachProcess(Dyninst::PID pid, std::string executable = "")

    Creates a new Process object by attaching to the PID ``pid``.

    The new Process object will be returned from this
    function upon success. The ``executable`` argument is optional, and can be
    used to assist ProcControlAPI in finding the process’ executable on
    operating systems where this cannot be easily determined. The new process
    will be returned with all of its threads in the stopped state.

    .. attention:: ProcControlAPI may deliver callbacks when this function is called.

    Returns an empty :cpp:type:`Process::ptr` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function from within a callback function

  .. cpp:function:: static bool handleEvents(bool block)

    Requests ProcControlAPI to handle any pending debug events and deliver callbacks.

    When an event requires a callback, ProcControlAPI
    needs control of the main thread in order to deliver the callback. This
    function gives control of the main thread to ProcControlAPI for callback
    delivery. A user can know when to call handleEvents by using the
    :cpp:class:`EventNotify` interface.

    If ``block`` is ``true``, the process blocks until at least one debug event
    has been handled. Otherwise, returns immediately if no events are ready to be handled.

    Returns ``true`` if it handled at least one event.

    .. error:: It is an error to call this function from within a callback function

  .. cpp:function:: static bool registerEventCallback(EventType evt, cb_func_t cbfunc)

    Register a new callback function with ProcControlAPI.

    Upon receiving an event with type ``evt``, ProcControlAPI will deliver a
    callback with that event to ``cbfunc``. Multiple functions can
    be registered to receive callbacks for a single :cpp:class:`EventType`, and a single
    function can be registered with multiple EventTypes.

    If multiple callback functions are registered with a single :cpp:class:`EventType`,
    then it is undefined what order those callback functions will be invoked
    in. In this case the result of the last callback function
    called will be used to determine what stop or continue operations should
    be performed on the process. If a single callback function is registered
    for the same :cpp:class:`EventType` multiple times, then ProcControlAPI will only
    invoke one call to the callback function for each instance of the
    :cpp:class:`EventType`.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

  .. cpp:function:: static bool removeEventCallback(EventType evt)

    Unregisters all callback functions associated with the :cpp:class:`EventType` ``evt``.

    On success, ProcControlAPI will stop delivering callbacks for evt until a new callback is
    registered.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

  .. cpp:function:: static bool removeEventCallback(cb_func_t func)

    Unregisters all instances ``func`` from any callback with any :cpp:class:`EventType`.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

  .. cpp:function:: Dyninst::PID getPid() const

    Returns an OS-specific handle referencing the process. On UNIX systems this is the pid of the process.

  .. cpp:function:: Dyninst::Architecture getArchitecture() const

    Returns the architecture of the target process.

  .. cpp:function:: Dyninst::OSType getOS () const

    Returns the OS of the target process.

  .. cpp:function:: bool supportsLWPEvents () const

    Returns ``true`` if the target process can throw :cpp:type:`Dyninst::LWP` create and destroy events.

  .. cpp:function:: bool supportsUserThreadEvents () const

    Returns ``true`` if the target process can throw user thread create and destroy events.

  .. cpp:function:: bool supportsFork () const

    Returns ``true`` if the fork system call is supported in the target process.

  .. cpp:function:: bool supportsExec () const

    Returns ``true`` if the POSIX `exec <https://www.man7.org/linux/man-pages/man3/exec.3.html>`_ system call is
    supported in the target process.

  .. cpp:function:: bool isTerminated() const

    Returns ``true`` if the target process has terminated (either via a crash or normal exit)
    or if the ProcControlAPI has detached from the target process.

  .. cpp:function:: bool isExited() const

    Returns ``true`` of the target process exited normally (e.g, calling the exit function or returning from main).

  .. cpp:function:: int getExitCode() const

    If a target process exited normally, then returns its exit code.

    .. warning:: The result of ``getExitCode`` is undefined if the process has not yet exited. See :cpp:func:`isExited`.

  .. cpp:function:: bool isCrashed() const

    Returns ``true`` if the target process exited because of a crash.

  .. cpp:function:: int getCrashSignal() const

    If a target process exited because of a crash, then returns the signal that caused the target process to crash.

    .. warning:: The result of ``getCrashSignal`` is undefined if the process has not yet exited. See :cpp:func:`isCrashed`.

  .. cpp:function:: bool hasStoppedThread() const

    Returns ``true`` if the target process has at least one thread in the stopped state.
    It returns ``false``, otherwise, or if an error occurs. The specific error can be
    retrieved from :cpp:func:`getLastError`.

  .. cpp:function:: bool hasRunningThread() const

    Returns ``true`` if the target process has at least one thread in the running state.

    It returns ``false``, otherwise, or if an error occurs. The specific error can be retrieved from :cpp:func:`getLastError`.

  .. cpp:function:: bool allThreadsStopped() const

    Returns ``true`` if **all** threads in the target process are in the stopped state.

    It returns ``false``, otherwise, or if an error occurs. The specific error can be retrieved from :cpp:func:`getLastError`.

  .. cpp:function:: bool allThreadsRunning() const

    Returns ``true`` if **all** threads in the target process are in the running state.

    Returns ``false``, otherwise, or if an error occurs. The specific error can be retrieved from :cpp:func:`getLastError`.

  .. cpp:function:: bool allThreadsRunningWhenAttached() const

    Returns ``true`` if **all** threads were running when the controller process attached to this process or
    if the target process was created instead of attached.

  .. cpp:function:: bool continueProc()

    Moves **all** threads in the target process into the running state.

    .. attention:: ProcControlAPI may deliver callbacks when this function is called.

    Returns ``true`` if at least one thread was continued as part of the call.

    .. error:: It is an error to call this function from within a callback function

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

  .. cpp:function:: bool stopProc()

    Moves **all** threads in the target process into the stopped state.

    Returns ``true`` if at least one thread was stopped as part of the call.

    .. attention:: ProcControlAPI may deliver callbacks when this function is called.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function from within a callback function

  .. cpp:function:: bool detach(bool leaveStopped = false)

    Detaches ProcControlAPI from the target process.

    ProcControlAPI will no longer be able to
    control or receive events from the target process. All breakpoints will be removed from the target.
    If the ``leaveStopped`` parameter is set to ``true``, and the process is in a
    stopped state, then the target process will be left in a stopped state
    after the detach.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function from within a callback function

  .. cpp:function:: bool temporaryDetach()

    Temporarily detaches from the target process.

    This functionality is commonly called `detach-on-the-fly`. The target process will not
    report new events nor be controllable or able to be queried by the user. Breakpoints are removed
    from the process. Call :cpp:func:`reAttach` to reconnect the process.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function from within a callback function

  .. cpp:function:: bool reAttach()

    Reconnects to the target process after calling :cpp:func:`temporaryDetach`.

    Any breakpoints will be re-inserted back into the function, and if threads have been created or destroyed during the
    time detached new events will be thrown for them.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function from within a callback function

  .. cpp:function:: bool terminate()

    Forcefully terminates the target process.

    On success, the target process will end execution. The :cpp:class:`Process` object will record the target process as having crashed.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function from within a callback function

  .. cpp:function:: ThreadPool const& threads() const

    Returns a reference to the internal :cpp:class:`ThreadPool` that can be used to iterate
    over and query the :cpp:class:`Thread` objects.

  .. cpp:function:: LibraryPool const& libraries() const

    Returns a reference to the internal :cpp:class:`LibraryPool` that can be used to iterate
    over and query the :cpp:class:`LibraryPool` objects.

  .. cpp:function:: bool addLibrary(std::string libname)

    Loads a library into the process' memory space. An event is triggered (and thus a user callback) for each
    library loaded, including dependencies.

  .. cpp:function:: void setData (void* p) const

    Inserts an opaque handle to user-defined data. The data is not interpreted by ProcControlAPI, but
    remains associated with the process.

  .. cpp:function:: void* getData() const

    Returns the value of the opaque handled inserted with :cpp:func:`setData`.

  .. cpp:function:: unsigned getMemoryPageSize() const

    Returns memory page size for the current OS on which the target process is running.

  .. cpp:function:: Dyninst::Address mallocMemory(size_t size)

    Allocates a region of memory in the target process’ address space of ``size`` bytes that is readable, writeable,
    and executable at any available address.

    .. attention:: ProcControlAPI may deliver callbacks when this function is called.

    On success, returns the start address of memory that was allocated.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function from within a callback function

  .. cpp:function:: Dyninst::Address mallocMemory(size_t size, Dyninst::Address addr)

    Allocates a region of memory in the target process’ address space of ``size`` bytes that is readable, writeable,
    and executable at the specified address.

    .. attention:: ProcControlAPI may deliver callbacks when this function is called.

    On success, returns the start address of memory that was allocated.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function from within a callback function

  .. cpp:function:: bool freeMemory(Dyninst::Address addr)

    Free memory allocated by :cpp:func:`mallocMemory`.

    On success, the area of memory starting at the address, ``addr``, will be unmapped and no longer accessible to
    the target process.

    .. attention:: ProcControlAPI may deliver callbacks when this function is called.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function from within a callback function

    .. error:: It is an error to call this function with an address that was not returned by :cpp:func:`mallocMemory`.

  .. cpp:function:: bool writeMemory(Dyninst::Address addr, void* buffer, size_t size) const

    Write to the target process’s memory, starting at address ``addr``, ``size`` bytes of ``buffer``

    .. error:: It is an error to call this function on a process that does not have at least one :cpp:class:`Thread` in a stopped state.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

  .. cpp:function:: bool readMemory(void* buffer, Dyninst::Address addr, size_t size) const

    Read into into ``buffer`` ``size`` bytes from the target process’ memory starting at address ``addr``.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function on a Process that does not have at least one :cpp:class:`Thread` in a stopped state.

  .. cpp:function:: bool getMemoryAccessRights(Dyninst::Address addr, mem_perm& rights)

    Returns the memory permissions at the specified address.

  .. cpp:function:: bool setMemoryAccessRights(Dyninst::Address addr, size_t size, mem_perm rights, mem_perm& oldRights)

    Starting at the address, ``addr``, and extending ``size`` bytes, sets the memory page's permissions to ``rights``. The
    previous value of the permissions are returned in ``oldRights``.

  .. cpp:function:: bool findAllocatedRegionAround(Dyninst::Address addr, MemoryRegion& memRegion)

    Searches for a region of allocated memory that contains the address, ``addr``.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

  .. cpp:function:: bool addBreakpoint(Dyninst::Address addr, Breakpoint::ptr bp) const

    Inserts a :cpp:class:`Breakpoint` into the target process at address ``addr``.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function on a process that does not have at least one thread in a stopped state.

  .. cpp:function:: bool rmBreakpoint(Dyninst::Address addr, Breakpoint::ptr bp) const

    Removes a :cpp:class:`Breakpoint` from the target process at address ``addr``.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

  .. cpp:function:: bool postIRPC(IRPC::ptr irpc) const

    Posts the given irpc to the process.

    ProcControlAPI selects a :cpp:class:`Thread` from the process to run the :ref:`sec:proccontrol-intro-irpcs` on
    and puts it into that Thread’s queue of posted IRPCs. Each instance of an IRPC object can be posted at most once.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to attempt to post a single IRPC object multiple times.

  .. cpp:function:: bool runIRPCSync(IRPC::ptr irpc)

    Posts an irpc, similar to Process::postIRPC, continues the
    thread the irpc was posted to, and returns when the irpc has completed
    running.

    The thread will be returned to its original running state upon completion.

    .. warning:: Stopping the thread that is running the irpc while this function waits for irpc completion causes this function to return an error.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function from within a callback function

  .. cpp:function:: bool runIRPCAsync(IRPC::ptr irpc)

    Posts an irpc, similar to Process::postIRPC, and then continues the thread the irpc was posted to.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function from within a callback function

  .. cpp:function:: bool getPostedIRPCs(std::vector<IRPC::ptr> &rpcs) const

    Retrieves all IRPCs posted to this process.

    This list does not include any IRPCs currently running (see :cpp:func:`Thread::getRunningIRPC()` for this functionality.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

  .. cpp:function:: LibraryTracking* getLibraryTracking()

    Returns platform-specific configuration for handling library events for the process.

  .. cpp:function:: ThreadTracking* getThreadTracking()

    Returns platform-specific configuration for handling thread events for the process.

    Return `nullptr`, if the specified feature is unsupported on the current platform.

  .. cpp:function:: LWPTracking* getLWPTracking()

    Returns platform-specific configuration for handling LWP events for the process.

    Return `nullptr`, if the specified feature is unsupported on the current platform.

  .. cpp:function:: FollowFork* getFollowFork()

    Returns platform-specific configuration for handling fork events for the process.

    Returns `nullptr`, if the specified feature is unsupported on the current platform.

  .. cpp:function:: SignalMask* getSignalMask()

    Returns platform-specific configuration for configuring signal masks for the process.

    Returns `nullptr`, if the specified feature is unsupported on the current platform.

  .. cpp:class:: mem_perm

    Represents general memory page permission for the given memory page in the process.

    .. cpp:function:: mem_perm()

      Initializes permissions to non-readable, non-writable, and non-executable.

    .. cpp:function:: mem_perm(bool r, bool w, bool x)

      Initializes permissions for reading, `r`, writing, `w`, and execution, `x`.

    .. cpp:function:: bool getR() const

      Returns ``true`` if the memory page is readable

    .. cpp:function:: bool getW() const

      Returns ``true`` if the memory page is writable

    .. cpp:function:: bool getX() const

      Returns ``true`` if the memory page is executable

    .. cpp:function:: bool isNone() const

      Returns ``true`` if the memory page is not accessible

    .. cpp:function:: bool isR() const

      Returns ``true`` if the memory page is readable

    .. cpp:function:: bool isX() const

      Returns ``true`` if the memory page is executable

    .. cpp:function:: bool isRW() const

      Returns ``true`` if the memory page is readable and writable

    .. cpp:function:: bool isRX() const

      Returns ``true`` if the memory page is readable and executable

    .. cpp:function:: bool isRWX() const

      Returns ``true`` if the memory page is readable, writable, and executable

    .. cpp:function:: mem_perm& setR()

      Make the memory page readable. Returns a reference to itself.

    .. cpp:function:: mem_perm& setW()

      Make the memory page writable. Returns a reference to itself.

    .. cpp:function:: mem_perm& setX()

      Make the memory page executable. Returns a reference to itself.

    .. cpp:function:: mem_perm& clrR()

      Make the memory page unreadable. Returns a reference to itself.

    .. cpp:function:: mem_perm& clrW()

      Make the memory page unwritable. Returns a reference to itself.

    .. cpp:function:: mem_perm& clrX()

      Make the memory page unexecutable. Returns a reference to itself.

    .. cpp:function:: bool operator<(const mem_perm& p) const

      Permissions are comparable in the sense that read, write, and execute permissions encode to the values in :cpp:var:`Breakpoint::BP_R`,
      :cpp:var:`Breakpoint::BP_W`, and :cpp:var:`Breakpoint::BP_X`, respectively.

    .. cpp:function:: std::string getPermName()

      Returns a string representation of the permissions.

.. cpp:class:: RegisterPool

  A collection of registers used to get or set all registers in a :cpp:class:`Thread` at once.

  .. cpp:class:: iterator

    Helper class modelling the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

    The underlying ``value_type`` is :cpp:texpr:`std::pair<Dyninst::MachRegister, Dyninst::MachRegisterVal>`.

  .. cpp:class:: const_iterator

    A ``const`` version of :cpp:class:`iterator`.

  .. cpp:function:: iterator begin()

    Returns a ``const`` iterator pointing to the beginning of the pool.

  .. cpp:function:: const_iterator begin() const

    Returns an iterator pointing to the beginning of the pool.

  .. cpp:function:: iterator end()

    Returns an iterator marking the end of the pool.

  .. cpp:function:: const_iterator end() const

    Returns a ``const`` iterator marking the end of the pool.

  .. cpp:function:: size_t size() const

    Returns the number of elements in the pool.

  .. cpp:function:: const_iterator find(Dyninst::MachRegister r) const

    Returns an iterator that points to the element in the register pool that equals
    register ``r``. If not found, then returns :cpp:func:`end()`.

  .. cpp:function:: Dyninst::MachRegisterVal& operator[](Dyninst::MachRegister r)

    Returns a reference to the value associated with the register ``r`` in this register pool.

    If ``r`` is not found, a default :cpp:type:`Dyninst::MachRegisterVal` is created and returned.

  .. cpp:function:: Dyninst::MachRegisterVal const& operator[](Dyninst::MachRegister r) const

    Returns a reference to the value associated with the register ``r`` in this register pool.

    If ``r`` is not found, a default :cpp:type:`Dyninst::MachRegisterVal` is created and returned.

.. cpp:class:: Thread

  Represents a single thread of execution in the target process. Any :cpp:class:`Process` has
  `at least` one Thread, and multi-threaded target processes may have more. Each Thread has an
  associated integral value known as its :cpp:type:`Dyninst::LWP` that serves as a handle for communicating with the OS
  about the thread (e.g., a PID value on Linux). On some systems, depending on availability, a Thread
  may have information from the user space threading library.

  .. cpp:type:: boost::shared_ptr<Thread> ptr
  .. cpp:type:: boost::shared_ptr<const Thread> const_ptr

    Convenience typedefs used in interfaces

  .. cpp:function:: Dyninst::LWP getLWP() const

    Returns an OS handle for this thread.

    On Linux, returns a `pid_t <https://man7.org/linux/man-pages/man2/getpid.2.html>`_.
    On FreeBSD, returns a `lwpid_t <https://man.freebsd.org/cgi/man.cgi?query=libthr&sektion=3&manpath=freebsd-release-ports>`_.

  .. cpp:function:: Process::ptr getProcess()

    Returns a pointer to the :cpp:class:`Process` that contains this thread.

  .. cpp:function:: Process::const_ptr getProcess() const

    Returns a ``const`` pointer to the :cpp:class:`Process` that contains this thread.

  .. cpp:function:: bool isStopped() const

    Returns ``true`` if this thread is in a stopped state.

  .. cpp:function:: bool isRunning() const

    Returns ``true`` if this thread is in a running state.

  .. cpp:function:: bool isLive() const

    Returns ``true`` if this thread is alive. Returns ``false`` if this thread has been destroyed.

  .. cpp:function:: bool isDetached() const

    Returns ``true`` if this thread has been detached via :cpp:func:`Process::temporaryDetach`.

  .. cpp:function:: bool isInitialThread() const

    Returns ``true`` if this thread is the initial thread for the process.

  .. cpp:function:: bool stopThread()

    Moves the thread to into a stopped state.

    On success, the thread will be paused and not resume execution until it is continued.

    .. attention:: ProcControlAPI may deliver callbacks when this function is called.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function from within a callback function.

    A callback can stop a thread by returning :cpp:enumerator:`Process::cb_action_t::cbThreadStop` or :cpp:enumerator:`Process::cb_action_t::cbProcStop`.

  .. cpp:function:: bool continueThread()

    Moves the thread into the running state.

    .. attention:: ProcControlAPI may deliver callbacks when this function is called.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function from within a callback function.

    Instead of calling this function, a callback can stop a thread by returning :cpp:enumerator:`Process::cb_action_t::cbThreadContinue` or
    :cpp:enumerator:`Process::cb_action_t::cbProcContinue`.

  .. cpp:function:: bool getRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal &val) const

    Gets the value of a single register from this thread.

    The register is specified by the reg parameter, and the value of the
    register is returned by the val parameter.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function on a thread that is not in the stopped state.

  .. cpp:function:: bool getAllRegisters(RegisterPool pool) const

    Reads the values of every register in the thread and saves them in ``pool``.

    Depending on the OS, this call may be more efficient that calling :cpp:func:`Thread::getRegister`
    multiple times.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function on a thread that is not in the stopped state.

  .. cpp:function:: bool setRegister(Dyninst::MachRegister reg, Dyninst::MachRegisterVal val) const

    Stores ``val`` in ``reg``. 

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function on a thread that is not in the stopped state.

  .. cpp:function:: bool setAllRegisters(RegisterPool &pool) const

    Sets the values of every register in this thread to the values specified in ``pool``.

    Depending on the OS, this call may be more efficient that calling :cpp:func:`Thread::setRegister`
    multiple times.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function on a thread that is not in the stopped state.

  .. cpp:function:: bool haveUserThreadInfo() const

    Returns ``true`` if information about this Thread’s underlying
    user-level thread is available.

  .. cpp:function:: Dyninst::THR_ID getTID() const

    Returns the unique identifier for the user-level thread.

    .. warning:: :cpp:func:`haveUserThreadInfo` should be checked before calling this function.

  .. cpp:function:: Dyninst::Address getStartFunction() const

    Returns the address of the initial function for the user-level thread.

    .. warning:: :cpp:func:`haveUserThreadInfo` should be checked before calling this function.

  .. cpp:function:: Dyninst::Address getStackBase() const

    Returns the address of the bottom of the user-level thread’s stack.

    .. warning:: :cpp:func:`haveUserThreadInfo` should be checked before calling this function.

  .. cpp:function:: unsigned long getStackSize() const

    Returns the size in bytes of the user-level thread’s allocated stack.

    .. warning:: :cpp:func:`haveUserThreadInfo` should be checked before calling this function.

  .. cpp:function:: Dyninst::Address getTLS() const

    Returns the address of the user-level thread’s thread local storage area.

    .. warning:: :cpp:func:`haveUserThreadInfo` should be checked before calling this function.

  .. cpp:function:: bool readThreadLocalMemory(void* buffer, Library::const_ptr lib, Dyninst::Offset tls_symbol_offset, size_t size) const

    Reads from a symbol in thread local storage (TLS) memory.

    TLS is memory that is local to a thread and has a lifetime matching the
    thread. The tls_symbol_offset is the TLS symbol’s offset in lib, and can
    be found by reading a TLS symbol’s value. The lib parameter can point to
    a library or the executable. The buffer parameter specifies an address
    in the controller process where ProcControlAPI should write the copied
    bytes.

    .. warning:: :cpp:func:`haveUserThreadInfo` should be checked before calling this function.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function on a Thread that is not in a stopped state.

  .. cpp:function:: bool writeThreadLocalMemory(Library::const_ptr lib, Dyninst::Offset tls_symbol_offset, const void* buffer, size_t size) const

    This function writes to a symbol in thread local storage (TLS) memory.
    TLS is memory that is local to a thread and has a lifetime matching the
    thread. The tls_symbol_offset is the TLS symbol’s offset in lib, and can
    be found by reading a TLS symbol’s value. The lib parameter can point to
    a library or the executable. The buffer parameter specifies an address
    in the controller process where ProcControlAPI should read the bytes to
    be copied.

    .. warning:: :cpp:func:`haveUserThreadInfo` should be checked before calling this function.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function on a Thread that is not in a stopped state.

  .. cpp:function:: bool getThreadLocalAddress(Library::const_ptr lib, Dyninst::Offset tls_symbol_offset, Dyninst::Address &result_addr) const

    This function looks up the address of a symbol in thread local storage
    (TLS) memory. The tls_symbol_offset is the TLS symbol’s offset in lib,
    and can be found by reading a TLS symbol’s value. The lib parameter can
    point to a library or the executable. The result_addr parameter will be
    set to the target address for the TLS symbol in this Thread.

    .. warning:: :cpp:func:`haveUserThreadInfo` should be checked before calling this function.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to call this function on a Thread that is not in a stopped state.

  .. cpp:function:: bool postIRPC(IRPC::ptr irpc) const

    Posts the given irpc to the thread.

    The IRPC is put irpc into the Thread’s queue of posted IRPCs and will be run when ready.
    See :ref:`sec:proccontrol-intro-irpcs` for details on iRPCs.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

    .. error:: It is an error to attempt to post a single IRPC object multiple times.

  .. cpp:function:: bool getPostedIRPCs(std::vector<IRPC::ptr> &rpcs) const

    Returns all IRPCs posted to this thread.

    This does not include any running IRPC.

    Returns ``false`` on error. The specific error can be retrieved from :cpp:func:`getLastError`.

  .. cpp:function:: IRPC::const_ptr getRunningIRPC() const

    Returns any IRPC that is actively running on this thread.

    If there is no IRPC actively running, then this function returns any empty :cpp:type:`IRPC::const_ptr`.

  .. cpp:function:: void setSingleStepMode(bool mode) const

    Toggle single-step mode for thread.

    A thread in single-step mode will pause execution at each instruction
    and trigger an :cpp:class:`EventSingleStep` event. After each ``EventSingleStep`` is
    handled (and presuming the thread is still running and in single-step
    mode) it will execute one more instruction and trigger another ``EventSingleStep``.

  .. cpp:function:: bool getSingleStepMode() const

    Returns ``true`` if the Thread is in single-step mode.

  .. cpp:function:: void setData(void *p) const

    Associate an opaque data object with the thread.

    ProcControlAPI does not try to interpret this value, but will return it with :cpp:func:`getData`.

  .. cpp:function:: void* getData() const

    Returns an opaque data object that user code can associate with this thread.

    Use :cpp:func:`setData` to set this opaque value.

.. cpp:class:: ThreadPool

  A collection holding the :cpp:class:`Thread` objects that make up
  a :cpp:class:`Process`. Each Process object has one ThreadPool, and each
  ThreadPool has one or more Threads.

  .. attention::
    It is not safe to make assumptions about having consistent contents of a ThreadPool
    for a running process. As the target runs, threads may be inserted or removed. It is
    generally safer to stop the process before operating on its ThreadPool. When used on a
    running process the :cpp:class:`iterator` methods are guaranteed not to return invalid
    Thread objects (e.g, nothing that would lead to a segfault), but they do not guarantee that
    they will refer to live threads or even return all threads.

  .. cpp:class:: iterator

    Helper class modelling the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

    The underlying ``value_type`` is :cpp:type:`Thread::ptr`.

  .. cpp:class:: const_iterator

    A ``const`` version of :cpp:class:`iterator`.

  .. cpp:function:: iterator begin()

    Returns a ``const`` iterator pointing to the beginning of the pool.

  .. cpp:function:: const_iterator begin() const

    Returns an iterator pointing to the beginning of the pool.

  .. cpp:function:: iterator end()

    Returns an iterator marking the end of the pool.

  .. cpp:function:: const_iterator end() const

    Returns a ``const`` iterator marking the end of the pool.

  .. cpp:function:: iterator find(Dyninst::LWP lwp)

    Return an iterator to the thread with a :cpp:type:`Dyninst::LWP` equal to ``lwp``.

    If not found, returns :cpp:func:`end`.

  .. cpp:function:: size_t size() const

    Returns the number of threads in the pool.

  .. cpp:function:: Process::ptr getProcess()

    Returns a pointer to the :cpp:class:`Process` that owns this pool.

  .. cpp:function:: Thread::ptr getInitialThread()

    Returns a pointer to the initial Thread in a Process.

    The initial thread is the thread that started execution of the process (i.e., the thread that called main).
