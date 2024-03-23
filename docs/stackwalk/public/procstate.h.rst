.. _`sec:procstate.h`:

procstate.h
###########

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:class:: ProcessState

  **An abstract class for accessing a process**

  It allows access to registers and memory, and provides basic information about the
  threads in the target process.

  .. cpp:function:: static ProcessState* getProcessStateByPid(Dyninst::PID pid)

      Return the ``ProcessState`` for the process with id ``pid``.

  .. cpp:function:: virtual bool getRegValue(Dyninst::MachRegister reg, Dyninst::THR_ID thread, \
                                             Dyninst::MachRegisterVal &val) = 0

      Returns in ``val`` the value contained in register ``reg`` in the thread with id ``thread``.

      Returns ``false`` on error.

  .. cpp:function:: virtual bool readMem(void *dest, Dyninst::Address source, size_t size) = 0

      Stores in ``dest`` ``size`` *bytes* of the memory contents starting at ``source`` in the
      target process.
       
      Returns ``false`` on error.

  .. cpp:function:: virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &threads) = 0

      Returns in ``threads`` the threads in this process with call stacks that can be walked.
      
      In some cases, such as with the default ``ProcDebug``, this method returns
      all of the threads in the target process. In other cases, such as with
      ``ProcSelf``, this method returns only the calling thread. The first thread in ``threads``
      will be used as the default thread if the user requests a stackwalk without specifying a
      thread.

      Returns ``false`` on error.

  .. cpp:function:: virtual bool getDefaultThread(Dyninst::THR_ID &default_tid) = 0

      Returns in ``default_tid`` the thread representing the initial process.

      Returns ``false`` on error.

  .. cpp:function:: virtual Dyninst::PID getProcessId()

      Returns the process ID for the target process.

  .. cpp:function:: virtual unsigned getAddressWidth() = 0

      Returns the number of bytes in a pointer for the target process.
      
      This is 4 for 32-bit platforms (x86, PowerPC-32) and 8 for 64-bit
      platforms (x86-64, PowerPC-64).

  .. cpp:function:: virtual Dyninst::Architecture getArchitecture() = 0

      Returns the architecture for the target process.

  .. cpp:function:: Walker *getWalker() const

      Returns the walker associated with the current process state.

  .. cpp:function:: virtual bool isFirstParty() = 0

      Checks if this is a first-party process.

  .. cpp:function:: std::string getExecutablePath()

      Returns the name of the executable associated with the current process state.


.. cpp:class:: ProcSelf : public ProcessState

  **State for first-party walkers**
  
  See :ref:`topic:stackwalk-first-party` for context.

  .. cpp:function:: ProcSelf(std::string exe_path = std::string(""))
  .. cpp:function:: void initialize()
  .. cpp:function:: virtual bool getRegValue(Dyninst::MachRegister reg, Dyninst::THR_ID thread, Dyninst::MachRegisterVal &val)
  .. cpp:function:: virtual bool readMem(void *dest, Dyninst::Address source, size_t size)
  .. cpp:function:: virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &threads)
  .. cpp:function:: virtual bool getDefaultThread(Dyninst::THR_ID &default_tid)
  .. cpp:function:: virtual unsigned getAddressWidth()
  .. cpp:function:: virtual bool isFirstParty()
  .. cpp:function:: virtual Dyninst::Architecture getArchitecture()
  .. cpp:function:: virtual ~ProcSelf()


