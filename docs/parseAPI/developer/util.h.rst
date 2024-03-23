.. _`sec:parseapi-util.h.rst`:

util.h
======

.. code:: c

  #define HASHDEF(h,k) (h.find(k) != h.end())

  #ifdef __GNUC__
  # define likely(x)    __builtin_expect(!!(x), 1)
  # define unlikely(x)  __builtin_expect(!!(x), 0)
  #else
  # define likely(x)    x
  # define unlikely(x)  x
  #endif
