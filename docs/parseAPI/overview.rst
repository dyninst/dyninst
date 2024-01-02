.. _`sec:parseapi-intro`:

ParseAPI
########

A binary code parser converts the machine code representation of a
program, library, or code snippet to abstractions such as the
instructions, basic blocks, functions, and loops that the binary code
represents. The ParseAPI is a multi-platform library for creating such
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

Since Dyninst 10.0, ParseAPI is officially supporting parallel binary
code analysis and parallel queries. We typically observe 4X speedup when
analyzing binaries with 8 threads. To control the number of threads used
during parallel parsing, please set environment variable
``OMP_NUM_THREADS``.

.. _`sec:parseapi-abstractions`:

Abstractions
************

The basic representation of code in this API is the control flow graph
(CFG). Binary code objects are represented as regions of contiguous
bytes that, when parsed, form the nodes and edges of this graph. The
following abstractions make up this CFG-oriented representation of
binary code:

.. container:: itemize

   block: Nodes in the CFG represent *basic blocks*: straight line
   sequences of instructions :math:`I_i \ldots I_j` where for each
   :math:`i < k
   \le j`, :math:`I_k` postdominates :math:`I_{k-1}`. Importantly, on
   some instruction set architectures basic blocks can *overlap* on the
   same address range—variable length instruction sets allow for
   multiple interpretations of the bytes making up the basic block.

   edge: Typed edges between the nodes in the CFG represent execution
   control flow, such as conditional and unconditional branches,
   fallthrough edges, and calls and returns. The graph therefore
   represents both *inter-* and *intraprocedural* control flow:
   traversal of nodes and edges can cross the boundaries of the higher
   level abstractions like *functions*.

   function: The *function* is the primary semantic grouping of code in
   the binary, mirroring the familiar abstraction of procedural
   languages like C. Functions represent the set of all basic blocks
   reachable from a *function entry point* through intraprocedural
   control flow only (that is, no calls or returns). Function entry
   points are determined in a variety of ways, such as hints from
   debugging symbols, recursive traversal along call edges and a machine
   learning based function entry point identification process.

   loop: The *loop* represents code in the binary that may execute
   repeatedly, corresponding to source language constructs like *for*
   loop or *while* loop. We use a formal definition of loops from
   “Nesting of Reducible and Irreducible Loops" by Paul Havlak. We
   support identifying both natural loops (single-entry loops) and
   irreducible loops (multi-entry loops).

.. container:: itemize

   code object: A collection of distinct code regions are represented as
   a single code object, such as an executable or library. Code objects
   can normally be thought of as a single, discontiguous unique address
   space. However, the ParseAPI supports code objects in which the
   different regions have overlapping address spaces, such as UNIX
   archive files containing unlinked code.

   instruction source: An instruction source describes a backing store
   containing binary code. A binary file, a library, a memory dump, or a
   process’s executing memory image can all be described as an
   instruction source, allowing parsing of a variety of binary code
   objects.

   code source: The code source implements the instruction source
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

Loop analysis
=============

The following code example shows how to get loop information using
ParseAPI once we have an parsed Function object.

.. code-block:: cpp

   void GetLoopInFunc(Function *f) {
       // Get all loops in the function
       vector<Loop*> loops;
       f->getLoops(loops);

       // Iterate over all loops
       for (auto lit = loops.begin(); lit != loops.end(); ++lit) {
           Loop *loop = *lit;

           // Get all the entry blocks of the loop
   	vector<Block*> entries;
   	loop->getLoopEntries(entries);

           // Get all the blocks in the loop
           vector<Block*> blocks;
   	loop->getLoopBasicBlocks(blocks);

   	// Get all the back edges in the loop
   	vector<Edge*> backEdges;
   	loop->getBackEdges(backEdges);
       }
   }

.. _`sec:extend`:

Extending ParseAPI
******************

The ParseAPI is design to be a low level toolkit for binary analysis
tools. Users can extend the ParseAPI in two ways: by extending the
control flow structures (Functions, Blocks, and Edges) to incorporate
additional data to support various analysis applications, and by adding
additional binary code sources that are unsupported by the default
SymtabAPI-based code source. For example, a code source that represents
a program image in memory could be implemented by fulfilling the
CodeSource and InstructionSource interfaces described in Section
`4.8 <#sec:codesource>`__ and below. Implementations that extend the CFG
structures need only provide a custom allocation factory in order for
these objects to be allocated during parsing.

Instruction and Code Sources
============================

