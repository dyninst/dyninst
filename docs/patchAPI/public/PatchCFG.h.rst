PatchCFG.h
==========

.. cpp:namespace:: Dyninst::patchAPI

PatchFunction
=============

**Declared in**: PatchCFG.h

The PatchFunction class is a wrapper of ParseAPI’s Function class
(has-a), which represents a function.

.. code-block:: cpp
    
    const string &name();

Returns the function’s mangled name.

.. code-block:: cpp
    
    Address addr() const;

Returns the address of the first instruction in this function.

.. code-block:: cpp
    
    ParseAPI::Function *function();

Returns the ParseAPI::Function associated with this PatchFunction.

.. code-block:: cpp
    
    PatchObject* obj();

Returns the PatchObject associated with this PatchFunction.

.. code-block:: cpp
    
    typedef std::set<PatchBlock *> PatchFunction::Blockset;
    const Blockset &blocks();

Returns a set of all PatchBlocks in this PatchFunction.

.. code-block:: cpp
    
    PatchBlock *entry();

Returns the entry block of this PatchFunction.

.. code-block:: cpp
    
    const Blockset &exitBlocks();

Returns a set of exit blocks of this PatchFunction.

.. code-block:: cpp
    
    const Blockset &callBlocks();

Returns a set of all call blocks of this PatchFunction.

.. code-block:: cpp
    
    PatchCallback *cb() const;

Returns the PatchCallback object associated with this PatchFunction.

.. code-block:: cpp
    
    PatchLoopTreeNode* getLoopTree()

Return the nesting tree of the loops in the function. See class
``PatchLoopTreeNode`` for more details

.. code-block:: cpp
    
    PatchLoop* findLoop(const char *name)

Return the loop with the given nesting name. See class
``PatchLoopTreeNode`` for more details about how loop nesting names are
assigned.

.. code-block:: cpp
    
    bool getLoops(vector<PatchLoop*> &loops);

Fill ``loops`` with all the loops in the function

.. code-block:: cpp
    
    bool getOuterLoops(vector<PatchLoop*> &loops);

Fill ``loops`` with all the outermost loops in the function

.. code-block:: cpp
    
    bool dominates(PatchBlock* A, PatchBlock *B);

Return true if block ``A`` dominates block ``B``

.. code-block:: cpp
    
    PatchBlock* getImmediateDominator(PatchBlock *A);

Return the immediate dominator of block ``A``\ ，\ ``NULL`` if the block
``A`` does not have an immediate dominator.

.. code-block:: cpp
    
    void getImmediateDominates(PatchBlock *A, set<PatchBlock*> &imm);

Fill ``imm`` with all the blocks immediate dominated by block ``A``

.. code-block:: cpp
    
    void getAllDominates(PatchBlock *A, set<PatchBlock*> &dom);

Fill ``dom`` with all the blocks dominated by block ``A``

.. code-block:: cpp
    
    bool postDominates(PatchBlock* A, PatchBlock *B);

Return true if block ``A`` post-dominates block ``B``

.. code-block:: cpp
    
    PatchBlock* getImmediatePostDominator(PatchBlock *A);

Return the immediate post-dominator of block ``A``\ ，\ ``NULL`` if the
block ``A`` does not have an immediate post-dominator.

.. code-block:: cpp
    
    void getImmediatePostDominates(PatchBlock *A, set<PatchBlock*> &imm);

Fill ``imm`` with all the blocks immediate post-dominated by block ``A``

.. code-block:: cpp
    
    void getAllPostDominates(PatchBlock *A, set<PatchBlock*> &dom);

Fill ``dom`` with all the blocks post-dominated by block ``A``

PatchBlock
==========

**Declared in**: PatchCFG.h

The PatchBlock class is a wrapper of ParseAPI’s Block class (has-a),
which represents a basic block.

.. code-block:: cpp
    
    Address start() const;

Returns the lower bound of this block (the address of the first
instruction).

.. code-block:: cpp
    
    Address end() const;

Returns the upper bound (open) of this block (the address immediately
following the last byte in the last instruction).

.. code-block:: cpp
    
    Address last() const;

Returns the address of the last instruction in this block.

.. code-block:: cpp
    
    Address size() const;

Returns end() - start().

.. code-block:: cpp
    
    bool isShared();

Indicates whether this block is contained by multiple functions.

.. code-block:: cpp
    
    int containingFuncs() const;

Returns the number of functions that contain this block.

.. code-block:: cpp
    
    typedef std::map<Address, InstructionAPI::Instruction::Ptr> Insns; void getInsns(Insns &insns) const;

This function outputs Instructions that are in this block to *insns*.

.. code-block:: cpp
    
    InstructionAPI::Instruction::Ptr getInsn(Address a) const;

Returns an Instruction that has the address *a* as its starting address.
If no any instruction can be found in this block with the starting
address *a*, it returns InstructionAPI::Instruction::Ptr().

.. code-block:: cpp
    
    std::string disassemble() const;

Returns a string containing the disassembled code for this block. This
is mainly for debugging purpose.

.. code-block:: cpp
    
    bool containsCall();

Indicates whether this PatchBlock contains a function call instruction.

.. code-block:: cpp
    
    bool containsDynamicCall();

Indicates whether this PatchBlock contains any indirect function call,
e.g., via function pointer.

.. code-block:: cpp
    
    PatchFunction* getCallee();

Returns the callee function, if this PatchBlock contains a function
call; otherwise, NULL is returned.

.. code-block:: cpp
    
    PatchFunction *function() const;

Returns a PatchFunction that contains this PatchBlock. If there are
multiple PatchFunctions containing this PatchBlock, then a random one of
them is returned.

.. code-block:: cpp
    
    ParseAPI::Block *block() const;

Returns the ParseAPI::Block associated with this PatchBlock.

.. code-block:: cpp
    
    PatchObject* obj() const;

Returns the PatchObject that contains this block.

.. code-block:: cpp
    
    typedef std::vector<PatchEdge*> PatchBlock::edgelist;
    const edgelist &sources();

Returns a list of the source PatchEdges. This PatchBlock is the target
block of the returned edges.

.. code-block:: cpp
    
    const edgelist &targets();

Returns a list of the target PatchEdges. This PatchBlock is the source
block of the returned edges.

.. code-block:: cpp
    
    template <class OutputIterator> void getFuncs(OutputIterator result);

Outputs all functions containing this PatchBlock to the STL inserter
*result*.

.. code-block:: cpp
    
    PatchCallback *cb() const;

Returns the PatchCallback object associated with this PatchBlock.

.. _sec-3.2.11:

PatchEdge
=========

**Declared in**: PatchCFG.h

The PatchEdge class is a wrapper of ParseAPI’s Edge class (has-a), which
joins two PatchBlocks in the CFG, indicating the type of control flow
transfer instruction that joins the basic blocks to each other.

.. code-block:: cpp
    
    ParseAPI::Edge *edge() const;

Returns a ParseAPI::Edge associated with this PatchEdge.

.. code-block:: cpp
    
    PatchBlock *src();

Returns the source PatchBlock.

.. code-block:: cpp
    
    PatchBlock *trg();

Returns the target PatchBlock.

.. code-block:: cpp
    
    ParseAPI::EdgeTypeEnum type() const;

Returns the edge type (ParseAPI::EdgeTypeEnum, please see `ParseAPI
Manual <ftp://ftp.cs.wisc.edu/paradyn/releases/release7.0/doc/parseapi.pdf>`__).

.. code-block:: cpp
    
    bool sinkEdge() const;

Indicates whether this edge targets the special sink block, where a sink
block is a block to which all unresolvable control flow instructions
will be linked.

.. code-block:: cpp
    
    bool interproc() const;

Indicates whether the edge should be interpreted as interprocedural
(e.g., calls, returns, direct branches under certain circumstances).

.. code-block:: cpp
    
    PatchCallback *cb() const;

Returns a Patchcallback object associated with this PatchEdge.

.. _sec-3.2.12:

PatchLoop
=========

**Declared in**: PatchCFG.h

The PatchLoop class is a wrapper of ParseAPI’s Loop class (has-a). It
represents code structure that may execute repeatedly.

.. code-block:: cpp
    
    PatchLoop* parent

Returns the loop which directly encloses this loop. NULL if no such
loop.

.. code-block:: cpp
    
    bool containsAddress(Address addr)

Returns true if the given address is within the range of this loop’s
basic blocks.

.. code-block:: cpp
    
    bool containsAddressInclusive(Address addr)

Returns true if the given address is within the range of this loop’s
basic blocks or its children.

.. code-block:: cpp
    
    int getLoopEntries(vector<PatchBlock*>& entries);

Fills ``entries`` with the set of entry basic blocks of the loop. Return
the number of the entries that this loop has

.. code-block:: cpp
    
    int getBackEdges(vector<PatchEdge*> &edges)

Sets ``edges`` to the set of back edges in this loop. It returns the
number of back edges that are in this loop.

.. code-block:: cpp
    
    bool getContainedLoops(vector<PatchLoop*> &loops)

Returns a vector of loops that are nested under this loop.

.. code-block:: cpp
    
    bool getOuterLoops(vector<PatchLoop*> &loops)

Returns a vector of loops that are directly nested under this loop.

.. code-block:: cpp
    
    bool getLoopBasicBlocks(vector<PatchBlock*> &blocks)

Fills ``blocks`` with all basic blocks in the loop

.. code-block:: cpp
    
    bool getLoopBasicBlocksExclusive(vector<PatchBlock*> &blocks)

Fills ``blocks`` with all basic blocks in this loop, excluding the
blocks of its sub loops.

.. code-block:: cpp
    
    bool hasBlock(PatchBlock *b);

Returns ``true`` if this loop contains basic block ``b``.

.. code-block:: cpp
    
    bool hasBlockExclusive(PatchBlock *b);

Returns ``true`` if this loop contains basic block ``b`` and ``b`` is
not in its sub loops.

.. code-block:: cpp
    
    bool hasAncestor(PatchLoop *loop)

Returns true if this loop is a descendant of the given loop.

.. code-block:: cpp
    
    PatchFunction * getFunction();

Returns the function that this loop is in.

.. _sec-3.2.13:

PatchLoopTreeNode
=================

**Declared in**: PatchCFG.h

The PatchLoopTreeNode class provides a tree interface to a collection of
instances of class PatchLoop contained in a function. The structure of
the tree follows the nesting relationship of the loops in a function.
Each PatchLoopTreeNode contains a pointer to a loop (represented by
PatchLoop), and a set of sub-loops (represented by other
PatchLoopTreeNode objects). The ``loop`` field at the root node is
always ``NULL`` since a function may contain multiple outer loops. The
``loop`` field is never ``NULL`` at any other node since it always
corresponds to a real loop. Therefore, the outer most loops in the
function are contained in the vector of ``children`` of the root.

Each instance of PatchLoopTreeNode is given a name that indicates its
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

The ``foo`` function will have a root PatchLoopTreeNode, containing a
NULL loop entry and two PatchLoopTreeNode children representing the
functions outermost loops. These children would have names ``loop_1``
and ``loop_2``, respectively representing the ``x`` and ``i`` loops.
``loop_2`` has no children. ``loop_1`` has two child PatchLoopTreeNode
objects, named ``loop_1.1`` and ``loop_1.2``, respectively representing
the ``y`` and ``z`` loops.


.. code-block:: cpp
    
    PatchLoop *loop;

The PatchLoop instance it points to.

.. code-block:: cpp
    
    std::vector<PatchLoopTreeNode *> children;

The PatchLoopTreeNode instances nested within this loop.

.. code-block:: cpp
    
    const char * name();

Returns the hierarchical name of this loop.

.. code-block:: cpp
    
    const char * getCalleeName(unsigned int i)

Returns the function name of the ith callee.

.. code-block:: cpp
    
    unsigned int numCallees()

Returns the number of callees contained in this loop’s body.

.. code-block:: cpp
    
    bool getCallees(vector<PatchFunction *> &v);

Fills ``v`` with a vector of the functions called inside this loop.

.. code-block:: cpp
    
    PatchLoop * findLoop(const char *name);

Looks up a loop by the hierarchical name

.. _sec-3.1: