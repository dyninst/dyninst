procstate.h
===========

.. cpp:namespace:: Dyninst::stackwalk

Class ProcDebug
~~~~~~~~~~~~~~~

Access to StackwalkerAPI’s debugger is through the ``ProcDebug`` class,
which inherits from the ``ProcessState`` interface. The easiest way to
get at a ``ProcDebug`` object is to cast the return value of
``Walker::getProcessState`` into a ``ProcDebug``. C++’s ``dynamic_cast``
operation can be used to test if a ``Walker`` uses the ``ProcDebug``
interface:

.. code-block:: cpp

   ProcDebug *debugger;
   debugger = dynamic_cast<ProcDebug*>(walker->getProcessState());
   if (debugger != NULL) {
       //3rd party
       ...
   } else {
       //1st party
       ...
   }

In addition to the handling of debug events, described in
Section `4.1 <#subsec:debugger>`__, the ``ProcDebug`` class provides a
process control interface; users can pause and resume process or
threads, detach from a process, and test for events such as process
death. As an implementation of the ``ProcessState`` class, ``ProcDebug``
also provides all of the functionality described in
Section `3.6.4 <#subsec:processstate>`__.

.. code-block:: cpp

    virtual bool pause(Dyninst::THR_ID tid = NULL_THR_ID)

This method pauses a process or thread. The paused object will not
resume execution until ``ProcDebug::resume`` is called. If the ``tid``
parameter is not ``NULL_THR_ID`` then StackwalkerAPI will pause the
thread specified by ``tid``. If ``tid`` is ``NULL_THR_ID`` then
StackwalkerAPI will pause every thread in the process.

When StackwalkerAPI collects a call stack from a running thread it first
pauses the thread, collects the stack walk, and then resumes the thread.
When collecting a call stack from a paused thread StackwalkerAPI will
collect the stack walk and leave the thread paused. This method is thus
useful for pausing threads before stack walks if the user needs to keep
the returned stack walk synchronized with the current state of the
thread.

This method returns ``true`` if successful and ``false`` on error.

.. code-block:: cpp

    virtual bool resume(Dyninst::THR_ID tid = NULL_THR_ID)

This method resumes execution on a paused process or thread. This method
only resumes threads that were paused by the ``ProcDebug::pause`` call,
using it on other threads is an error. If the ``tid`` parameter is not
``NULL_THR_ID`` then StackwalkerAPI will resume the thread specified by
``tid``. If ``tid`` is ``NULL_THR_ID`` then StackwalkerAPI will resume
all paused threads in the process.

This method returns ``true`` if successful and ``false`` on error.

.. code-block:: cpp

    virtual bool detach(bool leave_stopped = false)

This method detaches StackwalkerAPI from the target process.
StackwalkerAPI will no longer receive debug events on this target
process and will no longer be able to collect call stacks from it. This
method invalidates the associated ``Walker`` and ``ProcState`` objects,
they should be cleaned using C++’s ``delete`` operator after making this
call. It is an error to attempt to do operations on these objects after
a detach, and undefined behavior may result.

If the ``leave_stopped`` parameter is ``true`` StackwalkerAPI will
detach from the process but leave it in a paused state so that it does
resume progress. This is useful for attaching another debugger back to
the process for further analysis. The ``leave_stopped`` parameter is not
supported on the Linux platform and its value will have no affect on the
detach call.

This method returns ``true`` if successful and ``false`` on error.

.. code-block:: cpp

    virtual bool isTerminated()

This method returns ``true`` if the associated target process has
terminated and ``false`` otherwise. A target process may terminate
itself by calling exit, returning from main, or receiving an unhandled
signal. Attempting to collect stack walks or perform other operations on
a terminated process is illegal an will lead to undefined behavior.

A process termination will also be signaled through the notification FD.
Users should check processes for the isTerminated state after returning
from handleDebugEvent.

.. code-block:: cpp

    static int getNotificationFD()

