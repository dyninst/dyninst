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

  .. cpp:function:: Function(Address addr, string name, CodeObject * obj, CodeRegion * region, InstructionSource * isource)

      Creates a function at ``addr`` in the code region specified. Instructions
      for this function are given in ``isource``.

  .. cpp:function:: FuncReturnStatus retstatus() const
  .. cpp:function:: virtual const std::string &name() const
  .. cpp:function:: void rename(std::string n)
  .. cpp:function:: Address addr() const
  .. cpp:function:: const edgelist &callEdges()

  ......

  .. rubric::
    Loops

  .. cpp:function:: LoopTreeNode* getLoopTree()

      Returns the nesting tree of the loops in the function.

  .. cpp:function:: Loop* findLoop(const char *name)

      Returns the loop with the given nesting name.

  .. cpp:function:: bool getLoops(vector<Loop*> &loops);

      Fills ``loops`` with all the loops in the function.

  .. cpp:function:: bool getOuterLoops(vector<Loop*> &loops);

      Fills ``loops`` with all the outermost loops in the function.

  ......

  .. rubric::
    Dominator information

  .. cpp:function:: bool dominates(Block* A, Block *B);

      Checks if block ``A`` dominates block ``B``.

  .. cpp:function:: Block* getImmediateDominator(Block *A);

      Returns the immediate dominator of block ``A``.

      Returns ``NULL``, if no immediate dominator exists.

  .. cpp:function:: void getImmediateDominates(Block *A, set<Block*> &imm);

      Fills ``imm`` with all the blocks immediate dominated by block ``A``.

  .. cpp:function:: void getAllDominates(Block *A, set<Block*> &dom);

      Fills ``dom`` with all the blocks dominated by block ``A``.

  ......

  .. rubric::
    Post-dominator information

  .. cpp:function:: bool postDominates(Block* A, Block *B);

      Checks if block ``A`` post-dominates block ``B``.

  .. cpp:function:: Block* getImmediatePostDominator(Block *A);

      Returns the immediate post-dominator of block ``A``.

      Returns ``NULL``, if no immediate post-dominator exists.

  .. cpp:function:: void getImmediatePostDominates(Block *A, set<Block*> &imm);

      Fills ``imm`` with all the blocks immediate post-dominated by block ``A``.

  .. cpp:function:: void getAllPostDominates(Block *A, set<Block*> &dom);

      Fills ``dom`` with all the blocks post-dominated by block ``A``.

  ......

  .. rubric::
    Essential block information
    
  .. cpp:function:: Block *entry() const

  .. cpp:function:: inline std::pair<Address, Block *> get_next_block(Address addr, CodeRegion *codereg) const

  .. cpp:function:: bool contains(Block *b)

      Checks if this function contains the given block ``b``.


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

    fallthrough after call instruction

  .. cpp:enumerator:: RET
  .. cpp:enumerator:: NOEDGE
  .. cpp:enumerator:: _edgetype_end_

.. cpp:enum:: FuncSource

  Discovery method of functions

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
  .. cpp:type:: std::map<Offset, InstructionAPI::Instruction> Insns

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

  .. cpp:function:: InstructionAPI::Instruction getInsn(Offset o) const

      Returns the instruction starting at offset ``o`` within the block.
      Returns ``InstructionAPI::Instruction()`` if ``o`` is outside the
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


