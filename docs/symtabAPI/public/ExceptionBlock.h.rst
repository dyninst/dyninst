.. _`sec:ExceptionBlock.h`:

ExceptionBlock.h
################

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: ExceptionBlock : public AnnotatableSparse

  **A C++ try/catch block**

  Currently only used on Linux.

  .. note::
  
    Accessors provide consistent access to the *original* offsets.
    We allow this to be updated (e.g. to account for relocated code

  .. cpp:function:: ExceptionBlock(Offset tStart, unsigned tSize, Offset cStart)
  .. cpp:function:: ExceptionBlock(Offset cStart)
  .. cpp:function:: ExceptionBlock(const ExceptionBlock &eb) = default
  .. cpp:function:: ExceptionBlock() = default
  .. cpp:function:: bool hasTry() const
  .. cpp:function:: Offset tryStart() const
  .. cpp:function:: Offset tryEnd() const
  .. cpp:function:: Offset trySize() const
  .. cpp:function:: Offset catchStart() const
  .. cpp:function:: bool contains(Offset a) const
  .. cpp:function:: void setTryStart(Offset ts)
  .. cpp:function:: void setTryEnd(Offset te)
  .. cpp:function:: void setCatchStart(Offset cs)
  .. cpp:function:: void setFdeStart(Offset fs)
  .. cpp:function:: void setFdeEnd(Offset fe)
  .. cpp:function:: std::ostream &operator<<(std::ostream &os, const ExceptionBlock &q)

Class ExceptionBlock
--------------------

This class represents an exception block present in the object file.
This class gives all the information pertaining to that exception block.

=========== =========== ============================================
Method name Return type Method description
=========== =========== ============================================
hasTry      bool        True if the exception block has a try block.
tryStart    Offset      Start of the try block if it exists, else 0.
tryEnd      Offset      End of the try block if it exists, else 0.
trySize     Offset      Size of the try block if it exists, else 0.
catchStart  Offset      Start of the catch block.
=========== =========== ============================================

.. code-block:: cpp

    bool contains(Offset addr) const

This method returns ``true`` if the offset ``addr`` is contained with in
the try block. If there is no try block associated with this exception
block or the offset does not fall within the try block, it returns
``false``.