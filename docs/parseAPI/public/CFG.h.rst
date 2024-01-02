.. _`sec:CFG.h`:

CFG.h
#####

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:class:: Function : public AnnotatableSparse, public boost::lockable_adapter<boost::recursive_mutex>

  **The portion of the program CFG that is reachable through intraprocedural control flow transfers**

  Functions have a single entry point; multiple-entry functions such as those found
  in Fortran programs are represented as several functions that “share” a subset of
  the CFG. Functions may be non-contiguous and may share blocks with other
  functions.

  .. cpp:type:: std::map<Address, Block *> blockmap
  .. cpp:type:: select2nd<blockmap::value_type> selector
  .. cpp:type:: boost::transform_iterator<selector, blockmap::iterator> bmap_iterator
  .. cpp:type:: boost::transform_iterator<selector, blockmap::const_iterator> bmap_const_iterator
  .. cpp:type:: boost::iterator_range<bmap_iterator> blocklist
  .. cpp:type:: boost::iterator_range<bmap_const_iterator> const_blocklist
  .. cpp:type:: std::set<Edge *> edgelist

  .. cpp:function:: Function(Address addr, string name, CodeObject * obj, CodeRegion * region, InstructionSource * isource)

      Creates a function at ``addr`` in the code region specified. Instructions
      for this function are given in ``isource``.

  .. cpp:function:: static void destroy(Function *f)
  .. cpp:function:: boost::recursive_mutex &lockable()
  .. cpp:function:: FuncReturnStatus retstatus() const
  .. cpp:function:: void set_retstatus(FuncReturnStatus rs)

      Sets the return status for the function to ``rs``.

  .. cpp:function:: virtual const std::string &name() const
  .. cpp:function:: void rename(std::string n)
  .. cpp:function:: Address addr() const
  .. cpp:function:: CodeRegion *region() const
  .. cpp:function:: InstructionSource *isrc() const
  .. cpp:function:: CodeObject *obj() const
  .. cpp:function:: FuncSource src() const
  .. cpp:function:: bool parsed() const

  .. cpp:function:: const edgelist &callEdges()
  .. cpp:function:: bool hasNoStackFrame() const
  .. cpp:function:: bool savesFramePointer() const
  .. cpp:function:: bool cleansOwnStack() const
  .. cpp:function:: StackTamper tampersStack(bool recalculate = false)

  .. cpp:function:: LoopTreeNode* getLoopTree()

      Returns the nesting tree of the loops in the function.

  .. cpp:function:: Loop* findLoop(const char *name)

      Returns the loop with the given nesting name.

  .. cpp:function:: bool getLoops(vector<Loop*> &loops);

      Fills ``loops`` with all the loops in the function.

  .. cpp:function:: bool getOuterLoops(vector<Loop*> &loops);

      Fills ``loops`` with all the outermost loops in the function.

  .. cpp:function:: Block *entry() const
  .. cpp:function:: inline std::pair<Address, Block *> get_next_block(Address addr, CodeRegion *codereg) const
  .. cpp:function:: blocklist blocks()
  .. cpp:function:: const_blocklist blocks() const
  .. cpp:function:: size_t num_blocks()
  .. cpp:function:: const_blocklist returnBlocks()
  .. cpp:function:: const_blocklist exitBlocks()

  .. cpp:function:: bool dominates(Block* A, Block *B);

      Checks if block ``A`` dominates block ``B``.

  .. cpp:function:: Block* getImmediateDominator(Block *A);

      Returns the immediate dominator of block ``A``.

      Returns ``NULL``, if no immediate dominator exists.

  .. cpp:function:: void getImmediateDominates(Block *A, set<Block*> &imm);

      Fills ``imm`` with all the blocks immediate dominated by block ``A``.

  .. cpp:function:: void getAllDominates(Block *A, set<Block*> &dom);

      Fills ``dom`` with all the blocks dominated by block ``A``.

  .. cpp:function:: bool postDominates(Block* A, Block *B);

      Checks if block ``A`` post-dominates block ``B``.

  .. cpp:function:: Block* getImmediatePostDominator(Block *A);

      Returns the immediate post-dominator of block ``A``.

      Returns ``NULL``, if no immediate post-dominator exists.

  .. cpp:function:: void getImmediatePostDominates(Block *A, set<Block*> &imm);

      Fills ``imm`` with all the blocks immediate post-dominated by block ``A``.

  .. cpp:function:: void getAllPostDominates(Block *A, set<Block*> &dom);

      Fills ``dom`` with all the blocks post-dominated by block ``A``.

  .. cpp:function:: std::vector<FuncExtent *> const& extents()

      Returns the contiguous extents of binary code within the function.

  .. cpp:function:: void setEntryBlock(block * new_entry)

      Sets the entry block for this function to ``new_entry``.

  .. cpp:function:: bool contains(Block *b)

      Checks if this function contains the given block ``b``.

  .. cpp:function:: void removeBlock(Block *b)

      Remove the basic block ``b`` from this function.

  .. cpp:function:: std::map<Address, JumpTableInstance> &getJumpTables()

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

