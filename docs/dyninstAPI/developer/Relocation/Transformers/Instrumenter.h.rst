.. _`sec:dyninstAPI:Instrumenter.h`:

Instrumenter.h
##############

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: Instrumenter : public Transformer

  .. cpp:function:: virtual bool process(RelocBlock *cur, RelocGraph *)
  .. cpp:function:: Instrumenter()
  .. cpp:function:: virtual ~Instrumenter()

  .. cpp:type:: private std::pair<RelocBlock *, When> InsertPoint
  .. cpp:type:: private std::map<InsertPoint, std::list<RelocBlock *> > EdgeRelocBlocks
  .. cpp:type:: private boost::shared_ptr<CFWidget> CFWidgetPtr
  .. cpp:function:: private bool funcEntryInstrumentation(RelocBlock *trace, RelocGraph *cfg)

    The instrumenters that can add new RelocBlocks have the CFG as an argument

  .. cpp:function:: private bool edgeInstrumentation(RelocBlock *trace, RelocGraph *cfg)
  .. cpp:function:: private bool postCallInstrumentation(RelocBlock *trace, RelocGraph *cfg)
  .. cpp:function:: private bool funcExitInstrumentation(RelocBlock *trace, RelocGraph *cfg)
  .. cpp:function:: private bool blockEntryInstrumentation(RelocBlock *trace)
  .. cpp:function:: private bool blockExitInstrumentation(RelocBlock *trace)
  .. cpp:function:: private bool preCallInstrumentation(RelocBlock *trace)
  .. cpp:function:: private bool insnInstrumentation(RelocBlock *trace)
  .. cpp:function:: private bool handleUnconditionalExitInstrumentation(RelocBlock *trace, RelocGraph *cfg, instPoint *exit)
  .. cpp:function:: private bool handleCondIndExits(RelocBlock *trace, RelocGraph *cfg, instPoint *exit)
  .. cpp:function:: private bool handleCondDirExits(RelocBlock *trace, RelocGraph *cfg, instPoint *exit)
  .. cpp:function:: private WidgetPtr makeInstrumentation(PatchAPI::Point *point)


.. cpp:enum:: Instrumenter::When 

  .. cpp:enumerator:: Before
  .. cpp:enumerator:: After


.. cpp:struct:: Instrumenter::CallFallthroughPredicate

  .. cpp:function:: private bool operator()(RelocEdge *e)


.. cpp:struct:: Instrumenter::EdgePredicate

  .. cpp:function:: private EdgePredicate(edge_instance *e)
  .. cpp:function:: private bool operator()(RelocEdge *e)
  .. cpp:member:: private edge_instance *e_
