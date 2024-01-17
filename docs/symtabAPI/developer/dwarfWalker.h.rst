dwarfWalker.h
#############

.. cpp:namespace:: Dyninst::SymtabAPI

.. note:: Throughout, ``is_sup`` indicates it's in the dwarf supplemental file.

.. cpp:class:: DwarfWalker : public DwarfParseActions

  .. cpp:type:: ParsedFuncs = Dyninst::dyn_c_hash_map<FunctionBase *, bool>

  .. cpp:function:: protected virtual void setFuncReturnType()
  .. cpp:function:: protected virtual void createLocalVariable(const std::vector<VariableLocation> &locs, boost::shared_ptr<Type> type, Dwarf_Word variableLineNo, const std::string &fileName)
  .. cpp:function:: protected virtual bool createInlineFunc()
  .. cpp:function:: protected virtual void setFuncFromLowest(Address lowest)
  .. cpp:function:: protected virtual void createParameter(const std::vector<VariableLocation> &locs, boost::shared_ptr<Type> paramType, Dwarf_Word lineNo, const std::string &fileName)
  .. cpp:function:: protected virtual void setRanges(FunctionBase *func)
  .. cpp:function:: protected virtual void createGlobalVariable(const std::vector<VariableLocation> &locs, boost::shared_ptr<Type> type)
  .. cpp:function:: protected virtual bool addStaticClassVariable(const std::vector<VariableLocation> &locs, boost::shared_ptr<Type> type)
  .. cpp:function:: protected virtual boost::shared_ptr<Type> getCommonBlockType(std::string &commonBlockName)
  .. cpp:function:: protected virtual Symbol *findSymbolForCommonBlock(const std::string &commonBlockName)
  .. cpp:function:: DwarfWalker(Symtab *symtab, Dwarf* dbg, std::shared_ptr<ParsedFuncs> pf = nullptr)
  .. cpp:function:: virtual ~DwarfWalker()
  .. cpp:function:: bool parse()

  .. cpp:function:: bool parseModule(Dwarf_Die is_info, Module *&fixUnknownMod)

  .. cpp:function:: bool parse_int(Dwarf_Die entry, bool parseSiblings, bool dissociate_context=false)

      Non-recursive version of parse. A Context must be provided as an _input_ to this function,
      whereas parse creates a context.

  .. cpp:function:: std::string &curName()

      This is a handy scratch space that is cleared for each parse.

  .. cpp:function:: bool isMangledName()

      This is a handy scratch space that is cleared for each parse.

  .. cpp:function:: void setMangledName(bool b)

      This is a handy scratch space that is cleared for each parse.

  .. cpp:function:: bool nameDefined()

      This is a handy scratch space that is cleared for each parse.


  .. cpp:function:: static bool buildSrcFiles(Dwarf* dbg, Dwarf_Die entry, StringTablePtr strings)
  .. cpp:function:: static bool findConstant(Dwarf_Half attr, Address &value, Dwarf_Die *entry, Dwarf *dbg)
  .. cpp:function:: static bool findConstantWithForm(Dwarf_Attribute &attr, Dwarf_Half form, Address &value)
  .. cpp:function:: static std::vector<AddressRange> getDieRanges(Dwarf_Die die)

  .. cpp:member:: private Dwarf_Off compile_offset

      For debugging purposes; to match dwarfdump's output, we need to subtract a "header overall offset".

  .. cpp:type:: private dyn_c_hash_map<type_key, typeId_t> type_map

  .. cpp:member:: private type_map info_type_ids_

      .debug_info offset -> id

      Type IDs are just int, but Dwarf_Off is 64-bit and may be relative to
      either .debug_info or .debug_types.

  .. cpp:member:: private type_map types_type_ids_

      .debug_types offset -> id

      Type IDs are just int, but Dwarf_Off is 64-bit and may be relative to
      either .debug_info or .debug_types.

  .. cpp:member:: private dyn_c_hash_map<uint64_t, typeId_t> sig8_type_ids_

      Map to connect DW_FORM_ref_sig8 to type IDs.


.. cpp:struct:: Dwarf_Sig8

  .. cpp:member:: char signature[8]


.. cpp:enum:: DwarfWalker::Error

  .. cpp:enumerator:: NoError


