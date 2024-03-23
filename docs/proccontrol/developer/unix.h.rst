.. _`sec:unix.h`:

unix.h
######

.. cpp:class:: unix_process : virtual public int_process

  For our purposes, a UNIX process is one that supports fork/exec.

  .. cpp:function:: unix_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: unix_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~unix_process()
  .. cpp:function:: virtual void plat_execv()
  .. cpp:function:: virtual bool post_forked()
  .. cpp:function:: virtual unsigned getTargetPageSize()
  .. cpp:function:: virtual bool plat_decodeMemoryRights(Process::mem_perm& perm, unsigned long rights)
  .. cpp:function:: virtual bool plat_encodeMemoryRights(Process::mem_perm perm, unsigned long& rights)
  .. cpp:function:: virtual bool plat_getMemoryAccessRights(Dyninst::Address addr, Process::mem_perm& rights)
  .. cpp:function:: virtual bool plat_setMemoryAccessRights(Dyninst::Address addr, size_t size, Process::mem_perm rights, Process::mem_perm& oldRights)
  .. cpp:function:: virtual bool plat_findAllocatedRegionAround(Dyninst::Address addr, Process::MemoryRegion& memRegion)

    I'm not sure that unix_process is the proper place for this--it's really based on whether
    ``/proc/PID/maps`` exists.  Currently, that matches our platforms that use unix_process, so I'll leave
    it be for now.

  .. cpp:function:: virtual Dyninst::Address plat_mallocExecMemory(Dyninst::Address, unsigned size)
  .. cpp:function:: virtual bool plat_supportFork()
  .. cpp:function:: virtual bool plat_supportExec()
