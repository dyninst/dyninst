.. _`sec:emitElf.h`:

emitElf.h
#########

.. code:: cpp

  #define PT_PAX_FLAGS  (PT_LOOS + 0x5041580) /* PaX flags */

.. cpp:var:: extern const char *STRTAB_NAME
.. cpp:var:: extern const char *SYMTAB_NAME
.. cpp:var:: extern const char *INTERP_NAME

.. cpp:function:: extern const char *pdelf_get_shnames(Dyninst::Elf_X *elf)

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: template<class ElfTypes = ElfTypes64> emitElf : public ElfTypes

  .. cpp:type:: typename ElfTypes::Elf_Ehdr Elf_Ehdr
  .. cpp:type:: typename ElfTypes::Elf_Phdr Elf_Phdr
  .. cpp:type:: typename ElfTypes::Elf_Shdr Elf_Shdr
  .. cpp:type:: typename ElfTypes::Elf_Dyn Elf_Dyn
  .. cpp:type:: typename ElfTypes::Elf_Half Elf_Half
  .. cpp:type:: typename ElfTypes::Elf_Addr Elf_Addr
  .. cpp:type:: typename ElfTypes::Elf_Off Elf_Off
  .. cpp:type:: typename ElfTypes::Elf_Word Elf_Word
  .. cpp:type:: typename ElfTypes::Elf_Sym Elf_Sym
  .. cpp:type:: typename ElfTypes::Elf_Section Elf_Section
  .. cpp:type:: typename ElfTypes::Elf_Rel Elf_Rel
  .. cpp:type:: typename ElfTypes::Elf_Rela Elf_Rela
  .. cpp:type:: typename ElfTypes::Elf_Verneed Elf_Verneed
  .. cpp:type:: typename ElfTypes::Elf_Vernaux Elf_Vernaux
  .. cpp:type:: typename ElfTypes::Elf_Verdef Elf_Verdef
  .. cpp:type:: typename ElfTypes::Elf_Verdaux Elf_Verdaux

  .. cpp:function:: emitElf(Elf_X *pX, bool i, Object *pObject, void (*pFunction)(const char *), Symtab *pSymtab)
  .. cpp:function:: bool createSymbolTables(std::set<Symbol *> &allSymbols)

    Regenerates the .symtab and .strtab sections from the symbols. Also adds new .dynsym, .dynstr sections for the
    newly-added dynamic symbols

    For every symbol, call :cpp:func:`createElfSymbol` to get an ``Elf_Sym`` corresponding to a :cpp:class:`Symbol` object.
    Accumulate all and their names to form the sections and add them to the list of new sections.

  .. cpp:function:: bool driver(std::string fName)

  ......

  .. rubric:: Important data sections in the new Elf that need updated

  .. cpp:member:: private Elf_X *oldElfHandle
  .. cpp:member:: private Elf *newElf
  .. cpp:member:: private Elf *oldElf
  .. cpp:member:: private Symtab *obj
  .. cpp:member:: private Elf_Ehdr *newEhdr
  .. cpp:member:: private Elf_Ehdr *oldEhdr
  .. cpp:member:: private Elf_Phdr *newPhdr
  .. cpp:member:: private Elf_Phdr *oldPhdr
  .. cpp:member:: private Offset phdr_offset
  .. cpp:member:: private Elf_Data *textData
  .. cpp:member:: private Elf_Data *symStrData
  .. cpp:member:: private Elf_Data *dynStrData
  .. cpp:member:: private char *olddynStrData
  .. cpp:member:: private unsigned olddynStrSize
  .. cpp:member:: private Elf_Data *symTabData
  .. cpp:member:: private Elf_Data *dynsymData
  .. cpp:member:: private Elf_Data *dynData
  .. cpp:member:: private std::vector<Region *>nonLoadableSecs
  .. cpp:member:: private std::vector<Region *> newSecs
  .. cpp:member:: private std::map<unsigned, std::vector<Elf_Dyn *> > dynamicSecData
  .. cpp:member:: private std::vector<std::string> DT_NEEDEDEntries
  .. cpp:member:: private std::vector<std::pair<long, long> > new_dynamic_entries
  .. cpp:member:: private std::vector<std::string> unversionedNeededEntries
  .. cpp:member:: private std::map<std::string, std::map<std::string, unsigned> >verneedEntries
  .. cpp:member:: private std::map<std::string, unsigned> verdefEntries
  .. cpp:member:: private std::map<unsigned, std::vector<std::string> > verdauxEntries
  .. cpp:member:: private std::map<std::string, unsigned> versionNames
  .. cpp:member:: private std::vector<Elf_Half> versionSymTable
  .. cpp:member:: private int curVersionNum
  .. cpp:member:: private int verneednum
  .. cpp:member:: private int verdefnum
  .. cpp:member:: private int dynsym_info

  ......

  .. rubric:: Needed when adding a new segment

  .. cpp:member:: private Elf_Off newSegmentStart
  .. cpp:member:: private Elf_Shdr *firstNewLoadSec
  .. cpp:member:: private Elf_Off dataSegEnd
  .. cpp:member:: private Elf_Off dynSegOff
  .. cpp:member:: private Elf_Off dynSegAddr
  .. cpp:member:: private Elf_Off phdrSegOff
  .. cpp:member:: private Elf_Off phdrSegAddr
  .. cpp:member:: private unsigned dynSegSize
  .. cpp:member:: private unsigned secNameIndex
  .. cpp:member:: private Offset currEndOffset
  .. cpp:member:: private Address currEndAddress

  .. cpp:member:: private vector<std::string> secNames

      Section names for all sections

  .. cpp:member:: private char *linkedStaticData

      Pointer to all relocatable code and data allocated during a static link, to be deleted after written out.

  ......

  .. rubric:: Expand NOBITS sections within the object file to their size

  .. cpp:member:: private bool BSSExpandFlag
  .. cpp:member:: private bool movePHdrsFirst
  .. cpp:member:: private bool createNewPhdr
  .. cpp:member:: private bool replaceNOTE
  .. cpp:member:: private unsigned loadSecTotalSize

  ......

  .. cpp:member:: private bool isStripped
  .. cpp:member:: private int library_adjust
  .. cpp:member:: private Object *object
  .. cpp:type:: private void(*err_func_)(const char *)
  .. cpp:function:: private bool createElfSymbol(Symbol *symbol, unsigned strIndex, vector<Elf_Sym *> &symbols, bool dynSymFlag = false)
  .. cpp:function:: private void findSegmentEnds()

    Find the end of data/text segment

  .. cpp:function:: private void renameSection(const std::string &oldStr, const std::string &newStr, bool renameAll = true)

    Rename an old section. Lengths of old and new names must match.

    Only renames the *FIRST* matching section encountered.

  .. cpp:function:: private void fixPhdrs(unsigned &)
  .. cpp:function:: private void createNewPhdrRegion(std::unordered_map<std::string, unsigned> &newNameIndexMapping)
  .. cpp:function:: private bool addSectionHeaderTable(Elf_Shdr *shdr)
  .. cpp:function:: private bool createNonLoadableSections(Elf_Shdr *&shdr)
  .. cpp:function:: private bool createLoadableSections(Elf_Shdr *&shdr, unsigned &extraAlignSize, std::unordered_map<std::string, unsigned> &newIndexMapping, unsigned &sectionNumber)
  .. cpp:function:: private void createRelocationSections(std::vector<relocationEntry> &relocation_table, bool isDynRelocs, std::unordered_map<std::string, unsigned long> &dynSymNameMapping)
  .. cpp:function:: private void updateSymbols(Elf_Data *symtabData, Elf_Data *strData, unsigned long loadSecsSize)

    Sets ``_end`` and ``_END_`` to the starting position of the heap in the new binary.

  .. cpp:member:: private bool hasRewrittenTLS
  .. cpp:member:: private bool TLSExists
  .. cpp:member:: private Elf_Shdr *newTLSData
  .. cpp:function:: private void updateDynamic(unsigned tag, Elf_Addr val)

    Updates the .dynamic section to reflect the changes to the relocation section.

  .. cpp:function:: private void createSymbolVersions(Elf_Half *&symVers, char *&verneedSecData, unsigned &verneedSecSize, char *&verdefSecData, unsigned &verdefSecSize, unsigned &dynSymbolNamesLength, std::vector<std::string> &dynStrs)
  .. cpp:function:: private void createHashSection(Elf_Word *&hashsecData, unsigned &hashsecSize, std::vector<Symbol *> &dynSymbols)
  .. cpp:function:: private void createDynamicSection(void *dynData, unsigned size, Elf_Dyn *&dynsecData, unsigned &dynsecSize, unsigned &dynSymbolNamesLength, std::vector<std::string> &dynStrs)
  .. cpp:function:: private void addDTNeeded(std::string s)
  .. cpp:function:: private void log_elferror(void(*err_func)(const char *), const char *msg)
  .. cpp:function:: private bool cannotRelocatePhdrs()
  .. cpp:member:: private bool isStaticBinary
  .. cpp:member:: private std::vector<void *> buffers
  .. cpp:function:: private char *allocate_buffer(size_t)


