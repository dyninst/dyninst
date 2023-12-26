Region.h
========

.. cpp:namespace:: Dyninst::SymtabAPI

Class Region
------------

This class represents a contiguous range of code or data as encoded in
the object file. For ELF, regions represent ELF sections.

.. container:: center

   ====== ===============
   perm_t Meaning
   ====== ===============
   RP_R   Read-only data
   RP_RW  Read/write data
   RP_RX  Read-only code
   RP_RWX Read/write code
   ====== ===============

.. container:: center

   +-----------------+---------------------------------------------------+
   | RegionType      | Meaning                                           |
   +=================+===================================================+
   | RT_TEXT         | Executable code                                   |
   +-----------------+---------------------------------------------------+
   | RT_DATA         | Read/write data                                   |
   +-----------------+---------------------------------------------------+
   | RT_TEXTDATA     | Mix of code and data                              |
   +-----------------+---------------------------------------------------+
   | RT_SYMTAB       | Static symbol table                               |
   +-----------------+---------------------------------------------------+
   | RT_STRTAB       | String table used by the symbol table             |
   +-----------------+---------------------------------------------------+
   | RT_BSS          | 0-initialized memory                              |
   +-----------------+---------------------------------------------------+
   | RT_SYMVERSIONS  | Versioning information for symbols                |
   +-----------------+---------------------------------------------------+
   | RT_SYMVERDEF    | Versioning information for symbols                |
   +-----------------+---------------------------------------------------+
   | RT_SYMVERNEEDED | Versioning information for symbols                |
   +-----------------+---------------------------------------------------+
   | RT_REL          | Relocation section                                |
   +-----------------+---------------------------------------------------+
   | RT_RELA         | Relocation section                                |
   +-----------------+---------------------------------------------------+
   | RT_PLTREL       | Relocation section for PLT (inter-library         |
   |                 | references) entries                               |
   +-----------------+---------------------------------------------------+
   | RT_PLTRELA      | Relocation section for PLT (inter-library         |
   |                 | references) entries                               |
   +-----------------+---------------------------------------------------+
   | RT_DYNAMIC      | Decription of library dependencies                |
   +-----------------+---------------------------------------------------+
   | RT_HASH         | Fast symbol lookup section                        |
   +-----------------+---------------------------------------------------+
   | RT_GNU_HASH     | GNU-specific fast symbol lookup section           |
   +-----------------+---------------------------------------------------+
   | RT_OTHER        | Miscellaneous information                         |
   +-----------------+---------------------------------------------------+


.. list-table::
   :widths: 30  35 35
   :header-rows: 1

   * - Method name
     - Return type
     - Method description
   * - getRegionNumber
     - unsigned
     - Index of the region in the file, starting at 0.
   * - getRegionName
     - std::string
     - Name of the region (e.g., .text, .data).
   * - getPtrToRawData
     - void *
     - Read-only pointer to the region's raw data buffer.
   * - getDiskOffset
     - Offset
     - Offset within the file where the region begins.
   * - getDiskSize
     - unsigned long
     - Size of the region's data in the file.
   * - getMemOffset
     - Offset
     - Location where the region will be loaded into memory, modified by the file's base load address.
   * - getMemSize
     - unsigned long
     - Size of the region in memory, including zero padding.
   * - isBSS
     - bool
     - Type query for uninitialized data regions (zero disk size, non-zero memory size).
   * - isText
     - bool
     - Type query for executable code regions.
   * - isData
     - bool
     - Type query for initialized data regions.
   * - getRegionPermissions
     - perm_t
     - Permissions for the region; perm_t is defined above.
   * - getRegionType
     - RegionType
     - Type of the region as defined above.
   * - isLoadable
     - bool
     - True if the region will be loaded into memory (e.g., code or data) false otherwise (e.g., debug information).
   * - isDirty
     - bool
     - True if the region's raw data buffer has been modified by the user.

.. code-block:: cpp

    static Region *createRegion(Offset diskOff, perm_t perms, RegionType regType, unsigned long diskSize = 0, Offset memOff = 0, unsigned long memSize = 0, std::string name = "", char *rawDataPtr = NULL, bool isLoadable = false, bool isTLS = false, unsigned long memAlign =sizeof(unsigned))

This factory method creates a new region with the provided arguments.
The ``memOff`` and ``memSize`` parameters identify where the region
should be loaded in memory (modified by the base address of the file);
if ``memSize`` is larger than ``diskSize`` the remainder will be
zero-padded (e.g., bss regions).

.. code-block:: cpp

    bool isOffsetInRegion(const Offset &offset) const

Return ``true`` if the offset falls within the region data.

.. code-block:: cpp

    void setRegionNumber(unsigned index) const

Sets the region index; the value must not overlap with any other regions
and is not checked.

.. code-block:: cpp

    bool setPtrToRawData(void *newPtr, unsigned long rawsize)

Set the raw data pointer of the region to ``newPtr``. ``rawsize``
represents the size of the raw data buffer. Returns ``true`` if success
or ``false`` when unable to set/change the raw data of the region.
Implicitly changes the disk and memory sizes of the region.

.. code-block:: cpp

    bool setRegionPermissions(perm_t newPerms)

This sets the regions permissions to ``newPerms``. Returns ``true`` on
success.

.. code-block:: cpp

    bool setLoadable(bool isLoadable)

This method sets whether the region is loaded into memory at load time.
Returns ``true`` on success.

.. code-block:: cpp

    bool addRelocationEntry(Offset relocationAddr, Symbol *dynref, unsigned
    long relType, Region::RegionType rtype = Region::RT_REL)

Creates and adds a relocation entry for this region. The symbol
``dynref`` represents the symbol used by he relocation, ``relType`` is
the (platform-specific) relocation type, and ``rtype`` represents
whether the relocation is REL or RELA (ELF-specific).

.. code-block:: cpp

    vector<relocationEntry> &getRelocations()

Get the vector of relocation entries that will modify this region. The
vector should not be modified.

.. code-block:: cpp

    bool addRelocationEntry(const relocationEntry& rel)

Add the provided relocation entry to this region.

.. code-block:: cpp

    bool patchData(Offset off, void *buf, unsigned size);

Patch the raw data for this region. ``buf`` represents the buffer to be
patched at offset ``off`` and size ``size``.

REMOVED
~~~~~~~

The following methods were removed since they were inconsistent and
dangerous to use.

.. code-block:: cpp

    Offset getRegionAddr() const

Please use ``getDiskOffset`` or ``getMemOffset`` instead, as
appropriate.

.. code-block:: cpp

    unsigned long getRegionSize() const

Please use ``getDiskSize`` or ``getMemSize`` instead, as appropriate.