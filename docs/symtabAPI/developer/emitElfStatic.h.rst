.. _`sec:emitElfStatic.h`:

emitElfStatic.h
###############

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:var:: extern const std::string SYMTAB_CTOR_LIST_REL
.. cpp:var:: extern const std::string SYMTAB_DTOR_LIST_REL
.. cpp:var:: extern const std::string SYMTAB_IREL_START
.. cpp:var:: extern const std::string SYMTAB_IREL_END


.. cpp:class:: emitElfStatic

  This class is unnecessary. However, at the time of writing, emitElf was
  split into two different classes (one for 32-bit and 64-bit). Instead of
  duplicating code, this class was created to share code between the
  two emitElf classes. Once the emitElf classes are merged, this class can be merged with the new
  emitElf class.

  .. cpp:function:: emitElfStatic(unsigned addressWidth, bool isStripped)

      Entry point for static linking

  .. cpp:function:: static std::string printStaticLinkError(StaticLinkError)
  .. cpp:function:: char *linkStatic(Symtab *target, StaticLinkError &err, string &errMsg)
  .. cpp:function:: bool resolveSymbols(Symtab *target, vector<Symtab *> &relocatableObjects, LinkMap &lmap, StaticLinkError &err, string &errMsg)
  .. cpp:function:: bool createLinkMap(Symtab *target, vector<Symtab *> &relocatableObjects, Offset& globalOffset, LinkMap &lmap, \
                                       StaticLinkError &err, string &errMsg)
  .. cpp:function:: Offset layoutRegions(deque<Region *> &regions, map<Region *, LinkMap::AllocPair> &regionAllocs, Offset currentOffset, Offset globalOffset)
  .. cpp:function:: Offset allocStubRegions(LinkMap &lmap, Offset globalOffset)
  .. cpp:function:: bool addNewRegions(Symtab *target, Offset globalOffset, LinkMap &lmap)
  .. cpp:function:: void copyRegions(LinkMap &lmap)
  .. cpp:function:: bool applyRelocations(Symtab *target, vector<Symtab *> &relocatableObjects, Offset globalOffset, LinkMap &lmap, \
                                          StaticLinkError &err, string &errMsg)
  .. cpp:function:: bool buildPLT(Symtab *target, Offset globalOffset, LinkMap &lmap, StaticLinkError &err, string &errMsg)
  .. cpp:function:: bool buildRela(Symtab *target, Offset globalOffset, LinkMap &lmap, StaticLinkError &err, string &errMsg)
  .. cpp:function:: bool hasRewrittenTLS() const

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
  .. cpp:function:: static bool sort_reg(const Region*a, const Region*b)
  .. cpp:function:: static bool updateHeapVariables(Symtab *obj, unsigned long loadSecsSize)
  .. cpp:function:: static bool updateRelocation(Symtab *obj, relocationEntry &rel, int library_adjust)

