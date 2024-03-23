.. _`sec:DynAddrSpace.h`:

DynAddrSpace.h
##############

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: DynAddrSpace : public AddrSpace

  .. cpp:function:: static DynAddrSpace* create()
  .. cpp:function:: bool loadLibrary(DynObject*)
  .. cpp:function:: bool removeAddrSpace(AddressSpace *)
  .. cpp:type:: std::set<AddressSpace*> AsSet
  .. cpp:function:: AsSet& asSet()
  .. cpp:function:: virtual bool write(PatchObject*, Address to, Address from, size_t size)
  .. cpp:function:: virtual Address malloc(PatchObject*, size_t size, Address near)
  .. cpp:function:: virtual bool realloc(PatchObject*, Address orig, size_t size)
  .. cpp:function:: virtual bool free(PatchObject*, Address orig)
  .. cpp:function:: bool isRecursive()
  .. cpp:function:: void setRecursive(bool r)
  .. cpp:function:: protected DynAddrSpace()
  .. cpp:function:: protected DynAddrSpace(AddrSpace* par)
  .. cpp:member:: protected AsSet as_set_
  .. cpp:member:: protected bool recursive_
