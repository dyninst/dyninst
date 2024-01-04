.. _sec-patchapi-intro:

.. cpp:namespace:: Dyninst::PatchAPI

PatchAPI
########

A natural loop has a single entry block and an irreducible loop has multiple.

This manual describes PatchAPI, a programming interface and library for
binary code patching. A programmer uses PatchAPI to instrument (insert
code into) and modify a binary executable or library by manipulating the
binary’s control flow graph (CFG). We allow the user to instrument a
binary by annotating a CFG with *snippets*, or sequences of inserted
code, and to modify the binary by directly manipulating the CFG. The
PatchAPI interface, and thus tools written with PatchAPI, is designed to
be flexible and extensible. First, users may *inherit* from PatchAPI
abstractions in order to store their own data. Second, users may create
*plugins* to extend PatchAPI to handle new types of instrumentation,
different binary types, or different patching techniques.

PatchAPI represents the binary as an annotatable and modifiable CFG. The
CFG consists of abstractions for binary objects, functions, basic
blocks, edges connecting basic blocks, and loops representing code that
may execute repeatedly, which are similar to the CFG abstractions used
by the ParseAPI component.

Users instrument the binary by annotating this CFG using three
additional high-level abstractions: Point, Snippet, and Instance. A
Point supports instrumentation by representing a particular aspect of
program behavior (e.g., entering a function or traversing an edge) and
containing instances of Snippets. Point lookup is performed with a
single PatchAPI manager (PatchMgr) object by Scope (e.g., a CFG object)
and Type (e.g., function entry). In addition, a user may provide an
optional Filter that selects a subset of matching Points. A Snippet
represents a sequence of code to be inserted at certain points. To
maximize flexibility, PatchAPI does not prescribe a particular snippet
form; instead, users may provide their own (e.g., a binary buffer, a
Dyninst abstract syntax tree (AST), or code written in the DynC language).
Users instrument the binary by adding Snippets to the desired Points. An
Instance represents the insertion of a particular Snippet at a
particular Point.

The core PatchAPI representation of an annotatable and modifiable CFG
operates in several domains, including on a running process (dynamic
instrumentation) or a file on disk (binary rewriting). Furthermore,
PatchAPI may be used both in the same address space as the process
(1st-party instrumentation) or in a different address space via the
debug interface (3rd-party instrumentation). Similarly, developers may
define their own types of Snippets to encapsulate their own code
generation techniques. These capabilities are provided by a plugin
interface; by implementing a plugin a developer may extend PatchAPI’s
capabilities.

This manual is structured as follows. Section `2 <#sec-parseapi-abstractions>`__ presents
the core abstractions in the public and plugin interface of PatchAPI.
Section `3 <#sec-example>`__ shows several examples in C++ to illustrate
the usage of PatchAPI. Detailed API reference can be found in
Section `4 <#sec-public-api>`__ and Section `6 <#sec-plugin-api>`__.
Finally, Appendix `7 <#sec-dyn>`__ provides a quick tutorial of PatchAPI
to those who are already familiar with DyninstAPI.

.. _sec-patchapi-abstractions:

Abstractions
************

.. figure:: ./figure/abstraction/img.pdf
   :alt: Object Ownership
   :name: fig:abs
   :width: 85.0%

   Object Ownership

PatchAPI contains two interfaces: the public interface and the plugin
interface. The public interface is used to find instrumentation points,
insert or delete code snippets, and register plugins provided by
programmers. The plugin interface is used to customize different aspects
in the binary code patching. PatchAPI provides a set of default plugins
for first party code patching, which is easy to extend to meet different
requirements in practice.

Figure `1 <#fig:abs>`__ shows the ownership hierarchy for PatchAPI’s
classes. Ownership is a “contains” relationship. If one class owns
another, then instances of the owner class maintain exactly one or
possibly more than one instances of the other, which depends on whether
the relationship is a “1:1” or a “1:n” relationship. In Figure
`1 <#fig:abs>`__, for example, each PatchMgr instance contains exactly
one instance of a AddrSpace object, while a PatchMgr instance may
contains more than one instances of a Point object.

