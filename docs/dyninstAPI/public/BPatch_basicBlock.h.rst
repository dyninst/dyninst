.. _`sec:BPatch_basicBlock.h`:

BPatch_basicBlock.h
###################

.. cpp:class:: BPatch_basicBlock
   
  **Machine code basic blocks**

  .. caution:: Users should not create basic blocks using its constructor. It is not safe.

  Basic blocks are used for reading purposes, not for inserting a new code to the machine
  executable other than instrumentation code.

  .. cpp:function:: BPatch_flowGraph *fg() const
  .. cpp:function:: block_instance *block() const
  .. cpp:function:: BPatch_function *func() const
  .. cpp:function:: func_instance *ifunc() const

  .. cpp:function:: BPatch_point *convertPoint(instPoint *pt)

    Returns the ``BPatch_point`` for ``pt``.

    Returns ``NULL`` if ``pt`` isn't in this block.

  .. cpp:function:: BPatch_function *getCallTarget()
  .. cpp:function:: BPatch_flowGraph * getFlowGraph() const

  .. cpp:function:: void getSources(std::vector<BPatch_basicBlock*>&)

    Returns the predecessors for this basic
    block (i.e, basic blocks that have an outgoing edge in the control flow
    graph leading to this block).

  .. cpp:function:: void getTargets(std::vector<BPatch_basicBlock*>&)

    Returns the successors for this basic block
    (i.e, basic blocks that are the destinations of outgoing edges from this
    block in the control flow graph).

  .. cpp:function:: bool dominates(BPatch_basicBlock*)

    Checks if the argument is pre-dominated in the
    control flow graph by this block, and false if it is not.

  .. cpp:function:: BPatch_basicBlock* getImmediateDominator()

    Returns the basic block that immediately pre-dominates this block in the
    control flow graph.

  .. cpp:function:: void getImmediateDominates(std::vector<BPatch_basicBlock*>&)

    Returns the basic blocks that are immediately dominated by this basic block in the control flow graph.

  .. cpp:function:: void getAllDominates(std::set<BPatch_basicBlock*>&)

      Returns all basic blocks dominated by the basic block.

      Does not return duplicates even if some points belong to multiple categories.

  .. cpp:function:: void getAllDominates(BPatch_Set<BPatch_basicBlock*>&)

      Returns all basic blocks dominated by the basic block

  .. cpp:function:: bool getSourceBlocks(std::vector<BPatch_sourceBlock*>&)

    Returns the source blocks contributing to this basic block’s instruction sequence.

  .. cpp:function:: int getBlockNumber()

    Returns the ID number of this basic block.

    The ID numbers are consecutive
    from ``0`` to ``n-1``, where ``n`` is the number of basic blocks in the flow
    graph to which this basic block belongs.

  .. cpp:function:: bool isEntryBlock()

    Checks if this basic block is an entry block into a function.

  .. cpp:function:: bool isExitBlock()

    Checks if this basic block is an exit block of a function.

  .. cpp:function:: unsigned size()

    Returns the size of a basic block.

    The size is defined as the difference between the end address and the start address
    of the basic block.

  .. cpp:function:: unsigned long getStartAddress()

    Returns the starting address of the basic block. The
    address returned is an absolute address.

  .. cpp:function:: unsigned long getLastInsnAddress()

    Returns the address of the last instruction in a basic block.

  .. cpp:function:: unsigned long getEndAddress()

    Returns the end address of the basic block. The address
    returned is an absolute address.

  .. cpp:function:: bool getAddressRange(void*& _startAddress, void*& _endAddress)

      Returns the start and end addresses of the basic block

  .. cpp:function:: BPatch_point* findEntryPoint()

      Returns point at the start of the basic block

  .. cpp:function:: BPatch_point* findExitPoint()

      Returns point at the start of the basic block

  .. cpp:function:: BPatch_Vector<BPatch_point*>* findPoint(const BPatch_Set<BPatch_opCode>& ops)

      Returns in ``ops`` the points within the basic block.

  .. cpp:function:: BPatch_Vector<BPatch_point*>* findPoint(const std::set<BPatch_opCode>& ops)

      Returns in ``ops`` the points within the basic block.

  .. cpp:function:: BPatch_Vector<BPatch_point*> * findPoint(bool(*filter)(Dyninst::InstructionAPI::Instruction))
  .. cpp:function:: BPatch_point * findPoint(Dyninst::Address addr)

  .. cpp:function:: bool getInstructions(std::vector<Dyninst::InstructionAPI::Instruction>& insns)

      Returns the instructions that belong to the block

  .. cpp:function:: bool getInstructions(std::vector <std::pair<Dyninst::InstructionAPI::Instruction,Address> >&)

    Fills the given vector with InstructionAPI Instruction objects
    representing the instructions in this basic block, and Checks if
    successful. See the InstructionAPI Programmer’s Guide for details. The
    second call also returns the address each instruction starts at.

  .. cpp:function:: bool getInstructions(std::vector<std::pair<Dyninst::InstructionAPI::Instruction, Dyninst::Address> >& insnInstances)
  .. cpp:function:: void getIncomingEdges(BPatch_Vector<BPatch_edge*> &inc)

    Returns the incoming edges

  .. cpp:function:: void getOutgoingEdges(BPatch_Vector<BPatch_edge*> &out)

    Returns the outgoming edges

  .. cpp:function:: operator Dyninst::ParseAPI::Block *() const
  .. cpp:function:: operator Dyninst::PatchAPI::PatchBlock *() const
  .. cpp:function:: int blockNo() const


.. cpp:function:: std::ostream& operator<<(std::ostream&,BPatch_basicBlock&)

.. cpp:function:: Block * Dyninst::ParseAPI::convert(const BPatch_basicBlock*)

.. cpp:function:: PatchBlock * Dyninst::PatchAPI::convert(const BPatch_basicBlock*)

.. cpp:struct:: template <> std::less<BPatch_basicBlock *>

  .. cpp:function:: bool operator()(const BPatch_basicBlock * const &l, const BPatch_basicBlock * const &r) const


.. cpp:struct:: insnPredicate

  .. cpp:type:: result_type = bool
  .. cpp:type:: argument_type = Dyninst::InstructionAPI::Instruction
  .. cpp:function:: virtual result_type operator()(argument_type arg) = 0
  .. cpp:function:: virtual ~insnPredicate()

