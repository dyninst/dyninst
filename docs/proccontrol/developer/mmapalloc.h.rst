.. _`sec:mmapalloc.h`:

mmapalloc.h
===========

.. cpp:class:: mmap_alloc_process : virtual public int_process

  **A process that can use mmap to allocate memory**

  .. cpp:function:: mmap_alloc_process(Dyninst::PID p, std::string e, std::vector<std::string> a, std::vector<std::string> envp, std::map<int,int> f)
  .. cpp:function:: mmap_alloc_process(Dyninst::PID pid_, int_process *p)
  .. cpp:function:: virtual ~mmap_alloc_process()
  .. cpp:function:: virtual bool plat_collectAllocationResult(int_thread *thr, reg_response::ptr resp)
  .. cpp:function:: virtual bool plat_createAllocationSnippet(Dyninst::Address addr, bool use_addr, unsigned long size, void* &buffer, unsigned long &buffer_size, unsigned long &start_offset)
  .. cpp:function:: virtual bool plat_createDeallocationSnippet(Dyninst::Address addr, unsigned long size, void* &buffer, unsigned long &buffer_size, unsigned long &start_offset)