The remainder of this section briefly describes the classes that make up
PatchAPI’s two interfaces. For more details, see the class descriptions
in Section `4 <#sec-public-api>`__ and Section `6 <#sec-plugin-api>`__.

Public Interface
****************

PatchMgr, Point, and Snippet are used to perform the main process of
binary code patching: 1) find some **Point**; 2) insert or delete
**Snippet** at some **Point**.

-  *PatchMgr* - The PatchMgr class is the top-level class for finding
   instrumentation **Points**, inserting or deleting **Snippets**, and
   registering user-provided plugins.

-  *Point* - The Point class represents a location on the CFG that acts
   as a container of inserted snippet **Instances**. Points of different
   types are distinct even the underlying code relocation and generation
   engine happens to put instrumentation from them at the same place.

-  *Instance* - The Instance class is a representation of a particular
   snippet inserted at a particular point.

-  *PatchObject* - The PatchObject class is a wrapper of ParseAPI’s
   CodeObject class, which represents an individual binary code object,
   such as an executable or a library.

-  *PatchFunction* - The PatchFunction class is a wrapper of ParseAPI’s
   Function class, which represents a function.

-  *PatchBlock* - The PatchBlock class is a wrapper of ParseAPI’s Block
   class, which represents a basic block.

-  *PatchEdge* - The PatchEdge class is a wrapper of ParseAPI’s Edge
   class, which join two basic blocks in the CFG, indicating the type of
   control flow transfer instruction that joins the basic blocks to each
   other.

-  *PatchLoop* - The PatchLoop class is a wrapper of ParseAPI’s Loop
   class, which repreents a piece of code that may execute repeatedly.

-  *PatchLoopTreeNode* - The PatchLoopTreeNode class is a wrapper of
   ParseAPI’s LoopTreeNode class, which provides a tree interface to a
   collection of instances of class PatchLoop contained in a function.
   The structure of the tree follows the nesting relationship of the
   loops in a function.

Plugin Interface
****************

The address space abstraction determines whether the code patching is
1st party, 3rd party or binary rewriting.

-  *AddrSpace* - The AddrSpace class represents the address space of a
   **Mutatee** (a program that is instrumented), where it contains a
   collection of **PatchObjects** that represent shared libraries or a
   binary executable. In addition, programmers implement some memory
   management interfaces in the AddrSpace class to determines the type
   of the code patching - 1st party, 3rd party, or binary rewriting.

Programmers can decide the representation of a **Snippet**, for example,
the representation can be in high level language (e.g., C or C++), or
can simply be in binary code (e.g., 0s and 1s).

-  *Snippet* - The Snippet class allows programmers to easily plug in
   their own snippet representation and the corresponding mini-compiler
   to translate the representation into the binary code.

PatchAPI provides a thin layer on top of ParseAPI’s Control Flow Graph
(CFG) layer, which associates some useful information for the ease of
binary code patching, for example, a shared library’s load address. This
layer of CFG structures include PatchObject, PatchFunction, PatchBlock
and PatchEdge classes. Programmers can extend these four CFG classes,
and use the derived class of CFGMaker to build a CFG with the augmented
CFG structures.

-  *CFGMaker* - The CFGMaker class is a factory class that constructs
   the above CFG structures. This class is used in CFG parsing.

Similar to customizing the PatchAPI layer, programmers can also
customize the Point class by extending it.

-  *PointMaker* - The PointMaker class is a factory class that
   constructs a subclass of the Point class.

.. figure:: ./figure/command/img.pdf
   :alt: Inheritance Hierarchy
   :name: fig:inh
   :width: 85.0%

   Inheritance Hierarchy