This method returns StackwalkerAPI’s notification FD. The notification
FD is a file descriptor that StackwalkerAPI will write a byte to
whenever a debug event occurs that need. If the user code sees a byte on
this file descriptor it should call ``handleDebugEvent`` to let
StackwalkerAPI handle the debug event. Example code using
``getNotificationFD`` can be found in
Section `4.1 <#subsec:debugger>`__.

StackwalkerAPI will only create one notification FD, even if it is
attached to multiple 3rd party target processes.

.. code-block:: cpp

    static bool handleDebugEvent(bool block = false)

When this method is called StackwalkerAPI will receive and handle all
pending debug events from each 3rd party target process to which it is
attached. After handling debug events each target process will be
continued (unless it was explicitly stopped by the ProcDebug::pause
method) and any bytes on the notification FD will be cleared. It is
generally expected that users will call this method when a event is sent
to the notification FD, although it can be legally called at any time.

If the ``block`` parameter is ``true``, then ``handleDebugEvents`` will
block until it has handled at least one debug event. If the block
parameter is ``false``, then handleDebugEvents will handle any currently
pending debug events or immediately return if none are available.

StackwalkerAPI may receive process exit events for target processes
while handling debug events. The user should check for any exited
processes by calling ``ProcDebug::isTerminated`` after handling debug
events.

This method returns ``true`` if successful and ``false`` on error.


**Defined in:** ``procstate.h``

StackwalkerAPI provides an interface to access the addresses where
libraries are mapped in the target process.

.. code-block:: cpp

    typedef std::pair<std::string, Address> LibAddrPair;

A pair consisting of a library filename and its base address in the
target process.

.. code-block:: cpp

    class LibraryState

Class providing interfaces for library tracking. Only the public query
interfaces below are user-facing; the other public methods are callbacks
that allow StackwalkerAPI to update its internal state.

.. code-block:: cpp

    virtual bool getLibraryAtAddr(Address addr, LibAddrPair &lib) = 0;

Given an address ``addr`` in the target process, returns ``true`` and
sets ``lib`` to the name and base address of the library containing
addr. Given an address outside the target process, returns ``false``.

.. code-block:: cpp

    virtual bool getLibraries(std::vector<LibAddrPair> &libs, bool allow_refresh = true) = 0;

Fills ``libs`` with the libraries loaded in the target process. If
``allow_refresh`` is true, this method will attempt to ensure that this
list is freshly updated via inspection of the process; if it is false,
it will return a cached list.

.. code-block:: cpp

    virtual bool getLibc(LibAddrPair &lc);

Convenience function to find the name and base address of the standard C
runtime, if present.

.. code-block:: cpp

    virtual bool getLibthread(LibAddrPair &lt);

Convenience function to find the name and base address of the standard
thread library, if present (e.g. pthreads).

.. code-block:: cpp

    virtual bool getAOut(LibAddrPair &ao) = 0;

Convenience function to find the name and base address of the
executable.

Class ProcessState
~~~~~~~~~~~~~~~~~~

**Defined in:** ``procstate.h``

The ProcessState class is a virtual class that defines an interface
through which StackwalkerAPI can access the target process. It allows
access to registers and memory, and provides basic information about the
threads in the target process. StackwalkerAPI provides two default types
of ``ProcessState`` objects: ``ProcSelf`` does a first party stackwalk,
and ``ProcDebug`` does a third party stackwalk.

A new ``ProcessState`` class can be created by inheriting from this
class and implementing the necessary methods.

.. code-block:: cpp

    static ProcessState *getProcessStateByPid(Dyninst::PID pid)

Given a ``PID``, return the corresponding ``ProcessState`` object.

.. code-block:: cpp

    virtual unsigned getAddressWidth() = 0;

Return the number of bytes in a pointer for the target process. This
value is 4 for 32-bit platforms (x86, PowerPC-32) and 8 for 64-bit
platforms (x86-64, PowerPC-64).

.. code-block:: cpp

    typedef enum Arch_x86, Arch_x86_64, Arch_ppc32, Arch_ppc64 Architecture;
    virtual Dyninst::Architecture getArchitecture() = 0;

Return the appropriate architecture for the target process.

.. code-block:: cpp

    virtual bool getRegValue(Dyninst::MachRegister reg, Dyninst::THR_ID
    thread, Dyninst::MachRegisterVal &val) = 0

