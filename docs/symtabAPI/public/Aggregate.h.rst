.. _`sec:Aggregate.h`:

Aggregate.h
###########

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: Aggregate

  **A common superclass for all Symbol aggregates**

  .. cpp:type:: name_iter = boost::transform_iterator<decltype(std::mem_fn(&Symbol::getPrettyName)), std::vector<Symbol*>::const_iterator>
  .. cpp:function:: virtual ~Aggregate()
  .. cpp:function:: virtual Offset   getOffset() const
  .. cpp:function:: virtual unsigned getSize() const
  .. cpp:function:: Module * getModule() const
  .. cpp:function:: Region * getRegion() const
  .. cpp:function:: bool addSymbol(Symbol *sym)
  .. cpp:function:: virtual bool removeSymbol(Symbol *sym) = 0
  .. cpp:function:: bool getSymbols(std::vector<Symbol *> &syms) const
  .. cpp:function:: Symbol *getFirstSymbol() const
  .. cpp:function:: name_iter mangled_names_begin() const
  .. cpp:function:: name_iter mangled_names_end() const
  .. cpp:function:: name_iter pretty_names_begin() const
  .. cpp:function:: name_iter pretty_names_end() const
  .. cpp:function:: name_iter typed_names_begin() const
  .. cpp:function:: name_iter typed_names_end() const
  .. cpp:function:: virtual bool addMangledName(std::string name, bool isPrimary, bool isDebug=false)
  .. cpp:function:: virtual bool addPrettyName(std::string name, bool isPrimary, bool isDebug=false)
  .. cpp:function:: virtual bool addTypedName(std::string name, bool isPrimary, bool isDebug=false)
  .. cpp:function:: bool setSize(unsigned size)
  .. cpp:function:: bool setOffset(unsigned offset)
  .. cpp:function:: bool operator==(const Aggregate &a)
  .. cpp:function:: std::ostream& operator<<(std::ostream &os, Aggregate const& a)
