.. _`sec:freebsd-swk.h`:

freebsd-swk.h
#############

.. code:: cpp

  #define START_THREAD_FUNC_NAME "thread_start"
  #define START_FUNC_NAME "_start"

  // Note:
  // FreeBSD doesn't have an equivalent __clone frame so the __clone symbol won't
  // be found but the code will just ignore this. Set this to __clone to make
  // code uniform on both platforms.
  #define CLONE_FUNC_NAME "__clone"
