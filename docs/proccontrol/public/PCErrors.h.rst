.. _`sec:PCErrors.h`:

PCErrors.h
==========

.. cpp:var:: extern bool dyninst_debug_proccontrol
.. cpp:var:: extern FILE* pctrl_err_out

.. cpp:function:: extern const char *thrdName()

.. cpp:function:: extern unsigned long gettod();

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:type:: unsigned err_t

.. cpp:var:: const err_t err_none           = 0x0
.. cpp:var:: const err_t err_badparam       = 0x10000
.. cpp:var:: const err_t err_procread       = 0x10001
.. cpp:var:: const err_t err_internal       = 0x10002
.. cpp:var:: const err_t err_prem           = 0x10003
.. cpp:var:: const err_t err_noproc         = 0x10004
.. cpp:var:: const err_t err_interrupt      = 0x10005
.. cpp:var:: const err_t err_exited         = 0x10006
.. cpp:var:: const err_t err_nofile         = 0x10007
.. cpp:var:: const err_t err_unsupported    = 0x10008
.. cpp:var:: const err_t err_symtab         = 0x10009
.. cpp:var:: const err_t err_nothrd         = 0x1000a
.. cpp:var:: const err_t err_notstopped     = 0x1000b
.. cpp:var:: const err_t err_notrunning     = 0x1000c
.. cpp:var:: const err_t err_noevents       = 0x1000d
.. cpp:var:: const err_t err_incallback     = 0x1000e
.. cpp:var:: const err_t err_nouserthrd     = 0x1000f
.. cpp:var:: const err_t err_detached       = 0x10010
.. cpp:var:: const err_t err_attached       = 0x10011
.. cpp:var:: const err_t err_pendingirpcs   = 0x10012
.. cpp:var:: const err_t err_bpfull         = 0x10013
.. cpp:var:: const err_t err_notfound       = 0x10014
.. cpp:var:: const err_t err_dstack         = 0x10107
.. cpp:var:: const err_t err_eof            = 0x10108

.. cpp:function:: err_t getLastError()
.. cpp:function:: void clearLastError()
.. cpp:function:: const char* getLastErrorMsg()
.. cpp:function:: void globalSetLastError(err_t err, const char *msg = NULL)
.. cpp:function:: void setDebugChannel(FILE *f)
.. cpp:function:: void setDebug(bool enable)
.. cpp:function:: const char *getGenericErrorMsg(err_t e)
.. cpp:function:: FILE *getDebugChannel()
