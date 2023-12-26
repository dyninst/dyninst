BPatch_basicBlock.h
===================

``BPatch_basicBlock``
---------------------
.. cpp:namespace:: BPatch_basicBlock

.. cpp:class:: BPatch_basicBlock
   
   The **BPatch_basicBlock** class represents a basic block in the
   application being instrumented. Objects of this class representing the
   blocks within a function can be obtained using the BPatch_flowGraph
   object for the function. BPatch_basicBlock includes methods for
   navigating through the control flow graph of the containing function.
   
   .. cpp:function:: void getSources(std::vector<BPatch_basicBlock*>&)
      
      Fills the given vector with the list of predecessors for this basic
      block (i.e, basic blocks that have an outgoing edge in the control flow
      graph leading to this block).
      
   .. cpp:function:: void getTargets(std::vector<BPatch_basicBlock*>&)
      
      Fills the given vector with the list of successors for this basic block
      (i.e, basic blocks that are the destinations of outgoing edges from this
      block in the control flow graph).
      
   .. cpp:function:: void getOutgoingEdges(std::vector<BPatch_edge *> &out)
      
      Fill out with all of the control flow edges that leave this basic block.
      
   .. cpp:function:: void getIncomingEdges(std::vector<BPatch_edge *> &inc)
      
      Fills inc with all of the control flow edges that point to this basic
      block.
      
   .. cpp:function:: bool getInstructions(std::vector<Dyninst::InstructionAPI::Instruction>&)
      
   .. cpp:function:: bool getInstructions(std::vector <std::pair<Dyninst::InstructionAPI::Instruction,Address> >&)
      
      Fills the given vector with InstructionAPI Instruction objects
      representing the instructions in this basic block, and returns true if
      successful. See the InstructionAPI Programmer’s Guide for details. The
      second call also returns the address each instruction starts at.
      
   .. cpp:function:: bool dominates(BPatch_basicBlock*)
      
      This function returns true if the argument is pre-dominated in the
      control flow graph by this block, and false if it is not.
      
   .. cpp:function:: BPatch_basicBlock* getImmediateDominator()
      
      Return the basic block that immediately pre-dominates this block in the
      control flow graph.
      
   .. cpp:function:: void getImmediateDominates(std::vector<BPatch_basicBlock*>&)
      
      Fill the given vector with a list of pointers to the basic blocks that
      are immediately dominated by this basic block in the control flow graph.
      
   .. cpp:function:: void getAllDominates(std::set<BPatch_basicBlock*>&)
      
   .. cpp:function:: void getAllDominates(BPatch_Set<BPatch_basicBlock*>&)
      
      Fill the given set with pointers to all basic blocks that are dominated
      by this basic block in the control flow graph.
      
   .. cpp:function:: bool getSourceBlocks(std::vector<BPatch_sourceBlock*>&)
      
      Fill the given vector with pointers to the source blocks contributing to
      this basic block’s instruction sequence.
      
   .. cpp:function:: int getBlockNumber()
      
      Return the ID number of this basic block. The ID numbers are consecutive
      from 0 to *n-1,* where *n* is the number of basic blocks in the flow
      graph to which this basic block belongs.
      
   .. cpp:function:: std::vector<BPatch_point *> findPoint(const std::set<BPatch_opCode>&ops)
      
   .. cpp:function:: std::vector<BPatch_point *> findPoint(const BPatch_Set<BPatch_opCode>&ops)
      
      Find all points in the basic block that match the given operation.
      
   .. cpp:function:: BPatch_point *findEntryPoint()
      
   .. cpp:function:: BPatch_point *findExitPoint()
      
      Find the entry or exit point of the block.
      
   .. cpp:function:: unsigned long getStartAddress()
      
      This function returns the starting address of the basic block. The
      address returned is an absolute address.
      
   .. cpp:function:: unsigned long getEndAddress()
      
      This function returns the end address of the basic block. The address
      returned is an absolute address.
      
   .. cpp:function:: unsigned long getLastInsnAddress()
      
      Return the address of the last instruction in a basic block.
      
   .. cpp:function:: bool isEntryBlock()
      
      This function returns true if this basic block is an entry block into a
      function.
      
   .. cpp:function:: bool isExitBlock()
      
      This function returns true if this basic block is an exit block of a
      function.
      
   .. cpp:function:: unsigned size()
      
      Return the size of a basic block. The size is defined as the difference
      between the end address and the start address of the basic block.