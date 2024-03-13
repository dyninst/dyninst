.. _`sec-dev:Parser.h`:


Parser.h
########

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:type:: Dyninst::InsnAdapter::IA_IAPI InstructionAdapter_t

.. cpp:class:: Parser

  This is the internal parser

  .. code:: cpp

    // The CFG modifier needs to manipulate the lookup structures,
    // which are internal Parser data.
    friend class CFGModifier


  .. cpp:member:: private CodeObject &_obj

    Owning code object

  .. cpp:member:: private CFGFactory &_cfgfact

    CFG object factory

  .. cpp:member:: private ParseCallbackManager &_pcb

    Callback notifications

  .. cpp:member:: private ParseData *_parse_data

    region data store

  .. cpp:member:: private LockFreeQueue<ParseFrame *> frames

    All allocated frames

  .. cpp:member:: private boost::atomic<bool> delayed_frames_changed
  .. cpp:member:: private dyn_c_hash_map<Function *, std::set<ParseFrame *>> delayed_frames

  .. cpp:member:: private dyn_c_vector<Function *> hint_funcs

    differentiate those provided via hints and those found through RT or speculative parsing

  .. cpp:member:: private dyn_c_vector<Function *> discover_funcs
  .. cpp:member:: private set<Function *, Function::less> sorted_funcs
  .. cpp:member:: private set<Function *> deleted_func

  .. cpp:member:: private dyn_hash_map<Address, string> plt_entries

    PLT, IAT entries

  .. cpp:member:: private boost::atomic<Block *> _sink

    a sink block for unbound edges

  .. cpp:member:: private dyn_c_hash_map<unsigned int, unsigned int> time_histogram

  .. cpp:member:: private ParseState _parse_state

  .. cpp:function:: Parser(CodeObject &obj, CFGFactory &fact, ParseCallbackManager &pcb)
  .. cpp:function:: ~Parser()
  .. cpp:function:: void add_hint(Function *f)
  .. cpp:function:: Function *findFuncByEntry(CodeRegion *cr, Address entry)
  .. cpp:function:: int findFuncsByBlock(CodeRegion *cr, Block *b, set<Function *> &funcs)
  .. cpp:function:: int findFuncs(CodeRegion *cr, Address addr, set<Function *> &funcs)
  .. cpp:function:: int findFuncs(CodeRegion *cr, Address start, Address end, set<Function *> &funcs)
  .. cpp:function:: Block *findBlockByEntry(CodeRegion *cr, Address entry)
  .. cpp:function:: int findBlocks(CodeRegion *cr, Address addr, set<Block *> &blocks)

  .. cpp:function:: int findCurrentBlocks(CodeRegion* cr, Address addr, std::set<Block*>& blocks)

      Returns current blocks without parsing.

  .. cpp:function:: int findCurrentFuncs(CodeRegion *cr, Address addr, set<Function *> &funcs)
  .. cpp:function:: Block *findNextBlock(CodeRegion *cr, Address addr)
  .. cpp:function:: void parse()
  .. cpp:function:: void parse_at(CodeRegion *cr, Address addr, bool recursive, FuncSource src)
  .. cpp:function:: void parse_at(Address addr, bool recursive, FuncSource src)
  .. cpp:function:: void parse_at(const std::vector<Address> &addrs, bool recursive, FuncSource src)
  .. cpp:function:: void parse_at(const std::vector<std::pair<Address, CodeRegion *>> &addrs, bool recursive, FuncSource src)
  .. cpp:function:: void parse_edges(vector<ParseWorkElem *> &work_elems)
  .. cpp:function:: CFGFactory &factory() const
  .. cpp:function:: CodeObject &obj()
  .. cpp:function:: void remove_block(Block *)
  .. cpp:function:: void remove_func(Function *)
  .. cpp:function:: void move_func(Function *, Address new_entry, CodeRegion *new_reg)
  .. cpp:function:: Block *record_block(Block *b)

  ......

  .. rubric:: Everything below here is strictly internals

  .. cpp:function:: void record_func(Function *f)
  .. cpp:function:: void init_frame(ParseFrame &frame)

  .. cpp:function:: bool finalize(Function *f)

    Finalizing all functions for consumption:

    - Finish delayed parsing
    - Prepare and record :cpp:class:`FuncExtents` for range-based lookup

    Returns ``false`` if it flips tail call determination of an edge, and
    thus needs to re-finalize the function boundary.

    Returns ``true`` if it is done and finalized the function.

  .. cpp:function:: ParseData *parse_data()
  .. cpp:function:: private void parse_vanilla()
  .. cpp:function:: private void cleanup_frames()
  .. cpp:function:: private void parse_gap_heuristic(CodeRegion *cr)
  .. cpp:function:: private bool getGapRange(CodeRegion *, Address, Address &, Address &)
  .. cpp:function:: private void probabilistic_gap_parsing(CodeRegion *cr)
  .. cpp:function:: private ParseFrame::Status frame_status(CodeRegion *cr, Address addr)
  .. cpp:function:: private void end_block(Block *b, InstructionAdapter_t *ah)

  ......

  .. rubric:: CFG structure manipulations

  .. cpp:function:: private Block *block_at(ParseFrame &frame, Function *owner, Address addr, Block *&split)

    This should only be called when we know the we want to create a block within the function.

    .. caution:: Do not call this function to create the entry block of a callee function

  .. cpp:function:: private pair<Block *, Edge *> add_edge(ParseFrame &frame, Function *owner, Block *src, Address src_addr, Address dst, EdgeTypeEnum et, Edge *exist)
  .. cpp:function:: private Block *follow_fallthrough(Block *b, Address addr)
  .. cpp:function:: private Edge *link_addr(Address src, Block *dst, EdgeTypeEnum et, bool sink, Function *func)

    Creates an edge by specifying the source address.

    Looks up the source address in the block end map.

    .. caution:: Calls to this function should not acquire an accessor to the source address.

  .. cpp:function:: private Edge *link_block(Block *src, Block *dst, EdgeTypeEnum et, bool sink)

    Creates an edge by specifying the source Block.

    .. caution::
      Calls to this function should first acquire an accessor to the source address and lookup the block object.
      Otherwise, the source block may be split and cause wrong edges.

  .. cpp:function:: private Edge *link_tempsink(Block *src, EdgeTypeEnum et)

    During parsing, all edges are temporarily linked to the ``_tempsink`` block. Adding and then removing
    edges to and from this block is wasteful, especially given that removal is an ``O(N)`` operation with
    vector storage. This call thus does not record edges into the sink. These edges are guaranteed to
    have been relinked when parsing is in state :cpp:enumerator:`COMPLETE`.

    .. note::
      This introduces an inconsistency into the CFG invariant that all targets of an edge have that edge in their
      source list if the data structures are queried during parsing. Extenders of parsing callbacks are the only
      ones who can see this state; they have been warned and should know better.

  .. cpp:function:: private void relink(Edge *exist, Block *src, Block *dst)
  .. cpp:function:: private pair<Function *, Edge *> bind_call(ParseFrame &frame, Address target, Block *cur, Edge *exist)
  .. cpp:function:: private void parse_frames(LockFreeQueue<ParseFrame *> &, bool)
  .. cpp:function:: private void parse_frame(ParseFrame &frame, bool)
  .. cpp:function:: private bool parse_frame_one_iteration(ParseFrame &frame, bool)
  .. cpp:function:: private bool inspect_value_driven_jump_tables(ParseFrame &)
  .. cpp:function:: private void resumeFrames(Function *func, LockFreeQueue<ParseFrame *> &work)

    Add ParseFrames waiting on func back to the work queue

  .. cpp:function:: private void tamper_post_processing(LockFreeQueue<ParseFrame *> &, ParseFrame *)

  ......

  .. rubric:: Defensive parsing details

  .. cpp:function:: private ParseFrame *getTamperAbsFrame(Function *tamperFunc)
  .. cpp:function:: private void ProcessUnresBranchEdge(ParseFrame &, Block *, InstructionAdapter_t *, Address target)

  ......

  .. rubric:: Implementation of the parsing loop

  .. cpp:function:: private void ProcessCallInsn(ParseFrame &, Block *, InstructionAdapter_t *, bool, bool, bool, Address)
  .. cpp:function:: private void ProcessReturnInsn(ParseFrame &, Block *, InstructionAdapter_t *)
  .. cpp:function:: private bool ProcessCFInsn(ParseFrame &, Block *, InstructionAdapter_t *)
  .. cpp:function:: private void finalize()
  .. cpp:function:: private void finalize_funcs(dyn_c_vector<Function *> &funcs)
  .. cpp:function:: private void clean_bogus_funcs(dyn_c_vector<Function *> &funcs)
  .. cpp:function:: private void finalize_ranges()

    If range data is changed to use a concurrent data structure that supports concurrent writes. Finalizing ranges
    should then be moved back to normal finalization.

    .. caution:: This function should be run only with a single thread.

  .. cpp:function:: private void finalize_jump_tables()

    The goal of finalizing jump tables is to remove bogus control flow edges caused by
    over-approximating jump table size. During jump table analysis, we do not use any information about
    other jump tables.

    Here, we know all the jump table starts and we assume that "no jump  tables
    share any entries". Therefore, if a jump table overruns into another jump table, we trim the overrun
    entries.

  .. cpp:function:: private void delete_bogus_blocks(Edge *)

    Removed indirect jump edges may lead to other blocks and edges that should be removed

  .. cpp:function:: private bool set_edge_parsing_status(ParseFrame &, Address addr, Block *b)
  .. cpp:function:: private void update_function_ret_status(ParseFrame &, Function *, ParseWorkElem *)
  .. cpp:function:: private void record_hint_functions()
  .. cpp:function:: private void invalidateContainingFuncs(Function *, Block *)
  .. cpp:function:: private bool getSyscallNumber(Function *, Block *, Address, Architecture, long int &)
  .. cpp:member:: private Mutex<true> parse_mutex
  .. cpp:function:: LockFreeQueueItem<ParseFrame *> *ProcessOneFrame(ParseFrame *pf, bool recursive)
  .. cpp:function:: void SpawnProcessFrame(ParseFrame *frame, bool recursive)
  .. cpp:function:: void ProcessFrames(LockFreeQueue<ParseFrame *> *work_queue, bool recursive)
  .. cpp:function:: void LaunchWork(LockFreeQueueItem<ParseFrame *> *frame_list, bool recursive)
  .. cpp:function:: void processCycle(LockFreeQueue<ParseFrame *> &work, bool recursive)
  .. cpp:function:: void processFixedPoint(LockFreeQueue<ParseFrame *> &work, bool recursive)
  .. cpp:function:: LockFreeQueueItem<ParseFrame *> *postProcessFrame(ParseFrame *pf, bool recursive)
  .. cpp:function:: void updateBlockEnd(Block *b, Address addr, Address previnsn, region_data *rd) const
  .. cpp:member:: vector<Function *> funcs_to_ranges

    Range data is initialized through writing to interval trees. This is intrinsitcally mutual exclusive, so we
    delay this initialization until someone actually needs this.

    .. caution:: this has to be run in a single thread.

  .. cpp:member:: dyn_c_hash_map<Block *, std::set<Function *>> funcsByBlockMap


.. cpp:enum:: Parser::ParseState

  .. cpp:enumerator:: UNPARSED

    raw state

  .. cpp:enumerator:: PARTIAL

    parsing has started

  .. cpp:enumerator:: COMPLETE

    full parsing -- range queries are invalid

  .. cpp:enumerator:: RETURN_SET
  .. cpp:enumerator:: FINALIZED
  .. cpp:enumerator:: UNPARSEABLE

    error condition
