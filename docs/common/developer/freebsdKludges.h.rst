.. _`sec:freebsdKludges.h`:

freebsdKludges.h
################

.. cpp:namespace:: freebsd

.. cpp:function:: int sysctl_computeAddrWidth(pid_t pid)
.. cpp:function:: char *sysctl_getExecPathname(pid_t pid)
.. cpp:function:: bool sysctl_findProcLWPs(pid_t pid, std::vector<pid_t> &lwps)
.. cpp:function:: lwpid_t sysctl_getInitialLWP(pid_t pid)
.. cpp:function:: bool sysctl_getRunningStates(pid_t pid, std::map<Dyninst::LWP, bool> &runningStates)
.. cpp:function:: map_entries *getVMMaps(int pid, unsigned &maps_size)
.. cpp:function:: bool PtraceBulkRead(Dyninst::Address inTraced, unsigned size, void *inSelf, int pid)
.. cpp:function:: bool PtraceBulkWrite(Dyninst::Address inTraced, unsigned size, const void *inSelf, int pid)
>>>>>>> 3f49679 freebsd -- rebase
