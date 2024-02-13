.. _`sec:RTheap.h`:

RTheap.h
########

.. c:struct:: dyninstmm_t

  .. c:member:: Address pr_vaddr
  .. c:member:: unsigned long pr_size

......

Platform-specific variables

.. c:var:: extern int DYNINSTheap_align
.. c:var:: extern Address DYNINSTheap_loAddr
.. c:var:: extern Address DYNINSTheap_hiAddr
.. c:var:: extern int DYNINSTheap_mmapFlags

......

Platform-specific functions

.. c:function:: RT_Boolean DYNINSTheap_useMalloc(void *lo, void *hi)
.. c:function:: int DYNINSTheap_mmapFdOpen(void)
.. c:function:: void DYNINSTheap_mmapFdClose(int fd)
.. c:function:: int DYNINSTheap_getMemoryMap(unsigned *, dyninstmm_t **mmap)
.. c:function:: int DYNINSTgetMemoryMap(unsigned *nump, dyninstmm_t **mapp)
