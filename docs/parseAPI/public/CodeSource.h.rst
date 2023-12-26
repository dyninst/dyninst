CodeSource.h
============

.. cpp:namespace:: Dyninst::parseAPI

Class CodeSource
----------------

**Defined in:** ``CodeSource.h``

The CodeSource interface is used by the ParseAPI to retrieve binary code
from an executable, library, or other binary code object; it also can
provide hints of function entry points (such as those derived from
debugging symbols) to seed the parser. The ParseAPI provides a default
implementation based on the SymtabAPI that supports many common binary
formats. For details on implementing a custom CodeSource, see Appendix
`5 <#sec:extend>`__.

.. code-block:: cpp
    
    virtual bool nonReturning(Address func_entry) virtual bool
    nonReturning(std::string func_name)

Looks up whether a function returns (by name or location). This
information may be statically known for some code sources, and can lead
to better parsing accuracy.

.. code-block:: cpp
    
    virtual bool nonReturningSyscall(int /*number*/)

Looks up whether a system call returns (by system call number). This
information may be statically known for some code sources, and can lead
to better parsing accuracy.

.. code-block:: cpp
    
    virtual Address baseAddress() virtual Address loadAddress()

If the binary file type supplies non-zero base or load addresses (e.g.
Windows PE), implementations should override these functions.

.. code-block:: cpp
    
    std::map< Address, std::string > & linkage()

Returns a reference to the external linkage map, which may or may not be
filled in for a particular CodeSource implementation.

.. code-block:: cpp
    
    struct Hint Address _addr; CodeRegion *_region; std::string _name;
    Hint(Addr, CodeRegion *, std::string); std::vector< Hint > const&
    hints()

Returns a vector of the currently defined function entry hints.

.. code-block:: cpp
    
    std::vector<CodeRegion *> const& regions()

Returns a read-only vector of code regions within the binary represented
by this code source.

.. code-block:: cpp
    
    int findRegions(Address addr, set<CodeRegion *> & ret)

Finds all CodeRegion objects that overlap the provided address. Some
code sources (e.g. archive files) may have several regions with
overlapping address ranges; others (e.g. ELF binaries) do not.

.. code-block:: cpp
    
    bool regionsOverlap()

Indicates whether the CodeSource contains overlapping regions.

Class CodeRegion
----------------

**Defined in:** ``CodeSource.h``

The CodeRegion interface is an accounting structure used to divide
CodeSources into distinct regions. This interface is mostly of interest
to CodeSource implementors.

.. code-block:: cpp
    
    void names(Address addr, vector<std::string> & names)

Fills the provided vector with any names associated with the function at
a given address in the region, e.g. symbol names in an ELF or PE binary.

.. code-block:: cpp
    
    virtual bool findCatchBlock(Address addr, Address & catchStart)

Finds the exception handler associated with an address, if one exists.
This routine is only implemented for binary code sources that support
structured exception handling, such as the SymtabAPI-based
SymtabCodeSource provided as part of the ParseAPI.

.. code-block:: cpp
    
    Address low()

The lower bound of the interval of address space covered by this region.

.. code-block:: cpp
    
    Address high()

The upper bound of the interval of address space covered by this region.

.. code-block:: cpp
    
    bool contains(Address addr)

Returns true if
:math:`\small \texttt{addr} \in [\small \texttt{low()},\small \texttt{high()})`,
false otherwise.

.. code-block:: cpp
    
    virtual bool wasUserAdded() const

Return true if this region was added by the user, false otherwise.