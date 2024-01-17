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
  .. cpp:function:: bool driver(std::string fName)

  .. cpp:member:: private vector<std::string> secNames

      Section names for all sections

  .. cpp:member:: private char *linkedStaticData

      Pointer to all relocatable code and data allocated during a static link, to be deleted after written out.

  ......

  .. rubric:: Important data sections in the new Elf that need updated

  .. cpp:member:: private Elf_Data *textData
  .. cpp:member:: private Elf_Data *symStrData
  .. cpp:member:: private Elf_Data *dynStrData
  .. cpp:member:: private char *olddynStrData
  .. cpp:member:: private unsigned olddynStrSize
  .. cpp:member:: private Elf_Data *symTabData
  .. cpp:member:: private Elf_Data *dynsymData
  .. cpp:member:: private Elf_Data *dynData

  ......

  .. rubric:: Needed when adding a new segment

  .. cpp:member:: private Elf_Off newSegmentStart
  .. cpp:member:: private Elf_Shdr *firstNewLoadSec

  ......

  .. rubric:: Expand NOBITS sections within the object file to their size

  .. cpp:member:: private bool BSSExpandFlag
  .. cpp:member:: private bool movePHdrsFirst
  .. cpp:member:: private bool createNewPhdr
  .. cpp:member:: private bool replaceNOTE


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
