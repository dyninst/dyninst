\section{Introduction}
\label{sec:intro}

A binary code parser converts the machine code representation of a program,
library, or code snippet to abstractions such as the instructions, basic
blocks, functions, and loops that the binary code represents. The ParseAPI is a
multi-platform library for creating such abstractions from binary code sources.
The current incarnation uses the Dyninst SymtabAPI as the default binary code
source; all platforms and architectures handled by the SymtabAPI are supported.
The ParseAPI has cross-architecture binary analysis capabilities in analyzing
ELF binaries (parsing of ARM binaries on x86 and vice versa, for example).
The ParseAPI is designed to be easily extensible to other binary code sources.
Support for parsing binary code in memory dumps or other formats requires only
implementation of a small interface as described in this document.

This API provides the user with a control flow-oriented view of a binary code
source. Each code object such as a program binary or library is represented as
a top-level collection containing the loops, functions, basic blocks, and edges that
represent the control flow graph. A simple query interface is provided for
retrieving lower level objects like functions and basic blocks through address
or other attribute lookups. These objects can be used to navigate the program
structure as described below.

Since Dyninst 10.0, ParseAPI is officially supporting parallel binary code analysis
and parallel queries. We typically observe 4X speedup when analyzing binaries with
8 threads. To control the number of threads used during parallel parsing, please
set environment variable \code{OMP\_NUM\_THREADS}.