.. cpp:enum:: FuncReturnStatus

  Return status of an function, which indicates whether this function will
  return to its caller or not.

  .. cpp:enumerator:: UNSET

    unparsed function (default)

  .. cpp:enumerator:: NORETURN

    will not return

  .. cpp:enumerator:: UNKNOWN

    cannot be determined statically

  .. cpp:enumerator:: RETURN

    may return

.. cpp:enum:: EdgeTypeEnum

  .. cpp:enumerator:: CALL
  .. cpp:enumerator:: COND_TAKEN
  .. cpp:enumerator:: COND_NOT_TAKEN
  .. cpp:enumerator:: INDIRECT
  .. cpp:enumerator:: DIRECT
  .. cpp:enumerator:: FALLTHROUGH
  .. cpp:enumerator:: CATCH
  .. cpp:enumerator:: CALL_FT
  .. cpp:enumerator:: RET
  .. cpp:enumerator:: NOEDGE
  .. cpp:enumerator:: _edgetype_end_

.. cpp:enum:: FuncSource

  .. cpp:enumerator:: RT

    recursive traversal (default)

  .. cpp:enumerator:: HINT

    specified in code object hints

  .. cpp:enumerator:: GAP

    gap heuristics

  .. cpp:enumerator:: GAPRT

    RT from gap-discovered function

  .. cpp:enumerator:: ONDEMAND

    dynamically discovered

  .. cpp:enumerator:: MODIFICATION

    Added via user modification

  .. cpp:enumerator:: _funcsource_end_

.. cpp:function:: std::string format(EdgeTypeEnum e)

