.. _`sec:headers.h`:

headers.h
#########

Kludges to handle broken system includes and such...

.. code:: cpp

  #if defined(os_linux)
  # include "common/src/linuxHeaders.h"
  #elif defined(os_freebsd)
  # include "common/src/freebsdHeaders.h"
  #elif defined(os_windows)
  # include "common/src/ntHeaders.h"
  #endif

.. cpp:enum:: readReturnValue_t 

  .. cpp:enumerator:: RRVsuccess
  .. cpp:enumerator:: RRVnoData
  .. cpp:enumerator:: RRVinsufficientData
  .. cpp:enumerator:: RRVreadError
  .. cpp:enumerator:: RRVerror

.. cpp:function:: template <class T> readReturnValue_t P_socketRead(PDSOCKET fd, T &it, ssize_t sz)
.. cpp:function:: template<class T> readReturnValue_t P_socketRead(PDSOCKET fd, T &it)
