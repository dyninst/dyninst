.. _`sec-dev:Statement.h`:

Statement.h
###########

.. cpp:namespace:: Dyninst::SymtabAPI::dev

.. cpp:class:: Statement : public AddressRange

  **The base representation of line information**

  .. cpp:function:: StringTablePtr getStrings_() const
  .. cpp:function:: void setStrings_(StringTablePtr strings_)
  .. cpp:function:: bool operator==(const Statement &cmp) const
  .. cpp:function:: bool operator==(Offset addr) const
  .. cpp:function:: bool operator<(Offset addr) const
  .. cpp:function:: bool operator>(Offset addr) const
  .. cpp:function:: unsigned int getFileIndex() const
