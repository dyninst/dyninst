.. _`sec:PatchCFG.h`:

PatchCFG.h
##########

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:class:: PatchFunction

  **Wraps a ParseAPI function**

  .. cpp:function:: PatchFunction(ParseAPI::Function *f, PatchObject* o)
  .. cpp:function:: PatchFunction(const PatchFunction* parFunc, PatchObject* child)
  .. cpp:function:: static PatchFunction *create(ParseAPI::Function *, PatchObject*)

  .. cpp:type:: std::set<PatchBlock*, compare> PatchFunction::Blockset

  .. cpp:function:: const string& name()

      Returns the function’s mangled name.

  .. cpp:function:: Address addr() const

      Returns the address of the first instruction in this function.

  .. cpp:function:: ParseAPI::Function* function()

      Returns the ParseAPI::Function associated with this PatchFunction.

  .. cpp:function:: PatchObject* obj()

      Returns the PatchObject associated with this PatchFunction.

  .. cpp:function:: PatchBlock* entry()

      Returns the entry block of this PatchFunction.

  .. cpp:function:: const Blockset& blocks()

      Returns a set of all PatchBlocks in this PatchFunction.

  .. cpp:function:: const Blockset& exitBlocks()

      Returns a set of exit blocks of this PatchFunction.

  .. cpp:function:: const Blockset& callBlocks()

      Returns a set of all call blocks of this PatchFunction.

  .. cpp:function:: PatchCallback* cb() const

      Returns the PatchCallback object associated with this PatchFunction.

  .. cpp:function:: PatchLoopTreeNode* getLoopTree()

      Return the nesting tree of the loops in the function. See class
      ``PatchLoopTreeNode`` for more details

  .. cpp:function:: PatchLoop* findLoop(const char* name)

      Return the loop with the given nesting name. See class
      ``PatchLoopTreeNode`` for more details about how loop nesting names are
      assigned.

  .. cpp:function:: bool getLoops(vector<PatchLoop*>& loops)

      Fill ``loops`` with all the loops in the function

  .. cpp:function:: bool getOuterLoops(vector<PatchLoop*>& loops)

      Fill ``loops`` with all the outermost loops in the function

  .. cpp:function:: bool dominates(PatchBlock* A, PatchBlock* B)

      Return true if block ``A`` dominates block ``B``

  .. cpp:function:: PatchBlock* getImmediateDominator(PatchBlock* A)

      Return the immediate dominator of block ``A``\ ，\ ``NULL`` if the block
      ``A`` does not have an immediate dominator.

  .. cpp:function:: void getImmediateDominates(PatchBlock* A, set<PatchBlock*>& imm)

      Fill ``imm`` with all the blocks immediate dominated by block ``A``

  .. cpp:function:: void getAllDominates(PatchBlock* A, set<PatchBlock*>& dom)

      Fill ``dom`` with all the blocks dominated by block ``A``

  .. cpp:function:: bool postDominates(PatchBlock* A, PatchBlock* B)

      Return true if block ``A`` post-dominates block ``B``

  .. cpp:function:: PatchBlock* getImmediatePostDominator(PatchBlock* A)

      Return the immediate post-dominator of block ``A``\ ，\ ``NULL`` if the
      block ``A`` does not have an immediate post-dominator.

  .. cpp:function:: void getImmediatePostDominates(PatchBlock* A, set<PatchBlock*>& imm)

      Fill ``imm`` with all the blocks immediate post-dominated by block ``A``

  .. cpp:function:: void getAllPostDominates(PatchBlock* A, set<PatchBlock*>& dom)

      Fill ``dom`` with all the blocks post-dominated by block ``A``

  .. cpp:function:: virtual void markModified()

.. cpp:struct:: PatchFunction::compare

  **Orders PatchBlocks by starting address**

  .. cpp:function:: bool operator()(PatchBlock * const &b1, PatchBlock * const &b2)

