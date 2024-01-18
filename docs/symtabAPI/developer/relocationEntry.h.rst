.. _`sec:relocationEntry.h`:

relocationEntry.h
#################

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: relocationEntry : public AnnotatableSparse

  .. cpp:member:: private Offset target_addr_

    Target address of call instruction

  .. cpp:member:: private Offset rel_addr_

    Address of corresponding relocation entry

  .. cpp:member:: private Offset addend_

    Addend (from RELA entries)

  .. cpp:member:: private Region::RegionType rtype_

    RT_REL vs. RT_RELA

  .. cpp:function:: relocationEntry()
  .. cpp:function:: relocationEntry(Offset ta, Offset ra, Offset add, std::string n, Symbol *dynref = NULL, unsigned long relType = 0)
  .. cpp:function:: relocationEntry(Offset ta, Offset ra, std::string n, Symbol *dynref = NULL, unsigned long relType = 0)
  .. cpp:function:: relocationEntry(Offset ra, std::string n, Symbol *dynref = NULL, unsigned long relType = 0, Region::RegionType rtype = Region::RT_REL)
  .. cpp:function:: relocationEntry(Offset ta, Offset ra, Offset add, std::string n, Symbol *dynref = NULL, unsigned long relType = 0, Region::RegionType rtype = Region::RT_REL)
  .. cpp:function:: Offset target_addr() const
  .. cpp:function:: Offset rel_addr() const
  .. cpp:function:: Offset addend() const
  .. cpp:function:: Region::RegionType regionType() const
  .. cpp:function:: const std::string &name() const
  .. cpp:function:: Symbol *getDynSym() const
  .. cpp:function:: bool addDynSym(Symbol *dynref)
  .. cpp:function:: unsigned long getRelType() const
  .. cpp:function:: void setTargetAddr(const Offset)
  .. cpp:function:: void setRelAddr(const Offset)
  .. cpp:function:: void setAddend(const Offset)
  .. cpp:function:: void setRegionType(const Region::RegionType)
  .. cpp:function:: void setName(const std::string &newName)
  .. cpp:function:: void setRelType(unsigned long relType)
  .. cpp:function:: bool operator==(const relocationEntry &) const
  .. cpp:function:: static unsigned long getGlobalRelType(unsigned addressWidth, Symbol *sym = NULL)
  .. cpp:function:: static const char *relType2Str(unsigned long r, unsigned addressWidth = sizeof(Address))
  .. cpp:function:: category getCategory( unsigned addressWidth )

.. cpp:enum:: relocationEntry::@rel

  .. cpp:enumerator:: pltrel = 1
  .. cpp:enumerator:: dynrel = 2

.. cpp:enum:: category

  .. cpp:enumerator:: relative
  .. cpp:enumerator:: jump_slot
  .. cpp:enumerator:: absolute

relocationEntry
---------------

This class represents object relocation information.

.. code-block:: cpp

    Offset target_addr() const

Specifies the offset that will be overwritten when relocations are
processed.

.. code-block:: cpp

    Offset rel_addr() const

Specifies the offset of the relocation itself.

.. code-block:: cpp

    Offset addend() const

Specifies the value added to the relocation; whether this value is used
or not is specific to the relocation type.

.. code-block:: cpp

    const std::string name() const

Specifies the user-readable name of the relocation.

.. code-block:: cpp

    Symbol *getDynSym() const

Specifies the symbol whose final address will be used in the relocation
calculation. How this address is used is specific to the relocation
type.

.. code-block:: cpp

    unsigned long getRelType() const

Specifies the platform-specific relocation type.
