.. _`sec:CodeSource.h`:

CodeSource.h
############

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:class:: CodeSource : public Dyninst::InstructionSource

  ** Retrieve binary code from an executable, library, or other binary code object**

  It also can provide hints of function entry points (such as those derived from debugging
  symbols) to seed the parser.

  .. cpp:type:: dyn_c_hash_map<void*, CodeRegion*> RegionMap

  .. cpp:function:: virtual bool nonReturning(Address func_entry)

      Checks if a function returns by location ``func_entry``.

      This information may be statically known for some code sources, and can lead
      to better parsing accuracy.

  .. cpp:function:: virtual bool nonReturning(std::string func_name)

      Checks if a function returns by name ``func_name``.

      This information may be statically known for some code sources, and can lead
      to better parsing accuracy.

  .. cpp:function:: virtual bool nonReturningSyscall(int number)

      Checks if a system call returns by system call number ``number``.

      This information may be statically known for some code sources, and can lead
      to better parsing accuracy.

  .. cpp:function:: virtual Address baseAddress()

      Returns the base address of the code covered by this source.

      If the binary file type supplies non-zero base or load addresses (e.g. Windows PE),
      implementations should override these functions.

  .. cpp:function:: virtual Address loadAddress()

      Returns the load address of the code covered by this source.

      If the binary file type supplies non-zero base or load addresses (e.g. Windows PE),
      implementations should override these functions.

  .. cpp:function:: std::map<Address, std::string>& linkage()

      Returns the external linkage map.

      This may be empty.

.. cpp:struct:: CodeSource::Hint

  **A starting point for parsing**

  .. note:: This class satisfies the C++ `Compare <https://en.cppreference.com/w/cpp/named_req/Compare>`_ concept.

  .. cpp:member:: Address _addr
  .. cpp:member:: int _size
  .. cpp:member:: CodeRegion* _reg
  .. cpp:member:: std::string _name

  .. cpp:function:: Hint(Addr, CodeRegion*, std::string)
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
