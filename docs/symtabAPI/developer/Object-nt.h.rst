.. _`sec:Object-nt.h`:

Object-nt.h
###########

.. cpp:namespace:: Dyninst::SymtabAPI::windows

.. cpp:class:: Object : public AObject

  **Windows NT/2000 object files**

  .. cpp:function:: Object(MappedFile*, bool defensive, void(*)(const char *) = log_msg, \
                           bool alloc_syms = true, Symtab* st = NULL)
  .. cpp:function:: virtual ~Object( void )
  .. cpp:function:: std::string getFileName() const
  .. cpp:function:: bool isForwarded( Offset addr )
  .. cpp:function:: bool isEEL() const
  .. cpp:function:: bool isText( const Offset addr ) const
  .. cpp:function:: Offset get_base_addr() const
  .. cpp:function:: Module* GetCurrentModule( void )
  .. cpp:function:: bool getCatchBlock(ExceptionBlock &b, Offset addr, unsigned size = 0) const
  .. cpp:function:: unsigned int GetTextSectionId( void ) const
  .. cpp:function:: PIMAGE_NT_HEADERS GetImageHeader( void ) const
  .. cpp:function:: PVOID GetMapAddr( void ) const
  .. cpp:function:: Offset getEntryPoint( void ) const
  .. cpp:function:: Offset getLoadAddress() const
  .. cpp:function:: Offset getPreferedBase() const
  .. cpp:function:: Offset getEntryAddress() const
  .. cpp:function:: Offset getBaseAddress() const
  .. cpp:function:: Offset getTOCoffset(Offset) const
  .. cpp:function:: ObjectType objType() const
  .. cpp:function:: const char *interpreter_name() const
  .. cpp:function:: dyn_hash_map <std::string, LineInformation> &getLineInfo()
  .. cpp:function:: void parseTypeInfo()
  .. cpp:function:: virtual Dyninst::Architecture getArch() const
  .. cpp:function:: void ParseGlobalSymbol(PSYMBOL_INFO pSymInfo)
  .. cpp:function:: const std::vector<Offset> &getPossibleMains() const
  .. cpp:function:: void getModuleLanguageInfo(dyn_hash_map<std::string, supportedLanguages> *mod_langs)
  .. cpp:function:: bool emitDriver(std::string fName, std::set<Symbol*> &allSymbols, unsigned flag)
  .. cpp:function:: unsigned int getSecAlign() const
  .. cpp:function:: void insertPrereqLibrary(std::string lib)
  .. cpp:function:: virtual char *mem_image() const
  .. cpp:function:: void setTrapHeader(Offset ptr)
  .. cpp:function:: Offset trapHeader()
  .. cpp:function:: DWORD ImageOffset2SectionNum(DWORD dwRO)
  .. cpp:function:: PIMAGE_SECTION_HEADER ImageOffset2Section(DWORD dwRO)
  .. cpp:function:: PIMAGE_SECTION_HEADER ImageRVA2Section(DWORD dwRVA)
  .. cpp:function:: DWORD RVA2Offset(DWORD dwRVA)
  .. cpp:function:: DWORD Offset2RVA(DWORD dwRO)
  .. cpp:function:: void addReference(Offset, std::string, std::string)
  .. cpp:function:: std::map<std::string, std::map<Offset, std::string> > & getRefs()
  .. cpp:function:: std::vector<std::pair<std::string, IMAGE_IMPORT_DESCRIPTOR> > & getImportDescriptorTable()
  .. cpp:function:: std::map<std::string, std::map<std::string, WORD> > & getHintNameTable()
  .. cpp:function:: PIMAGE_NT_HEADERS getPEHdr()
  .. cpp:function:: void setTOCoffset(Offset)
  .. cpp:function:: void rebase(Offset off)

      Adjusts the data in all the sections to reflect what the loader will do if the binary is loaded
      at actualBaseAddress.

  .. cpp:function:: Region* findRegionByName(const std::string& name) const
  .. cpp:function:: void applyRelocs(Region* relocs, Offset delta)
  .. cpp:function:: virtual void getSegmentsSymReader(std::vector<SymSegment> &)

  .. cpp:member:: private Offset baseAddr

     location of this object in mutatee address space

  .. cpp:member:: private Offset preferedBase

     Virtual address at which the binary is prefered to be loaded

  .. cpp:member:: private Offset imageBase

     Virtual Address at which the binary is loaded in its address space

  .. cpp:member:: private PIMAGE_NT_HEADERS peHdr

     PE file headers

  .. cpp:member:: private Offset trapHeaderPtr_

     address & size

  .. cpp:member:: private unsigned int SecAlignment

    Section Alignment

  ......

  Structure of import table

  .. cpp:member:: private std::vector<std::pair<std::string, IMAGE_IMPORT_DESCRIPTOR> > idt_
  .. cpp:member:: private std::map<std::string, std::map<std::string, WORD> > hnt_

  ......

  .. cpp:member:: private std::map<std::string,std::map<Offset, std::string> > ref

      external reference info

  .. cpp:member:: private unsigned int textSectionId

     id of .text segment (section)

  .. cpp:member:: private unsigned int dataSectionId

     id of .data segment (section)

  .. cpp:member:: private HANDLE hProc

     Process Handle

  .. cpp:member:: private std::vector<Offset> possible_mains

    Addresses of functions that may be main


