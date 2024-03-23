.. _`sec:sha1.h`:

sha1.h
######

.. code:: c

  #define SHA1_DIGEST_LEN 20
  #define SHA1_STRING_LEN (SHA1_DIGEST_LEN * 2 + 1)

.. cpp:function:: char *sha1_file(const char *filename, char *result_ptr = NULL)
