.. _`sec-dev:walker.h`:

walker.h
########

.. cpp:namespace:: Dyninst::Stackwalker::dev

.. cpp:class:: WalkerSet

  .. cpp:function:: static WalkerSet *newWalkerSet()
  .. cpp:function:: ~WalkerSet()
  .. cpp:type:: std::set<Walker *>::iterator iterator
  .. cpp:type:: std::set<Walker *>::const_iterator const_iterator
  .. cpp:function:: iterator begin()
  .. cpp:function:: iterator end()
  .. cpp:function:: iterator find(Walker *)
  .. cpp:function:: const_iterator begin() const
  .. cpp:function:: const_iterator end() const
  .. cpp:function:: const_iterator find(Walker *) const
  .. cpp:function:: std::pair<iterator, bool> insert(Walker *walker)
  .. cpp:function:: void erase(iterator i)
  .. cpp:function:: bool empty() const
  .. cpp:function:: size_t size() const
  .. cpp:function:: bool walkStacks(CallTree &tree, bool walk_initial_only = false) const
