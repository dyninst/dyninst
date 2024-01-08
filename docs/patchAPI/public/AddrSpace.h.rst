.. _`sec:AddrSpace.h`:

AddrSpace.h
###########

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:class:: AddrSpace

  **The address space of a Mutatee**

  Contains a collection of :cpp:class::`PatchObject`\ s that represent shared
  libraries or a binary executable. Programmers implement
  some memory management interfaces in the AddrSpace class to determine
  the type of the code patching - 1st party, 3rd party, or binary
  rewriting.

  .. cpp:type:: std::map<const CodeObject*, PatchObject*> ObjMap

  .. cpp:function:: static AddrSpace* create(PatchObject* obj)

      Creates an address space containing ``obj``.

  .. cpp:function:: virtual bool write(PatchObject* obj, Address to, Address from, size_t size)

      Copies ``size`` bytes from the source address ``from`` in the
      mutator to the destination address ``to`` in the mutatee.

      If the instrumentation is for binary rewriting, the source address is the relative offset
      for ``obj``. Otherwise, it's an absolute address.

      Returns ``false`` on error.

  .. cpp:function:: virtual Address malloc(PatchObject* obj, size_t size, Address near)

      Allocates a buffer of ``size`` bytes in the mutatee.

      The address ``near`` is a relative address in the object ``obj``, if the
      instrumentation is for binary rewriting; otherwise, ``near`` is an
      absolute address, where this method tries to allocate a buffer near the
      address ``near``.

      Returns 0 on failure.

  .. cpp:function:: virtual Address realloc(PatchObject* obj, Address orig, size_t size)

      Reallocates a buffer of ``size`` bytes in the mutatee.

      The original buffer is at the address ``orig``. This method tries to
      reallocate the buffer near the address ``orig``, where ``orig`` is a
      relative address in the PatchObject. If the instrumentation is for binary
      rewriting, the source address is the relative offset for ``obj``. Otherwise,
      it's an absolute address.

      Returns 0 on failure.

  .. cpp:function:: virtual bool free(PatchObject* obj, Address orig)

      Deallocates a buffer in the mutatee at the address ``orig``.

      If the instrumentation is for binary rewriting, the source address is the relative
      offset for ``obj``. Otherwise, it's an absolute address.

      If this method succeeds, it returns true; otherwise, it returns false.

  .. cpp:function:: virtual bool loadObject(PatchObject* obj)

      Loads ``obj`` into the address space.

      Returns ``false`` on error.

  .. cpp:type:: ObjMap& objMap()

      Returns a mapping from code objects to patch objects.

      The ``PatchObject``\ s in all mappings represent all binary objects (either
      executable or libraries loaded) in this address space.

  .. cpp:function:: PatchObject* findObject(const ParseAPI::CodeObject* obj) const

      Find the patch corresponding to ``obj`` in the address space.

      Returns ``NULL`` if ``obj`` does not correspond to any patch.

  .. cpp:function:: template <class Iter> void objs(Iter iter)

      Writes all of the patch objects into ``iter``.

      ``Iter`` must be at least a C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_.

  .. cpp:function:: PatchObject* executable()

      Returns the PatchObject of the executable of the mutatee.

  .. cpp:function:: PatchMgrPtr mgr()

      Returns the PatchMgr’s pointer, where the PatchMgr contains this address
      space.

  .. cpp:function:: std::string format() const

      Returns a string representation of the starting address of the address space.

  .. cpp:function:: bool consistency(const PatchMgr *mgr) const

      Checks if all contained patches are managed by ``mgr``.
