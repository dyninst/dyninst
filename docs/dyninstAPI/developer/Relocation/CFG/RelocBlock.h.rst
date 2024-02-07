.. _`sec:RelocBlock.h`:

RelocBlock.h
############

.. cpp:namespace:: Dyninst::Relocation

.. cpp:type:: boost::shared_ptr<CFWidget> CFWidgetPtr
.. cpp:type:: boost::shared_ptr<Widget> WidgetPtr



.. cpp:class:: RelocBlock

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
  .. cpp:function:: private void preserveBlockGap()
  .. cpp:function:: private std::pair<bool, Address> getJumpTarget()
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