.. code:: cpp

  extern template class emitElf<ElfTypes32>
  extern template class emitElf<ElfTypes64>


.. cpp:struct:: sortByOffsetNewIndices

  .. cpp:function:: bool operator()(Symbol *lhs, Symbol *rhs) const


.. cpp:struct:: sortByIndex

  .. cpp:function:: bool operator()(Symbol *lhs, Symbol *rhs) const


.. cpp:struct:: ElfTypes32

  .. cpp:type:: Elf32_Ehdr Elf_Ehdr
  .. cpp:type:: Elf32_Phdr Elf_Phdr
  .. cpp:type:: Elf32_Shdr Elf_Shdr
  .. cpp:type:: Elf32_Dyn Elf_Dyn
  .. cpp:type:: Elf32_Half Elf_Half
  .. cpp:type:: Elf32_Addr Elf_Addr
  .. cpp:type:: Elf32_Off Elf_Off
  .. cpp:type:: Elf32_Word Elf_Word
  .. cpp:type:: Elf32_Sym Elf_Sym
  .. cpp:type:: Elf32_Section Elf_Section
  .. cpp:type:: Elf32_Rel Elf_Rel
  .. cpp:type:: Elf32_Rela Elf_Rela
  .. cpp:type:: Elf32_Verneed Elf_Verneed
  .. cpp:type:: Elf32_Vernaux Elf_Vernaux
  .. cpp:type:: Elf32_Verdef Elf_Verdef
  .. cpp:type:: Elf32_Verdaux Elf_Verdaux
  .. cpp:function:: Elf_Ehdr *elf_newehdr(Elf *elf)
  .. cpp:function:: Elf_Phdr *elf_newphdr(Elf *elf, size_t num)
  .. cpp:function:: Elf_Ehdr *elf_getehdr(Elf *elf)
  .. cpp:function:: Elf_Phdr *elf_getphdr(Elf *elf)
  .. cpp:function:: Elf_Shdr *elf_getshdr(Elf_Scn *scn)
  .. cpp:function:: Elf32_Word makeRelocInfo(Elf32_Word sym, Elf32_Word type)


