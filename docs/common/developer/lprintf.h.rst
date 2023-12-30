.. _`sec:lprintf.h`:

lprintf.h
#########

Interface to printf-like error functions.

.. cpp:function:: void log_msg(const char *)
.. cpp:function:: void log_printf(void (*)(const char *), const char *, ...)
.. cpp:function:: void log_perror(void (*)(const char *), const char *)
