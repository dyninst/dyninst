.. _`sec:emitElfStatic.h`:

emitElfStatic.h
###############

.. cpp:namespace:: Dyninst::SymtabAPI
  
This class is unnecessary. However, at the time of writing, emitElf was split into two
different classes (one for 32-bit and 64-bit). Instead of duplicating code, this class was created
to share code between the two emitElf classes. Once the emitElf classes are merged, this class can
be merged with the new emitElf class.

.. cpp:class:: emitElfStatic

  This class is unnecessary. However, at the time of writing, emitElf was
  split into two different classes (one for 32-bit and 64-bit). Instead of
  duplicating code, this class was created to share code between the
  two emitElf classes. Once the emitElf classes are merged, this class can be merged with the new
  emitElf class.

  .. note::
    Most of these functions take a reference to a :cpp:type:`StaticLinkError` and a string
    for error reporting purposes. These should prove useful in identifying the cause of an error.

  .. cpp:function:: emitElfStatic(unsigned addressWidth, bool isStripped)

      Entry point for static linking

  .. cpp:function:: static std::string printStaticLinkError(StaticLinkError)

    A string representation of the StaticLinkError returned by other functions.

  .. cpp:function:: char *linkStatic(Symtab *target, StaticLinkError &err, string &errMsg)

    Statically links relocatable files into the specified Symtab object, as specified by state in the
    Symtab object.

    target: relocatable files will be linked into this Symtab

    Returns a pointer to a block of data containing the results of the link The caller is responsible for
    ``delete``\ ing this block of data.

  .. cpp:function:: bool resolveSymbols(Symtab *target, vector<Symtab *> &relocatableObjects, LinkMap &lmap, StaticLinkError &err, string &errMsg)

    Resolves undefined symbols in the specified Symtab object, usually due to the addition of new
    Symbols to the Symtab object. The target Symtab object must have a collection of Archives associated
    with it. These Archives will be searched for the defined versions of the undefined symbols in the
    specified Symtab objects.

    target: the Symtab containing the undefined symbols

    relocatableObjects: populated by this function, this collection specifies all the Symtabs needed to
    ensure that the target has no undefined Symbols once the link is performed

  .. cpp:function:: bool createLinkMap(Symtab *target, vector<Symtab *> &relocatableObjects, Offset& globalOffset, LinkMap &lmap, \
                                       StaticLinkError &err, string &errMsg)

    Given a collection of Symtab objects, combines the code, data, bss and other miscellaneous Regions
    into groups and places them in a new block of data.

    Allocates COMMON symbols in the collection of Symtab objects as bss.

    Creates a new TLS initialization image, combining the target image and the
    image that exists in the collection of Symtab objects.

    Creates a GOT used for indirect memory accesses that is required by some relocations.

    Creates a new global constructor and/or destructor table if necessary, combining tables from
    the target and collection of Symtab objects.

    target:  New code/data/etc. will be linked into this Symtab

    relocatableObjects: The new code/data/etc.

    globalOffset: The location of the new block of data in the target

    lmap: The LinkMap to be populated by this function

  .. cpp:function:: Offset layoutRegions(deque<Region *> &regions, map<Region *, LinkMap::AllocPair> &regionAllocs,\
                                         Offset currentOffset, Offset globalOffset)

    Lays out the specified regions, storing the layout info in the passed map.

    regions: A collection of Regions to layout

    regionAllocs: A map of Regions to their layout information

    currentOffset: The starting offset for the passed Regions in the new storage space

    globalOffset: The location of the new storage space in the target (used for padding calculation)

  .. cpp:function:: Offset allocStubRegions(LinkMap &lmap, Offset globalOffset)

  .. cpp:function:: bool addNewRegions(Symtab *target, Offset globalOffset, LinkMap &lmap)

    Adds new combined Regions to the target at the specified globalOffset

    target: The Symtab object to which the new Regions will be added

    globalOffset: The offset of the first new Region in the target

    lmap: Contains all the information about the LinkMap

  .. cpp:function:: void copyRegions(LinkMap &lmap)

    Copies the new Regions, as indicated by the LinkMap, into the allocated storage space.

    lmap: Contains all the information necessary to perform the copy

  .. cpp:function:: bool applyRelocations(Symtab *target, vector<Symtab *> &relocatableObjects, Offset globalOffset, LinkMap &lmap, \
                                          StaticLinkError &err, string &errMsg)

    Given a collection of newly allocated regions in the specified storage space,
    computes relocations and places the values at the location specified by the
    relocation entry (stored with the Regions)

    target               The Symtab object being rewritten

    relocatableObjects   A list of relocatable files being linked into target

    globalOffset         The location of the new storage space in target

    lmap                 Contains all the information necessary to apply relocations

  .. cpp:function:: bool buildPLT(Symtab *target, Offset globalOffset, LinkMap &lmap, StaticLinkError &err, string &errMsg)
  .. cpp:function:: bool buildRela(Symtab *target, Offset globalOffset, LinkMap &lmap, StaticLinkError &err, string &errMsg)
  .. cpp:function:: bool hasRewrittenTLS() const

    Checks if a new TLS initialization image has been created.

  .. cpp:function:: private Offset computePadding(Offset candidateOffset, Offset alignment)

    Computes the padding necessary to satisfy the specified alignment

    candidateOffset: A possible offset for an item

    alignment: The alignment for an item

  .. cpp:function:: private char getPaddingValue(Region::RegionType rtype)

      Architecture specific

      Given the Region type of a combined Region, gets the padding value to
      use in between Regions that make up the combined Region.

      rtype    The Region type for the combined Region

      Returns the padding character

  .. cpp:function:: private bool archSpecificRelocation(Symtab *targetSymtab,Symtab *srcSymtab, char *targetData, relocationEntry &rel, \
                                                        Offset dest, Offset relOffset, Offset globalOffset, LinkMap &lmap, string &errMsg)

      Architecture specific

      Calculates a relocation and applies it to the specified location in the
      target.

      targetData       The target buffer
      rel              The relocation entry
      dest             The offset in the target buffer
      relOffset        The absolute offset of the relocation
      globalOffset     The absolute offset of the newly linked code
      lmap             Holds information necessary to compute relocation

      Returns true, on success false, otherwise and sets errMsg

  .. cpp:function:: private bool handleInterModuleSpecialCase(Symtab *target, Symtab *src, LinkMap &lmap, char *data,\
                                                              relocationEntry rel, Offset newTOC, Offset oldTOC, Offset dest,\
                                                              Offset relOffset, Offset globalOffset)

    PPC64 TOC-changing inter-module calls

  .. cpp:function:: private Offset findOrCreateStub(Symbol *sym, LinkMap &lmap, Offset newTOC, Offset oldTOC, char *data,\
                                                    Offset global)
  .. cpp:function:: private void createStub(unsigned *stub, Offset stubOffset, Offset newTOC, Offset oldTOC, Offset dest)


  .. cpp:function:: private Offset layoutTLSImage(Offset globalOffset, Region *dataTLS, Region *bssTLS, LinkMap &lmap)

      Architecture specific (similar to layoutRegions)

      Creates a new TLS initialization image from the existing TLS Regions in the
      target and any new TLS Regions from the relocatable objects.

      globalOffset     The absolute offset of the newly linked code
      dataTLS          The original TLS data Region from the target (can be NULL)
      bssTLS           The original TLS bss Region from the target (can be NULL)
      lmap             Holds information necessary to do layout

      Returns the ending Offset of the Region

  .. cpp:function:: private Offset tlsLayoutVariant1(Offset globalOffset, Region *dataTLS, Region *bssTLS, LinkMap &lmap)

      See above.

  .. cpp:function:: private Offset tlsLayoutVariant2(Offset globalOffset, Region *dataTLS, Region *bssTLS, LinkMap &lmap)

      See above.

  .. cpp:function:: private Offset adjustTLSOffset(Offset curOffset, Offset tlsSize)

      Architecture specific

      Updates the TLS offset of a Symbol, given the size of the new TLS initialization image.

      curOffset        The current offset of the TLS symbol
      tlsSize          The size of the new TLS initialization image

      Returns the adjusted offset

  .. cpp:function:: private Offset tlsAdjustVariant2(Offset curOffset, Offset tlsSize)

      See above.

      .. note:: Variant 1 does not require any modifications, so a separate function is not necessary

  .. cpp:function:: private void cleanupTLSRegionOffsets(map<Region *, LinkMap::AllocPair> &regionAllocs, Region *dataTLS, Region *bssTLS)

      Architecture specific

      In order to simplify the creation of a new TLS initialization image, some cleanup
      work may be necessary after the new TLS initialization image is created.

      regionAllocs     The map of Regions to their place in the newly linked code
      dataTLS          The original TLS data section from the target (can be NULL)
      bssTLS           The original TLS bss section from the target (can be NULL)

  .. cpp:function:: private void tlsCleanupVariant1(map<Region *, LinkMap::AllocPair> &regionAllocs, Region *dataTLS, Region *bssTLS)

      See above.

  .. cpp:function:: private void tlsCleanupVariant2(map<Region *, LinkMap::AllocPair> &regionAllocs, Region *dataTLS, Region *bssTLS)

      See above.

  .. cpp:function:: private bool isGOTRelocation(unsigned long relType)

      Architecture specific

      Determines if the passed relocation type requires the building of a GOT

      relType          The relocation type to check

      Returns true if the relocation type requires a GOT

  .. cpp:function:: private void buildGOT(Symtab *target, LinkMap &lmap)

      Architecture specific

      Constructions a new GOT Region from information in the LinkMap

  .. cpp:function:: private Offset getGOTSize(Symtab *target, LinkMap &lmap, Offset &layoutStart)

      Architecture specific

      Determines the size of the GOT Region from information in the LinkMap

  .. cpp:function:: private Offset getGOTAlign(LinkMap &lmap)

      Architecture specific

      Determines the GOT Region alignment from information in the LinkMap

  .. cpp:function:: private bool isConstructorRegion(Region *reg)

      Architecture specific

      Determines if the passed Region corresponds to a constructor table Region

  .. cpp:function:: private Offset layoutNewCtorRegion(LinkMap &lmap)

      Architecture specific

      Lays out a new constructor table Region from the existing constructor
      table in the target and any new constructor Regions in the relocatable files

      Returns the ending offset of the new Region

  .. cpp:function:: private bool createNewCtorRegion(LinkMap &lmap)

      Creates a new constructor Table Region using information stored in the LinkMap

      Returns true on success

  .. cpp:function:: private bool isDestructorRegion(Region *reg)

      Architecture specific

      Determines if the passed Region corresponds to a destructor table Region

  .. cpp:function:: private bool isGOTRegion(Region *reg)

      Architecture specific

      Determines if the passed Region corresponds to a global offset table Region

  .. cpp:function:: private Offset layoutNewDtorRegion(LinkMap &lmap)

      Architecture specific

      Lays out a new destructor table Region from the existing destructor
      table in the target and any new destructor Regions in the relocatable files

      Returns the ending offset of the new Region

  .. cpp:function:: private bool createNewDtorRegion(LinkMap &lmap)

      Architecture specific

      Creates a new destructor Table Region using information stored in the LinkMap

      Returns true on success

  .. cpp:function:: private void getExcludedSymbolNames(std::set<std::string> &symNames)

      Architecture specific

      Gets the symbols that should be excluded when resolving symbols

      symNames         This set is populated by the function

  .. cpp:function:: private bool checkSpecialCaseSymbols(Symtab *member, Symbol *checkSym)

      Architecture specific

      Checks if the specified symbol satisfies a special case that is
      currently not handled by emitElfStatic.

      member           The reloctable object to examine
      checkSym         The symbol to check

      Returns false if the symbol satisfies a special case

  .. cpp:function:: private bool calculateTOCs(Symtab *target, deque<Region *> &regions, Offset GOTbase, Offset newGOToffset, Offset globalOffset)

      More with the architecture specific

      Calculate new TOC values if we care (PPC64)

  .. cpp:function:: private Offset allocatePLTEntries(std::map<Symbol *, std::pair<Offset, Offset> > &entries, Offset pltOffset, Offset &size)

      Somewhat architecture specific

      Allocate PLT entries for each INDIRECT-typed symbol
      Each PLT entry has an arch-specific size

  .. cpp:function:: private Offset allocateRelocationSection(std::map<Symbol *, std::pair<Offset, Offset> > &entries, Offset relocOffset, Offset &size, Symtab *target)

      Architecture Specific

      Generate a new relocation section that combines relocs from any indirect symbols with original relocs

  .. cpp:function:: private Offset allocateRelGOTSection(const std::map<Symbol *, std::pair<Offset, Offset> > &entries, Offset relocOffset, Offset &size)

      See above.

  .. cpp:function:: private bool addIndirectSymbol(Symbol *sym, LinkMap &lmap)

      See above.

  .. cpp:function:: private bool updateTOC(Symtab *file, LinkMap &lmap, Offset globalOffset)

      Update the TOC pointer if necessary (PPC, 64-bit)

  .. cpp:member:: private unsigned addressWidth_
  .. cpp:member:: private bool isStripped_
  .. cpp:member:: private bool hasRewrittenTLS_
  .. cpp:type:: private boost::tuple<Offset, Offset, Offset> TOCstub
  .. cpp:member:: private std::map<Symbol *, TOCstub> stubMap
  .. cpp:function:: private Offset getStubOffset(TOCstub &t)
  .. cpp:function:: private Offset getNewTOC(TOCstub &t)
  .. cpp:function:: private Offset getOldTOC(TOCstub &t)