A CodeSource, as described above, exports its own and the
InstructionSource interface for access to binary code and other details.
In addition to implementing the virtual methods in the CodeSource base
class (Section `4.8 <#sec:codesource>`__), the methods in the
pure-virtual InstructionSource class must be implemented:

.. code-block:: cpp
    
    virtual bool isValidAddress(const Address)

Returns true if the address is a valid code location.

.. code-block:: cpp
    
    virtual void* getPtrToInstruction(const Address)

Returns pointer to raw memory in the binary at the provided address.

.. code-block:: cpp
    
    virtual void* getPtrToData(const Address)

Returns pointer to raw memory in the binary at the provided address. The
address need not correspond to an executable code region.

.. code-block:: cpp
    
    virtual unsigned int getAddressWidth()

Returns the address width (e.g. four or eight bytes) for the represented
binary.

.. code-block:: cpp
    
    virtual bool isCode(const Address)

Indicates whether the location is in a code region.

.. code-block:: cpp
    
    virtual bool isData(const Address)

Indicates whether the location is in a data region.

.. code-block:: cpp
    
    virtual Address offset()

The start of the region covered by this instruction source.

.. code-block:: cpp
    
    virtual Address length()

The size of the region.

.. code-block:: cpp
    
    virtual Architecture getArch()

The architecture of the instruction source. See the Dyninst manual for
details on architecture differences.

.. code-block:: cpp
    
    virtual bool isAligned(const Address)

For fixed-width instruction architectures, must return true if the
address is a valid instruction boundary and false otherwise; otherwise
returns true. This method has a default implementation that should be
sufficient.

CodeSource implementors need to fill in several data structures in the
base CodeSource class:

.. code-block:: cpp
    
    std::map<Address, std::string> _linkage

Entries in the linkage map represent external linkage, e.g. the PLT in
ELF binaries. Filling in this map is optional.

.. code-block:: cpp
    
    Address _table_of_contents

Many binary format have “table of contents” structures for position
independant references. If such a structure exists, its address should
be filled in.

.. code-block:: cpp
    
    std::vector<CodeRegion *> _regions Dyninst::IBSTree<CodeRegion> _region_tree

One or more contiguous regions of code or data in the binary object must
be registered with the base class. Keeping these structures in sync is
the responsibility of the implementing class.

.. code-block:: cpp
    
    std::vector<Hint> _hints

CodeSource implementors can supply a set of Hint objects describing
where functions are known to start in the binary. These hints are used
to seed the parsing algorithm. Refer to the CodeSource header file for
implementation details.

.. _`sec:factories`:

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

.. code-block:: cpp
    
    virtual Function * mkfunc(Address addr, FuncSource src, std::string
    name, CodeObject * obj, CodeRegion * region,
    Dyninst::InstructionSource * isrc)

Returns an object derived from Function as though the provided
parameters had been passed to the Function constructor. The ParseAPI
parser will never invoke ``mkfunc()`` twice with identical ``addr``, and
``region`` parameters—that is, Functions are guaranteed to be unique by
address within a region.

.. code-block:: cpp
    
    virtual Block * mkblock(Function * func, CodeRegion * region, Address addr)

Returns an object derived from Block as though the provided parameters
had been passed to the Block constructor. The parser will never invoke
``mkblock()`` with identical ``addr`` and ``region`` parameters.

.. code-block:: cpp
    
    virtual Edge * mkedge(Block * src, Block * trg, EdgeTypeEnum type)

Returns an object derived from Edge as though the provided parameters
had been passed to the Edge constructor. The parser *may* invoke
``mkedge()`` multiple times with identical parameters.

.. code-block:: cpp
    
    virtual Block * mksink(CodeObject *obj, CodeRegion *r)

Returns a “sink” block derived from Block to which all unresolvable
control flow instructions will be linked. Implementors may return a
unique sink block per CodeObject or a single global sink.

Implementors of extended CFG classes are required to override the
default implementations of the *mk** functions to allocate and return
the appropriate derived types statically cast to the base type.
Implementors must also add all allocated objects to the following
internal lists:

.. code-block:: cpp
    
    fact_list<Edge> edges_ fact_list<Block> blocks_ fact_list<Function> funcs_

O(1) allocation lists for CFG types. See the CFG.h header file for list
insertion and removal operations.

Implementors *may* but are *not required to* override the deallocation
following deallocation routines. The primary reason to override these
routines is if additional action or cleanup is necessary upon CFG object
release; the default routines simply remove the objects from the
allocation list and invoke their destructors.

