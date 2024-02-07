.. _`sec:BPatch_statement.h`:

BPatch_statement.h
##################

.. cpp:class:: BPatch_statement

  .. cpp:function:: BPatch_module* module()

    Returns the BPatch_module that contains this statement

  .. cpp:function:: int lineNumber()

    Returns the line number of this statement

  .. cpp:function:: int lineOffset()

    Returns the line offset of this statement (its start column in the source file).

    This may not be supported on all platforms. Returns -1 if not supported.

  .. cpp:function:: const char* fileName()

    Returns the name of the file that contains this statement

  .. cpp:function:: void *startAddr()

    Returns the starting address of this statement

  .. cpp:function:: void *endAddr()

    Returns the last address associated with this statement