.. cpp:enum:: emitElfStatic::StaticLinkError

  .. cpp:enumerator:: No_Static_Link_Error
  .. cpp:enumerator:: Link_Location_Error
  .. cpp:enumerator:: Symbol_Resolution_Failure
  .. cpp:enumerator:: Relocation_Computation_Failure
  .. cpp:enumerator:: Storage_Allocation_Failure


.. cpp:class:: emitElfUtils

  These routines should be in a private namespace inside a unified
  emit class file or something.

  .. cpp:function:: static Address orderLoadableSections(Symtab *obj, vector<Region*> & sections)

    Sort the sections array so that sections with a non-zero memory offset come first (and are sorted in
    increasing order of offset). Preserves the ordering of zero-offset sections.

    If no section has a non-zero offset, the return value will be an address in the virtual memory space
    big enough to hold all the loadable sections. Otherwise it will be the address of the first non-zero
    offset section.

    .. note::
      if we need to create a new segment to hold these sections, it needs to be clear up to the next
      page boundary to avoid potentially clobbering other loadable segments.

  .. cpp:function:: static bool sort_reg(const Region*a, const Region*b)
  .. cpp:function:: static bool updateHeapVariables(Symtab *obj, unsigned long loadSecsSize)

    There are also some known variables that point to the heap.

  .. cpp:function:: static bool updateRelocation(Symtab *obj, relocationEntry &rel, int library_adjust)


