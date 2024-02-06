.. _`example:dyninstapi-binary-analysis`:

Binary Analysis
###############

This example is similar to :ref:`example:dyninstapi-instrumenting-memory-accesses`, but
only one function is instrumented here.

This example illustrates how to use Dyninst to iterate over a
function’s control flow graph and inspect instructions. These are steps
that would usually be part of a larger data flow or control flow
analysis. Specifically, this example collects every basic block in a
function, iterates over them, and counts the number of instructions that
read or write memory.

A mutator program must create a single :cpp:class:`BPatch` instance.
This object is used to access functions and information that are global
to the library. It must not be destroyed until the mutator has completely
finished using the library. All instrumentation is done through
:cpp:class:`BPatch_addressSpace` that allows working with both dynamic and
static instrumentation with the same mutator code. This is a key feature
of Dyninst.

``startInstrumenting`` allows dynamic instrumentation by either creating or
attaching to a running process as well as static instrumentation for a file
from disk. In this example, a process is created for dynamic instrumentation.

In ``binaryAnalysis``, the target function is looked up by name. Alternatively,
it could be looked up by iterating over every function in :cpp:class:`BPatch_image`
or :cpp:class:`BPatch_module`. After getting a handle to the CFG, the blocks are
retrieved using :cpp:func:`BPatch_flowGraph::getAllBasicBlocks`. Each basic block has
a list of instructions retrievable by :cpp:func:`BPatch_basicBlock::getInstructions`.
Instructions that either read or write memory- that is, they **access** memory- are counted.

.. rli:: https://raw.githubusercontent.com/dyninst/examples/master/memoryAccessCounter/counter.cpp
   :language: cpp
   :linenos:
