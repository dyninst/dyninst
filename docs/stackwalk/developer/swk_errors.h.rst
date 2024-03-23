.. _`sec-dev:swk_errors.h`:

swk_errors.h
############

.. cpp:namespace:: Dyninst::Stackwalker::dev

.. cpp:var:: const err_t err_badparam       = 0x10000
.. cpp:var:: const err_t err_nostepper      = 0x10001
.. cpp:var:: const err_t err_nosymlookup    = 0x10002
.. cpp:var:: const err_t err_procread       = 0x10003
.. cpp:var:: const err_t err_nosymbol       = 0x10004
.. cpp:var:: const err_t err_internal       = 0x10005
.. cpp:var:: const err_t err_prem           = 0x10006
.. cpp:var:: const err_t err_perm           = 0x10006
.. cpp:var:: const err_t err_noproc         = 0x10007
.. cpp:var:: const err_t err_interrupt      = 0x10008
.. cpp:var:: const err_t err_procexit       = 0x10009
.. cpp:var:: const err_t err_stackbottom    = 0x10010
.. cpp:var:: const err_t err_nofile         = 0x10011
.. cpp:var:: const err_t err_unsupported    = 0x10012
.. cpp:var:: const err_t err_symtab         = 0x10013
.. cpp:var:: const err_t err_nolibtracker   = 0x10014
.. cpp:var:: const err_t err_nogroup        = 0x10015
.. cpp:var:: const err_t err_nothrd         = 0x10016
.. cpp:var:: const err_t err_proccontrol    = 0x10017

.. cpp:function:: void clearLastError()
.. cpp:function:: void setLastError(err_t err, const char *msg = NULL)
.. cpp:function:: void setDebugChannel(FILE *f)
.. cpp:function:: void setDebug(bool enable)
.. cpp:function:: FILE *getDebugChannel()
.. cpp:function:: extern int sw_printf(const char *format, ...)
