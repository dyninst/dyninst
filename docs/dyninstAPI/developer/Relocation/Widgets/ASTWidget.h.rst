.. _`sec:ASTWidget.h`:

ASTWidget.h
###########

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: ASTWidget : public Widget

  .. cpp:type:: boost::shared_ptr<ASTWidget> Ptr
  .. cpp:function:: static Ptr create(AstNodePtr, instPoint *)
  .. cpp:function:: ASTWidget(AstNodePtr a, instPoint *p)
  .. cpp:function:: bool generate(const codeGen &, const RelocBlock *, CodeBuffer &)
  .. cpp:function:: TrackerElement *tracker() const
  .. cpp:function:: virtual ~ASTWidget()
  .. cpp:function:: virtual std::string format() const
  .. cpp:member:: private AstNodePtr ast_
  .. cpp:member:: private instPoint *point_

    We need this for liveness


.. cpp:struct:: AstPatch : public Patch

  .. cpp:function:: private AstPatch(AstNodePtr a, instPoint *b)
  .. cpp:function:: private virtual bool apply(codeGen &gen, CodeBuffer *buf)
  .. cpp:function:: private virtual unsigned estimate(codeGen &templ)
  .. cpp:function:: private virtual ~AstPatch()
  .. cpp:member:: private AstNodePtr ast
  .. cpp:member:: private instPoint *point