.. code-block:: cpp
    
    virtual void free_func(Function * f) virtual void free_block(Block *
    b) virtual void free_edge(Edge * e) virtual void free_all()

CFG objects should be freed using these functions, rather than delete,
to avoid leaking memory.

.. _`sec:defmode`:

Defensive Mode Parsing
**********************

Binary code that defends itself against analysis may violate the
assumptions made by the the ParseAPI’s standard parsing algorithm.
Enabling defensive mode parsing activates more conservative assumptions
that substantially reduce the percentage of code that is analyzed by the
ParseAPI. For this reason, defensive mode parsing is best-suited for use
of ParseAPI in conjunction with dynamic analysis techniques that can
compensate for its limited coverage of the binary code.

Containers
**********

Several of the ParseAPI data structures export containers of CFG
objects; the CodeObject provides a list of functions in the binary, for
example, while functions provide lists of blocks and so on. To avoid
tying the internal storage for these structures to any particular
container type, ParseAPI objects export a ContainerWrapper that provides
an iterator interface to the internal containers. These wrappers and
predicate interfaces are designed to add minimal overhead while
protecting ParseAPI users from exposure to internal container storage
details. Users *must not* rely on properties of the underlying container
type (e.g. storage order) unless that property is explicity stated in
this manual.

ContainerWrapper containers export the following interface (``iterator``
types vary depending on the template parameters of the ContainerWrapper,
but are always instantiations of the PredicateIterator described below):

.. code-block:: cpp
    
    iterator begin() iterator begin(predicate *)

Return an iterator pointing to the beginning of the container, with or
without a filtering predicate implementation (see Section
`4.11 <#sec:pred>`__ for details on filter predicates).

.. code-block:: cpp
    
    iterator const& end()

Return the iterator pointing to the end of the container (past the last
element).

.. code-block:: cpp
    
    size_t size()

Returns the number of elements in the container. Execution cost may vary
depending on the underlying container type.

.. code-block:: cpp
    
    bool empty()

Indicates whether the container is empty or not.

The elements in ParseAPI containers can be accessed by iteration using
an instantiation of the PredicateIterator. These iterators can
optionally act as filters, evaluating a boolean predicate for each
element and only returning those elements for which the predicate
returns true. *Iterators with non-null predicates may return fewer
elements during iteration than their ``size()`` method indicates.*
Currently PredicateIterators only support forward iteration. The
operators ``++`` (prefix and postfix), ``==``, ``!=``, and ``*``
(dereference) are supported.


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

.. code-block:: cpp
    
       #include <boost/filter_iterator.hpp>
       using boost::make_filter_iterator;
       struct target_block
       {
         Block* operator()(Edge* e) { return e->trg(); }
       };


       vector<Block*> work;
       Intraproc epred; // ignore calls, returns
      
       work.push_back(func->entry()); // assuming `func' is a Function*

       // do_stuff is a functor taking a Block* as its argument
       while(!work.empty()) {
           Block * b = work.back();
           work.pop_back();

           Block::edgelist & targets = block->targets();
           // Do stuff for each out edge
           std::for_each(make_filter_iterator(targets.begin(), epred), 
                         make_filter_iterator(targets.end(), epred),
                         do_stuff());
           std::transform(make_filter_iterator(targets.begin(), epred),
                          make_filter_iterator(targets.end(), epred), 
                          std::back_inserter(work), 
                          std::mem_fun(Edge::trg));
           Block::edgelist::const_iterator found_interproc =
                   std::find_if(targets.begin(), targets.end(), Interproc());
           if(interproc != targets.end()) {
                   // do something with the interprocedural edge you found
           }
       }

Anything that can be treated as a function from ``Edge*`` to a ``bool``
can be used in this manner. This replaces the beta interface where all
``EdgePredicate``\ s needed to descend from a common parent class. Code
that previously constructed iterators from an edge predicate should be
replaced with equivalent code using filter iterators as follows:

.. code-block:: cpp
    
     // OLD
     for(Block::edgelist::iterator i = targets.begin(epred); 
         i != targets.end(epred); 
         i++)
     {
       // ...
     }
     // NEW
     for_each(make_filter_iterator(epred, targets.begin(), targets.end()),
              make_filter_iterator(epred, targets.end(), targets,end()),
              loop_body_as_function);
     // NEW (C++11)
     for(auto i = make_filter_iterator(epred, targets.begin(), targets.end()); 
         i != make_filter_iterator(epred, targets.end(), targets.end()); 
         i++)
     {
       // ...
     }