.. cpp:class:: Loop

  **Node that may execute repeatedly**

  .. cpp:function:: Loop* parentLoop()

      Returns loop which directly encloses this loop; ``NULL`` if no such loop.

  .. cpp:function:: bool containsAddress(Address addr)

      Checks if the given address is within the range of this loop’s basic blocks.

  .. cpp:function:: bool containsAddressInclusive(Address addr)

      Checks if the given address is within the range of this loop’s basic blocks or its children.

  .. cpp:function:: int getLoopEntries(vector<Block*>& entries);

      Fills ``entries`` with the set of entry basic blocks of the loop, and return
      the number of the entries.

      A natural loop has a single entry block and an irreducible loop has multiple entry blocks.

  .. cpp:function:: int getBackEdges(vector<Edge*> &edges)

      Sets ``edges`` to the set of back edges in this loop, and returns the
      number of back edges.

  .. cpp:function:: bool getContainedLoops(vector<Loop*> &loops)

      Fills ``loops`` with the loops that are nested under this loop.

  .. cpp:function:: bool getOuterLoops(vector<Loop*> &loops)

      Fills ``loops`` with the loops that are directly nested under this loop.

  .. cpp:function:: bool getLoopBasicBlocks(vector<Block*> &blocks)

      Fills ``blocks`` with all basic blocks in this loop.

  .. cpp:function:: bool getLoopBasicBlocksExclusive(vector<Block*> &blocks)

      Fills ``blocks`` with all basic blocks in this loop, excluding the
      blocks of its sub loops.

  .. cpp:function:: bool hasBlock(Block *b);

      Checks if this loop contains basic block ``b``.

  .. cpp:function:: bool hasBlockExclusive(Block *b);

      Checks if this loop contains basic block ``b`` and ``b`` is not in its sub loops.

  .. cpp:function:: bool hasAncestor(Loop *loop)

      Checks if this loop is a descendant of ``loop``.

  .. cpp:function:: Function * getFunction();

      Returns the function that this loop is in.


.. cpp:class:: LoopTreeNode

  **The tree of nested loops and callees (functions) in the control flow graph**

  .. cpp:member:: Loop *loop;

      The Loop instance it points to.

  .. cpp:member:: std::vector<LoopTreeNode *> children

      The LoopTreeNode instances nested within this loop.

  .. cpp:function:: LoopTreeNode(Loop *l, const char *n)

      Creates a loop tree node for ``l`` with name ``n``.

  .. cpp:function:: const char * name()

      Returns the hierarchical name of this loop.

  .. cpp:function:: const char * getCalleeName(unsigned int i)

      Returns the function name of the ith callee.

  .. cpp:function:: unsigned int numCallees()

      Returns the number of callees contained in this loop’s body.

  .. cpp:function:: bool getCallees(vector<Function *> &v)

      Fills ``v`` with a vector of the functions called inside this loop.

  .. cpp:function:: Loop * findLoop(const char *name)

      Looks up a loop by the hierarchical name.


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

LoopTreeNode
^^^^^^^^^^^^
The LoopTreeNode class provides a tree interface to a collection of
instances of class Loop contained in a function. The structure of the
tree follows the nesting relationship of the loops in a function. Each
LoopTreeNode contains a pointer to a loop (represented by Loop), and a
set of sub-loops (represented by other LoopTreeNode objects). The
``loop`` field at the root node is always ``NULL`` since a function may
contain multiple outer loops. The ``loop`` field is never ``NULL`` at
any other node since it always corresponds to a real loop. Therefore,
the outer most loops in the function are contained in the vector of
``children`` of the root.

Each instance of LoopTreeNode is given a name that indicates its
position in the hierarchy of loops. The name of each outermost loop
takes the form of ``loop_x``, where ``x`` is an integer from 1 to n,
where n is the number of outer loops in the function. Each sub-loop has
the name of its parent, followed by a ``.y``, where ``y`` is 1 to m,
where m is the number of sub-loops under the outer loop. For example,
consider the following C function:

.. code-block:: cpp

    void foo() {
     int x, y, z, i;
     for (x=0; x<10; x++) {
       for (y = 0; y<10; y++)
         ...
       for (z = 0; z<10; z++)
         ...
     }
     for (i = 0; i<10; i++) {
        ...
     }
   }

The ``foo`` function will have a root LoopTreeNode, containing a NULL
loop entry and two LoopTreeNode children representing the functions
outermost loops. These children would have names ``loop_1`` and
``loop_2``, respectively representing the ``x`` and ``i`` loops.
``loop_2`` has no children. ``loop_1`` has two child LoopTreeNode
objects, named ``loop_1.1`` and ``loop_1.2``, respectively representing
the ``y`` and ``z`` loops.

Loop
^^^^
Both loops that have a single entry block (aka, natural loops) and
loops that have multiple entry blocks (aka, irreducible loops). A back edge
is defined as an edge that has its source in the loop and has its target
being an entry block of the loop. It represents the end of an iteration
of the loop. For all the loops detected in a function, a loop nesting tree
to represent the nesting relations between the loops is also constructed.
See :cpp:class:`LoopTreeNode` for more details.