......

.. rubric:: Used by architecture-specific functions, but are not architecture-specific themselves

.. code:: cpp

  const string Dyninst::SymtabAPI::SYMTAB_CTOR_LIST_REL("__SYMTABAPI_CTOR_LIST__")
  const string Dyninst::SymtabAPI::SYMTAB_DTOR_LIST_REL("__SYMTABAPI_DTOR_LIST__")
  const string Dyninst::SymtabAPI::SYMTAB_IREL_START("__SYMTABAPI_IREL_START__")
  const string Dyninst::SymtabAPI::SYMTAB_IREL_END("__SYMTABAPI_IREL_END__")

......

.. rubric:: Section names

.. code:: cpp

  static const string CODE_NAME(".dyninstCode")
  static const string STUB_NAME(".dyninstStub")
  static const string DATA_NAME(".dyninstData")
  static const string BSS_NAME(".dyninstBss")
  static const string GOT_NAME(".dyninstGot")
  static const string CTOR_NAME(".dyninstCtors")
  static const string DTOR_NAME(".dyninstDtors")
  static const string TLS_DATA_NAME(".dyninstTdata")
  static const string DEFAULT_COM_NAME(".common")
  static const string PLT_NAME(".dyninstPLT")
  static const string REL_NAME(".dyninstRELA")
  static const string REL_GOT_NAME(".dyninstRELAgot")

