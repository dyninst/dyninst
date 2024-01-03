.. _`sec-dev:CodeSource.h`:

CodeSource.h
############

.. cpp:namespace:: Dyninst::ParseAPI::dev

.. cpp:class:: CodeSource

  Implementers of CodeSource can fill the following structures with available
  information. Some of this information is optional.

  .. cpp:member:: protected  mutable std::map<Address, std::string> _linkage

      Entries in the linkage map represent external linkage, e.g. the PLT in
      ELF binaries. Filling in this map is optional.

  .. cpp:member:: protected Address _table_of_contents

      Many binary format have “table of contents” structures for position
      independant references. If such a structure exists, its address should
      be filled in.

  .. cpp:member:: protected std::vector<CodeRegion *> _regions

      Code regions in the binary. At least one region is required for parsing.

  .. cpp:member:: protected Dyninst::IBSTree<CodeRegion> _region_tree

      Code region lookup. Must be consistent with the _regions vector. Mandatory.

  .. cpp:member:: protected std::vector<Hint> _hints

      Hints for where to begin parsing.

      CodeSource implementors can supply a set of Hint objects describing
      where functions are known to start in the binary. These hints are used
      to seed the parsing algorithm. These are required when using the default parsing mode,
      but usage of one of the direct parsing modes (parsing particular locations or using
      speculative methods) is supported without hints.

  .. cpp:member:: protected static dyn_hash_map<std::string, bool> non_returning_funcs

      Lists of known non-returning functions

  .. cpp:member:: protected static dyn_hash_map<int, bool> non_returning_syscalls_x86

      Lists of known non-returning functions by syscall

  .. cpp:member:: protected static dyn_hash_map<int, bool> non_returning_syscalls_x86_64

      Lists of known non-returning functions by syscall number on x86_64

  .. cpp:function:: std::vector< Hint > const& hints()

      Returns the currently-defined function entry hints.

  .. cpp:function:: std::vector<CodeRegion *> const& regions()

      Returns a read-only vector of code regions within the binary represented
      by this code source.

  .. cpp:function:: int findRegions(Address addr, set<CodeRegion *> & ret)

      Finds all CodeRegion objects that overlap the provided address. Some
      code sources (e.g. archive files) may have several regions with
      overlapping address ranges; others (e.g. ELF binaries) do not.

  .. cpp:function:: bool regionsOverlap()

      Indicates whether the CodeSource contains overlapping regions.

  .. cpp:function:: Address getTOC() const
  .. cpp:function:: virtual Address getTOC(Address) const

      If the binary file type supplies per-function TOC's (e.g. ppc64 Linux), override.

  .. cpp:function:: virtual void print_stats() const
  .. cpp:function:: virtual bool have_stats() const
  .. cpp:function:: virtual void incrementCounter(const std::string& name) const
  .. cpp:function:: virtual void addCounter(const std::string& name, int num) const
  .. cpp:function:: virtual void decrementCounter(const std::string& name) const
  .. cpp:function:: virtual void startTimer(const std::string& name) const
  .. cpp:function:: virtual void stopTimer(const std::string& name) const
  .. cpp:function:: virtual bool findCatchBlockByTryRange(Address address, std::set<Address>&) const
  .. cpp:function:: void addRegion(CodeRegion*)
  .. cpp:function:: void removeRegion(CodeRegion*)

.. cpp:class:: CodeRegion

  **Divide a CodeSource into distinct regions**

  This interface is mostly of interest to CodeSource implementors.

  .. cpp:function:: void names(Address addr, vector<std::string>& names)

      Retrieves the names associated with the function address ``addr`` in the
      region, e.g. symbol names in an ELF or PE binary.

  .. cpp:function:: virtual bool findCatchBlock(Address addr, Address & catchStart)

      Finds the exception handler associated with the address ``addr``, if one exists.

      This routine is only implemented for binary code sources that support structured
      exception handling.

  .. cpp:function:: Address low()

      Returns the lower bound of the interval of the address space covered by this region.

  .. cpp:function:: Address high()

      Returns the upper bound of the interval of the address space covered by this region.

  .. cpp:function:: bool contains(Address addr)

      Checks if :cpp:func:`low` :math:`\le` ``addr`` :math:`\lt` :cpp:func:`high`.

  .. cpp:function:: virtual bool wasUserAdded() const

      Return true if this region was added by the user, false otherwise.

.. cpp:class:: SymtabCodeRegion : public CodeRegion

  .. cpp:function:: SymtabCodeRegion(SymtabAPI::Symtab*, SymtabAPI::Region*)
  .. cpp:function:: SymtabCodeRegion(SymtabAPI::Symtab*, SymtabAPI::Region*, std::vector<SymtabAPI::Symbol*> &symbols)
  .. cpp:function:: void names(Address, std::vector<std::string>&)
  .. cpp:function:: bool findCatchBlock(Address addr, Address& catchStart)
  .. cpp:function:: bool isValidAddress(const Address) const
  .. cpp:function:: void* getPtrToInstruction(const Address) const
  .. cpp:function:: void* getPtrToData(const Address) const
  .. cpp:function:: unsigned int getAddressWidth() const
  .. cpp:function:: bool isCode(const Address) const
  .. cpp:function:: bool isData(const Address) const
  .. cpp:function:: bool isReadOnly(const Address) const
  .. cpp:function:: Address offset() const
  .. cpp:function:: Address length() const
  .. cpp:function:: Architecture getArch() const
  .. cpp:function:: Address low() const
  .. cpp:function:: Address high() const
  .. cpp:function:: SymtabAPI::Region* symRegion() const

Notes
=====

One or more contiguous :cpp:class:`CodeRegion`\ s of code or data in the binary object must
be registered with the base class. Keeping :cpp:member:`CodeRegion::_regions` and
:cpp:member:`CodeRegion::_region_tree` structures in sync is
the responsibility of the implementing class.
