.. _`sec:RTcommon.h`:

RTcommon.h
##########

.. cpp:function:: void DYNINSTtrapFunction(void)
.. cpp:function:: void DYNINSTbreakPoint(void)
.. cpp:function:: void DYNINSTsafeBreakPoint(void)

  Use a signal that is safe if we're not attached.

.. cpp:function:: void DYNINSTinit(void)
.. cpp:function:: int DYNINSTreturnZero(void)
.. cpp:function:: int DYNINSTwriteEvent(void *ev, size_t sz)
.. cpp:function:: int DYNINSTasyncConnect(int pid)
.. cpp:function:: int DYNINSTinitializeTrapHandler(void)
.. cpp:function:: void *dyninstTrapTranslate(void *source, volatile unsigned long* table_used, \
                                             volatile unsigned long* table_version, volatile trapMapping_t **trap_table,\
                                             volatile unsigned long *is_sorted)
.. cpp:var:: extern int DYNINST_mutatorPid
.. cpp:var:: extern int libdyninstAPI_RT_init_localCause
.. cpp:var:: extern int libdyninstAPI_RT_init_localPid
.. cpp:var:: extern int libdyninstAPI_RT_init_maxthreads
.. cpp:var:: extern int libdyninstAPI_RT_init_debug_flag
.. cpp:var:: extern int DYNINSTdebugPrintRT
.. cpp:var:: extern tc_lock_t DYNINST_trace_lock
.. cpp:function:: extern void *map_region(void *addr, int len, int fd)
.. cpp:function:: extern int unmap_region(void *addr, int len)
.. cpp:function:: extern void mark_heaps_exec(void)
.. cpp:var:: extern int DYNINSTdebugRTlib
.. cpp:var:: extern int DYNINSTstaticMode
.. cpp:function:: int rtdebug_printf(const char *format, ...)
