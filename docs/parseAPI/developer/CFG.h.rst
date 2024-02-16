.. _`sec-dev:CFG.h`:

CFG.h
#####

.. cpp:namespace:: Dyninst::ParseAPI::dev

.. cpp:class:: Function : public AnnotatableSparse, public boost::lockable_adapter<boost::recursive_mutex>

  .. cpp:type:: std::map<Address, Block*> blockmap
  .. cpp:type:: select2nd<blockmap::value_type> selector
  .. cpp:type:: boost::transform_iterator<selector, blockmap::iterator> bmap_iterator
  .. cpp:type:: boost::transform_iterator<selector, blockmap::const_iterator> bmap_const_iterator
  .. cpp:type:: boost::iterator_range<bmap_iterator> blocklist
  .. cpp:type:: boost::iterator_range<bmap_const_iterator> const_blocklist
  .. cpp:type:: std::set<Edge*> edgelist

  .. cpp:member:: bool _is_leaf_function
  .. cpp:member:: Address _ret_addr

    return address of a function stored in stack at function entry

  .. cpp:function:: static void destroy(Function *f)
  .. cpp:function:: void set_retstatus(FuncReturnStatus rs)

      Sets the return status for the function to ``rs``.

  .. cpp:function:: std::vector<FuncExtent *> const& extents()

      Returns the contiguous code segments of binary code within the function.

  ......

  .. rubric::
    Basic block and CFG access
    
  .. cpp:function:: blocklist blocks()
  .. cpp:function:: const_blocklist blocks() const
  .. cpp:function:: size_t num_blocks()
  .. cpp:function:: const_blocklist returnBlocks()
  .. cpp:function:: const_blocklist exitBlocks()
  .. cpp:function:: std::map<Address, JumpTableInstance> &getJumpTables()

  .. cpp:function:: void setEntryBlock(block * new_entry)

      Sets the entry block for this function to ``new_entry``.

  .. cpp:function:: void removeBlock(Block *b)

      Remove the basic block ``b`` from this function.

  .. cpp:function:: void add_block(Block *b)

  ......

  .. rubric::
    Function details
 
  .. cpp:function:: bool hasNoStackFrame() const
  .. cpp:function:: bool savesFramePointer() const
  .. cpp:function:: bool cleansOwnStack() const

  ......

  .. cpp:function:: void invalidateCache()

      This should not remain here - this is an experimental fix for defensive mode
      CFG inconsistency

  .. cpp:function:: StackTamper tampersStack(bool recalculate = false)
  .. cpp:function:: boost::recursive_mutex &lockable()
  .. cpp:function:: CodeRegion *region() const
  .. cpp:function:: InstructionSource *isrc() const
  .. cpp:function:: CodeObject *obj() const
  .. cpp:function:: FuncSource src() const
  .. cpp:function:: bool parsed() const

  .. cpp:member:: protected Address _start
  .. cpp:member:: protected CodeObject * _obj
  .. cpp:member:: protected CodeRegion * _region
  .. cpp:member:: protected InstructionSource * _isrc
  .. cpp:member:: protected bool _cache_valid
  .. cpp:member:: protected FuncSource _src
  .. cpp:member:: protected boost::atomic<FuncReturnStatus> _rs
  .. cpp:member:: protected std::string _name
  .. cpp:member:: protected Block * _entry
  .. cpp:function:: protected Function()


  .. cpp:function:: private void delayed_link_return(CodeObject * co, Block * retblk)
  .. cpp:function:: private void finalize()
  .. cpp:member:: private bool _parsed
  .. cpp:member:: private std::vector<FuncExtent *> _extents

  .. cpp:function:: private blocklist blocks_int()

    rapid lookup for edge predicate tests

  .. cpp:member:: private blockmap _bmap
  .. cpp:function:: private bmap_iterator blocks_begin()
  .. cpp:function:: private bmap_iterator blocks_end()
  .. cpp:function:: private bmap_const_iterator blocks_begin() const
  .. cpp:function:: private bmap_const_iterator blocks_end() const
  .. cpp:member:: private edgelist _call_edge_list

    rapid lookup for interprocedural queries

  .. cpp:member:: private blockmap _retBL
  .. cpp:function:: private bmap_const_iterator ret_begin() const
  .. cpp:function:: private bmap_const_iterator ret_end() const
  .. cpp:member:: private blockmap _exitBL

    Superset of return blocks this includes all blocks where execution leaves the function
    without coming back, including returns, calls to non-returning calls, tail calls, etc.
    Might want to include exceptions.

  .. cpp:function:: private bmap_const_iterator exit_begin() const
  .. cpp:function:: private bmap_const_iterator exit_end() const

  ......

  .. rubric::
    function details

  .. cpp:member:: private bool _no_stack_frame
  .. cpp:member:: private bool _saves_fp
  .. cpp:member:: private bool _cleans_stack
  .. cpp:member:: private StackTamper _tamper
  .. cpp:member:: private Address _tamper_addr

  ......

  .. rubric::
    Loop details
    
  .. cpp:member:: private mutable bool _loop_analyzed

    true if loops in the function have been found and stored in _loops

  .. cpp:member:: private mutable std::set<Loop*> _loops
  .. cpp:member:: private mutable LoopTreeNode *_loop_root

    NULL if the tree structure has not be calculated

  .. cpp:function:: private void getLoopsByNestingLevel(std::vector<Loop*>& lbb, bool outerMostOnly) const
  .. cpp:member:: private std::map<Address, JumpTableInstance> jumptables
  .. cpp:member:: private mutable bool isDominatorInfoReady

    Dominator and post-dominator info details

  .. cpp:member:: private mutable bool isPostDominatorInfoReady
  .. cpp:function:: private void fillDominatorInfo() const
  .. cpp:function:: private void fillPostDominatorInfo() const

  ......

  .. rubric::
    Dominator and post-dominator info details
    
  .. cpp:member:: private mutable std::map<Block*, std::set<Block*>*> immediateDominates

    set of basic blocks that this basicblock dominates immediately

  .. cpp:member:: private mutable std::map<Block*, Block*> immediateDominator

    basic block which is the immediate dominator of the basic block

  .. cpp:member:: private mutable std::map<Block*, std::set<Block*>*> immediatePostDominates

    same as previous two fields, but for postdominator tree

  .. cpp:member:: private mutable std::map<Block*, Block*> immediatePostDominator


