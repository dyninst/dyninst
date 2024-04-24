.. _`sec:Region.h`:

Region.h
########

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: Region : public AnnotatableSparse

  **A contiguous range of code or data as encoded in an object file**

  For ELF, regions represent ELF sections.

  .. attention:: Users should not create or manipulate regions.

  .. cpp:function:: Region()

  .. cpp:function:: bool operator== (const Region &reg)

      Checks if this region is equal to ``reg``.

      Two regions are equal if

  .. cpp:function:: unsigned getRegionNumber() const

      Returns the index of the region in the file, starting at 0.

  .. cpp:function:: std::string getRegionName() const

      Returns the name of the region (e.g., .text, .data, etc.).

  .. cpp:function:: Offset getDiskOffset() const

      Returns the offset within the file where the region begins.

  .. cpp:function:: unsigned long getDiskSize() const

      Returns the size  in **bytes** of the region's data in the on-disk file.

  .. cpp:function:: unsigned long getFileOffset()

  .. cpp:function:: Offset getMemOffset() const

      Returns the location where the region will be loaded into memory, modified by the file's
      base load address.

  .. cpp:function:: unsigned long getMemSize() const

      Returns the size **in bytes** of the region in memory, including zero padding.

  .. cpp:function:: unsigned long getMemAlignment() const

  .. cpp:function:: bool isBSS() const

      Checks if this region is for uninitialized data (zero disk size, non-zero memory size).

  .. cpp:function:: bool isText() const

      Checks if this region is for executable code.

  .. cpp:function:: bool isData() const

      Checks if this region is for initialized data.

  .. cpp:function:: bool isTLS() const

      Checks if this region is for thread-local storage.

  .. cpp:function:: bool isOffsetInRegion(const Offset &offset) const

      Return ``true`` if the offset falls within the region data.

  .. cpp:function:: bool isLoadable() const

      Checks if this region will be loaded into memory (e.g., code or data).

  .. cpp:function:: bool isStandardCode()

      Checks if this region conforms to the platform-specific (e.g., ELF, PE) notion of a code region.

  .. cpp:function:: perm_t getRegionPermissions() const

      Returns the permissions for the region.

  .. cpp:function:: RegionType getRegionType() const

      Returns the type of this region.

  .. cpp:function:: std::ostream& operator<< (std::ostream &os)

      Writes a string representation of this region to the stream ``os``.

  .. cpp:function:: static const char *permissions2Str(perm_t p)

      Returns a string representation of ``p``.

  .. cpp:function:: static const char *regionType2Str(RegionType r)

      Returns a string representation of ``p``.


.. cpp:enum:: Region::perm_t

  .. cpp:enumerator:: RP_R

    Read-only data

  .. cpp:enumerator:: RP_RW

    Read/write data

  .. cpp:enumerator:: RP_RX

    Read-only code

  .. cpp:enumerator:: RP_RWX

    Read/write code

.. cpp:enum:: Region::RegionType

  .. cpp:enumerator:: RT_TEXT

    Executable code

  .. cpp:enumerator:: RT_DATA

    Read/write data

  .. cpp:enumerator:: RT_TEXTDATA

    Mix of code and data

  .. cpp:enumerator:: RT_SYMTAB

    Static symbol table

  .. cpp:enumerator:: RT_STRTAB

    String table used by the symbol table

  .. cpp:enumerator:: RT_BSS

    0-initialized memory

  .. cpp:enumerator:: RT_SYMVERSIONS

    Versioning information for symbols

  .. cpp:enumerator:: RT_SYMVERDEF

    Versioning information for symbols

  .. cpp:enumerator:: RT_SYMVERNEEDED

    Versioning information for symbols

  .. cpp:enumerator:: RT_REL

    Relocation section

  .. cpp:enumerator:: RT_RELA

    Relocation section

  .. cpp:enumerator:: RT_PLTREL

    Relocation section for PLT (inter-library references) entries

  .. cpp:enumerator:: RT_PLTRELA

    Relocation section for PLT (inter-library references) entries

  .. cpp:enumerator:: RT_DYNAMIC

    Decription of library dependencies

  .. cpp:enumerator:: RT_HASH

    Fast symbol lookup section

  .. cpp:enumerator:: RT_GNU_HASH

    GNU-specific fast symbol lookup section

  .. cpp:enumerator:: RT_OTHER

    Miscellaneous information
