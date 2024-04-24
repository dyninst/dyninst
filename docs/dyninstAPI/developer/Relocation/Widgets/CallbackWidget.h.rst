.. _`sec:CallbackWidget.h`:

CallbackWidget.h
################

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: CallbackWidget : public Widget

  .. cpp:type:: boost::shared_ptr<CallbackWidget> Ptr
  .. cpp:function:: static Ptr create(Patch *patch)

    I believe I can patch in the current code generation system here...

  .. cpp:function:: CallbackWidget(Patch *p)
  .. cpp:function:: bool generate(const codeGen &, const RelocBlock *, CodeBuffer &)
  .. cpp:function:: TrackerElement *tracker() const
  .. cpp:function:: virtual ~CallbackWidget()
  .. cpp:function:: virtual std::string format() const
  .. cpp:member:: private Patch *patch_
