.. _ExceptionBlock:

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

bool contains(Offset addr) const
