.. _`sec:InstructionSource.h`:

InstructionSource.h
###################

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:class:: InstructionSource

  .. cpp:function:: InstructionSource()

  .. cpp:function:: virtual bool isValidAddress(const Address)

      Returns true if the address is a valid code location.

  .. cpp:function:: virtual void* getPtrToInstruction(const Address)

      Returns pointer to raw memory in the binary at the provided address.

  .. cpp:function:: virtual void* getPtrToData(const Address)

      Returns pointer to raw memory in the binary at the provided address. The
      address need not correspond to an executable code region.

  .. cpp:function:: virtual unsigned int getAddressWidth()

      Returns the address width (e.g. four or eight bytes) for the represented
      binary.

  .. cpp:function:: virtual bool isCode(const Address)

      Indicates whether the location is in a code region.

  .. cpp:function:: virtual bool isData(const Address)

      Indicates whether the location is in a data region.

  .. cpp:function:: virtual Address offset()

      The start of the region covered by this instruction source.

  .. cpp:function:: virtual Address length()

      The size of the region.

  .. cpp:function:: virtual Architecture getArch()

      The architecture of the instruction source. See the Dyninst manual for
      details on architecture differences.

  .. cpp:function:: virtual bool isAligned(const Address)

      For fixed-width instruction architectures, must return true if the
      address is a valid instruction boundary and false otherwise; otherwise
      returns true. This method has a default implementation that should be
      sufficient.

      CodeSource implementors need to fill in several data structures in the
      base CodeSource class:

  .. cpp:function:: std::map<Address, std::string> _linkage

      Entries in the linkage map represent external linkage, e.g. the PLT in
      ELF binaries. Filling in this map is optional.

  .. cpp:function:: Address _table_of_contents

      Many binary format have “table of contents” structures for position
      independant references. If such a structure exists, its address should
      be filled in.

  .. cpp:function:: std::vector<CodeRegion *> _regions Dyninst::IBSTree<CodeRegion> _region_tree

      One or more contiguous regions of code or data in the binary object must
      be registered with the base class. Keeping these structures in sync is
      the responsibility of the implementing class.

  .. cpp:function:: std::vector<Hint> _hints

      CodeSource implementors can supply a set of Hint objects describing
      where functions are known to start in the binary. These hints are used
      to seed the parsing algorithm. Refer to the CodeSource header file for
      implementation details.

  .. cpp:function:: virtual bool isReadOnly(const Address) const = 0
  .. cpp:function:: virtual Architecture getArch() const = 0
  .. cpp:function:: virtual bool isAligned(const Address) const
