.. _`sec-dev:indexed_symbols.h`:

indexed_symbols.h
#################

.. cpp:namespace:: Dyninst::SymtabAPI::dev

.. cpp:struct:: indexed_symbols

  .. cpp:type:: Dyninst::dyn_c_hash_map<st::Symbol *, Dyninst::Offset> master_t
  .. cpp:type:: std::vector<st::Symbol *> symvec_t
  .. cpp:type:: Dyninst::dyn_c_hash_map<Dyninst::Offset, symvec_t> by_offset_t
  .. cpp:type:: Dyninst::dyn_c_hash_map<std::string, symvec_t> by_name_t
  .. cpp:member:: master_t master
  .. cpp:member:: by_offset_t by_offset
  .. cpp:member:: by_name_t by_mangled
  .. cpp:member:: by_name_t by_pretty
  .. cpp:member:: by_name_t by_typed

  .. cpp:function:: bool insert(st::Symbol *s)

    Only inserts if not present. Returns whether it inserted. Operations on the indexed_symbols compound table.

  .. cpp:function:: void clear()

    Clears the table. Do not use in parallel.

  .. cpp:function:: void erase(st::Symbol *s)

    Erases Symbols from the table. Do not use in parallel.

  .. cpp:function:: iterator begin()
  .. cpp:function:: iterator end()

.. cpp:class:: indexed_symbols::iterator


    Iterator for the symbols. Do not use in parallel.

  .. cpp:member:: private master_t::iterator m
  .. cpp:type:: iterator_category = std::forward_iterator_tag
  .. cpp:type:: value_type = st::Symbol *
  .. cpp:type:: difference_type = std::ptrdiff_t
  .. cpp:type:: pointer = value_type *
  .. cpp:type:: reference = value_type &
  .. cpp:function:: iterator(master_t::iterator i)
  .. cpp:function:: bool operator==(const iterator &x) const
  .. cpp:function:: bool operator!=(const iterator &x) const
  .. cpp:function:: st::Symbol *const &operator*() const
  .. cpp:function:: st::Symbol *const *operator->() const
  .. cpp:function:: iterator &operator++()
  .. cpp:function:: iterator operator++(int)
