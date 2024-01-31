.. _sec:dyninstapi-intro:

DyninstAPI
##########

The key features of this interface are the abilities to:

-  Insert and change instrumentation in a running program.

-  Insert instrumentation into a binary on disk and write a new copy of
   that binary back to disk.

-  Perform static and dynamic analysis on binaries and processes.

The goal of this API is to keep the interface small and easy to
understand. At the same time, it needs to be sufficiently expressive to
be useful for a variety of applications. We accomplished this goal by
providing a simple set of abstractions and a way to specify which code
to insert into the application.

.. note::
   To generate more complex code, extra (initially un-called) subroutines can be
   linked into the application program, and calls to these subroutines can be
   inserted at runtime via this interface.

Abstractions
************

The DyninstAPI library provides an interface for instrumenting and
working with binaries and processes. The user writes a *mutator*, which
uses the DyninstAPI library to operate on the application. The process
that contains the *mutator* and DyninstAPI library is known as the
*mutator process*. The *mutator process* operates on other processes or
on-disk binaries, which are known as *mutatees.*

The API is based on abstractions of a program. For dynamic
instrumentation, it can be based on the state while in execution. The
two primary abstractions in the API are *points* and *snippets*. A
*point* is a location in a program where instrumentation can be
inserted. A *snippet* is a representation of some executable code to be
inserted into a program at a point. For example, if we wished to record
the number of times a procedure was invoked, the *point* would be entry
point of the procedure, and the *snippets* would be a statement to
increment a counter. *Snippets* can include conditionals and function
calls.

*Mutatees* are represented using an *address space* abstraction. For
dynamic instrumentation, the *address space* represents a process and
includes any dynamic libraries loaded with the process. For static
instrumentation, the *address space* includes a disk executable and
includes any dynamic library files on which the executable depends. The
*address space* abstraction is extended by *process* and *binary*
abstractions for dynamic and static instrumentation. The *process*
abstraction represents information about a running process such as
threads or stack state. The *binary* abstraction represents information
about a binary found on disk.

The code and data represented by an *address space* is broken up into
*function* and *variable* abstractions. *Function*\ s contain *points*,
which specify locations to insert instrumentation. *Functions* also
contain a *control flow graph* abstraction, which contains information
about *basic blocks*, *edges*, *loops*, and *instructions*. If the
*mutatee* contains debug information, DyninstAPI will also provide
abstractions about variable and function *types*, *local variables,*
*function parameters*, and *source code line information*. The
collection of *functions* and *variables* in a mutatee is represented as
an *image*.

The API includes a simple type system based on structural equivalence.
If mutatee programs have been compiled with debugging symbols and the
symbols are in a format that Dyninst understands, type checking is
performed on code to be inserted into the mutatee. See Section 4.28 for
a complete description of the type system.

Due to language constructs or compiler optimizations, it may be possible
for multiple functions to *overlap* (that is, share part of the same
function body) or for a single function to have multiple *entry points*.
In practice, it is impossible to determine the difference between
multiple overlapping functions and a single function with multiple entry
points. The DyninstAPI uses a model where each function (:cpp:class:`BPatch_function`
object) has a single entry point, and multiple functions may overlap
(share code). We guarantee that instrumentation inserted in a particular
function is only executed in the context of that function, even if
instrumentation is inserted into a location that exists in multiple
functions.

Usage
*****

We refer to the application process or binary that is being modified as the
mutatee, and the program that uses the API to modify the application as
the mutator. The mutator is a separate process from the application
process.


Instrumenting a function
========================

See the :ref:`example:dyninstapi-instr-func` example.


Wrapping a function
===================

The following code wraps malloc with fastMalloc, while allowing functions to still access the original
malloc function by calling origMalloc.

..
  rli:: https://raw.githubusercontent.com/dyninst/examples/master/dyninstAPI/wrapFunction.cpp
   :language: cpp
   :linenos:


Binary Analysis
===============

See the :ref:`example:dyninstapi-binary-analysis` example.


Instrumenting Memory Accesses
=============================

See the :ref:`example:dyninstapi-instrumenting-memory-accesses` example.


Using DyninstAPI with the component libraries
*********************************************

The component libraries (SymtabAPI, InstructionAPI, ParseAPI, and PatchAPI)
often provide greater functionality and cleaner interfaces than Dyninst,
and thus users may wish to use a mix of abstractions. In general, users
may access component library abstractions via a convert function, which
is overloaded and namespaced to give consistent behavior. The
definitions of all component library abstractions are located in the
appropriate documentation.

.. code-block:: cpp

   PatchAPI::PatchMgrPtr PatchAPI::convert(BPatch_addressSpace *);
   
   PatchAPI::PatchObject *PatchAPI::convert(BPatch_object *);
   
   ParseAPI::CodeObject *ParseAPI::convert(BPatch_object *);
   
   SymtabAPI::Symtab *SymtabAPI::convert(BPatch_object *);
   
   SymtabAPI::Module *SymtabAPI::convert(BPatch_module *);
   
   PatchAPI::PatchFunction *PatchAPI::convert(BPatch_function *);
   
   ParseAPI::Function *ParseAPI::convert(BPatch_function *);
   
   PatchAPI::PatchBlock *PatchAPI::convert(BPatch_basicBlock *);
   
   ParseAPI::Block *ParseAPI::convert(BPatch_basicBlock *);
   
   PatchAPI::PatchEdge *PatchAPI::convert(BPatch_edge *);
   
   ParseAPI::Edge *ParseAPI::convert(BPatch_edge *);
   
   PatchAPI::Point *PatchAPI::convert(BPatch_point *, BPatch_callWhen);
   
   PatchAPI::SnippetPtr PatchAPI::convert(BPatch_snippet *);
   
   SymtabAPI::Type *SymtabAPI::convert(BPatch_type *);

