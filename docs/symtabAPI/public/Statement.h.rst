.. _`sec:Statement.h`:

Statement.h
###########

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: Statement : public AddressRange

  **The base representation of line information**

  .. attention:: Users should not create or modify statements.

  .. cpp:type:: Statement *Ptr
  .. cpp:type:: const Statement *ConstPtr

  .. cpp:function:: Statement()

      Creates an empty statement covering no addresses.

  .. cpp:function:: Offset startAddr() const

      Returns the starting address of this line in the file.

  .. cpp:function:: Offset endAddr() const

      Returns the ending address of this line in the file.

  .. cpp:function:: const std::string &getFile() const

      Returns the name of the file that contains the statement.

  .. cpp:function:: unsigned int getLine() const

      Returns the line number for this statement.

  .. cpp:function:: unsigned int getColumn() const

      Returns the starting column number for this statement.


Notes
=====

.. cpp:type:: Statement LineNoTuple

  For backwards compatibility only. Do not use in new code.
