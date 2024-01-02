.. _`sec:Parser.h`:

Parser.h
########

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:type:: Dyninst::InsnAdapter::IA_IAPI InstructionAdapter_t

.. cpp:class:: Parser

  .. cpp:function:: Parser(CodeObject& obj, CFGFactory& fact, ParseCallbackManager& pcb)
  .. cpp:function:: void add_hint(Function* f)
  .. cpp:function:: Function* findFuncByEntry(CodeRegion* cr, Address entry)
  .. cpp:function:: int findFuncsByBlock(CodeRegion* cr, Block* b, set<Function*>& funcs)
  .. cpp:function:: int findFuncs(CodeRegion* cr, Address addr, set<Function*>& funcs)
  .. cpp:function:: int findFuncs(CodeRegion* cr, Address start, Address end, set<Function*>& funcs)
  .. cpp:function:: Block* findBlockByEntry(CodeRegion* cr, Address entry)
  .. cpp:function:: int findBlocks(CodeRegion* cr, Address addr, set<Block*>& blocks)
  .. cpp:function:: int findCurrentBlocks(CodeRegion* cr, Address addr, std::set<Block*>& blocks)

      Returns current blocks without parsing.

  .. cpp:function:: int findCurrentFuncs(CodeRegion* cr, Address addr, set<Function*>& funcs)
  .. cpp:function:: Block* findNextBlock(CodeRegion* cr, Address addr)
  .. cpp:function:: void parse()
  .. cpp:function:: void parse_at(CodeRegion* cr, Address addr, bool recursive, FuncSource src)
  .. cpp:function:: void parse_at(Address addr, bool recursive, FuncSource src)
  .. cpp:function:: void parse_edges(vector<ParseWorkElem*>& work_elems)
  .. cpp:function:: CFGFactory& factory() const
  .. cpp:function:: CodeObject& obj()
  .. cpp:function:: void remove_block(Block* )
  .. cpp:function:: void remove_func(Function* )
  .. cpp:function:: void move_func(Function* , Address new_entry, CodeRegion* new_reg)
  .. cpp:function:: Block* record_block(Block *b)
  .. cpp:function:: void record_func(Function *f)
  .. cpp:function:: void init_frame(ParseFrame &frame)
  .. cpp:function:: bool finalize(Function *f)
  .. cpp:function:: ParseData *parse_data()
