.. _`sec:linuxKludges.h`:

linuxKludges.h
##############

.. cpp:function:: bool PtraceBulkRead(Dyninst::Address inTraced, unsigned size, void *inSelf, int pid)
.. cpp:function:: bool PtraceBulkWrite(Dyninst::Address inTraced, unsigned size, const void *inSelf, int pid)
.. cpp:function:: bool findProcLWPs(pid_t pid, std::vector<pid_t> &lwps)
.. cpp:function:: map_entries *getVMMaps(int pid, unsigned &maps_size)
.. cpp:function:: map_entries *getLinuxMaps(int pid, unsigned &maps_size)

.. code:: cpp

  #define getVMMaps getLinuxMaps
