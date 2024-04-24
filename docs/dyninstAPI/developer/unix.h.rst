.. _`sec:dyninstAPI:unix.h`:

unix.h
######

.. cpp:namespace:: dev::unix

.. cpp:type:: int procWaitpidStatus_t
.. cpp:type:: unsigned long eventInfo_t
.. cpp:type:: void * eventMoreInfo_t
.. cpp:type:: int eventWhat_t
.. cpp:type:: void *(*thread_main_t)(void *)
.. cpp:type:: pthread_t internal_thread_t
.. cpp:type:: pthread_mutex_t EventLock_t
.. cpp:type:: pthread_cond_t EventCond_t


.. code:: cpp

  #define CAN_DUMP_CORE true
  #define SLEEP_ON_MUTATEE_CRASH 300 /*seconds*/

  #define INFO_TO_EXIT_CODE(info) info
  #define INFO_TO_PID(info) info
  #define INFO_TO_ADDRESS(info) (Address) 0

  #define POLL_FD status_fd()
  #define POLL_TIMEOUT -1

  //  On /proc platforms we have predefined system call mappings (SYS_fork, etc).
  //  Define them here for platforms which don't have them

  #if !defined(SYS_fork)
  # define SYS_fork 1001
  #endif
  #if !defined(SYS_exec)
  # define SYS_exec 1002
  #endif
  #if !defined(SYS_exit)
  # define SYS_exit 1003
  #endif
  #if !defined(SYS_load)
  # define SYS_load 1004
  #endif
  #if !defined(SYS_execve)
  # define SYS_execve 1005
  #endif
  #if !defined(SYS_fork1)
  # define SYS_fork1 1006
  #endif
  #if !defined(SYS_vfork)
  # define SYS_vfork 1007
  #endif
  #if !defined(SYS_execv)
  # define SYS_execv 1008
  #endif
  #if !defined(SYS_lwp_exit)
  # define SYS_lwp_exit 1009
  #endif

  #define SYSSET_MAP(x, pid)  (x)

  #define THREAD_RETURN void *
  #define DO_THREAD_RETURN return NULL

  #define VSNPRINTF vsnprintf
  #define SNPRINTF snprinf

  #if defined(os_linux)
  # define PTHREAD_MUTEX_TYPE PTHREAD_MUTEX_RECURSIVE_NP
  # define STRERROR_BUFSIZE 512
  # define ERROR_BUFFER char buf[STRERROR_BUFSIZE]
  # define STRERROR(x,y) strerror_r(x,y,STRERROR_BUFSIZE)
  #else
  # define ERROR_BUFFER
  # define PTHREAD_MUTEX_TYPE PTHREAD_MUTEX_RECURSIVE
  # define STRERROR_BUFSIZE 0
  # define STRERROR(x,y) strerror(x)
  #endif

  #define PDSOCKET_ERRNO errno
  #define INVALID_PDSOCKET (-1)
  #define SOCKET_TYPE PF_UNIX
  #define THREAD_RETURN void *
  #define DO_THREAD_RETURN return NULL

  #define SOCKLEN_T socklen_t

  #ifndef INVALID_HANDLE_VALUE
  # define INVALID_HANDLE_VALUE -1
  #endif

  // Hybrid Analysis Compatibility definitions
  #define PAGE_READ 1
  #define PAGE_WRITE 2
  #define PAGE_EXECUTE 4
  #define PAGE_READONLY PAGE_READ
  #define PAGE_READWRITE (PAGE_READ | PAGE_WRITE)
  #define PAGE_EXECUTE_READ (PAGE_READ | PAGE_EXECUTE)
  #define PAGE_EXECUTE_READWRITE (PAGE_READ | PAGE_EXECUTE | PAGE_WRITE)


.. cpp:function:: static void findThreadFuncs(PCProcess *p, std::string func, std::vector<func_instance *> &result)

  Searches for function in order, with preference given first to libpthread, then to libc, then to the process.
