.. _`sec-dev:SymLiteCodeSource.h`:

SymLiteCodeSource.h
###################

.. cpp:namespace:: Dyninst::ParseAPI::dev

.. cpp:class:: SymReaderCodeSource : public CodeSource

  .. cpp:function:: void removeHint(Hint)
  .. cpp:function:: static void addNonReturning(std::string func_name)
  .. cpp:function:: void print_stats() const
  .. cpp:function:: bool have_stats() const
  .. cpp:function:: void incrementCounter(const std::string& name) const
  .. cpp:function:: void addCounter(const std::string& name, int num) const
  .. cpp:function:: void decrementCounter(const std::string& name) const