......

.. cpp:function:: static bool computeCtorDtorAddress(relocationEntry &rel, Offset globalOffset, LinkMap &lmap,\
                                                     string &, Offset &symbolOffset)

  Specific to x86

  Given a relocation, determines if the relocation corresponds to a .ctors or .dtors
  table that requires special consideration. Modifies the passed symbol offset to
  point to the right table, if applicable.

  rel          The relocation entry to examine

  globalOffset The offset of the linked code (used for symbol offset calculation)

  lmap         Holds information about .ctors/.dtors tables

  errMsg       Set on error

  symbolOffset Modified by this routine to contain the offset of the table

  Returns true, if there are no errors including the case where the relocation
  entry doesn't reference the .ctors/.dtors tables.



TLS Info
********

TLS - Thread-local Storage

TCB - Thread-control block

TLS handling is pseudo-architecture dependent. The implementation of the TLS
functions depend on the implementation of TLS on a specific architecture.

The following material is documented in more detail in the "ELF Handling For TLS" white paper.
According to this paper, their are two variants w.r.t. creating a TLS initialization image.

**First variant**::

               beginning of image
               |
               V                              high address
    +----+-----+----------+---------+---------+
    |    | TCB | image 1  | image 2 | image 3 |
    +----+---- +----------+---------+---------+

