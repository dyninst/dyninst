LinkMap.h
#########

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: LinkMap

  A data structure that holds all the information necessary to perform a
  static link once all the relocatable files have been copied into a new data
  block.

  .. cpp:type:: pair<Offset, Offset> AllocPair

      A pair representing an allocation for a Region
      The first Offset is the amount of padding before the Region
      The second Offset is the offset of the location in the allocatedData

  .. cpp:member:: char *allocatedData

      All other members describe this block of data.

  .. cpp:member:: Offset allocatedSize
  .. cpp:member:: map<Region *, AllocPair> regionAllocs

      map of Regions placed in allocatedData

  .. cpp:member:: Region *commonStorage

      Keep track of the dynamically allocated COMMON symbol Region

  .. note::
  
    New Region info
    
    | Offset -> offset in allocatedData
    | Size -> size of Region
    | Regions -> existing Regions which make up this Region

  .. cpp:member:: Offset bssRegionOffset
  .. cpp:member:: Offset bssSize
  .. cpp:member:: Offset bssRegionAlign
  .. cpp:member:: deque<Region *> bssRegions
  .. cpp:member:: Offset dataRegionOffset
  .. cpp:member:: Offset dataSize
  .. cpp:member:: Offset dataRegionAlign
  .. cpp:member:: deque<Region *> dataRegions
  .. cpp:member:: Offset stubRegionOffset
  .. cpp:member:: Offset stubSize
  .. cpp:member:: std::map<Symbol *, Offset> stubMap
  .. cpp:member:: Offset codeRegionOffset
  .. cpp:member:: Offset codeSize
  .. cpp:member:: Offset codeRegionAlign
  .. cpp:member:: deque<Region *> codeRegions
  .. cpp:member:: Offset tlsRegionOffset
  .. cpp:member:: Offset tlsSize
  .. cpp:member:: Offset tlsRegionAlign
  .. cpp:member:: deque<Region *> tlsRegions
  .. cpp:member:: vector<Symbol *> tlsSymbols
  .. cpp:member:: Offset gotRegionOffset
  .. cpp:member:: Offset gotSize
  .. cpp:member:: Offset gotRegionAlign
  .. cpp:member:: vector<pair<Symbol *, Offset>>gotSymbolTable
  .. cpp:member:: map <Symbol *, Offset> gotSymbols
  .. cpp:member:: deque<Region *> gotRegions
  .. cpp:member:: Symtab *ctorDtorHandler
  .. cpp:member:: Offset ctorRegionOffset
  .. cpp:member:: Offset ctorSize
  .. cpp:member:: Offset ctorRegionAlign
  .. cpp:member:: Region *originalCtorRegion
  .. cpp:member:: set<Region *, CtorComp<Region*>> newCtorRegions
  .. cpp:member:: Offset dtorRegionOffset
  .. cpp:member:: Offset dtorSize
  .. cpp:member:: Offset dtorRegionAlign
  .. cpp:member:: Region *originalDtorRegion
  .. cpp:member:: set<Region *, CtorComp<Region*>> newDtorRegions

  .. cpp:member:: vector< pair<Symbol *, Offset>> origSymbols

      Keep track of changes made to symbols and relocations

  .. cpp:member:: vector< pair<relocationEntry *, Symbol *>> origRels

      Keep track of changes made to symbols and relocations

  .. cpp:member:: Offset pltRegionOffset

      GNU extension: indirect functions and IRELATIV relocations
      We basically create mini-PLT entries for load-time  decisions of which function to call

  .. cpp:member:: Offset pltSize

      GNU extension: indirect functions and IRELATIV relocations
      We basically create mini-PLT entries for load-time  decisions of which function to call

  .. cpp:member:: Offset pltRegionAlign

      GNU extension: indirect functions and IRELATIV relocations
      We basically create mini-PLT entries for load-time  decisions of which function to call

  .. cpp:member:: std::map<Symbol *, std::pair<Offset, Offset>> pltEntries

      First Offset: offset of the PLT stub.
      Second Offset: offset of the GOT entry referenced by the stub.

  .. cpp:member:: std::map<Symbol *, Offset> pltEntriesInGOT

  .. note::
  
    GNU extension: create a new rel section

  .. cpp:member:: Offset relRegionOffset
  .. cpp:member:: Offset relSize
  .. cpp:member:: Offset relRegionAlign

  .. note::
  
    GNU extension: create a new rel section with a GOT-equivalent
    
  .. cpp:member:: Offset relGotRegionOffset
  .. cpp:member:: Offset relGotSize
  .. cpp:member:: Offset relGotRegionAlign

  .. cpp:function:: LinkMap()
  .. cpp:function:: void print(Offset globalOffset)
  .. cpp:function:: void printAll(ostream &os, Offset globalOffset)
  .. cpp:function:: void printBySymtab(ostream &os, vector<Symtab *> &symtabs, Offset globalOffset)
  .. cpp:function:: void printRegions(ostream &os, deque<Region *> &regions, Offset globalOffset)
  .. cpp:function:: void printRegion(ostream &os, Region *region, Offset globalOffset)
  .. cpp:function:: void printRegionFromInfo(ostream &os, Region *region, Offset regionOffset, Offset padding)
  .. cpp:function:: friend ostream & operator<<(ostream &os, LinkMap &lm)


.. cpp:struct:: template <typename T> LinkMap::CtorComp

  .. cpp:function:: bool operator()(T lhs, T rhs) const
