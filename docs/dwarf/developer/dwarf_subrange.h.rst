.. _`sec:dwarf_subrange.h`:

dwarf_subrange.h
################



.. cpp:struct:: dwarf_error

.. cpp:struct:: dwarf_result

  .. cpp:member:: boost::optional<Dwarf_Word> value
  .. cpp:member:: bool error = false
  .. cpp:function:: dwarf_result() = default
  .. cpp:function:: dwarf_result(Dwarf_Word t)
  .. cpp:function:: dwarf_result(dwarf_error)
  .. cpp:function:: explicit operator bool() const


.. cpp:struct:: dwarf_bounds

  .. cpp:member:: dwarf_result lower
  .. cpp:member:: dwarf_result upper
