.. _`sec:nt_signal_emul.h`:

nt_signal_emul.h
################

.. code:: cpp

  #define SIGEM_EVENTMASK   0xff00
  #define SIGEM_SIGNALED    0x100
  #define SIGEM_EXITED    0x200
  #define SIGEM_STOPPED   0x300

  #define SIGEM_SIGMASK   0xff

  #define WIFSIGNALED(x)    (((x) & SIGEM_EVENTMASK) == SIGEM_SIGNALED)
  #define WIFEXITED(x)    (((x) & SIGEM_EVENTMASK) == SIGEM_EXITED)
  #define WIFSTOPPED(x)   (((x) & SIGEM_EVENTMASK) == SIGEM_STOPPED)

  #define WSTOPSIG(x) ((x) & SIGEM_SIGMASK)
  #define WTERMSIG(x) ((x) & SIGEM_SIGMASK)