.. cpp:class:: Block : public SimpleInterval<Address, int>, public boost::lockable_adapter<boost::recursive_mutex>

  **A basic block**

  .. cpp:type:: std::vector<Edge *> edgelist
  .. cpp:type:: std::map<Offset, InstructionAPI::Instruction::Ptr> Insns

  .. cpp:function:: Block(CodeObject * o, CodeRegion * r, Address start, Function* f = NULL)

      Creates a block at ``start`` in the code region and code object
      specified. Optionally, one can specify the function that will parse the
      block. This constructor is used by the ParseAPI parser, which will
      update its end address during parsing.

  .. cpp:function:: Block(CodeObject * o, CodeRegion * r, Address start, Address end, Address last, Function* f = NULL)

      Creates a block at ``start`` in the code region and code object
      specified. The block has its last instruction at address ``last`` and
      ends at address ``end``. This constructor allows external parsers to
      construct their own blocks.

  .. cpp:function:: bool consistent(Address addr, Address & prev_insn)

      Check whether address ``addr`` is *consistent* with this basic block. An
      address is consistent if it is the boundary between two instructions in
      the block. As long as ``addr`` is within the range of the block,
      ``prev_insn`` will contain the address of the previous instruction
      boundary before ``addr``, regardless of whether ``addr`` is consistent
      or not.

  .. cpp:function:: void getFuncs(std::vector<Function *> & funcs)

      Fills in the provided vector with all functions that share this basic
      block.

  .. cpp:function:: template <class OutputIterator> void getFuncs(OutputIterator result)

      Generic version of the above; adds each Function that contains this
      block to the provided OutputIterator. For example:

  .. cpp:function:: void getInsns(Insns &insns) const

      Disassembles the block and stores the result in ``Insns``.

  .. cpp:function:: InstructionAPI::Instruction::Ptr getInsn(Offset o) const

      Returns the instruction starting at offset ``o`` within the block.
      Returns ``InstructionAPI::Instruction::Ptr()`` if ``o`` is outside the
      block, or if an instruction does not begin at ``o``.

  .. cpp:function:: Address start() const
  .. cpp:function:: Address end() const
  .. cpp:function:: Address lastInsnAddr() const
  .. cpp:function:: virtual Address last() const
  .. cpp:function:: Address size() const
  .. cpp:function:: bool containsAddr(Address addr) const
  .. cpp:function:: bool parsed() const
  .. cpp:function:: CodeObject* obj() const
  .. cpp:function:: CodeRegion* region() const
  .. cpp:function:: const edgelist& sources() const
  .. cpp:function:: const edgelist& targets() const
  .. cpp:function:: void copy_sources(edgelist & src) const
  .. cpp:function:: void copy_targets(edgelist & trg) const
  .. cpp:function:: bool hasCallSource() const
  .. cpp:function:: Edge* getOnlyIncomingEdge() const
  .. cpp:function:: int  containingFuncs() const
  .. cpp:function:: bool wasUserAdded() const
  .. cpp:function:: Address low() const override
  .. cpp:function:: Address high() const override
  .. cpp:function:: static void destroy(Block *b)
  .. cpp:function:: Function* createdByFunc()


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

  .. cpp:function:: Edge(Block * source, Block * target, EdgeTypeEnum type)
  .. cpp:function:: void ignore_index()
  .. cpp:function:: void from_index()
  .. cpp:function:: Block * src() const
  .. cpp:function:: Block * trg() const
  .. cpp:function:: Address trg_addr()
  .. cpp:function:: EdgeTypeEnum type() const
  .. cpp:function:: bool sinkEdge() const
  .. cpp:function:: bool interproc() const
  .. cpp:function:: bool intraproc() const
  .. cpp:function:: void install()
  .. cpp:function:: void uninstall()
  .. cpp:function:: static void destroy(Edge *, CodeObject *)

.. cpp:class:: EdgePredicate

  **Selection criterion for tree traversal**

  .. cpp:function:: virtual bool pred_impl(Edge *) const
  .. cpp:function:: bool operator()(Edge* e) const

.. cpp:class:: Intraproc : public EdgePredicate

  **Predicate to follow branches into a function if there is shared code**

  .. cpp:function:: bool pred_impl(Edge *) const

.. cpp:class:: Interproc : public EdgePredicate

  **Predicate to follow interprocedural edges**

  .. cpp:function:: bool pred_impl(Edge *) const

.. cpp:class:: NoSinkPredicate : public ParseAPI::EdgePredicate

  **Reject unresolved control flow edges**

  .. cpp:function:: bool pred_impl(ParseAPI::Edge * e) const

.. cpp:class:: SingleContext : public EdgePredicate

  **Don't follow branches into the function if there is shared code**

  .. cpp:function:: SingleContext(const Function * f, bool forward, bool backward)
  .. cpp:function:: bool pred_impl(Edge *) const

.. cpp:class:: SingleContextOrInterproc : public EdgePredicate

  **Don't follow branches into the function if there is shared code**

  Unlike :cpp:class:`SingleContext`, this will follow interprocedural call/return edges.

  .. cpp:function:: SingleContextOrInterproc(const Function * f, bool forward, bool backward)
  .. cpp:function:: bool pred_impl(Edge *) const
  .. cpp:function:: bool operator()(Edge* e) const


Notes
=====

Edge
^^^^
Typed Edges join two blocks in the CFG, indicating the type of control
flow transfer instruction that joins the blocks to each other. Edges may
not correspond to a control flow transfer instruction at all, as in the
case of the fallthrough edge that indicates where straight-line control
flow is split by incoming transfers from another location, such as a
branch. While not all blocks end in a control transfer instruction, all
control transfer instructions end basic blocks and have outgoing edges;
in the case of unresolvable control flow, the edge will target a special
“sink” block (see ``sinkEdge()``, below).
