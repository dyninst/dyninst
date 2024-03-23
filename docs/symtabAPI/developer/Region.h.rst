.. _`sec-dev:Region.h`:

Region.h
########

.. cpp:namespace:: Dyninst::SymtabAPI::dev

.. cpp:class:: Region : public AnnotatableSparse

  .. cpp:function:: static Region* createRegion(Offset diskOff, perm_t perms, RegionType regType, unsigned long diskSize = 0, \
                                                Offset memOff = 0, unsigned long memSize = 0, std::string name = "", \
                                                char *rawDataPtr = NULL, bool isLoadable = false, bool isTLS = false, \
                                                unsigned long memAlign = sizeof(unsigned))

      Calls ``Region(0, ...)``.

  .. cpp:function:: bool setRegionNumber(unsigned regnumber)

      Sets the region index; the value must not overlap with any other regions
      and is not checked.

  .. cpp:function:: void setMemOffset(Offset)
  .. cpp:function:: void setMemSize(unsigned long)
  .. cpp:function:: void setDiskSize(unsigned long)
  .. cpp:function:: void setFileOffset(Offset)

  .. cpp:function:: bool setLoadable(bool isLoadable)

      This method sets whether the region is loaded into memory at load time.

      Returns ``true`` on success.

  .. cpp:function:: bool setRegionPermissions(perm_t newPerms)

      This sets the regions permissions to ``newPerms``.

      Returns ``true`` on success.

  .. cpp:function:: bool addRelocationEntry(Offset relocationAddr, Symbol *dynref, unsigned long relType, Region::RegionType rtype = Region::RT_REL)

      Creates and adds a relocation entry for this region. The symbol ``dynref`` represents the symbol used by he relocation, ``relType`` is
      the (platform-specific) relocation type, and ``rtype`` represents whether the relocation is REL or RELA (ELF-specific).

  .. cpp:function:: bool addRelocationEntry(const relocationEntry& rel)

      Add the provided relocation entry to this region.

  .. cpp:function:: bool updateRelocations(Address start, Address end, Symbol *oldsym, Symbol *newsym)

  .. cpp:function:: Symtab *symtab() const

  .. cpp:function:: protected Region(unsigned regnum, std::string name, Offset diskOff, unsigned long diskSize, Offset memOff, unsigned long memSize, \
                                     char *rawDataPtr, perm_t perms, RegionType regType, bool isLoadable = false, bool isTLS = false, \
                                     unsigned long memAlign = sizeof(unsigned))

        Creates a region with number ``regnum``, name ``name``, permissions ``perms``, and type ``regType``. For the on-disk file, the region starts at
        at ``diskOff`` with size ``diskSize``. When loaded into memory, it will start at offset ``memOff`` (modified by the base address of the file)
        with size ``memSize``. If ``memSize`` is larger than ``diskSize`` the remainder will be zero-padded (e.g., bss regions). ``rawDataPtr`` is
        currently unused in all other parts of Dyninst. ``isLoadable`` indicates the region can be loaded into memory (only used for rewriting). ``isTLS``
        indicates the region is located in thread-local storage. ``memAlign`` is not used internally, but can be stored and retrieved.

  .. cpp:function:: protected void setSymtab(Symtab *sym)

  .. cpp:function:: void* getPtrToRawData() const

      Returns the read-only pointer to the region's raw data buffer.

  .. cpp:function:: bool setPtrToRawData(void *newPtr, unsigned long rawsize)

      Set the raw data pointer of the region to ``newPtr``. ``rawsize``
      represents the size of the raw data buffer. Returns ``true`` if success
      or ``false`` when unable to set/change the raw data of the region.
      Implicitly changes the disk and memory sizes of the region.

  .. cpp:function:: bool patchData(Offset off, void *buf, unsigned size)

      Patch the raw data for this region. ``buf`` represents the buffer to be
      patched at offset ``off`` and size ``size``.

  .. cpp:function:: bool isDirty() const

      Checks if this region's raw data buffer has been modified by the user.

  .. cpp:function:: std::vector<relocationEntry> &getRelocations()

      Returns the relocation entries that will modify this region.