.. cpp:class:: Object::intSymbol

  .. cpp:function:: intSymbol(std::string _name, DWORD64 _addr, DWORD _type, DWORD _linkage, DWORD _size, Region *_region)
  .. cpp:function:: std::string GetName(void) const
  .. cpp:function:: DWORD64 GetAddr(void) const
  .. cpp:function:: DWORD GetSize(void) const
  .. cpp:function:: DWORD GetType(void) const
  .. cpp:function:: DWORD GetLinkage(void) const
  .. cpp:function:: Region *GetRegion(void) const
  .. cpp:function:: void SetSize(DWORD cb)
  .. cpp:function:: void DefineSymbol(dyn_hash_map<std::string, std::vector< Symbol *> >& syms, std::map<Symbol *, std::string> &symsToMods, const std::string& modName) const


.. cpp:class:: Object::File

  .. cpp:function:: File(std::string _name = "")
  .. cpp:function:: void AddSymbol(intSymbol* pSym)
  .. cpp:function:: void DefineSymbols(dyn_hash_map<std::string, std::vector< Symbol *> >& syms, std::map<Symbol *, std::string> &symsToMods, const std::string& modName) const
  .. cpp:function:: std::string GetName(void) const
  .. cpp:function:: const std::vector<intSymbol*>& GetSymbols(void) const


.. cpp:class:: Object::Module

  .. cpp:function:: Module(std::string name, DWORD64 baseAddr, DWORD64 extent = 0)
  .. cpp:function:: File* GetDefaultFile(void)
  .. cpp:function:: File* FindFile(std::string name)
  .. cpp:function:: void AddFile(File* pFile)
  .. cpp:function:: void DefineSymbols(const Object* obj, dyn_hash_map<std::string, std::vector< Symbol *> > & syms, std::map<Symbol *, std::string> &symsToMods) const
  .. cpp:function:: void BuildSymbolMap(const Object* obj) const
  .. cpp:function:: std::string GetName(void) const
  .. cpp:function:: bool IsDll(void) const
  .. cpp:function:: void SetIsDll(bool v)


.. code:: c

  // In recent versions of the Platform SDK, the macros naming
  // the value for the Flags field of the SYMBOL_INFO struct have
  // names with a SYMFLAG_ prefix.  Older Platform SDKs, including
  // the version that shipped with the Visual Studio .NET product
  // (i.e., VC7), use names for these macros with a SYMF_ prefix.
  // If we find we are using these older headers, we define the
  // new-style names.

  #if !defined(SYMFLAG_FUNCTION)
  #  define SYMFLAG_FUNCTION      SYMF_FUNCTION
  #  define SYMFLAG_LOCAL         SYMF_LOCAL
  #  define SYMFLAG_PARAMETER     SYMF_PARAMETER
  #  define SYMFLAG_EXPORT        SYMF_EXPORT
  #endif

