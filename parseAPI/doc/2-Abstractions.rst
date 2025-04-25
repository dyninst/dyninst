.. _sec:abstractions:

Abstractions
============

The basic representation of code in this API is the control flow graph
(CFG). Binary code objects are represented as regions of contiguous
bytes that, when parsed, form the nodes and edges of this graph. The
following abstractions make up this CFG-oriented representation of
binary code:

block: Nodes in the CFG represent *basic blocks*: straight line
sequences of instructions :math:`I_i \ldots I_j` where for each
:math:`i < k
\le j`, :math:`I_k` postdominates :math:`I_{k-1}`. Importantly, on some
instruction set architectures basic blocks can *overlap* on the same
address range—variable length instruction sets allow for multiple
interpretations of the bytes making up the basic block.

edge: Typed edges between the nodes in the CFG represent execution
control flow, such as conditional and unconditional branches,
fallthrough edges, and calls and returns. The graph therefore represents
both *inter-* and *intraprocedural* control flow: traversal of nodes and
edges can cross the boundaries of the higher level abstractions like
*functions*.

function: The *function* is the primary semantic grouping of code in the
binary, mirroring the familiar abstraction of procedural languages like
C. Functions represent the set of all basic blocks reachable from a
*function entry point* through intraprocedural control flow only (that
is, no calls or returns). Function entry points are determined in a
variety of ways, such as hints from debugging symbols, recursive
traversal along call edges and a machine learning based function entry
point identification process.

loop: The *loop* represents code in the binary that may execute
repeatedly, corresponding to source language constructs like *for* loop
or *while* loop. We use a formal definition of loops from “Nesting of
Reducible and Irreducible Loops" by Paul Havlak. We support identifying
both natural loops (single-entry loops) and irreducible loops
(multi-entry loops).

code object: A collection of distinct code regions are represented as a
single code object, such as an executable or library. Code objects can
normally be thought of as a single, discontiguous unique address space.
However, the ParseAPI supports code objects in which the different
regions have overlapping address spaces, such as UNIX archive files
containing unlinked code.

instruction source: An instruction source describes a backing store
containing binary code. A binary file, a library, a memory dump, or a
process’s executing memory image can all be described as an instruction
source, allowing parsing of a variety of binary code objects.

code source: The code source implements the instruction source
interface, exporting methods that can access the underlying bytes of the
binary code for parsing. It also exports a number of additional helper
methods that do things such as returning the location of structured
exception handling routines and function symbols. Code sources are
tailored to particular binary types; the ParseAPI provides a
SymtabAPI-based code source that understands ELF, COFF and PE file
formats.
