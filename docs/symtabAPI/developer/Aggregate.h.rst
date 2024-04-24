.. _`sec-dev:Aggregate.h`:

Aggregate.h
###########

.. cpp:namespace:: Dyninst::SymtabAPI::dev

.. cpp:class:: Aggregate

  .. cpp:member:: protected Module *module_

      Module we keep here so we can have the correct "primary"

  .. cpp:member:: protected mutable dyn_mutex lock_
  .. cpp:member:: protected std::vector<Symbol *> symbols_
  .. cpp:member:: protected Symbol *firstSymbol

      cached for speed

  .. cpp:member:: protected Offset offset_

      cached for speed

  .. cpp:function:: protected Aggregate()
  .. cpp:function:: protected Aggregate(Symbol *sym)
  .. cpp:function:: protected Aggregate(Module *m)
  .. cpp:function:: protected bool addMangledNameInternal(std::string name, bool isPrimary, bool demangle)
  .. cpp:function:: protected void print(std::ostream &) const
  .. cpp:function:: protected bool removeSymbolInt(Symbol *sym)
  .. cpp:function:: protected virtual bool changeSymbolOffset(Symbol *sym)