where TCB = thread control block, and each image is the
TLS initialization image for an object (in this context an executable or
shared library).

**Second variant**::

    beginning of image
    |
    V                                        high address
    +---------+---------+---------+-----+
    | image 3 | image 2 | image 1 | TCB |
    +---------+---------+---------+-----+

An image is::

    +------+-----+
    | DATA | BSS |
    +------+-----+

New TLS data and bss is added to the original initialization image as follows::

    +----------+------------------+-------------+------------+-----+
    | NEW DATA | EXPANDED NEW BSS | TARGET DATA | TARGET BSS | TCB |
    +----------+------------------+-------------+------------+-----+

It is important to note that the TARGET DATA and TARGET BSS blocks are not moved.
This ensures that the modifications to the TLS image are safe.

These are the two variants one would see when working with ELF files. So, an
architecture either uses variant 1 or 2.

.. note::
  The TLS implementation on ppc is Variant 1.

  The TLS implementation on x86 is Variant 2.

  Static re-writing on ARM is not supported.


ctor/dtor sections
******************

.ctors/.dtors sections are not defined by the ELF standard, LSB defines them.
This is why this implementation is specific to Linux and x86.

Layout of .ctors and .dtors sections on Linux

Executable .ctors/.dtors format (size in bytes = n)::

    byte 0..3    byte 4..7     byte 8..11        byte n-4..n-1
    0xffffffff <func. ptr 1> <func. ptr 2> ...  0x00000000

Relocatable file .ctors/.dtors format (size in bytes = n)::

      byte 0..3         byte n-4..n-1
    <func. ptr 1> ... <last func. ptr>

The layout is the same on Linux x86_64 except each entry is 8 bytes
instead of 4. So the header and trailler are the same, but extended to
8 bytes.

.. code:: cpp

  static const string DTOR_NAME(".fini_array");
  static const string CTOR_NAME(".init_array");
  static const string TOC_NAME(".toc");

