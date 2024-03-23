.. _`sec:ThunkData.h`:

ThunkData.h
###########

.. cpp:struct:: ThunkInfo

  .. cpp:member:: Dyninst::MachRegister reg
  .. cpp:member:: Dyninst::Address value
  .. cpp:member:: Dyninst::ParseAPI::Block *block

.. cpp:type:: std::map<Dyninst::Address, ThunkInfo> ThunkData

.. cpp:struct:: ReachFact

  .. cpp:member:: ThunkData& thunks
  .. cpp:member:: std::map<Dyninst::ParseAPI::Block*, set<Dyninst::ParseAPI::Block*>> thunk_ins
  .. cpp:member:: std::map<Dyninst::ParseAPI::Block*, set<Dyninst::ParseAPI::Block*>> thunk_outs

  .. cpp:function:: void ReverseDFS(Dyninst::ParseAPI::Block *cur, set<Dyninst::ParseAPI::Block*>& visited)
  .. cpp:function:: void NaturalDFS(Dyninst::ParseAPI::Block *cur, set<Dyninst::ParseAPI::Block*>& visited)
  .. cpp:function:: void ReachBlocks()
  .. cpp:function:: ReachFact(ThunkData& t)
