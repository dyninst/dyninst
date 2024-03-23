.. _`sec:Transformer.h`:

Transformer.h
#############

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: Transformer

  One of the things a Transformer 'returns' (modifies, really) is a list of where
  we require patches (branches from original code to new code).

  This list is prioritized - Required,  Suggested, and Not Required.

  Required means we have proof that a patch is necessary for correct control flow.
  Suggested means that, assuming correct parsing, no patch is necessary. Not Required
  means that even with incorrect parsing no patch is necessary. ... not sure how we can
  have that, but hey, we might as well design it in.

  .. cpp:type:: boost::shared_ptr<Widget> WidgetPtr
  .. cpp:type:: std::list<WidgetPtr> WidgetList
  .. cpp:type:: std::map<block_instance*, RelocBlock*> RelocBlockMap
  .. cpp:function:: virtual bool processGraph(RelocGraph *)
  .. cpp:function:: virtual bool process(RelocBlock *, RelocGraph *) = 0
  .. cpp:function:: virtual ~Transformer()
