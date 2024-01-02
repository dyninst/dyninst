.. _`sec-dev:CFG.h`:

CFG.h
#####

.. cpp:namespace:: Dyninst::ParseAPI::dev

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

.. cpp:enum:: StackTamper

  .. cpp:enumerator:: TAMPER_UNSET
  .. cpp:enumerator:: TAMPER_NONE
  .. cpp:enumerator:: TAMPER_REL
  .. cpp:enumerator:: TAMPER_ABS
  .. cpp:enumerator:: TAMPER_NONZERO

Notes
=====

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
