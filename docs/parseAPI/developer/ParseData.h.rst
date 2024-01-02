.. _`sec:ParseData.h`:

ParseData.h
###########

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:class:: ParseFrame : public boost::lockable_adapter<boost::recursive_mutex>

  **Describes a saved frame during recursive parsing**

  .. cpp:type:: std::priority_queue<ParseWorkElem*, vector<ParseWorkElem*>, ParseWorkElem::compare> worklist_t

  .. cpp:member:: vector<ParseWorkBundle*> work_bundles
  .. cpp:member:: worklist_t worklist
  .. cpp:member:: std::set<Address> knownTargets

      This set contains known potential targets in this function

  .. cpp:member:: std::map<ParseWorkElem*, Function* > delayedWork

      Delayed work elements

  .. cpp:member:: std::map<Address, Block*> leadersToBlock
  .. cpp:member:: Address curAddr
  .. cpp:member:: unsigned num_insns
  .. cpp:member:: dyn_hash_map<Address, bool> visited
  .. cpp:member:: Function* call_target

      Set when status goes to CALL_BLOCKED

  .. cpp:member:: Function* func
  .. cpp:member:: CodeRegion* codereg
  .. cpp:member:: ParseWorkElem* seed

      Stored for cleanup

  .. cpp:member:: std::set<Address> value_driven_jump_tables

  .. cpp:function:: ParseFrame(Function* f,ParseData* pd)
  .. cpp:function:: ParseWorkElem* mkWork(ParseWorkBundle* b, Edge* e, Address source, Address target, bool resolvable, bool tailcall)
  .. cpp:function:: ParseWorkElem* mkWork(ParseWorkBundle* b, Block* block, const InsnAdapter::IA_IAPI* ah)
  .. cpp:function:: ParseWorkElem* mkWork(ParseWorkBundle* b, Function* shared_func)
  .. cpp:function:: void pushWork(ParseWorkElem* elem)
  .. cpp:function:: ParseWorkElem* popWork()
  .. cpp:function:: void pushDelayedWork(ParseWorkElem* elem, Function* ct)
  .. cpp:function:: void cleanup()
  .. cpp:function:: Status status() const
  .. cpp:function:: void set_status(Status)
  .. cpp:function:: void set_internal_status(Status s)

.. cpp:class:: edge_parsing_data

  .. cpp:member:: Block* b
  .. cpp:member:: Function*f;

  .. cpp:function:: edge_parsing_data(Function* ff, Block* bb)
  .. cpp:function:: edge_parsing_data()

.. cpp:enum:: ParseFrame::Status

  .. cpp:enumerator:: UNPARSED
  .. cpp:enumerator:: PROGRESS
  .. cpp:enumerator:: CALL_BLOCKED
  .. cpp:enumerator:: RETURN_SET
  .. cpp:enumerator:: PARSED
  .. cpp:enumerator:: FRAME_ERROR
  .. cpp:enumerator:: BAD_LOOKUP

    error for lookups that return Status

  .. cpp:enumerator:: FRAME_DELAYED

      discovered cyclic dependency; delaying parse

.. cpp:class:: region_data

 **per-CodeRegion parsing data**

  .. cpp:type:: dyn_c_hash_map<Address, edge_parsing_data> edge_data_map

  .. cpp:member:: Dyninst::IBSTree_fast<FuncExtent> funcsByRange

      Function lookups

  .. cpp:member:: dyn_c_hash_map<Address, Function*> funcsByAddr
  .. cpp:member:: Dyninst::IBSTree_fast<Block > blocksByRange
  .. cpp:member:: dyn_c_hash_map<Address, Block*> blocksByAddr
  .. cpp:member:: dyn_c_hash_map<Address, ParseFrame*> frame_map
  .. cpp:member:: dyn_c_hash_map<Address, ParseFrame::Status> frame_status
  .. cpp:member:: edge_data_map edge_parsing_status

      We only want one thread to create edges for a location

.. cpp:class:: ParseData

  .. cpp:function:: protected ParseData(Parser*p)

  .. cpp:member:: protected Parser* _parser;

  .. cpp:function:: virtual Function* findFunc(CodeRegion*, Address) = 0
  .. cpp:function:: virtual Block* findBlock(CodeRegion*, Address) = 0
  .. cpp:function:: virtual int findFuncs(CodeRegion*, Address, set<Function*> &) = 0
  .. cpp:function:: virtual int findFuncs(CodeRegion*, Address, Address, set<Function*> &) = 0
  .. cpp:function:: virtual int findBlocks(CodeRegion*, Address, set<Block*> &) = 0
  .. cpp:function:: virtual ParseFrame* findFrame(CodeRegion*, Address) = 0
  .. cpp:function:: virtual ParseFrame::Status frameStatus(CodeRegion*, Address addr) = 0
  .. cpp:function:: virtual void setFrameStatus(CodeRegion*,Address,ParseFrame::Status) = 0
  .. cpp:function:: virtual ParseFrame* createAndRecordFrame(Function*) = 0

      0Atomically lookup whether there is a frame for a Function object.
      If there is no frame for the Function, create a new frame and record it.
      Return NULL if a frame already exists;
      Return the pointer to the new frame if a new frame is created

  .. cpp:function:: virtual Function* createAndRecordFunc(CodeRegion*, Address, FuncSource) = 0
  .. cpp:function:: virtual region_data* findRegion(CodeRegion*) = 0
  .. cpp:function:: virtual Function* record_func(Function*) = 0
  .. cpp:function:: virtual Block* record_block(CodeRegion*, Block*) = 0
  .. cpp:function:: virtual void remove_frame(ParseFrame*) = 0
  .. cpp:function:: virtual void remove_func(Function*) = 0
  .. cpp:function:: virtual void remove_block(Block*) = 0
  .. cpp:function:: virtual void remove_extents(const std::vector<FuncExtent*> &extents) = 0
  .. cpp:function:: virtual CodeRegion*  reglookup(CodeRegion* cr, Address addr) = 0

      Does the Right Thing(TM) for standard- and overlapping-region object types

  .. cpp:function:: virtual edge_parsing_data setEdgeParsingStatus(CodeRegion* cr, Address addr, Function* f, Block* b) = 0
  .. cpp:function:: virtual void getAllRegionData(std::vector<region_data*>&) = 0
  .. cpp:function:: virtual region_data::edge_data_map* get_edge_data_map(CodeRegion*) = 0


.. cpp:class:: StandardParseData : public ParseData

  **Parse data for parsers that disallow overlapping CodeRegions**

  It has fast paths for lookup.

  .. cpp:function:: StandardParseData(Parser* p)
  .. cpp:function:: Function*  findFunc(CodeRegion* pf, Address addr)
  .. cpp:function:: Block*  findBlock(CodeRegion* pf, Address addr)
  .. cpp:function:: int findFuncs(CodeRegion*, Address, set<Function*> &)
  .. cpp:function:: int findFuncs(CodeRegion*, Address, Address, set<Function*> &)
  .. cpp:function:: int findBlocks(CodeRegion*, Address, set<Block*> &)
  .. cpp:function:: ParseFrame*  findFrame(CodeRegion*, Address)
  .. cpp:function:: ParseFrame::Status frameStatus(CodeRegion*, Address)
  .. cpp:function:: void setFrameStatus(CodeRegion*,Address,ParseFrame::Status)
  .. cpp:function:: virtual ParseFrame* createAndRecordFrame(Function*)
  .. cpp:function:: Function*  createAndRecordFunc(CodeRegion*  cr, Address addr, FuncSource src)
  .. cpp:function:: region_data*  findRegion(CodeRegion* cr)
  .. cpp:function:: Function* record_func(Function* f)
  .. cpp:function:: Block* record_block(CodeRegion* cr, Block* b)
  .. cpp:function:: void remove_frame(ParseFrame* )
  .. cpp:function:: void remove_func(Function* )
  .. cpp:function:: void remove_block(Block* )
  .. cpp:function:: void remove_extents(const std::vector<FuncExtent*> &extents)
  .. cpp:function:: CodeRegion*  reglookup(CodeRegion* cr, Address addr)
  .. cpp:function:: edge_parsing_data setEdgeParsingStatus(CodeRegion* cr, Address addr, Function* f, Block* b)
  .. cpp:function:: void getAllRegionData(std::vector<region_data*>& rds)
  .. cpp:function:: region_data::edge_data_map* get_edge_data_map(CodeRegion* cr)


.. cpp:class:: OverlappingParseData : public ParseData

  .. cpp:function:: OverlappingParseData(Parser* p, vector<CodeRegion* > & regions)
  .. cpp:function:: Function*  findFunc(CodeRegion*, Address addr)
  .. cpp:function:: Block*  findBlock(CodeRegion*, Address addr)
  .. cpp:function:: int findFuncs(CodeRegion*, Address, set<Function*> &)
  .. cpp:function:: int findFuncs(CodeRegion*, Address, Address, set<Function*> &)
  .. cpp:function:: int findBlocks(CodeRegion*, Address, set<Block*> &)
  .. cpp:function:: ParseFrame*  findFrame(CodeRegion*, Address)
  .. cpp:function:: ParseFrame::Status frameStatus(CodeRegion*, Address)
  .. cpp:function:: void setFrameStatus(CodeRegion*,Address,ParseFrame::Status)
  .. cpp:function:: virtual ParseFrame* createAndRecordFrame(Function*)
  .. cpp:function:: Function*  createAndRecordFunc(CodeRegion*  cr, Address addr, FuncSource src)
  .. cpp:function:: region_data*  findRegion(CodeRegion* cr)
  .. cpp:function:: Function* record_func(Function* f)
  .. cpp:function:: Block* record_block(CodeRegion* cr, Block* b)
  .. cpp:function:: void remove_frame(ParseFrame* )
  .. cpp:function:: void remove_func(Function* )
  .. cpp:function:: void remove_block(Block* )
  .. cpp:function:: void remove_extents(const std::vector<FuncExtent*> &extents)
  .. cpp:function:: CodeRegion*  reglookup(CodeRegion* cr, Address addr)
  .. cpp:function:: edge_parsing_data setEdgeParsingStatus(CodeRegion* cr, Address addr, Function* f, Block* b)
  .. cpp:function:: void getAllRegionData(std::vector<region_data*>&)
  .. cpp:function:: region_data::edge_data_map* get_edge_data_map(CodeRegion* cr)