The implementation of an instrumentation engine may be very
sophisticated (e.g., relocating a function), or very simple (e.g.,
simply overwrite an instruction). Therefore, PatchAPI provides a
flexible framework for programmers to customize the instrumentation
engine. This framework is based on Command Pattern. The
instrumentation engine has transactional semantics, where all
instrumentation requests should succeed or all should fail. In our
framework, the **Command** abstraction represents an instrumentation
request or a logical step in the code patching process. We accumulate a
list of **Commands**, and execute them one by one. If one **Command**
fails, we undo all preceding finished **Commands**. Figure
`2 <#fig:inh>`__ illustrates the inheritance hierarchy for related
classes. There is a default implementation of instrumentation engine in
PatchAPI for 1st party code patching.

-  *Command* - The Command class represents an instrumentation request
   (e.g., snippet insertion or removal), or a logical step in the code
   patching (e.g., install instrumentation). This class provides a run()
   method and an undo() method, where run() will be called for normal
   execution, and undo() will be called for undoing this Command.

-  *BatchCommand* - The BatchCommand class is a subclass of Command, and
   it is in fact a container of a list of Commands to be executed
   atomically.

-  *Instrumenter* - The Instrumenter class inherits BatchCommand to
   encapsulate the core code patching logic, which includes binary code
   generation. Instrumenter would contain several logical steps that are
   individual Commands.

-  *Patcher* - The Patcher class is also a subclass of BatchCommand. It
   accepts instrumentation requests from users, where these
   instrumentation requests are Commands (e.g., snippet insertion).
   Furthermore, Patcher implicitly adds Instrumenter to the end of the
   Command list to generate binary code and install the instrumentation.

.. _sec-examples:

Patch API Examples
******************

To illustrate the ideas of PatchAPI, we present some simple code
examples that demonstrate how the API can be used.

Using the public interface
==========================

The basic flow of doing code patching is to first find some points in a
program, and then to insert, delete or update a piece of code at these
points.

CFG Traversal
^^^^^^^^^^^^^

.. code-block:: cpp
    
   ParseAPI::CodeObject* co = ...
   PatchObject* obj = PatchObject::create(co, code_base);

   // Find all functions in the object
   std::vector<PatchFunction*> all;
   obj->funcs(back_inserter(all));

   for (std::vector<PatchFunction*>::iterator fi = all.begin();
        fi != all.end(); fi++) {
     // Print out each function's name
     PatchFunction* func = *fi;
     std::cout << func->name() << std::endl;

     const PatchFunction::Blockset& blks = func->blocks();
     for (PatchFunction::BlockSet::iterator bi = blks.begin();
          bi != blks.end(); bi++) {
       // Print out each block's size
       PatchBlock* blk = *bi;
       std::cout << "\tBlock size:" << blk->size() << std::endl;
     }
    }

In the above code, we illustrate how to traverse CFG structures in
PatchAPI. First, we construct an instance of PatchObject using an
instance of ParseAPI’s CodeObject. Then, we traverse all functions in
that object, and print out each function’s name. For each function, we
also print out the size of each basic block.

.. _sec-example-pt:

Point Finding
^^^^^^^^^^^^^

.. code-block:: cpp
    
   PatchFunction *func = ...;
   PatchBlock *block = ...;
   PatchEdge *edge = ...;

   PatchMgr *mgr = ...;

   std::vector<Point*> pts;
   mgr->findPoints(Scope(func),
                   Point::FuncEntry | 
                   Point::PreCall | 
                   Point::FuncExit,
                   back_inserter(pts));
   mgr->findPoints(Scope(block),
                   Point::BlockEntry,
                   back_inserter(pts));
   mgr->findPoints(Scope(edge),
                   Point::EdgeDuring,
                   back_inserter(pts));

The above code shows how to use the PatchMgr::findPoints method to find
some instrumentation points. There are three invocations of findPoints.
For the first invocation (Line 8), it finds points only within a
specific function *func*, and output the found points to a vector *pts*.
The result should include all points at this function’s entry, before
all function calls inside this function, and at the function’s exit.
Similarly, for the second invocation (Line 13), it finds points only
within a specific basic *block*, and the result should include the point
at the block entry. Finally, for the third invocation (Line 16), it
finds the point at a specific CFG *edge* that connects two basic blocks.

