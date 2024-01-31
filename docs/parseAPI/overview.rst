.. _`sec:parseapi-intro`:

.. cpp:namespace:: Dyninst::ParseAPI

ParseAPI
########

A binary code parser converts the machine code representation of a
program, library, or code snippet to abstractions such as the
instructions, basic blocks, functions, and loops that the binary code
represents. ParseAPI is a multi-platform library for creating such
abstractions from binary code sources. The current incarnation uses the
Dyninst SymtabAPI as the default binary code source; all platforms and
architectures handled by the SymtabAPI are supported. The ParseAPI has
cross-architecture binary analysis capabilities in analyzing ELF
binaries (parsing of ARM binaries on x86 and vice versa, for example).
The ParseAPI is designed to be easily extensible to other binary code
sources. Support for parsing binary code in memory dumps or other
formats requires only implementation of a small interface as described
in this document.

This API provides the user with a control flow-oriented view of a binary
code source. Each code object such as a program binary or library is
represented as a top-level collection containing the loops, functions,
basic blocks, and edges that represent the control flow graph. A simple
query interface is provided for retrieving lower level objects like
functions and basic blocks through address or other attribute lookups.
These objects can be used to navigate the program structure as described
below.

ParseAPI supports parallel binary code analysis and parallel queries
using OpenMP. The level of parallelism can be controlled with the
``OMP_NUM_THREADS`` environment variable. Speedups as large as 4X have
been observed when using 8 threads.
See `Parallelizing Binary Code Analysis <https://paradyn.org/publications/publications-by-year.html#2020>`_
for further details.

.. _`sec:parseapi-abstractions`:

Abstractions
************

The core component of ParseAPI is the control flow graph (:cpp:class:`CFG`).
Binary code objects are represented as regions of contiguous
bytes that, when parsed, form the nodes and edges of this graph. The
following abstractions make up the CFG:

Blocks
  Nodes in the CFG represent basic :cpp:class:`Block`\ s: straight line
  sequences of instructions :math:`I_i \ldots I_j` where for each
  :math:`i < k \le j`, :math:`I_k` postdominates :math:`I_{k-1}`.
  Importantly, on some instruction set architectures basic blocks
  can overlap on the same address range—variable length instruction
  sets allow for multiple interpretations of the bytes making up the
  basic block.

Edges
  Typed :cpp:class:`Edge`\ s between the nodes in the CFG represent execution
  control flow, such as conditional and unconditional branches,
  fallthrough edges, and calls and returns. The graph therefore
  represents both inter- and intraprocedural control flow:
  traversal of nodes and edges can cross the boundaries of the higher
  level abstractions like functions.

Functions
  The :cpp:class:`Function` is the primary semantic grouping of code in
  the binary, mirroring the familiar abstraction of procedural
  languages like C. Functions represent the set of all basic blocks
  reachable from a function entry point through intraprocedural
  control flow only (that is, no calls or returns). Function entry
  points are determined in a variety of ways, such as hints from
  debugging symbols, recursive traversal along call edges and a machine
  learning based function entry point identification process.

Loops
  The :cpp:class:`Loop` represents code in the binary that may execute
  repeatedly, corresponding to source language constructs like *for*
  loop or *while* loop. We use a formal definition of loops from
  “Nesting of Reducible and Irreducible Loops" by Paul Havlak. We
  support identifying both natural loops (single-entry loops) and
  irreducible loops (multi-entry loops).

Code Objects
  A collection of distinct code regions are represented as
  a single :cpp:class:`code object <CodeObject>`, such as an executable or
  library. Code objects
  can normally be thought of as a single, discontiguous unique address
  space. However, the ParseAPI supports code objects in which the
  different regions have overlapping address spaces, such as UNIX
  archive files containing unlinked code.

Instruction Source
  An :cpp:class:`instruction source <InstructionSource>` describes a backing store
  containing binary code. A binary file, a library, a memory dump, or a
  process’s executing memory image can all be described as an
  instruction source, allowing parsing of a variety of binary code
  objects.

Code Source
  The :cpp:class:`code source <CodeSource>` implements the instruction source
  interface, exporting methods that can access the underlying bytes of
  the binary code for parsing. It also exports a number of additional
  helper methods that do things such as returning the location of
  structured exception handling routines and function symbols. Code
  sources are tailored to particular binary types; the ParseAPI
  provides a SymtabAPI-based code source that understands ELF, COFF and
  PE file formats.

.. _`sec:parseapi-usage`:

Usage
*****

Traversing a CFG
================

See the :ref:`example:parseapi-cfg-traversal` example.

Loop analysis
=============

The following code example shows how to get loop information using
ParseAPI once we have an parsed Function object.

.. rli:: https://raw.githubusercontent.com/dyninst/examples/master/parseAPI/loopAnalysis.cpp
  :language: cpp
  :linenos:


Edge Predicates
===============

Edge predicates control iteration over edges. For example, the provided
``Intraproc`` edge predicate can be used with filter iterators and
standard algorithms, ensuring that only intraprocedural edges are
visited during iteration. Two other examples of edge predicates are
provided: ``SingleContext`` only visits edges that stay in a single
function context, and ``NoSinkPredicate`` does not visit edges to the
*sink* block. The following code traverses all of the basic blocks
within a function:

.. rli:: https://raw.githubusercontent.com/dyninst/examples/master/parseAPI/edgePredicate.cpp
  :language: cpp
  :linenos:

Extending ParseAPI
******************

The ParseAPI is design to be a low level toolkit for binary analysis
tools. Users can extend the ParseAPI in two ways: by extending the
control flow structures (Functions, Blocks, and Edges) to incorporate
additional data to support various analysis applications, and by adding
additional binary code sources that are unsupported by the default
SymtabAPI-based code source. For example, a code source that represents
a program image in memory could be implemented by fulfilling the
CodeSource and InstructionSource interfaces. Implementations that extend the CFG
structures need only provide a custom allocation factory in order for
these objects to be allocated during parsing.

CFG Object Factories
====================

Users who which to incorporate the ParseAPI into large projects may need
to store additional information about CFG objects like Functions,
Blocks, and Edges. The simplest way to associate the ParseAPI-level CFG
representation with higher-level implementation is to extend the CFG
classes provided as part of the ParseAPI. Because the parser itself does
not know how to construct such extended types, implementors must provide
an implementation of the CFGFactory that is specialized for their CFG
classes. The CFGFactory exports the following simple interface:

Defensive Mode Parsing
**********************

Binary code that defends itself against analysis may violate the
assumptions made by the the ParseAPI’s standard parsing algorithm.
Enabling defensive mode parsing activates more conservative assumptions
that substantially reduce the percentage of code that is analyzed by the
ParseAPI. For this reason, defensive mode parsing is best-suited for use
of ParseAPI in conjunction with dynamic analysis techniques that can
compensate for its limited coverage of the binary code.