.. cpp:class:: ProcDebug : public ProcessState

  **State for third-party walkers**
  
  See :ref:`topic:stackwalk-third-party` for context.

  In addition to the handling of debug events, ``ProcDebug`` provides a
  process control interface. Users can pause and resume process or
  threads, detach from a process, and test for events such as process
  death.

  .. cpp:function:: static ProcDebug *newProcDebug(Dyninst::PID pid, std::string executable="")
  .. cpp:function:: static ProcDebug *newProcDebug(Dyninst::ProcControlAPI::Process::ptr proc)
  .. cpp:function:: static bool newProcDebugSet(const std::vector<Dyninst::PID> &pids, std::vector<ProcDebug *> &out_set)
  .. cpp:function:: static ProcDebug *newProcDebug(std::string executable, const std::vector<std::string> &argv)

  .. cpp:function:: virtual bool getRegValue(Dyninst::MachRegister reg, Dyninst::THR_ID thread, Dyninst::MachRegisterVal &val)
  .. cpp:function:: virtual bool readMem(void *dest, Dyninst::Address source, size_t size)
  .. cpp:function:: virtual bool getThreadIds(std::vector<Dyninst::THR_ID> &thrds)
  .. cpp:function:: virtual bool getDefaultThread(Dyninst::THR_ID &default_tid)
  .. cpp:function:: virtual unsigned getAddressWidth()
  .. cpp:function:: virtual bool preStackwalk(Dyninst::THR_ID tid)
  .. cpp:function:: virtual bool postStackwalk(Dyninst::THR_ID tid)

  .. cpp:function:: virtual bool pause(Dyninst::THR_ID tid = NULL_THR_ID)

      Pauses the thread with id ``tid``.
      
      Execution doesn't resume until :cpp:func:`resume` is called. If no id
      is given, then every thread in the process is paused.

      When a call stack is collected from a running thread, it is automatically
      paused and resumed. When a call stack is collected from a paused thread,
      the thread is left in the paused state and **not** automatically resumed.
      Explicitly pausing threads before stack walk can be useful for keeping
      the returned stack walk synchronized with the current state of the thread.

      Returns ``false`` on error.

  .. cpp:function:: virtual bool resume(Dyninst::THR_ID tid = NULL_THR_ID)

      Resumes execution on a paused thread with id ``tid``.
      
      If no id is given, then every thread in the process is resumed.

      Returns ``false`` on error.
      
      .. note::
        Only threads paused by :cpp:func:`pause` are resumed. Using it on other threads
        is an error.

  .. cpp:function:: virtual bool isTerminated()

      Checks if the associated process has terminated.

      A process termination will also be signaled through the notification FD.
      Users should check processes for the isTerminated state after returning
      from handleDebugEvent.
      
      .. attention::
        A target process may terminate itself by calling exit, returning from main, or
        receiving an unhandled signal. Attempting to collect stack walks or perform other
        operations on a terminated process is illegal an will lead to undefined behavior.

  .. cpp:function:: virtual bool detach(bool leave_stopped = false)

      Detaches from the associated process.

      If the ``leave_stopped`` is ``true``, the process is detached from but left in a
      paused state so that it does resume progress. This is useful for attaching another
      debugger back to the process for further analysis. This behavior is not
      supported on the Linux platform and its value will have no affect on the
      detach call.
      
      Debug events on this process can no longer receive signals, call stacks can no
      longer be collected, and associated walker and procstate are invalidated. It is
      an error to attempt to do operations on these objects after a detach, and
      undefined behavior may result.

      Returns ``false`` on error.

  .. cpp:function:: Dyninst::ProcControlAPI::Process::ptr getProc()
  
        Returns the process associated with this state.

  .. cpp:function:: static int getNotificationFD()

      Returns the internal notification file descriptor.
      
      The notification file descriptor is used to write a byte to
      whenever a debug event occurs that need. If user code sees a byte on
      this file descriptor it should call :cpp:func:`handleDebugEvent` to invoke
      handling of the debug event.

      Stackwalker will only create one notification FD, even if it is
      attached to multiple 3rd party target processes.

  .. cpp:function:: std::string getExecutablePath()

      The same as :cpp:func:`ProcessState::getExecutablePath`.

  .. cpp:function:: static bool handleDebugEvent(bool block = false)

      Receives and handles all pending debug events from each 3rd party target process to which
      it is attached. If the ``block`` is ``true``, then execution is blocked until at least one
      debug event is handled. Otherwise, any pending debug events are handled.
      
      After handling debug events each target process will be continued (unless it was
      explicitly stopped by the :cpp:func:`ProcDebug::pause`) and any bytes on the
      notification FD will be cleared. It is generally expected that users will call this
      method when an event is sent to the notification FD, although it can be legally called
      at any time.

      Process exit events may be received while handling debug events. In this case, users
      should check :cpp:func:`isTerminated` after handling debug events.

      Returns ``false`` on error.

  .. cpp:function:: virtual bool isFirstParty()

      The same as :cpp:func:`ProcessState::isFirstParty`.

  .. cpp:function:: virtual Dyninst::Architecture getArchitecture()

      The same as :cpp:func:`ProcessState::getArchitecture`.


.. cpp:type:: std::pair<std::string, Dyninst::Address> LibAddrPair

  The first field is the file path of the library that was loaded. The second is the load
  address of that library in the process’ address space. The load address of a library can be
  added to a symbol offset from the file in order to get its absolute address.


.. cpp:enum:: lib_change_t

  .. cpp:enumerator:: library_load
  .. cpp:enumerator:: library_unload
