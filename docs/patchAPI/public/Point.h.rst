Point.h
=======

.. cpp:namespace:: Dyninst::patchAPI

Point
=====

**Declared in**: Point.h

The Point class is in essence a container of a list of snippet
instances. Therefore, the Point class has methods similar to those in
STL.

.. code-block:: cpp
    
    struct Location static Location Function(PatchFunction *f); static
    Location Block(PatchBlock *b); static Location
    BlockInstance(PatchFunction *f, PatchBlock *b, bool trusted = false);
    static Location Edge(PatchEdge *e); static Location
    EdgeInstance(PatchFunction *f, PatchEdge *e); static Location
    Instruction(PatchBlock *b, Address a); static Location
    InstructionInstance(PatchFunction *f, PatchBlock *b, Address a);
    static Location InstructionInstance(PatchFunction *f, PatchBlock *b,
    Address a, InstructionAPI::Instruction::Ptr i, bool trusted = false);
    static Location EntrySite(PatchFunction *f, PatchBlock *b, bool
    trusted = false); static Location CallSite(PatchFunction *f, PatchBlock
    *b); static Location ExitSite(PatchFunction *f, PatchBlock *b);;

The Location structure uniquely identifies the physical location of a
point. A Location object plus a Point::Type value uniquely identifies a
point, because multiple Points with different types can exist at the
same physical location. The Location structure provides a set of static
functions to create an object of Location, where each function takes the
corresponding CFG structures to identify a physical location. In
addition, some functions above (e.g., InstructionInstance) takes input
the *trusted* parameter that is to indicate PatchAPI whether the CFG
structures passed in is trusted. If the *trusted* parameter is false,
then PatchAPI would have additional checking to verify the CFG
structures passed by users, which causes nontrivial overhead.

.. code-block:: cpp

    enum Point::Type PreInsn, PostInsn, BlockEntry, BlockExit, BlockDuring, FuncEntry, FuncExit, FuncDuring, EdgeDuring, PreCall, PostCall, OtherPoint, None, InsnTypes = PreInsn | PostInsn, BlockTypes = BlockEntry | BlockExit | BlockDuring, FuncTypes = FuncEntry | FuncExit | FuncDuring, EdgeTypes = EdgeDuring, CallTypes = PreCall | PostCall;

The enum Point::Type specifies the logical point type. Multiple enum
values can be OR-ed to form a composite type. For example, the composite
type of “PreCall \| BlockEntry \| FuncExit” is to specify a set of
points with the type PreCall, or BlockEntry, or FuncExit.

.. code-block:: cpp
    
    typedef std::list<InstancePtr>::iterator instance_iter; instance_iter
    begin(); instance_iter end();

The method begin() returns an iterator pointing to the beginning of the
container storing snippet Instances, while the method end() returns an
iterator pointing to the end of the container (past the last element).

.. code-block:: cpp
    
    InstancePtr pushBack(SnippetPtr); InstancePtr pushFront(SnippetPtr);

Multiple instances can be inserted at the same Point. We maintain the
instances in an ordered list. The pushBack method is to push the
specified Snippet to the end of the list, while the pushFront method is
to push to the front of the list.

Both methods return the Instance that uniquely identifies the inserted
snippet.

.. code-block:: cpp
    
    bool remove(InstancePtr instance);

This method removes the given snippet *instance* from this Point.

.. code-block:: cpp
    
    void clear();

This method removes all snippet instances inserted to this Point.

.. code-block:: cpp
    
    size_t size();

Returns the number of snippet instances inserted at this Point.

.. code-block:: cpp
    
    Address addr() const;

Returns the address associated with this point, if it has one;
otherwise, it returns 0.

.. code-block:: cpp
    
    Type type() const;

Returns the Point type of this point.

.. code-block:: cpp
    
    bool empty() const;

Indicates whether the container of instances at this Point is empty or
not.

.. code-block:: cpp
    
    PatchFunction* getCallee();

Returns the function that is invoked at this Point, which should have
Point::Type of Point::PreCall or Point::PostCall. It there is not a
function invoked at this point, it returns NULL.

.. code-block:: cpp
    
    const PatchObject* obj() const;

Returns the PatchObject where the Point resides.

.. code-block:: cpp
    
    const InstructionAPI::Instruction::Ptr insn() const;

Returns the Instruction where the Point resides.

.. code-block:: cpp
    
    PatchFunction* func() const;

Returns the function where the Point resides.

.. code-block:: cpp
    
    PatchBlock* block() const;

Returns the PatchBlock where the Point resides.

.. code-block:: cpp
    
    PatchEdge* edge() const;

Returns the Edge where the Point resides.

.. code-block:: cpp
    
    PatchCallback *cb() const;

Returns the PatchCallback object that is associated with this Point.

.. code-block:: cpp
    
    static bool TestType(Point::Type types, Point::Type type);

This static method tests whether a set of *types* contains a specific
*type*.

.. code-block:: cpp
    
    static void AddType(Point::Type& types, Point::Type type);

This static method adds a specific *type* to a set of *types*.

.. code-block:: cpp
    
    static void RemoveType(Point::Type& types, Point::Type trg);

This static method removes a specific *type* from a set of *types*.

Instance
========

**Declared in**: Point.h

The Instance class is a representation of a particular snippet inserted
at a particular point. If a Snippet is inserted to N points or to the
same point for N times (N :math:`>` 1), then there will be N Instances.

.. code-block:: cpp
    
    bool destroy();

This method destroys the snippet Instance itself.

.. code-block:: cpp
    
    Point* point() const;

Returns the Point where the Instance is inserted.

.. code-block:: cpp
    
    SnippetPtr snippet() const;

Returns the Snippet. Please note that, the same Snippet may have
multiple instances inserted at different Points or the same Point.

PointMaker
==========

**Declared in**: Point.h

The PointMaker class is a factory class that constructs instances of the
Point class. The methods of the PointMaker class are invoked by
PatchMgr’s findPoint methods. Programmers can extend the Point class,
and then implement a set of virtual methods in this class to instantiate
the subclasses of Point.

.. code-block:: cpp
    
    PointMaker(PatchMgrPtr mgr);

The constructor takes input the relevant PatchMgr *mgr*.

.. code-block:: cpp
    
    virtual Point *mkFuncPoint(Point::Type t, PatchMgrPtr m, PatchFunction
    *f); virtual Point *mkFuncSitePoint(Point::Type t, PatchMgrPtr m,
    PatchFunction *f, PatchBlock *b); virtual Point
    *mkBlockPoint(Point::Type t, PatchMgrPtr m, PatchBlock *b,
    PatchFunction *context); virtual Point *mkInsnPoint(Point::Type t,
    PatchMgrPtr m, PatchBlock *, Address a,
    InstructionAPI::Instruction::Ptr i, PatchFunction *context); virtual
    Point *mkEdgePoint(Point::Type t, PatchMgrPtr m, PatchEdge *e,
    PatchFunction *context);

Programmers implement the above virtual methods to instantiate the
subclasses of Point.