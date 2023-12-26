.. _`sec:Decoder.h`:

Decoder.h
=========

.. cpp:namespace:: Dyninst::ProcControlAPI

.. cpp:class:: Decoder

   .. cpp:function:: virtual ~Decoder()

   .. cpp:function:: virtual unsigned getPriority() const = 0;

   .. cpp:function:: virtual bool decode(ArchEvent *archE, std::vector<Event::ptr> &events) = 0;
