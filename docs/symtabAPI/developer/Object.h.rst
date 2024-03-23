.. _`sec:Object.h`:

Object.h
########

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:var:: const char WILDCARD_CHARACTER
.. cpp:var:: const char MULTIPLE_WILDCARD_CHARACTER

.. cpp:class:: AObject

  WHAT IS THIS CLASS????  COMMENTS????

  Looks like it has a dictionary hash of symbols, as well as a ptr to to the code section, an offset into the code section,
  and a length of the code section, and ditto for the data section....

  This class is intentionally not copyable.

  .. cpp:function:: unsigned nsymbols() const
  .. cpp:function:: bool get_symbols( std::string & name, std::vector< Symbol *> & symbols)
  .. cpp:function:: char* code_ptr() const
  .. cpp:function:: Offset code_off() const
  .. cpp:function:: Offset code_len() const
  .. cpp:function:: char* data_ptr() const
  .. cpp:function:: Offset data_off() const
  .. cpp:function:: Offset data_len() const
  .. cpp:function:: bool is_aout() const
  .. cpp:function:: bool isDynamic() const
  .. cpp:function:: unsigned no_of_sections() const
  .. cpp:function:: unsigned no_of_symbols() const
  .. cpp:function:: bool getAllExceptions(std::vector<ExceptionBlock *>&excpBlocks) const
  .. cpp:function:: std::vector<Region *> getAllRegions() const
  .. cpp:function:: Offset loader_off() const
  .. cpp:function:: unsigned loader_len() const
  .. cpp:function:: int getAddressWidth() const
  .. cpp:function:: bool isStaticBinary() const
  .. cpp:function:: virtual char * mem_image() const
  .. cpp:function:: virtual bool needs_function_binding() const
  .. cpp:function:: virtual bool get_func_binding_table(std::vector<relocationEntry> &) const
  .. cpp:function:: virtual bool get_func_binding_table_ptr(const std::vector<relocationEntry> *&) const
  .. cpp:function:: virtual bool addRelocationEntry(relocationEntry &re)
  .. cpp:function:: bool getSegments(std::vector<Segment> &segs) const

      Only implemented for ELF right now

  .. cpp:function:: bool have_deferred_parsing( void ) const
  .. cpp:function:: const std::ostream &dump_state_info(std::ostream &s)
  .. cpp:function:: void * getErrFunc() const
  .. cpp:function:: dyn_c_hash_map< std::string, std::vector< Symbol *> > *getAllSymbols()
  .. cpp:function:: virtual bool hasFrameDebugInfo()
  .. cpp:function:: virtual bool getRegValueAtFrame(Address pc, Dyninst::MachRegister reg, \
                                                    Dyninst::MachRegisterVal & reg_result, \
                                                    Dyninst::SymtabAPI::MemRegReader * reader)
  .. cpp:function:: virtual Dyninst::Architecture getArch() const
  .. cpp:function:: bool hasError() const
  .. cpp:function:: virtual bool isBigEndianDataEncoding() const
  .. cpp:function:: virtual bool getABIVersion(int & major, int & minor) const
  .. cpp:function:: virtual void setTruncateLinePaths(bool value)
  .. cpp:function:: virtual bool getTruncateLinePaths()
  .. cpp:function:: virtual Region::RegionType getRelType() const
  .. cpp:function:: virtual void getSegmentsSymReader(std::vector<SymSegment> &)
  .. cpp:function:: virtual void rebase(Offset)
  .. cpp:function:: virtual void addModule(SymtabAPI::Module *)
  .. cpp:function:: protected virtual ~AObject()
  .. cpp:function:: protected AObject(MappedFile *, void(*err_func)(const char *), Symtab*)
  .. cpp:function:: protected virtual void parseLineInfoForCU(Offset , LineInformation* )

  .. cpp:member:: protected MappedFile *mf
  .. cpp:member:: protected std::vector< Region *> regions_
  .. cpp:member:: protected dyn_c_hash_map< std::string, std::vector<Symbol*>> symbols_

      The owner of Symbol pointers; memory is reclaimed from this structure

  .. cpp:member:: protected dyn_hash_map< std::string, std::vector< Symbol *> > symbols_tmp_
  .. cpp:member:: protected dyn_c_hash_map<Offset, std::vector<Symbol *> > symsByOffset_
  .. cpp:member:: protected std::vector<std::pair<std::string, Offset> > modules_
  .. cpp:member:: protected char* code_ptr_
  .. cpp:member:: protected Offset code_off_
  .. cpp:member:: protected Offset code_len_
  .. cpp:member:: protected char* data_ptr_
  .. cpp:member:: protected Offset data_off_
  .. cpp:member:: protected Offset data_len_
  .. cpp:member:: protected Offset code_vldS_
  .. cpp:member:: protected Offset code_vldE_
  .. cpp:member:: protected Offset data_vldS_
  .. cpp:member:: protected Offset data_vldE_
  .. cpp:member:: protected Offset loader_off_
  .. cpp:member:: protected Offset loader_len_
  .. cpp:member:: protected bool is_aout_
  .. cpp:member:: protected bool is_dynamic_
  .. cpp:member:: protected bool has_error
  .. cpp:member:: protected bool is_static_binary_
  .. cpp:member:: protected unsigned no_of_sections_
  .. cpp:member:: protected unsigned no_of_symbols_
  .. cpp:member:: protected bool deferredParse
  .. cpp:member:: protected bool parsedAllLineInfo
  .. cpp:member:: protected int addressWidth_nbytes
  .. cpp:member:: protected std::vector<ExceptionBlock> catch_addrs_

      Addresses of C++ try/catch blocks

  .. cpp:member:: protected Symtab* associated_symtab

  .. cpp:type:: protected void(*err_func_)(const char*)


.. cpp:class:: SymbolIter

  .. cpp:function:: SymbolIter(Object& obj)
  .. cpp:function:: SymbolIter(const SymbolIter& src)
  .. cpp:function:: ~SymbolIter()
  .. cpp:function:: void reset()
  .. cpp:function:: operator bool() const
  .. cpp:function:: void operator++(int)
  .. cpp:function:: const std::string & currkey() const
  .. cpp:function:: Symbol *currval()

      If it's important that this be const, we could try to initialize
      currentVector to '& symbolIterator.currval()' in the constructor.

