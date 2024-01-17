.. _`sec:indexed_modules.h`:

indexed_modules.h
#################

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: indexed_modules

  .. cpp:function:: void insert(Module *m)
  .. cpp:function:: bool contains(Module *m) const
  .. cpp:function:: std::vector<Module *> find(std::string const& name) const
  .. cpp:function:: Module *find(Dyninst::Offset offset) const
  .. cpp:function:: bool empty() const
  .. cpp:function:: iterator begin()
  .. cpp:function:: iterator end()
  .. cpp:function:: const_iterator cbegin() const
  .. cpp:function:: const_iterator cend() const


.. cpp:namespace-push:: detail

.. cpp:struct:: hash

  A Module is uniquely identified by its name and offset

  .. cpp:function:: size_t operator()(Module *m) const


.. cpp:struct:: equal

  .. cpp:function:: bool operator()(Module *m1, Module *m2) const

.. cpp:namespace-pop::
  