.. cpp:struct:: ElfTypes64

  .. cpp:type:: Elf64_Ehdr Elf_Ehdr
  .. cpp:type:: Elf64_Phdr Elf_Phdr
  .. cpp:type:: Elf64_Shdr Elf_Shdr
  .. cpp:type:: Elf64_Dyn Elf_Dyn
  .. cpp:type:: Elf64_Half Elf_Half
  .. cpp:type:: Elf64_Addr Elf_Addr
  .. cpp:type:: Elf64_Off Elf_Off
  .. cpp:type:: Elf64_Word Elf_Word
  .. cpp:type:: Elf64_Sym Elf_Sym
  .. cpp:type:: Elf64_Section Elf_Section
  .. cpp:type:: Elf64_Rel Elf_Rel
  .. cpp:type:: Elf64_Rela Elf_Rela
  .. cpp:type:: Elf64_Verneed Elf_Verneed
  .. cpp:type:: Elf64_Vernaux Elf_Vernaux
  .. cpp:type:: Elf64_Verdef Elf_Verdef
  .. cpp:type:: Elf64_Verdaux Elf_Verdaux
  .. cpp:function:: Elf_Ehdr *elf_newehdr(Elf *elf)
  .. cpp:function:: Elf_Phdr *elf_newphdr(Elf *elf, size_t num)
  .. cpp:function:: Elf_Ehdr *elf_getehdr(Elf *elf)
  .. cpp:function:: Elf_Phdr *elf_getphdr(Elf *elf)
  .. cpp:function:: Elf_Shdr *elf_getshdr(Elf_Scn *scn)
  .. cpp:function:: Elf64_Xword makeRelocInfo(Elf64_Word sym, Elf64_Word type)