Code Patching
^^^^^^^^^^^^^

.. code-block:: cpp
    
   MySnippet::ptr snippet = MySnippet::create(new MySnippet);

   Patcher patcher(mgr);
   for (vector<Point*>::iterator iter = pts.begin();
        iter != pts.end(); ++iter) {
     Point* pt = *iter;
     patcher.add(PushBackCommand::create(pt, snippet));
   }
   patcher.commit();

The above code is to insert the same code *snippet* to all points *pts*
found in Section `3.1.2 <#sec-example-pt>`__. We’ll explain the snippet
(Line 1) in the example in Section `3.2.2 <#sec-example-snip>`__. Each
point maintains a list of snippet instances, and the PushBackCommand is
to push a snippet instance to the end of that list. An instance of
Patcher is to represent a transaction of code patching. In this example,
all snippet insertions (or all PushBackCommands) are performed
atomically when the Patcher::commit method is invoked. That is, all
snippet insertions would succeed or all would fail.

Using the plugin interface
==========================

Address Space
^^^^^^^^^^^^^

.. code-block:: cpp
    
   class MyAddrSpace : public AddrSpace {
     public:
       ...
       virtual Address malloc(PatchObject* obj, size_t size, Address near) {
         Address buffer = ...
         // do memory allocation here
         return buffer;
       }
       virtual bool write(PatchObject* obj, Address to_addr, Address from_addr,
                          size_t size) {
         // copy data from the address from_addr to the address to_addr
         return true;
       }
       ...
   };

The above code is to implement the address space plugin, in which, a set
of memory management methods should be specified, including malloc,
free, realloc, write and so forth. The instrumentation engine will
utilize these memory management methods during the code patching
process. For example, the instrumentation engine needs to *malloc* a
buffer in Mutatee’s address space, and then *write* the code snippet
into this buffer.

.. _sec-example-snip:

Snippet Representation
^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp
    
   class MySnippet : public Snippet {
     public:
       virtual bool generate(Point *pt, Buffer &buf) {
         // Generate and store binary code in the Buffer buf
         return true;
       }
   };
   MySnippet::ptr snippet = MySnippet::create(new MySnippet);

The above code illustrates how to customize a user-defined snippet
*MySnippet* by implementing the “mini-compiler” in the *generate*
method, which will be used later in the instrumentation engine to
generate binary code.

Code Parsing
^^^^^^^^^^^^

.. code-block:: cpp
    
   class MyFunction : public PatchFunction {
     ...
   };
   class MyCFGMaker : public CFGMaker {
     public:
       ...
       virtual PatchFunction* makeFunction(ParseAPI::Function *f, PatchObject* o) {
         return new MyFunction(f, o);
       }
       ...
   };

Programmers can augment PatchAPI’s CFG structures by annotating their
own data. In this case, a factory class should be built by inheriting
from the CFGMaker class, to create the augmented CFG structures. The
factory class will be used for CFG parsing.

Point Making
^^^^^^^^^^^^

.. code-block:: cpp
    
   class MyPoint : public Point {
     public:
       MyPoint(Point::Type t, PatchMgrPtr m, PatchFunction *f);
       ...
   };

   class MyPointMaker: public PointMaker {
     protected:
       virtual Point *mkFuncPoint(Point::Type t, PatchMgrPtr m, PatchFunction *f) {
         return new MyPoint(t, m, f);
       }
   };

In the above example, the MyPoint class inherits from the Point class,
and the MyPointMaker class inherits from the PointMaker class. The
mkFuncPoint method in MyPointMaker simply returns a new instance of
MyPoint. The mkFuncPoint method will be invoked by
PatchMgr::findPoint(s).

Instrumentation Engine
^^^^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp
    
   class MyInstrumenter : public Instrumenter {
     public:
       virtual bool run() {
         // Specify how to install instrumentation
       }
   };

