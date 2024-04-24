.. _`sec:pdwinnt.h`:

pdwinnt.h
#########

.. cpp:type:: HANDLE handleT



.. cpp:struct:: dyn_saved_regs

  .. cpp:member:: CONTEXT cont


.. cpp:struct:: EXCEPTION_REGISTRATION

  .. cpp:member:: EXCEPTION_REGISTRATION *prev
  .. cpp:member:: Address handler

.. cpp:var:: static const auto sleep = Sleep

.. cpp:type:: DEBUG_EVENT eventInfo_t
.. cpp:type:: DWORD eventWhat_t
.. cpp:type:: void * eventMoreInfo_t
.. cpp:type:: void (*thread_main_t)(void *)
.. cpp:type:: unsigned long internal_thread_t
.. cpp:type:: CRITICAL_SECTION EventLock_t
.. cpp:type:: HANDLE EventCond_t


.. code:: cpp

  #define EXIT_NAME "_exit"
  # define SIGNAL_HANDLER "no_signal_handler"
  #endif

  // Number of bytes to save in an overwrite operation
  #define BYTES_TO_SAVE 256

  #define CAN_DUMP_CORE false
  #define SLEEP_ON_MUTATEE_CRASH 0 /*seconds*/


  #define INFO_TO_EXIT_CODE(info) info.u.ExitProcess.dwExitCode
  #define INFO_TO_ADDRESS(info) info.u.Exception.ExceptionRecord.ExceptionAddress
  #define INFO_TO_PID(info) -1

  #define THREAD_RETURN void
  #define DO_THREAD_RETURN return

  #define VSNPRINTF _vsnprintf
  #define SNPRINTF _snprinf

  #define INDEPENDENT_LWP_CONTROL true

  #define ssize_t int
  #define DYNINST_ASYNC_PORT 28003
  #define PDSOCKET_ERRNO WSAGetLastError()
  #define INVALID_PDSOCKET (INVALID_SOCKET)
  #define SOCKET_TYPE PF_INET
  #define THREAD_RETURN void
  #define DO_THREAD_RETURN return
  #define SOCKLEN_T unsigned int
