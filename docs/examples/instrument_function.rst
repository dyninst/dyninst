.. _`example:dyninstapi-instr-func`:

Instrumenting a Function
########################

A mutator program must create a single :cpp:class:`BPatch` instance.
This object is used to access functions and information that are global
to the library. It must not be destroyed until the mutator has completely
finished using the library. All instrumentation is done through
:cpp:class:`BPatch_addressSpace` that allows working with both dynamic and
static instrumentation with the same mutator code. This is a key feature
of Dyninst.

This example demonstrates instrumentation by inserting a variable to count the
number of times the function ``InterestingProcedure`` is called. This is similar
to how a non-sampling code coverage tool might work.

``startInstrumenting`` allows dynamic instrumentation by either creating or
attaching to a running process as well as static instrumentation for a file
from disk. In this example, a process is created for dynamic instrumentation.

Once the address space has been created, ``findPoint`` searches for a place
where Dyninst can insert instrumentation code. ``BPatch_entry`` tells Dyninst
to insert code at the beginning of the function.

``createAndInsertSnippet`` creates an AST snippet for a variable
``myCounter`` of type ``int`` and another snippet for add 1 to the
variable. This forms the C-like syntax ``int myCounter; myCounter++;``
and inserts the relevant code into the binary. It is important to note
that the AST abstraction works for *any* platform supported by Dyninst.
A single mutator can work for many different combinations of binaries,
computer architectures, and OSes.

``findPoint`` is then used again to find an instrumentation point where
the ``main`` function exits. ``createAndInsertSnippet2`` generates an
AST snippet to insert a call to the C standard library ``printf`` function
to display the value of ``myCounter`` previously inserted. Dyninst can
readily generate code that results from interactions of separate snippets.

``finishInstrumenting`` finalizes the instrumentation by either resuming
execution of the process created or attached to in ``startInstrumenting``
or by writing a new binary to disk (for static instrumentation).

..  rli:: https://raw.githubusercontent.com/dyninst/examples/master/instrumentAFunction/instrumenting_a_function.cpp
    :language: cpp
    :linenos:
