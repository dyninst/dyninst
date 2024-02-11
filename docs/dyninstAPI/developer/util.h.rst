.. _`sec:dyninstAPI:util.h`:

util.h
######

.. cpp:function:: extern void printDyninstStats()
.. cpp:var:: extern CntStatistic insnGenerated
.. cpp:var:: extern CntStatistic totalMiniTramps
.. cpp:var:: extern CntStatistic trampBytes
.. cpp:var:: extern CntStatistic ptraceOps
.. cpp:var:: extern CntStatistic ptraceOtherOps
.. cpp:var:: extern CntStatistic ptraceBytes
.. cpp:var:: extern CntStatistic pointsUsed
.. cpp:function:: bool waitForFileToExist(char *fname, int timeout_seconds)
.. cpp:function:: int openFileWhenNotBusy(char *fname, int flags, int mode, int timeout_seconds)
.. cpp:function:: inline unsigned uiHash(const unsigned &val) 
.. cpp:function:: inline unsigned CThash(const unsigned &val) 
.. cpp:function:: unsigned ptrHash4(void *ptr)
.. cpp:function:: unsigned ptrHash16(void *ptr)
.. cpp:function:: inline unsigned intHash(const int &val) 
.. cpp:function:: void dyninst_log_perror(const char *msg)