Programmers can customize the instrumentation engine by extending the
Instrumenter class, and implement the installation of instrumentation
inside the method *run()*.

Plugin Registration
^^^^^^^^^^^^^^^^^^^

.. code-block:: cpp
    
   MyCFGMakerPtr cm = ...
   PatchObject* obj = PatchObject::create(..., cm);

   MyAddrSpacePtr as = ...
   as->loadObject(obj);

   MyInstrumenter inst = ...
   PatchMgrPtr mgr = PatchMgr::create(as, ..., inst);

   MySnippet::ptr snippet = MySnippet::create(new MySnippet);

The above code shows how to register the above four types of plugins. An
instance of the factory class for creating CFG structures is registered
to an PatchObject (Line 1 and 2), which is in turn loaded into an
instance of AddrSpace (Line 4 and 5). The AddrSpace (or its subclass
implemented by programmers) instance is passed to PatchMgr::create (Line
7 and 8), together with an instance of Instrumenter (or its subclass).
Finally, a snippet of custom snippet representation MySnippet is created
(Line 10). Therefore, all plugins are glued together in PatchAPI.


.. _sec-dyn:

PatchAPI for Dyninst Programmers
********************************

The PatchAPI is a Dyninst component and as such is accessible through
the main Dyninst interface (BPatch objects). However, the PatchAPI
instrumentation and CFG models differ from the Dyninst models in several
critical ways that should be accounted for by users. This section
summarizes those differences and describes how to access PatchAPI
abstractions from the DyninstAPI interface.

Differences Between DyninstAPI and PatchAPI
===========================================

The DyninstAPI and PatchAPI differ primarily in their CFG
representations and instrumentation point abstractions. In general,
PatchAPI is more powerful and can better represent complex binaries
(e.g., highly optimized code or malware). In order to maintain backwards
compatibility, the DyninstAPI interface has not been extended to match
the PatchAPI. As a result, there are some caveats.

The PatchAPI uses the same CFG model as the ParseAPI. The primary
representation is an interprocedural graph of basic blocks and edges.
Functions are defined on top of this graph as collections of blocks. **A
block may be contained by more than one function;** we call this the
*shared block* model. Functions are defined to have a single entry
block, and functions may overlap if they contain the same blocks. Call
and return edges exist in the graph, and therefore traversing the graph
may enter different functions. PatchAPI users may specify instrumenting
a particular block within a particular function (a *block instance*) by
specifying both the block and the function.

The DyninstAPI uses a historic CFG model. The primary representation is
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
``BPatch_module`` represents a collection of functions from a particular
source file (for the executable) or from an entire library (for all
libraries). PatchAPI, like ParseAPI, instead uses an *object*
representation, where a ``PatchObject`` object represents a collection
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
same (see
`Bernat_AWAT <ftp://ftp.cs.wisc.edu/paradyn/papers/Bernat11AWAT.pdf>`__).
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

PatchAPI accessor methods in Dyninst
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To access a PatchAPI class from a Dyninst class, use the
``PatchAPI::convert`` function, as in the following example:

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

We do not support a direct mapping between ``BPatch_point`` s and
``Point`` s, as the failure of Dyninst to properly represent behavior
instPoints leads to confusing results. Instead, use the PatchAPI point
lookup methods.

Classes in PatchAPI use either the C++ raw pointer or the boost shared
pointer (*boost::shared_ptr<T>*) for memory management. A class uses a
raw pointer whenever it is returning a handle to the user that is
controlled and destroyed by the PatchAPI runtime library. Classes that
use a raw pointer include the CFG objects, a Point, and various plugins,
e.g., AddrSpace, CFGMaker, PointMaker, and Instrumenter. A class uses a
shared_pointer whenever it is handing something to the user that the
PatchAPI runtime library is not controlling and destroying. Classes that
use a boost shared pointer include a Snippet, PatchMgr, and Instance,
where we typedef a class’s shared pointer by appending the Ptr to the
class name, e.g., PatchMgrPtr for PatchMgr.

CFG Interface
^^^^^^^^^^^^^