.. cpp:class:: PatchBlock

  **A wrapper around a ParseAPI::Block**.

  .. cpp:type:: std::map<Address, InstructionAPI::Instruction> Insns
  .. cpp:type:: std::vector<PatchEdge*> edgelist

  .. cpp:function:: PatchBlock(const PatchBlock *parblk, PatchObject *child)
  .. cpp:function:: PatchBlock(ParseAPI::Block *block, PatchObject *obj)
  .. cpp:function:: static PatchBlock *create(ParseAPI::Block *, PatchFunction *)

  .. cpp:function:: Address start() const

      Returns the lower bound of this block (the address of the first instruction).

  .. cpp:function:: Address end() const

      Returns the upper bound (open) of this block (the address immediately
      following the last byte in the last instruction).

  .. cpp:function:: Address last() const

      Returns the address of the last instruction in this block.

  .. cpp:function:: Address size() const

      Returns end() - start().

  .. cpp:function:: PatchFunction* getFunction(ParseAPI::Function* f)

      Returns the underlying ParseAPI function.

  .. cpp:function:: bool isShared()

      Checks if this block is contained by multiple functions.

  .. cpp:function:: int containingFuncs() const

      Returns the number of functions that contain this block.

  .. cpp:function:: void getInsns(Insns& insns) const

      Stores all instructions in this block in ``insns``.

  .. cpp:function:: InstructionAPI::Instruction getInsn(Address a) const

      Returns the instruction starting at address ``a``.

  .. cpp:function:: std::string disassemble() const

      Returns a string representing the disassembled code for this block.

  .. cpp:function:: bool containsCall()

      Checks if this PatchBlock contains a function call.

  .. cpp:function:: bool containsDynamicCall()

      Checks if this PatchBlock contains any indirect function call, e.g., via function pointer.

  .. cpp:function:: std::string format() const

      Returns a string representation of this block.

  .. cpp:function:: std::string long_format() const

      Returns a string representation of this block and its contained instructions.

  .. cpp:function:: PatchFunction* getCallee()

      Returns the callee function.

      If this PatchBlock does not contain a function call, returns ``NULL``.

  .. cpp:function:: ParseAPI::Block* block() const

      Returns the ParseAPI::Block associated with this PatchBlock.

  .. cpp:function:: PatchObject* object() const

      Returns the PatchObject that contains this block.

  .. cpp:function:: PatchObject* obj() const

      Synonym for :cpp:func:`object`.

  .. cpp:function:: const edgelist& sources()

      Returns the source edges that target this block (i.e., the inbound edges).

  .. cpp:function:: const edgelist& targets()

      Returns the target edges of this block (i.e., the outbound edges).

  .. cpp:function:: PatchEdge* findSource(ParseAPI::EdgeTypeEnum type)

      Find the source edge for this block of type ``type``.

  .. cpp:function:: PatchEdge* findTarget(ParseAPI::EdgeTypeEnum type)

      Find the target edge for this block of type ``type``.

  .. cpp:function:: template <class OutputIterator> void getFuncs(OutputIterator result)

      Writes all functions containing this PatchBlock to ``result``.

      ``OutputIterator`` must be at least a C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_.

.. cpp:class:: PatchEdge

  **A wrapper around a ParseAPI::Edge**

  .. cpp:function:: PatchEdge(ParseAPI::Edge* internalEdge, PatchBlock* source, PatchBlock* target)

      Creates a wrapper around ``internalEdge`` between ``source`` and ``target``.

  .. cpp:function:: PatchEdge(const PatchEdge* parent, PatchBlock* child_src, PatchBlock* child_trg)

      Creates a wrapper around ``parent`` that adds the blocks ``source`` and ``target`` as children to it.

  .. cpp:function:: static PatchEdge* create(ParseAPI::Edge* e, PatchBlock* src, PatchBlock* trg)

      Helper to create a ``PatchEdge``.

  .. cpp:function:: ParseAPI::Edge* edge() const

      Returns a ParseAPI::Edge associated with this PatchEdge.

  .. cpp:function:: PatchBlock* src()

      Returns the source PatchBlock.

  .. cpp:function:: PatchBlock* trg()

      Returns the target PatchBlock.

  .. cpp:function:: ParseAPI::EdgeTypeEnum type() const

      Returns the edge type.

  .. cpp:function:: bool sinkEdge() const

      Checks if this edge targets the special sink block, where a sink
      block is a block to which all unresolvable control flow instructions
      will be linked.

  .. cpp:function:: bool interproc() const

      Checks if the edge should be interpreted as interprocedural
      (e.g., calls, returns, direct branches under certain circumstances).

  .. cpp:function:: bool intraproc() const

      The opposite of :cpp:func:`interproc`;

  .. cpp:function:: std::string format() const

      Returns a string representation of this edge.

.. cpp:class:: PatchLoop

  **A wrapper around a ParseAPI::Loop**

  .. cpp:member:: PatchLoop* parent

      The loop that directly encloses this loop.

  .. cpp:function:: bool containsAddress(Address addr)

      Checks if ``addr`` is contained in the range of this loop’s basic blocks.

  .. cpp:function:: bool containsAddressInclusive(Address addr)

      Checks if ``addr`` is contained in the range of this loop’s basic blocks or its children.

  .. cpp:function:: int getLoopEntries(vector<PatchBlock*>& entries)

      Inserts the entry basic blocks of the loop into ``entries``.

      Returns the number of the entries added.

  .. cpp:function:: int getBackEdges(vector<PatchEdge*>& edges)

      Inserts the back edges in this loop into ``edges``.

      Returns the number of back edges added.

  .. cpp:function:: bool getContainedLoops(vector<PatchLoop*>& loops)

      Inserts the loops nested under this loop into ``loops``.

      Returns ``false`` on error.

  .. cpp:function:: bool getOuterLoops(vector<PatchLoop*>& loops)

      Inserts the loops that contain this loop into ``loops``.

      Returns ``false`` on error.

  .. cpp:function:: bool getLoopBasicBlocks(vector<PatchBlock*>& blocks)

      Inserts the basic blocks under this loop into ``blocks``.

      Returns ``false`` on error.

  .. cpp:function:: bool getLoopBasicBlocksExclusive(vector<PatchBlock*>& blocks)

      Inserts the basic blocks under this loop into ``blocks``, excluding the
      blocks of its sub loops.

      Returns ``false`` on error.

  .. cpp:function:: bool hasBlock(PatchBlock* b)

      Checks if this loop or its children contains basic block ``b``.

  .. cpp:function:: bool hasBlockExclusive(PatchBlock* b)

      Checks if this loop contains basic block ``b``, and ``b`` is not in its sub loops.

  .. cpp:function:: bool hasAncestor(PatchLoop* loop)

      Checks if this loop is a descendant of ``loop``.

  .. cpp:function:: PatchFunction*  getFunction()

      Returns the function that this loop is in.

  .. cpp:function:: std::string format() const

      Returns a string representation of this loop.

.. cpp:class:: PatchLoopTreeNode

  .. cpp:member:: PatchLoop* loop

      The PatchLoop instance it points to.

  .. cpp:member:: std::vector<PatchLoopTreeNode*> children

      The PatchLoopTreeNode instances nested within this loop.

  .. cpp:function:: PatchLoopTreeNode(PatchObject *obj, ParseAPI::LoopTreeNode *l, std::map<ParseAPI::Loop*, PatchLoop*>&)

      Creates a loop tree node for Loop with name ``n``.

  .. cpp:function:: const char* name()

      Returns the hierarchical name of this loop.

  .. cpp:function:: const char* getCalleeName(unsigned int i)

      Returns the function name of the ith callee.

  .. cpp:function:: unsigned int numCallees()

      Returns the number of callees contained in this loop’s body.

  .. cpp:function:: bool getCallees(vector<PatchFunction* >& v)

      Fills ``v`` with a vector of the functions called inside this loop.

  .. cpp:function:: PatchLoop*  findLoop(const char* name)

      Looks up a loop by the hierarchical name
