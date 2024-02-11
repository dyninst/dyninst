.. _`sec:parse-cfg.h`:

parse-cfg.h
###########

.. cpp:class:: parse_block : public codeRange, public Dyninst::ParseAPI::Block

  .. cpp:function:: private parse_block(Dyninst::ParseAPI::CodeObject *, Dyninst::ParseAPI::CodeRegion*, Dyninst::Address)
  .. cpp:function:: parse_block(parse_func*,Dyninst::ParseAPI::CodeRegion*,Dyninst::Address)
  .. cpp:function:: ~parse_block()
  .. cpp:function:: Dyninst::Address firstInsnOffset() const

    just pass through to Block

  .. cpp:function:: Dyninst::Address lastInsnOffset() const
  .. cpp:function:: Dyninst::Address endOffset() const
  .. cpp:function:: Dyninst::Address getSize() const
  .. cpp:function:: bool isShared() const

    cfg access & various predicates

  .. cpp:function:: bool isExitBlock()
  .. cpp:function:: bool isCallBlock()
  .. cpp:function:: bool isIndirectTailCallBlock()
  .. cpp:function:: bool isEntryBlock(parse_func * f) const
  .. cpp:function:: parse_func *getEntryFunc() const

    func starting with this bock

  .. cpp:function:: bool unresolvedCF() const
  .. cpp:function:: bool abruptEnd() const
  .. cpp:function:: void setUnresolvedCF(bool newVal)
  .. cpp:function:: void setAbruptEnd(bool newVal)
  .. cpp:function:: int id() const

    misc utility

  .. cpp:function:: void debugPrint()
  .. cpp:function:: image *img()
  .. cpp:function:: parse_func *getCallee()

    Find callees

  .. cpp:function:: std::pair<bool, Dyninst::Address> callTarget()

    Returns the address of our callee (if we're a call block, of course)

  .. cpp:function:: bool needsRelocation() const

    instrumentation-related

  .. cpp:function:: void markAsNeedingRelocation()
  .. cpp:function:: void *getPtrToInstruction(Dyninst::Address addr) const

    codeRange implementation

  .. cpp:function:: Dyninst::Address get_address() const
  .. cpp:function:: unsigned get_size() const
  .. cpp:type:: std::set<parse_block *, parse_block::compare> blockSet
  .. cpp:function:: const bitArray &getLivenessIn(parse_func * context)
  .. cpp:function:: const bitArray getLivenessOut(parse_func * context)

    This is copied from the union of all successor blocks

  .. cpp:type:: std::map<Offset, Dyninst::InstructionAPI::Instruction> Insns
  .. cpp:function:: void getInsns(Insns &instances, Dyninst::Address offset = 0)

    The provided parameter is a magic offset to add to each instruction's address we do this to avoid a copy when getting Insns from block_instances

  .. cpp:type:: private Block::getInsns
  .. cpp:member:: private bool needsRelocation_
  .. cpp:member:: private int blockNumber_
  .. cpp:member:: private bool unresolvedCF_
  .. cpp:member:: private bool abruptEnd_


.. cpp:struct parse_block::compare

  .. cpp:function:: bool operator()(parse_block * const &b1, parse_block * const &b2) const


.. cpp:class:: image_edge : public Dyninst::ParseAPI::Edge

  .. cpp:function:: image_edge(parse_block *source, parse_block *target, Dyninst::ParseAPI::EdgeTypeEnum type)
  .. cpp:function:: virtual parse_block *src() const
  .. cpp:function:: virtual parse_block *trg() const
  .. cpp:function:: const char *getTypeString()


.. cpp:class:: parse_func_registers

  .. cpp:member:: std::set<Dyninst::Register> generalPurposeRegisters
  .. cpp:member:: std::set<Dyninst::Register> floatingPointRegisters
  .. cpp:member:: std::set<Dyninst::Register> specialPurposeRegisters


.. cpp:class:: parse_func : public Dyninst::ParseAPI::Function

  .. cpp:function:: parse_func()

    Annotatable requires a default constructor

  .. cpp:function:: parse_func(Dyninst::SymtabAPI::Function *func, pdmodule *m, image *i, Dyninst::ParseAPI::CodeObject *obj, Dyninst::ParseAPI::CodeRegion *reg, InstructionSource *isrc, FuncSource src)
  .. cpp:function:: ~parse_func()
  .. cpp:function:: Dyninst::SymtabAPI::Function *getSymtabFunction() const
  .. cpp:function:: string symTabName() const
  .. cpp:function:: string prettyName() const
  .. cpp:function:: string typedName() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter symtab_names_begin() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter symtab_names_end() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter pretty_names_begin() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter pretty_names_end() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter typed_names_begin() const
  .. cpp:function:: Dyninst::SymtabAPI::Aggregate::name_iter typed_names_end() const
  .. cpp:function:: void copyNames(parse_func *duplicate)

  ......

  .. rubric::
    Return true if the name is new (and therefore added)

  .. cpp:function:: bool addSymTabName(std::string name, bool isPrimary = false)
  .. cpp:function:: bool addPrettyName(std::string name, bool isPrimary = false)
  .. cpp:function:: bool addTypedName(std::string name, bool isPrimary = false)

  .......

  .. rubric::
    Location queries
    
  .. cpp:function:: Dyninst::Address getOffset() const
  .. cpp:function:: Dyninst::Address getPtrOffset() const
  .. cpp:function:: unsigned getSymTabSize() const
  .. cpp:function:: Dyninst::Address getEndOffset()

    May trigger parsing

  .......

  .. cpp:function:: void *getPtrToInstruction(Dyninst::Address addr) const
  .. cpp:function:: pdmodule *pdmod() const
  .. cpp:function:: image *img() const
  .. cpp:function:: void changeModule(pdmodule *mod)

  ......

  .. rubric::
    CFG and other function body methods
  
  .. cpp:function:: bool makesNoCalls()
  .. cpp:function:: bool parse()

    Initiate parsing on this function

  .. cpp:function:: const std::vector<image_parRegion *> &parRegions()
  .. cpp:function:: bool isInstrumentable()
  .. cpp:function:: bool hasUnresolvedCF()

  ......

  .. rubric::
    Mutable function code, used for hybrid analysis

  .. cpp:function:: void getReachableBlocks(const std::set<parse_block *> &exceptBlocks, const std::list<parse_block *> &seedBlocks, std::set<parse_block *> &reachableBlocks)
  .. cpp:function:: Dyninst::ParseAPI::FuncReturnStatus init_retstatus() const

    Only call on defensive binaries

  .. cpp:function:: void setinit_retstatus(Dyninst::ParseAPI::FuncReturnStatus rs)

    also sets retstatus

  .. cpp:function:: bool hasWeirdInsns()

    true if we stopped the parse at a weird instruction (e.g., arpl)

  .. cpp:function:: void setHasWeirdInsns(bool wi)
  .. cpp:function:: void setPrevBlocksUnresolvedCF(size_t newVal)
  .. cpp:function:: size_t getPrevBlocksUnresolvedCF() const
  .. cpp:function:: bool isTrueCallInsn(const instruction insn)
  .. cpp:function:: bool savesReturnAddr() const
  .. cpp:function:: bool containsSharedBlocks() const
  .. cpp:function:: parse_block *entryBlock()

  ......

  .. rubric::
    OpenMP Parsing Functions

  .. cpp:function:: std::string calcParentFunc(const parse_func *imf, std::vector<image_parRegion *> &pR)
  .. cpp:function:: void parseOMP(image_parRegion *parReg, parse_func *parentFunc, int &currentSectionNum)
  .. cpp:function:: void parseOMPSectFunc(parse_func *parentFunc)
  .. cpp:function:: void parseOMPFunc(bool hasLoop)
  .. cpp:function:: bool parseOMPParent(image_parRegion *iPar, int desiredNum, int &currentSectionNum)
  .. cpp:function:: void addRegion(image_parRegion *iPar)
  .. cpp:function:: bool OMPparsed()

  ......

  .. cpp:function:: bool isPLTFunction()
  .. cpp:function:: std::set<Dyninst::Register> *usedGPRs()
  .. cpp:function:: std::set<Dyninst::Register> *usedFPRs()
  .. cpp:function:: bool isLeafFunc()
  .. cpp:function:: bool writesFPRs(unsigned level = 0)
  .. cpp:function:: bool writesSPRs(unsigned level = 0)
  .. cpp:function:: void invalidateLiveness()
  .. cpp:function:: void calcBlockLevelLiveness()
  .. cpp:function:: const Dyninst::SymtabAPI::Function *func() const
  .. cpp:function:: bool containsPowerPreamble()
  .. cpp:function:: void setContainsPowerPreamble(bool c)
  .. cpp:function:: parse_func *getNoPowerPreambleFunc()
  .. cpp:function:: void setNoPowerPreambleFunc(parse_func *f)
  .. cpp:function:: Dyninst::Address getPowerTOCBaseAddress()
  .. cpp:function:: void setPowerTOCBaseAddress(Dyninst::Address addr)
  .. cpp:function:: private void calcUsedRegs()

    Does one time calculation of registers used in a function, if called again it just refers to the stored
    values and returns that

  .. cpp:member:: private Dyninst::SymtabAPI::Function *func_{nullptr}

      pointer to the underlying symtab Function

  .. cpp:member:: private pdmodule *mod_{nullptr}

      pointer to file that defines func.

  .. cpp:member:: private image *image_{nullptr}
  .. cpp:member:: private bool OMPparsed_{false}

      Set true in parseOMPFunc

  ......

  .. rubric::
    Variables for liveness Analysis
    
  .. cpp:member:: private parse_func_registers *usedRegisters{nullptr}
  .. cpp:member:: private regUseState containsFPRWrites_{unknown}

      floating point registers

  .. cpp:member:: private regUseState containsSPRWrites_{unknown}

      stack pointer registers

  ......

  .. rubric::
    CFG and function body

  .. cpp:member:: private bool containsSharedBlocks_{false}

    True if one or more blocks in this function are shared with another function.

  ......

  .. rubric::
    OpenMP (and other parallel language) support

  .. cpp:member:: private std::vector<image_parRegion *> parRegionsList

    vector of all parallel regions within function

  .. cpp:function:: private void addParRegion(Dyninst::Address begin, Dyninst::Address end, parRegType t)

  ......

  .. cpp:member:: private bool hasWeirdInsns_

    true if we stopped the parse at a weird instruction(e.g., arpl)

  .. cpp:member:: private size_t prevBlocksUnresolvedCF_{}

    num func blocks when calculated

  .. cpp:function:: private bool isInstrumentableByFunctionName()

    Some functions are known to be unparesable by name

  .. cpp:member:: private UnresolvedCF unresolvedCF_{UNSET_CF}
  .. cpp:member:: private Dyninst::ParseAPI::FuncReturnStatus init_retstatus_{Dyninst::ParseAPI::FuncReturnStatus::UNSET}

  ......

  .. rubric::
    Architecture specific data

  .. cpp:member:: private bool o7_live{false}
  .. cpp:member:: private bool saves_return_addr_{false}
  .. cpp:member:: private bool livenessCalculated_{false}
  .. cpp:member:: private bool isPLTFunction_{false}
  .. cpp:member:: private bool containsPowerPreamble_{false}
  .. cpp:member:: private parse_func *noPowerPreambleFunc_{nullptr}

    If the function contains the power preamble, this field points the corresponding function that does not
    contain the preamble

  .. cpp:member:: private Dyninst::Address baseTOC_{}


.. cpp:enum:: parse_func::UnresolvedCF 

  .. cpp:enumerator:: UNSET_CF
  .. cpp:enumerator:: HAS_UNRESOLVED_CF
  .. cpp:enumerator:: NO_UNRESOLVED_CF


.. cpp:enum:: parse_func::regUseState 

  .. cpp:enumerator:: unknown
  .. cpp:enumerator:: used
  .. cpp:enumerator:: unused
