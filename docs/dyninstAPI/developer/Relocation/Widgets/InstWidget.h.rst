.. _`sec:InstWidget.h`:

InstWidget.h
############

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: InstWidget : public Widget

  .. cpp:type:: boost::shared_ptr<InstWidget> Ptr
  .. cpp:function:: static Ptr create(instPoint *i)

    I believe I can patch in the current code generation system here...

  .. cpp:function:: InstWidget(instPoint *i)

    This sucks. It seriously sucks. But hey... this points to all the baseTramps
    with instrumentation at this point. This can be 0, 1, or 2 - 2 if we have post
    instruction + pre instruction instrumentation.

  .. cpp:function:: bool empty() const
  .. cpp:function:: bool generate(const codeGen &, const RelocBlock *, CodeBuffer &)
  .. cpp:function:: TrackerElement *tracker() const
  .. cpp:function:: virtual ~InstWidget()
  .. cpp:function:: virtual std::string format() const
  .. cpp:member:: private instPoint *point_


.. cpp:struct:: InstWidgetPatch : public Patch

  .. cpp:function:: private InstWidgetPatch(baseTramp *a)
  .. cpp:function:: private virtual bool apply(codeGen &gen, CodeBuffer *buf)
  .. cpp:function:: private virtual unsigned estimate(codeGen &templ)
  .. cpp:function:: private virtual ~InstWidgetPatch()
  .. cpp:member:: private baseTramp *tramp


.. cpp:struct:: RemovedInstWidgetPatch : public Patch

  .. cpp:function:: private RemovedInstWidgetPatch(baseTramp *a)
  .. cpp:function:: private virtual bool apply(codeGen &gen, CodeBuffer *)
  .. cpp:function:: private virtual unsigned estimate(codeGen &templ)
  .. cpp:function:: private virtual ~RemovedInstWidgetPatch()
  .. cpp:member:: private baseTramp *tramp
