.. _`sec:Object-elf.h`:

Object-elf.h
############


.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: open_statement

  .. cpp:function:: open_statement()
  .. cpp:function:: open_statement(const open_statement&) = default
  .. cpp:function:: Dwarf_Addr noAddress()
  .. cpp:function:: bool uninitialized()
  .. cpp:function:: void reset()
  .. cpp:function:: bool sameFileLineColumn(const open_statement &rhs)
  .. cpp:function:: void operator=(const open_statement &rhs)
  .. cpp:function:: std::ostream& operator<<(std::ostream& os, const open_statement& st)
  .. cpp:function:: void dump(std::ostream& os, bool addrRange) const
  .. cpp:member:: Dwarf_Word string_table_index
  .. cpp:member:: Dwarf_Addr start_addr
  .. cpp:member:: Dwarf_Addr end_addr
  .. cpp:member:: int line_number
  .. cpp:member:: int column_number
  .. cpp:member:: Dwarf_Line* context
  .. cpp:member:: const char* funcname


.. cpp:class:: Object : public AObject

  .. cpp:member:: static bool truncateLineFilenames
  .. cpp:member:: Dyninst::DwarfDyninst::DwarfHandle::ptr dwarf
  .. cpp:member:: std::set<std::string> prereq_libs
  .. cpp:member:: std::vector<std::pair<long, long> > new_dynamic_entries

  .. cpp:function:: Object(MappedFile *, bool, void(*)(const char *) = log_msg, bool alloc_syms = true, Symtab* st = NULL)
  .. cpp:function:: virtual ~Object()
  .. cpp:function:: bool emitDriver(std::string fName, std::set<Symbol *> &allSymbols, unsigned flag)
  .. cpp:function:: bool hasDwarfInfo() const
  .. cpp:function:: void getModuleLanguageInfo(dyn_hash_map<std::string, supportedLanguages> *mod_langs)
  .. cpp:function:: void parseFileLineInfo()
  .. cpp:function:: void parseTypeInfo()
  .. cpp:function:: void addModule(SymtabAPI::Module* m) override
  .. cpp:function:: bool needs_function_binding() const override
  .. cpp:function:: bool get_func_binding_table(std::vector<relocationEntry> &fbt) const override
  .. cpp:function:: bool get_func_binding_table_ptr(const std::vector<relocationEntry> *&fbt) const override
  .. cpp:function:: void getDependencies(std::vector<std::string> &deps)
  .. cpp:function:: std::vector<std::string> &libsRMd()
  .. cpp:function:: bool addRelocationEntry(relocationEntry &re) override
  .. cpp:function:: Offset getLoadAddress() const

        May return 0 on shared objects.

  .. cpp:function:: Offset getEntryAddress() const
  .. cpp:function:: Offset getBaseAddress() const
  .. cpp:function:: void insertPrereqLibrary(std::string libname)
  .. cpp:function:: bool removePrereqLibrary(std::string libname)
  .. cpp:function:: void insertDynamicEntry(long name, long value)
  .. cpp:function:: virtual char *mem_image() const override
  .. cpp:function:: ObjectType objType() const
  .. cpp:function:: const char *interpreter_name() const
  .. cpp:function:: Offset getTOCoffset(Offset off) const

      On most platforms, the TOC offset doesn't exist and is thus null.
      On PPC64, it varies **by function** and is used to point into the GOT,
      a big data table. We can look it up by parsing the OPD, a function
      descriptor table.

  .. cpp:function:: void setTOCoffset(Offset off)

      This is an override for the whole thing; we could do per-function but
      we're missing a _lot_ of hardware for that.

  .. cpp:function:: const std::ostream &dump_state_info(std::ostream &s)
  .. cpp:function:: bool isEEL()
  .. cpp:function:: bool isinText(Offset addr, Offset baseaddr) const

      To determine if a mutation falls in the text section of a shared library.

  .. cpp:function:: Offset getPltSlot(std::string funcName) const

      To determine where in the .plt this function is listed
      returns an offset from the base address of the object
      so the entry can easily be located in memory.

  .. cpp:function:: bool isText( Offset addr ) const
  .. cpp:function:: Dyninst::Architecture getArch() const override
  .. cpp:function:: bool isBigEndianDataEncoding() const override
  .. cpp:function:: bool getABIVersion(int &major, int &minor) const override
  .. cpp:function:: bool is_offset_in_plt(Offset offset) const
  .. cpp:function:: Elf_X_Shdr *getRegionHdrByAddr(Offset addr)
  .. cpp:function:: int getRegionHdrIndexByAddr(Offset addr)
  .. cpp:function:: Elf_X_Shdr *getRegionHdrByIndex(unsigned index)
  .. cpp:function:: bool isRegionPresent(Offset segmentStart, Offset segmentSize, unsigned newPerms)
  .. cpp:function:: bool getRegValueAtFrame(Address pc, Dyninst::MachRegister reg, Dyninst::MachRegisterVal &reg_result, MemRegReader *reader) override
  .. cpp:function:: bool hasFrameDebugInfo() override
  .. cpp:function:: bool convertDebugOffset(Offset off, Offset &new_off)
  .. cpp:function:: std::vector< std::vector<Offset> > getMoveSecAddrRange() const
  .. cpp:function:: dyn_hash_map<int, Region*> getTagRegionMapping() const
  .. cpp:function:: bool hasReldyn() const
  .. cpp:function:: bool hasReladyn() const
  .. cpp:function:: bool hasRelplt() const
  .. cpp:function:: bool hasRelaplt() const
  .. cpp:function:: bool hasNoteSection() const
  .. cpp:function:: Region::RegionType getRelType() const override
  .. cpp:function:: Offset getTextAddr() const
  .. cpp:function:: Offset getSymtabAddr() const
  .. cpp:function:: Offset getStrtabAddr() const
  .. cpp:function:: Offset getDynamicAddr() const
  .. cpp:function:: Offset getDynsymSize() const
  .. cpp:function:: Offset getElfHashAddr() const
  .. cpp:function:: Offset getGnuHashAddr() const
  .. cpp:function:: Offset getRelPLTAddr() const
  .. cpp:function:: Offset getRelPLTSize() const
  .. cpp:function:: Offset getRelDynAddr() const
  .. cpp:function:: Offset getRelDynSize() const
  .. cpp:function:: const char* getSoname() const
  .. cpp:function:: bool hasPieFlag() const
  .. cpp:function:: bool hasProgramLoad() const
  .. cpp:function:: bool hasDtDebug() const
  .. cpp:function:: bool hasBitsAlloc() const
  .. cpp:function:: bool hasDebugSections() const
  .. cpp:function:: bool hasModinfo() const
  .. cpp:function:: bool hasGnuLinkonceThisModule() const
  .. cpp:function:: bool isLoadable() const
  .. cpp:function:: bool isOnlyExecutable() const
  .. cpp:function:: bool isExecutable() const
  .. cpp:function:: bool isSharedLibrary() const
  .. cpp:function:: bool isOnlySharedLibrary() const
  .. cpp:function:: bool isDebugOnly() const
  .. cpp:function:: bool isLinuxKernelModule() const
  .. cpp:function:: std::vector<relocationEntry> &getPLTRelocs()
  .. cpp:function:: std::vector<relocationEntry> &getDynRelocs()
  .. cpp:function:: Offset getInitAddr() const
  .. cpp:function:: Offset getFiniAddr() const
  .. cpp:function:: virtual void setTruncateLinePaths(bool value) override
  .. cpp:function:: virtual bool getTruncateLinePaths() override
  .. cpp:function:: Elf_X * getElfHandle()
  .. cpp:function:: unsigned gotSize() const
  .. cpp:function:: Offset gotAddr() const
  .. cpp:function:: virtual void getSegmentsSymReader(std::vector<SymSegment> &segs) override
  .. cpp:function:: void parseDwarfFileLineInfo()
  .. cpp:function:: void parseLineInfoForAddr(Offset addr_to_find)
  .. cpp:function:: bool hasDebugInfo()

  .. cpp:member:: private Offset elf_hash_addr_

    .hash section

  .. cpp:member:: private Offset gnu_hash_addr_

    .gnu.hash section

  .. cpp:member:: private Offset text_addr_

    .text section

  .. cpp:member:: private Offset text_size_

    .text section size

  .. cpp:member:: private Offset dynamic_addr_

    .dynamic section

  .. cpp:member:: private Offset dynsym_addr_

     .dynsym section

  .. cpp:member:: private Offset dynstr_addr_

     .dynstr section

  .. cpp:member:: private Offset got_addr_

     global offset table

  .. cpp:member:: private unsigned got_size_

     global offset table

  .. cpp:member:: private Offset plt_addr_

     procedure linkage table

  .. cpp:member:: private unsigned plt_size_

     procedure linkage table

  .. cpp:member:: private unsigned plt_entry_size_

     procedure linkage table

  .. cpp:member:: private Offset rel_plt_addr_

     .rel[a].plt section

  .. cpp:member:: private unsigned rel_plt_size_

     .rel[a].plt section

  .. cpp:member:: private unsigned rel_plt_entry_size_

     .rel[a].plt section

  .. cpp:member:: private unsigned rel_size_

     DT_REL/DT_RELA in dynamic section

  .. cpp:member:: private unsigned rel_entry_size_

     DT_REL/DT_RELA in dynamic section

  .. cpp:member:: private bool dwarvenDebugInfo

     is DWARF debug info present?

  .. cpp:member:: private Offset loadAddress_

     The object may specify a load address. Set to 0 if it may load anywhere

  .. cpp:member:: private bool EEL

     true if EEL rewritten

  .. cpp:member:: private bool did_open

     true if the file has been mmapped

  .. cpp:member:: private std::vector<relocationEntry> relocation_table_

      for sparc-solaris this is a table of PLT entry addr, function_name
      for x86-solaris this is a table of GOT entry addr, function_name
      on sparc-solaris the runtime linker modifies the PLT entry when it
      binds a function, on X86 the PLT entry is not modified, but it uses
      an indirect jump to a GOT entry that is modified when the function
      is bound....is this correct???? or should it be <PLTentry_addr, name>
      for both?

  ......

  All section headers, sorted by address. we use these to do a better job
  of finding the end of symbols.

  .. cpp:member:: private std::vector<Elf_X_Shdr*> allRegionHdrs
  .. cpp:member:: private std::vector<Elf_X_Shdr*> allRegionHdrsByShndx

  ......

  Symbol version mappings. used to store symbol version names.

  .. cpp:member:: private dyn_hash_map<unsigned, std::vector<std::string> >versionMapping
  .. cpp:member:: private dyn_hash_map<unsigned, std::string> versionFileNameMapping

  ......

  .. cpp:function:: bool get_relocation_entries(Elf_X_Shdr *&rel_plt_scnp, Elf_X_Shdr *&dynsym_scnp, \
                                                Elf_X_Shdr *&dynstr_scnp)

      Initialize ``relocation_table_`` from ``.rel[a].plt`` section entries.

  .. cpp:function:: bool parse_all_relocations(Elf_X_Shdr *, Elf_X_Shdr *, Elf_X_Shdr *, Elf_X_Shdr *)

      Parses sections with relocations and links these relocations to existing symbols.

  .. cpp:member:: std::set<std::string> modules_parsed_for_line_info

      Line info: CUs to skip

.. cpp:struct:: Object::DbgAddrConversion_t

.. cpp:struct:: DbgAddrConversion_t

  .. cpp:function:: DbgAddrConversion_t()
  .. cpp:member:: std::string name
  .. cpp:member:: Offset dbg_offset
  .. cpp:member:: unsigned dbg_size
  .. cpp:member:: Offset orig_offset