Point/Snippet Interface
^^^^^^^^^^^^^^^^^^^^^^^

Callback Interface
^^^^^^^^^^^^^^^^^^

Modification API Reference
**************************

This section describes the modification interface of PatchAPI. While
PatchAPI’s main goal is to allow users to insert new code into a
program, a secondary goal is to allow safe modification of the original
program code as well.

To modify the binary, a user interacts with the ``PatchModifier`` class
to manipulate a PatchAPI CFG. CFG modifications are then instantiated as
new code by the PatchAPI. For example, if PatchAPI is being used as part
of Dyninst, executing a ``finalizeInsertionSet`` will generate modified
code.

The three key benefits of the PatchAPI modification interface are
abstraction, safety, and interactivity. We use the CFG as a mechanism
for transforming binaries in a platform-independent way that requires no
instruction-level knowledge by the user. These transformations are
limited to ensure that the CFG can always be used to instantiate code,
and thus the user can avoid unintended side-effects of modification.
Finally, modifications to the CFG are represented in that CFG, allowing
users to iteratively combine multiple CFG transformations to achieve
their goals.

Since modification can modify the CFG, it may invalidate any analyses
the user has performed over the CFG. We suggest that users take
advantage of the callback interface described in Section
`4.3.1 <#sec-3.2.7>`__ to update any such analysis information.

The PatchAPI modification capabilities are currently in beta; if you
experience any problems or bugs, please contact ``bugs@dyninst.org``.

Many of these methods return a boolean type; true indicates a successful
operation, and false indicates a failure. For methods that return a
pointer, a ``NULL`` return value indicates a failure.

.. code-block:: cpp
    
    bool redirect(PatchEdge *edge, PatchBlock *target);

Redirects the edge specified by ``edge`` to a new target specified by
``target``. In the current implementation, the edge may not be indirect.

.. code-block:: cpp
    
    PatchBlock *split(PatchBlock *orig, Address addr, bool trust = false,
    Address newlast = (Address) -1);

Splits the block specified by ``orig``, creating a new block starting at
``addr``. If ``trust`` is true, we do not verify that ``addr`` is a
valid instruction address; this may be useful to reduce overhead. If
``newlast`` is not -1, we use it as the last instruction address of the
first block. All Points are updated to belong to the appropriate block.
The second block is returned.

.. code-block:: cpp
    
    bool remove(std::vector<PatchBlock *> &blocks, bool force = true)

Removes the blocks specified by ``blocks`` from the CFG. If ``force`` is
true, blocks are removed even if they have incoming edges; this may
leave the CFG in an unsafe state but may be useful for reducing
overhead.

.. code-block:: cpp
    
    bool remove(PatchFunction *func)

Removes ``func`` and all of its non-shared blocks from the CFG; any
shared blocks remain.

.. code-block:: cpp
    
    class InsertedCode typedef boost::shared_ptr<...> Ptr; PatchBlock
    *entry(); const std::vector<PatchEdge *> &exits(); const
    std::set<PatchBlock *> &blocks();

    InsertedCode::Ptr insert(PatchObject *obj, SnippetPtr snip, Point
    *point); InsertedCode::Ptr insert(PatchObject *obj, void *start,
    unsigned size);

Methods for inserting new code into a CFG. The ``InsertedCode``
structure represents a CFG subgraph generated by inserting new code; the
graph has a single entry point and multiple exits, represented by edges
to the sink node. The first ``insert`` call takes a PatchAPI Snippet
structure and a Point that is used to generate that Snippet; the point
is only passed through to the snippet code generator and thus may be
``NULL`` if the snippet does not use Point information. The second
``insert`` call takes a raw code buffer.

.. _sec-plugin-api:

Plugin API Reference
====================

This section describes the various plugin interfaces for extending
PatchAPI. We expect that most users should not have to ever explicitly
use an interface from this section; instead, they will use plugins
previously implemented by PatchAPI developers.

Default Plugin
^^^^^^^^^^^^^^
