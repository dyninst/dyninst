.. _`sec-dev:PCProcess.h`:

PCProcess.h
###########

.. code:: cpp

  #define pc_const_cast boost::const_pointer_cast


.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:var:: extern bool is_restricted_ptrace

.. cpp:namespace-push:: dev

.. cpp:class:: Process

  .. cpp:member:: static const std::map<int,int> emptyFDs
  .. cpp:member:: static const std::vector<std::string> emptyEnvp

  .. cpp:function:: int_process *llproc() const
  .. cpp:function:: proc_exitstate *exitstate() const
  .. cpp:function:: void setLastError(ProcControlAPI::err_t err_code, const char *err_str) const
  .. cpp:function:: void clearLastError() const
  .. cpp:function:: static bool setThreadingMode(thread_mode_t tm)
  .. cpp:function:: static thread_mode_t getThreadingMode()
  .. cpp:function:: static Process::ptr createProcess(std::string executable, const std::vector<std::string> &argv, const std::vector<std::string> &envp = emptyEnvp, const std::map<int,int> &fds = emptyFDs)
  .. cpp:function:: static Process::ptr attachProcess(Dyninst::PID pid, std::string executable = "")
  .. cpp:function:: void *getData() const

      Retrieve user-defined data retrievable during a callback

  .. cpp:function:: Dyninst::Address findFreeMemory(size_t size)

      Currently Windows-only, needed for the test infrastructure but possibly useful elsewhere


.. cpp:class:: Thread : public boost::enable_shared_from_this<Thread>

  .. cpp:member:: protected int_thread *llthread_
  .. cpp:member:: protected thread_exitstate *exitstate_
  .. cpp:function:: protected Thread()
  .. cpp:function:: int_thread *llthrd() const
  .. cpp:function:: void setLastError(err_t ec, const char *es) const

  .. cpp:function:: bool isUser() const

      Added for Windows. Windows creates internal threads which are bound to
      the process but are used for OS-level work. We hide these from the user,
      but need to represent them in ProcControlAPI.

.. cpp:namespace-pop::

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

  .. cpp:member:: private rpc_wrapper *wrapper
  .. cpp:function:: private IRPC(rpc_wrapper *wrapper_)
  .. cpp:function:: private ~IRPC()

