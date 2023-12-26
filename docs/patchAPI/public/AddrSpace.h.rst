AddrSpace.h
===========

.. cpp:namespace:: Dyninst::patchAPI

AddrSpace
=========

**Declared in**: AddrSpace.h

The AddrSpace class represents the address space of a **Mutatee**, where
it contains a collection of **PatchObjects** that represent shared
libraries or a binary executable. In addition, programmers implement
some memory management interfaces in the AddrSpace class to determine
the type of the code patching - 1st party, 3rd party, or binary
rewriting.

.. code-block:: cpp
    
    virtual bool write(PatchObject* obj, Address to, Address from, size_t size);

This method copies *size*-byte data stored at the address *from* on the
**Mutator** side to the address *to* on the **Mutatee** side. The
parameter *to* is the relative offset for the PatchObject *obj*, if the
instrumentation is for binary rewriting; otherwise *to* is an absolute
address.

If the write operation succeeds, this method returns true; otherwise,
false.

.. code-block:: cpp
    
    virtual Address malloc(PatchObject* obj, size_t size, Address near);

This method allocates a buffer of *size* bytes on the **Mutatee** side.
The address *near* is a relative address in the object *obj*, if the
instrumentation is for binary rewriting; otherwise, *near* is an
absolute address, where this method tries to allocate a buffer near the
address *near*.

If this method succeeds, it returns a non-zero address; otherwise, it
returns 0.

.. code-block:: cpp
    
    virtual Address realloc(PatchObject* obj, Address orig, size_t size);

This method reallocates a buffer of *size* bytes on the **Mutatee**
side. The original buffer is at the address *orig*. This method tries to
reallocate the buffer near the address *orig*, where *orig* is a
relative address in the PatchObject *obj* if the instrumentation is for
binary rewriting; otherwise, *orig* is an absolute address.

If this method succeeds, it returns a non-zero address; otherwise, it
returns 0.

.. code-block:: cpp
    
    virtual bool free(PatchObject* obj, Address orig);

This method deallocates a buffer on the **Mutatee** side at the address
*orig*. If the instrumentation is for binary rewriting, then the
parameter *orig* is a relative address in the object *obj*; otherwise,
*orig* is an absolute address.

If this method succeeds, it returns true; otherwise, it returns false.

.. code-block:: cpp
    
    virtual bool loadObject(PatchObject* obj);

This method loads a PatchObject into the address space. If this method
succeeds, it returns true; otherwise, it returns false.

.. code-block:: cpp
    
    typedef std::map<const ParseAPI::CodeObject*, PatchObject*> AddrSpace::ObjMap;
    ObjMap& objMap();

Returns a set of mappings from ParseAPI::CodeObjects to PatchObjects,
where PatchObjects in all mappings represent all binary objects (either
executable or libraries loaded) in this address space.

.. code-block:: cpp
    
    PatchObject* executable();

Returns the PatchObject of the executable of the **Mutatee**.

.. code-block:: cpp
    
    PatchMgrPtr mgr();

Returns the PatchMgr’s pointer, where the PatchMgr contains this address
space.