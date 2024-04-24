.. _`sec:Relocation.h`:

Relocation.h
############

A quick header file defining externally visible types from the PatchAPI
namespace. This allows us to reduce cross-pollination of header files.
This avoids the requirement to include CodeMover.h

.. cpp:namespace:: Dyninst::Relocation

.. cpp:type:: boost::shared_ptr<Dyninst::Relocation::CodeMover> CodeMoverPtr
.. cpp:type:: boost::shared_ptr<Dyninst::Relocation::SpringboardBuilder> SpringboardBuilderPtr
