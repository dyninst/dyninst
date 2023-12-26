.. _`sec:PCErrors.h`:

PCErrors.h
==========

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:type:: unsigned err_t

.. csv-table:: err_t values
   :header: "Name", "Value", "", "Name", "Value"
   :widths: 15 10 10 15 10

    "err_attached","0x10011","","err_badparam","0x10000"
    "err_bpfull","0x10013","","err_detached","0x10010"
    "err_dstack","0x10107","","err_eof","0x10108"
    "err_exited","0x10006","","err_incallback","0x1000e"
    "err_internal","0x10002","","err_interrupt","0x10005"
    "err_noevents","0x1000d","","err_nofile","0x10007"
    "err_none","0x0","","err_noproc","0x10004"
    "err_notfound","0x10014","","err_nothrd","0x1000a"
    "err_notrunning","0x1000c","","err_notstopped","0x1000b"
    "err_nouserthrd","0x1000f","","err_pendingirpcs","0x10012"
    "err_prem","0x10003","","err_procread","0x10001"
    "err_symtab","0x10009","","err_unsupported","0x10008"

.. cpp:function:: err_t getLastError();
.. cpp:function:: void clearLastError();
.. cpp:function:: const char* getLastErrorMsg();
.. cpp:function:: void globalSetLastError(err_t err, const char *msg = NULL);
.. cpp:function:: void setDebugChannel(FILE *f);
.. cpp:function:: void setDebug(bool enable);
.. cpp:function:: const char *getGenericErrorMsg(err_t e);
.. cpp:function:: FILE *getDebugChannel();
