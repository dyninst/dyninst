.. _`sec:DecoderWindows.h`:

DecoderWindows.h
################

.. cpp:class:: DecoderWindows : public Decoder

  .. cpp:function:: DecoderWindows()
  .. cpp:function:: virtual ~DecoderWindows()
  .. cpp:function:: virtual unsigned getPriority() const
  .. cpp:function:: virtual bool decode(ArchEvent *ae, std::vector<Event::ptr> &events)
  .. cpp:function:: Dyninst::Address adjustTrapAddr(Dyninst::Address address, Dyninst::Architecture arch)