.. cpp:enum:: inline_t

  .. cpp:enumerator:: NormalFunc
  .. cpp:enumerator:: InlinedFunc


.. cpp:struct:: type_key

  .. cpp:member:: Dwarf_Off off
  .. cpp:member:: bool file
  .. cpp:member:: Module * m


.. cpp:function:: inline bool operator==(type_key const& k1, type_key const& k2)

.. cpp:namespace-push:: concurrent

.. cpp:class:: template<> hasher<SymtabAPI::type_key>

  .. cpp:function:: size_t operator()(const SymtabAPI::type_key& k) const

.. cpp:namespace-pop::


.. cpp:class:: DwarfParseActions

  .. cpp:member:: protected Symtab *symtab_
  .. cpp:member:: protected FunctionBase *currentSubprogramFunction = nullptr

  .. cpp:function:: protected Dwarf* dbg()
  .. cpp:function:: protected Module *& mod()
  .. cpp:function:: protected typeCollection *tc()
  .. cpp:function:: protected Symtab* symtab() const
  .. cpp:function:: protected virtual Object * obj() const

  .. cpp:type:: std::vector<std::pair<Address, Address>> range_set_t
  .. cpp:type:: boost::shared_ptr<std::vector<std::pair<Address, Address>>> range_set_ptr

  .. cpp:function:: DwarfParseActions(Symtab* s, Dwarf* d)
  .. cpp:function:: DwarfParseActions(const DwarfParseActions& o)
  .. cpp:function:: virtual ~DwarfParseActions() = default
  .. cpp:function:: void push(bool dissociate_context=false)
  .. cpp:function:: void pop()
  .. cpp:function:: int stack_size() const
  .. cpp:function:: FunctionBase *curFunc()
  .. cpp:function:: virtual std::vector<VariableLocation>& getFramePtrRefForInit()
  .. cpp:function:: virtual void addMangledFuncName(std::string)
  .. cpp:function:: virtual void addPrettyFuncName(std::string)
  .. cpp:function:: boost::shared_ptr<Type> curCommon()
  .. cpp:function:: boost::shared_ptr<Type> curEnum()
  .. cpp:function:: boost::shared_ptr<Type> curEnclosure()
  .. cpp:function:: bool parseSibling()
  .. cpp:function:: bool parseChild()
  .. cpp:function:: Dwarf_Die entry()
  .. cpp:function:: Dwarf_Die specEntry()
  .. cpp:function:: Dwarf_Die abstractEntry()
  .. cpp:function:: Dwarf_Off offset()
  .. cpp:function:: unsigned int tag()
  .. cpp:function:: Address base()
  .. cpp:function:: range_set_ptr ranges()
  .. cpp:function:: void setFunc(FunctionBase *f)
  .. cpp:function:: void setCommon(boost::shared_ptr<Type> tc)
  .. cpp:function:: void setEnum(boost::shared_ptr<Type> e)
  .. cpp:function:: void setEnclosure(boost::shared_ptr<Type> f)
  .. cpp:function:: void setParseSibling(bool p)
  .. cpp:function:: void setParseChild(bool p)
  .. cpp:function:: virtual void setEntry(Dwarf_Die e)
  .. cpp:function:: void setSpecEntry(Dwarf_Die e)
  .. cpp:function:: void setAbstractEntry(Dwarf_Die e)
  .. cpp:function:: void setTag(unsigned int t)
  .. cpp:function:: void setBase(Address a)
  .. cpp:function:: virtual void setRange(const AddressRange& range)
  .. cpp:function:: void clearRanges()
  .. cpp:function:: void clearFunc()
  .. cpp:function:: virtual std::string filename() const
  .. cpp:function:: virtual Dyninst::Architecture getArchitecture() const
  .. cpp:function:: virtual Offset convertDebugOffset(Offset from)
  .. cpp:function:: virtual void setFuncReturnType()
  .. cpp:function:: virtual Symbol* findSymbolByName(std::string name, Symbol::SymbolType type)


.. cpp:struct:: ContextGuard

  .. cpp:member:: DwarfParseActions& walker
  .. cpp:function:: ContextGuard(DwarfParseActions& w, bool dissociate_context)