.. cpp:struct:: template <typename P> Function::select2nd

  .. cpp:type:: typename P::second_type result_type
  .. cpp:function:: result_type operator()(const P &p) const

.. cpp:struct:: Function::JumpTableInstance

  .. cpp:member:: AST::Ptr jumpTargetExpr
  .. cpp:member:: Address tableStart
  .. cpp:member:: Address tableEnd
  .. cpp:member:: int indexStride
  .. cpp:member:: int memoryReadSize
  .. cpp:member:: bool isZeroExtend
  .. cpp:member:: std::map<Address, Address> tableEntryMap
  .. cpp:member:: Block* block


.. cpp:struct:: Function::less

  If there are more than one guest binary file loaded, multiple
  functions may have the same entry point address in different
  code regions. And regions themselves may use the same
  address ranges.

  We order functions by their regions first, by their address second.

  We order regions by their start first, by their end second,
  by the numeric value of their pointers third. We consider NULL
  to be less than any non-NULL region.

  The algorithm below is the same as ordering with per-component
  comparison vectors

    ``(Region::low(), Region::high(), Region::ptr, Function::addr(), Function::ptr)``

  where low() and high() for NULL region are considered to be -INF.

  For typical shared libraries and executables this should order
  functions by their address. For static libraries it should group
  functions by their object files and order object files by their
  size.

  .. cpp:function:: bool operator()(const Function * f1, const Function * f2) const


.. cpp:class:: FuncExtent : public Dyninst::SimpleInterval<Address, Function*>

  **A contiguous extent of a function**

  Function Extents are used internally for accounting and lookup purposes.
  They may be useful for users who wish to precisely identify the ranges
  of the address space spanned by functions (functions are often
  discontiguous, particularly on architectures with variable length
  instruction sets).

  .. cpp:function:: FuncExtent(Function * f, Address start, Address end)
  .. cpp:function:: Function * func()
  .. cpp:function:: Address start() const
  .. cpp:function:: Address end() const
  .. cpp:function:: Address low() const
  .. cpp:function:: Address high() const
  .. cpp:function:: Function* id() const


.. cpp:class:: Edge

  .. cpp:member:: protected boost::atomic<Block *> _source
  .. cpp:member:: protected Block *_target
  .. cpp:member:: protected ParseData *index
  .. cpp:member:: protected Offset _target_off
  .. cpp:member:: protected bool _from_index
  .. cpp:member:: private EdgeType _type
  .. cpp:function:: void install()

  .. cpp:function:: void uninstall()

    removes from blocks (and if of type :cpp:enumerator:`EdgeTypeEnum::CALL`,
    from finalized source functions)

  .. cpp:function:: static void destroy(Edge *, CodeObject *)


.. cpp:struct:: Edge::EdgeType

  .. cpp:function:: EdgeType(EdgeTypeEnum t, bool s)
  .. cpp:member:: uint16_t _type_enum
  .. cpp:member:: uint8_t _sink
  .. cpp:member:: uint8_t _interproc

    modifier for interprocedural branches(tail calls)


.. cpp:class:: Loop

  .. cpp:member:: private std::set<Edge*> backEdges
  .. cpp:member:: private std::set<Block*> entries
  .. cpp:member:: private const Function * func

    the function this loop is part of

  .. cpp:member:: private std::set<Loop*> containedLoops

    set of loops that are contained (nested) in this loop.

  .. cpp:member:: private std::set<Block*> childBlocks

    the basic blocks in the loop

  .. cpp:member:: private std::set<Block*> exclusiveBlocks
  .. cpp:member:: private Loop* parent

  .. cpp:function:: private Loop(const Function *)

    internal use only constructor of class

  .. cpp:function:: private Loop(Edge *, const Function *)

    constructor of the class

  .. cpp:function:: private bool getLoops(std::vector<Loop*>&, bool outerMostOnly) const

    get either contained or outer loops, determined by outerMostOnly


.. cpp:class:: LoopTreeNode

  .. cpp:member:: private char *hierarchicalName

    name which indicates this loop's relative nesting

  .. cpp:member:: private std::vector<Function *> callees

    A vector of functions called within the body of this loop (and not the body of sub loops).


.. cpp:enum:: StackTamper

  .. cpp:enumerator:: TAMPER_UNSET
  .. cpp:enumerator:: TAMPER_NONE
  .. cpp:enumerator:: TAMPER_REL
  .. cpp:enumerator:: TAMPER_ABS
  .. cpp:enumerator:: TAMPER_NONZERO