Differences Between DyninstAPI and PatchAPI
*******************************************

:ref:`DyninstAPI <sec:dyninstapi-intro>` and PatchAPI differ primarily in their CFG
representations and instrumentation point abstractions. In general,
PatchAPI is more powerful and can better represent complex binaries
(e.g., highly optimized code or malware). In order to maintain backwards
compatibility, the DyninstAPI interface has not been extended to match
the PatchAPI. As a result, there are some caveats.

PatchAPI uses the same CFG model as :ref:`ParseAPI <sec:parseapi-intro>`. The primary
representation is an interprocedural graph of basic blocks and edges.
Functions are defined on top of this graph as collections of blocks. **A
block may be contained by more than one function;** we call this the
*shared block* model. Functions are defined to have a single entry
block, and functions may overlap if they contain the same blocks. Call
and return edges exist in the graph, and therefore traversing the graph
may enter different functions. PatchAPI users may specify instrumenting
a particular block within a particular function (a *block instance*) by
specifying both the block and the function.

DyninstAPI uses a historic CFG model. The primary representation is
the function. Functions contain a intraprocedural graph of blocks and
edges. As a result, a basic block belongs to only one function, but two
blocks from different functions may be *clones* of each other. No
interprocedural edges are represented in the graph, and thus traversing
the CFG from a particular function is guaranteed to remain inside that
function.

As a result, multiple DyninstAPI blocks may map to the same PatchAPI
block. If instrumenting a particular block instance is desired, the user
should provide both the DyninstAPI basic block and function.

In addition, DyninstAPI uses a *module* abstraction, where a
:cpp:class:`BPatch_module` represents a collection of functions from a particular
source file (for the executable) or from an entire library (for all
libraries). PatchAPI, like ParseAPI, instead uses an *object*
representation, where a :cpp:class:`PatchObject` object represents a collection
of functions from a file on disk (executable or libraries).

The instrumentation point (*instPoint*) models also differ between
DyninstAPI and PatchAPI. We classify an instPoint either as a *behavior*
point (e.g., function entry) or *location* point (e.g., a particular
instruction). PatchAPI fully supports both of these models, with the
added extension that a location point explicitly specifies whether
instrumentation will execute before or after the corresponding location.
Dyninst does not support the behavior model, instead mapping behavior
instPoints to a corresponding instruction. For example, if a user
requests a function entry instPoint they instead receive an instPoint
for the first instruction in the function. These may not always be the
same (see `Bernat_AWAT <ftp://ftp.cs.wisc.edu/paradyn/papers/Bernat11AWAT.pdf>`__).
In addition, location instPoints represent an instruction, and the user
must later specify whether they wish to instrument before or after that
instruction.

As a result, there are complications for using both DyninstAPI and
PatchAPI. We cannot emphasize enough, though, that users *can combine
DyninstAPI and PatchAPI* with some care. Doing so offers several
benefits:

-  The ability to extend legacy code that is written for DyninstAPI.

-  The ability to use the DyninstAPI extensions and plugins for
   PatchAPI, including snippet-based or dynC-based code generation and
   our instrumentation optimizer.

We suggest the following best practices to be followed when coding for
PatchAPI via Dyninst:

-  For legacy code, do not attempt to map between DyninstAPI instPoints
   and PatchAPI instPoints. Instead, use DyninstAPI CFG objects to
   acquire PatchAPI CFG objects, and use a ``PatchMgr`` (acquired
   through a ``BPatch_addressSpace``) to look up PatchAPI instPoints.

-  For new code, acquire a ``PatchMgr`` directly from a
   ``BPatch_addressSpace`` and use its methods to look up both CFG
   objects and instPoints.

PatchAPI accessor methods in DyninstAPI
=======================================

To access a PatchAPI class from a Dyninst class, use the
:cpp:func:`PatchAPI::convert` function, as in the following example:

.. code-block:: cpp
    
    BPatch_basicBlock *bp_block = ...;
    PatchAPI::PatchBlock *block = PatchAPI::convert(bp_block);

.. csv-table:: BPatch <-> PatchAPI mappings
  :header: "From", "To"

  "BPatch_function", "PatchFunction"
  "BPatch_basicBlock", "PatchBlock"
  "BPatch_edge", "PatchEdge"
  "BPatch_module", "PatchObject"
  "BPatch_image", "PatchMgr"
  "BPatch_addressSpace", "PatchMgr"
  "BPatch_snippet", "Snippet"

We do not support a direct mapping between :cpp:class:`BPatch_point`\ s and
:cpp:class:`PatchAPI::Point`\ s, as the failure of Dyninst to properly represent behavior
instPoints leads to confusing results. Instead, use the PatchAPI point
lookup methods.
