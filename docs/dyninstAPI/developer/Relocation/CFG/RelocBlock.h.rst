.. _`sec:RelocBlock.h`:

RelocBlock.h
############

.. cpp:namespace:: Dyninst::Relocation

.. cpp:type:: boost::shared_ptr<CFWidget> CFWidgetPtr
.. cpp:type:: boost::shared_ptr<Widget> WidgetPtr



.. cpp:class:: RelocBlock

  A RelocBlock is a representation for instructions and instrumentation with
  a single entry point and (possibly) multiple exit points. For simplicity,
  we initially map a RelocBlock to a basic block. However, edge instrumentation
  or post-call padding (in defensive mode) may add additional RelocBlocks. Also,
  a RelocBlock may exit early if instrumentation explicitly branches out of a
  RelocBlock.

  A RelocBlock is represented as a list of Widgets. An Widget represents a single
  code generation unit: an instruction, an instrumentation sequence, and
  the like.

  Each RelocBlock ends in a distinguished "CFWidget" that tracks the successors
  of the RelocBlock. Arguably this information should be stored in the RelocBlock itself,
  but then we'd still need the code generation techniques in a CFWidget anyway.

  .. cpp:type:: int Label
  .. cpp:member:: static int RelocBlockID
  .. cpp:type:: std::list<WidgetPtr> WidgetList
  .. cpp:function:: static RelocBlock *createReloc(block_instance *block, func_instance *func)

    Standard creation

  .. cpp:function:: static RelocBlock *createInst(instPoint *point, Address a, block_instance *block, func_instance *func)

    Instpoint creation

  .. cpp:function:: static RelocBlock *createStub(block_instance *block, func_instance *func)

    Stub creation we're creating an empty RelocBlock associated with some other block/function/thing.

  .. cpp:function:: RelocBlock *next()
  .. cpp:function:: RelocBlock *prev()
  .. cpp:function:: ~RelocBlock()
  .. cpp:function:: void setNext(RelocBlock *next)
  .. cpp:function:: void setPrev(RelocBlock *prev)
  .. cpp:function:: bool linkRelocBlocks(RelocGraph *cfg)
  .. cpp:function:: bool determineSpringboards(PriorityMap &p)
  .. cpp:function:: void determineNecessaryBranches(RelocBlock *successor)

      We put in edges for everything - jumps, fallthroughs, you name
      it. Some of these can be obviated by code layout. Instead of trying
      to precompute which branches are needed, we do a final
      pre-generation pass to run through and see which branches we need
      based on where we plan to lay blocks out.

      Side note: necessary is set on a per-Target basis.

  .. cpp:function:: Address origAddr() const
  .. cpp:function:: int id() const
  .. cpp:function:: func_instance *func() const
  .. cpp:function:: block_instance *block() const
  .. cpp:function:: mapped_object *obj() const
  .. cpp:function:: std::string format() const
  .. cpp:function:: Label getLabel() const
  .. cpp:function:: WidgetList &elements()
  .. cpp:function:: CFWidgetPtr &cfWidget()
  .. cpp:function:: void setCF(CFWidgetPtr cf)
  .. cpp:function:: bool applyPatches(codeGen &gen, bool &regenerate, unsigned &totalSize, int &shift)
  .. cpp:function:: bool extractTrackers(CodeTracker &)
  .. cpp:function:: bool generate(const codeGen &templ, CodeBuffer &buffer)

    Each RelocBlock generates a mixture of PIC and non-PIC code. For
    efficiency, we precompute the PIC code and generate function callbacks
    for the non-PIC code. Thus, what we hand into the code generator
    looks like

      |  PIC
      |  non-PIC callback
      |  PIC
      |  non-PIC callback
      |  PIC
      |  PIC
      |  ...

    This allows us to minimize unnecessary regeneration when we're trying
    to produce the final code sequence. This function generates the mixture
    of PIC (as a byte buffer) and non-PIC (in terms of Patch objects) sequences.

  .. cpp:function:: void setType(Type type)
  .. cpp:function:: Type type() const
  .. cpp:function:: bool finalizeCF()

    Set up the CFWidget with our out-edges

  .. cpp:function:: RelocEdges *ins()
  .. cpp:function:: RelocEdges *outs()
  .. cpp:function:: private RelocBlock(block_instance *block, func_instance *f)
  .. cpp:function:: private RelocBlock(Address a, block_instance *b, func_instance *f)

    Constructor for a trace inserted later

  .. cpp:function:: private RelocBlock(Address a, block_instance *b, func_instance *f, bool relocateType)
  .. cpp:function:: private void createCFWidget()
  .. cpp:function:: private void getPredecessors(RelocGraph *cfg)
  .. cpp:function:: private void getSuccessors(RelocGraph *cfg)
  .. cpp:function:: private void processEdge(EdgeDirection e, edge_instance *edge, RelocGraph *cfg)

    There's some tricky logic going on here. We want to create the following edges:

      1. All out-edges, as _someone_ has to create them
      2. In-edges that aren't from RelocBlocks; if it's from a RelocBlock we assume we'll
         get it in out-edge construction.

  .. cpp:function:: private void preserveBlockGap()
  .. cpp:function:: private std::pair<bool, Address> getJumpTarget()

    Do the raw computation to determine the target (if it exists) of a
    jump instruction that we may not have encoded in ParseAPI.

  .. cpp:function:: private bool isNecessary(TargetInt *target, ParseAPI::EdgeTypeEnum edgeType)
  .. cpp:member:: private Address origAddr_
  .. cpp:member:: private block_instance *block_
  .. cpp:member:: private func_instance *func_

    If we're a func-specific copy

  .. cpp:member:: private int id_
  .. cpp:member:: private Label label_
  .. cpp:member:: private bool origRelocBlock_
  .. cpp:member:: private WidgetList elements_
  .. cpp:member:: private CFWidgetPtr cfWidget_

    This is convienient to avoid tons of dynamic_cast equivalents

  .. cpp:member:: private RelocEdges inEdges_

    We're building a mini-CFG, so might as well make it obvious.  Also, this lets us reassign edges. We sort by edge type.

  .. cpp:member:: private RelocEdges outEdges_
  .. cpp:member:: private RelocBlock *prev_

    We use the standard code generation mechanism of having a doubly-linked list overlaid on a graph.
    Use the list to traverse in layout order use the graph to traverse in control flow order.

  .. cpp:member:: private RelocBlock *next_
  .. cpp:member:: private Type type_


.. cpp:enum:: RelocBlock::Type

  .. cpp:enumerator:: Relocated
  .. cpp:enumerator:: Instrumentation
  .. cpp:enumerator:: Stub


.. cpp:enum:: EdgeDirection

  .. cpp:enumerator:: InEdge
  .. cpp:enumerator:: OutEdge

.. c:macro:: DEFENSIVE_GAP_SIZE

  Some defensive binaries put gaps in after call instructions to try
  and confuse parsing; it's typically something like so:

  .. code:: asm

    call foo
    jmp offset

  where ``foo`` contains code that increments the stack pointer by one,
  and ``offset`` is the encoding of a legal instruction. We really need
  to preserve that gap if it exists, and to make life easy we bundle
  it into the CFWidget.

.. code:: cpp

  #if defined(arch_x86) || defined(arch_x86_64)
  # define DEFENSIVE_GAP_SIZE 10
  #else
  # define DEFENSIVE_GAP_SIZE 12
  #endif
