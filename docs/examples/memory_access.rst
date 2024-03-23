.. _`example:dyninstapi-instrumenting-memory-accesses`:

Instrumenting Memory Accesses
#############################

This example is similar to :ref:`example:dyninstapi-binary-analysis`, but
**all** functions in the binary are instrumented here.

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

The bulk of the work happens in ``instrumentMemoryAccess``.
There are two snippets useful for memory access instrumentation:
:cpp:class:`BPatch_effectiveAddressExpr` and :cpp:class:`BPatch_bytesAccessedExpr`.
Both have nullary constructors; the result of the snippet depends on the
instrumentation point where the snippet is inserted. ``BPatch_effectiveAddressExpr``
has type ``void*``, and ``BPatch_bytesAccessedExpr`` has type ``int``.

These snippets may be used to instrument a given instrumentation point
if and only if the point has memory access information attached to it.
In effect, the snippet "wraps" the instruction and provides a handle to particular
components of instruction behavior.

``finishInstrumenting`` finalizes the instrumentation by either resuming
execution of the process created or attached to in ``startInstrumenting``
or by writing a new binary to disk (for static instrumentation).

.. rli:: https://raw.githubusercontent.com/dyninst/examples/master/instrumentMemoryAccess/instrumenting_memory_access.cpp
   :language: cpp
   :linenos:
