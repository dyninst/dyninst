.. _`sec:dyninstAPI:linux.h`:

linux.h
#######

.. code:: cpp

  #define EXIT_NAME "_exit"

  #if !defined(arch_x86_64)
  # define SIGNAL_HANDLER   "__restore"
  #else
  # define SIGNAL_HANDLER   "__restore_rt"
  #endif

  #ifndef WNOWAIT
  # define WNOWAIT WNOHANG
  #endif

.. cpp:function:: bool get_linux_version(int &major, int &minor, int &subvers)
.. cpp:function:: bool get_linux_version(int &major, int &minor, int &subvers, int &subsubvers)