This method takes a register name as input, ``reg``, and returns the
value in that register in ``val`` in the thread thread.

This method returns ``true`` on success and ``false`` on error.

.. code-block:: cpp

    virtual bool readMem(void *dest, Address source, size_t size) = 0

This method reads memory from the target process. Parameter ``dest``
should point to an allocated buffer of memory at least ``size`` bytes in
the host process. Parameter ``source`` should contain an address in the
target process to be read from. If this method succeeds, ``size`` bytes
of memory is copied from ``source``, stored in ``dest``, and ``true`` is
returned. This method returns ``false`` otherwise.

.. code-block:: cpp

    virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &threads) = 0

This method returns a list of threads whose call stacks can be walked in
the target process. Thread are returned in the ``threads`` vector. In
some cases, such as with the default ``ProcDebug``, this method returns
all of the threads in the target process. In other cases, such as with
``ProcSelf``, this method returns only the calling thread.

The first thread in the ``threads`` vector (index 0) will be used as the
default thread if the user requests a stackwalk without specifying an
thread (see ``Walker::WalkStack``).

This method returns ``true`` on success and ``false`` on error.

.. code-block:: cpp

    virtual bool getDefaultThread(Dyninst::THR_ID &default_tid) = 0

This method returns the thread representing the initial process in the
``default_tid`` output parameter.

This method returns ``true`` on success and ``false`` on error.

.. code-block:: cpp

    virtual Dyninst::PID getProcessId()

This method returns a process ID for the target process. The default
``ProcessState`` implementations (``ProcDebug`` and ``ProcSelf``) will
return a PID on UNIX systems and a HANDLE object on Windows.

.. code-block:: cpp

    Walker *getWalker() const;

Return the ``Walker`` associated with the current process state.

.. code-block:: cpp

    std::string getExecutablePath();

Returns the name of the executable associated with the current process
state.

Class LibraryState
~~~~~~~~~~~~~~~~~~

**Defined in:** ``procstate.h``

``LibraryState`` is a helper class for ``ProcessState`` that provides
information about the current DSOs (libraries and executables) that are
loaded into a process’ address space. FrameSteppers frequently use the
LibraryState to get the DSO through which they are attempting to stack
walk.

Each ``Library`` is represented using a ``LibAddrPair`` object, which is
defined as follows:

.. code-block:: cpp

    typedef std::pair<std::string, Dyninst::Address> LibAddrPair

``LibAddrPair.first`` refers to the file path of the library that was
loaded, and ``LibAddrPair.second`` is the load address of that library
in the process’ address space. The load address of a library can be
added to a symbol offset from the file in order to get the absolute
address of a symbol.

.. code-block:: cpp

    virtual bool getLibraryAtAddr(Address addr, LibAddrPair &lib) = 0

This method returns a DSO, using the ``lib`` output parameter, that is
loaded over address ``addr`` in the current process.

This method returns ``false`` if no library is loaded over ``addr`` or
an error occurs, and ``true`` if it successfully found a library.

.. code-block:: cpp

    virtual bool getLibraries(std::vector<LibAddrPair> &libs) = 0

This method returns all DSOs that are loaded into the process’ address
space in the output vector parameter, ``libs``.

This method returns ``true`` on success and ``false`` on error.

.. code-block:: cpp

    virtual void notifyOfUpdate() = 0

This method is called by the ``ProcessState`` when it detects a change
in the process’ list of loaded libraries. Implementations of
``LibraryStates`` should use this method to refresh their lists of
loaded libraries.

.. code-block:: cpp

    virtual Address getLibTrapAddress() = 0

Some platforms that implement the System/V standard (Linux) use a trap
event to determine when a process loads a library. A trap instruction is
inserted into a certain address, and that trap will execute whenever the
list of loaded libraries change.

On System/V platforms this method should return the address where a trap
should be inserted to watch for libraries loading and unloading. The
ProcessState object will insert a trap at this address and then call
notifyOfUpdate when that trap triggers.

On non-System/V platforms this method should return 0.
