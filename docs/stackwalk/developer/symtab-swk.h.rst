.. _`sec:symtab-swk.h`:

symtab-swk.h
############

.. cpp:namespace:: Dyninst::Stackwalker

.. cpp:class:: SymtabWrapper

  .. cpp:function:: protected SymtabWrapper()
  .. cpp:function:: static Symtab *getSymtab(std::string filename)
  .. cpp:function:: static void notifyOfSymtab(Symtab *symtab, std::string name)
  .. cpp:function:: ~SymtabWrapper()
