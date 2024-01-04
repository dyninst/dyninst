.. _`sec:Point.h`:

Point.h
#######

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:type:: std::map<Dyninst::Address, Point*> InsnPoints

.. cpp:class:: Point

  **A container of snippets**

  .. cpp:type:: std::list<InstancePtr>::iterator instance_iter

  .. cpp:function:: Point()

      Creates an empty point with no type, blocks, or edges.

  .. cpp:function:: Point(Type t, PatchMgrPtr mgr, PatchBlock *b, PatchFunction *f = NULL)

      Creates an instrumentation point of type ``t`` for the block ``b`` managed by ``mgr`` with
      an optional function context ``f``.

  .. cpp:function:: Point(Type t, PatchMgrPtr mgr, PatchBlock *b, Dyninst::Address addr, \
                          InstructionAPI::Instruction i, PatchFunction * = NULL)

      Creates an instrumentation point of type ``t`` for the block ``b`` at instruction ``i``
      managed by ``mgr`` with an optional function context ``f``.

  .. cpp:function:: Point(Type t, PatchMgrPtr mgr, PatchFunction *f)

      Creates an instrumentation point of type ``t`` in the function ``f`` managed by ``mgr``.

  .. cpp:function:: Point(Type t, PatchMgrPtr mgr, PatchFunction *f, PatchBlock *b)

      Creates an instrumentation point of type ``t`` for the block ``b`` managed by ``mgr`` for
      the function call or exit site ``f``.

  .. cpp:function:: Point(Type t, PatchMgrPtr mgr, PatchEdge *e, PatchFunction *f = NULL)

      Creates an instrumentation point of type ``t`` for the edge ``e`` managed by ``mgr``
      with an optional function context ``f``.

  .. cpp:function:: instance_iter begin()

      Returns an iterator to the first instance contained in this point.

  .. cpp:function:: instance_iter end()

      Returns an iterator to one-past the last instance contained in this point.

  .. cpp:function:: InstancePtr pushBack(SnippetPtr s)

      Adds the snippet ``s`` to the end of the sequence of instances at this point.

      Multiple instances can be inserted at the same point.

      Returns the created instance.

  .. cpp:function:: InstancePtr pushFront(SnippetPtr s)

      Adds the snippet ``s`` to the front of the sequence of instances at this point.

      Multiple instances can be inserted at the same point.

      Returns the created instance.

  .. cpp:function:: bool remove(InstancePtr instance)

      Removes the snippet ``instance`` from this point.

  .. cpp:function:: void clear()

      Removes all snippet instances inserted in this Point.

  .. cpp:function:: size_t size()

      Returns the number of snippet instances inserted at this Point.

  .. cpp:function:: Address addr() const

      Returns the address associated with this point, if it has one.

      Otherwise, returns 0.

  .. cpp:function:: Type type() const

      Returns the type of this point.

  .. cpp:function:: bool empty() const

      Checks if this point contains any instrumentation instances.

  .. cpp:function:: PatchFunction* getCallee()

      Returns the function invoked at this Point.

      The function should have type of :cpp:enumerator:`Point::Type::PreCall`
      or :cpp:enumerator:`Point::Type::PostCall`.

      Returns ``NULL`` if there is no function invoked at this point.

  .. cpp:function:: const PatchObject* obj() const

      Returns the object where this point resides.

  .. cpp:function:: const InstructionAPI::Instruction::Ptr insn() const

      Returns the instruction where this point resides.

  .. cpp:function:: PatchFunction* func() const

      Returns the function where this point resides.

  .. cpp:function:: PatchBlock* block() const

      Returns the block where this point resides.

  .. cpp:function:: PatchEdge* edge() const

      Returns the edge where this point resides.

  .. cpp:function:: PatchMgrPtr mgr() const

      Returns the patch factory for this point.

  .. cpp:function:: PatchCallback *cb() const

      Returns the callback associated with this point.

  .. cpp:function:: static bool TestType(Type types, Type type)

      Checks if a set of ``types`` contains the value of ``type``.

  .. cpp:function:: static void AddType(Type& types, Type type)

      Adds ``type`` to the set of types in ``types``.

  .. cpp:function:: static void RemoveType(Type& types, Type trg)

      Removes the ``type`` from the set of types in ``types``.

.. cpp:function:: inline Point::Type operator|(Point::Type a, Point::Type b)

  Combines ``a`` and ``b`` into a single set of types.

.. cpp:function:: inline const char* type_str(Point::Type type)

  Returns a string representation of ``type``.

.. cpp:enum:: SnippetType

  .. cpp:enumerator:: SYSTEM
  .. cpp:enumerator:: USER

.. cpp:enum:: SnippetState

  .. cpp:enumerator:: FAILED
  .. cpp:enumerator:: PENDING
  .. cpp:enumerator:: INSERTED


.. cpp:enum:: Point::Type

  .. cpp:enumerator:: PreInsn
  .. cpp:enumerator:: PostInsn
  .. cpp:enumerator:: BlockEntry
  .. cpp:enumerator:: BlockExit
  .. cpp:enumerator:: BlockDuring
  .. cpp:enumerator:: FuncEntry
  .. cpp:enumerator:: FuncExit
  .. cpp:enumerator:: FuncDuring
  .. cpp:enumerator:: EdgeDuring
  .. cpp:enumerator:: LoopStart
  .. cpp:enumerator:: LoopEnd
  .. cpp:enumerator:: LoopIterStart
  .. cpp:enumerator:: LoopIterEnd
  .. cpp:enumerator:: PreCall
  .. cpp:enumerator:: PostCall
  .. cpp:enumerator:: OtherPoint
  .. cpp:enumerator:: None
  .. cpp:enumerator:: InsnTypes
  .. cpp:enumerator:: BlockTypes
  .. cpp:enumerator:: FuncTypes
  .. cpp:enumerator:: EdgeTypes
  .. cpp:enumerator:: LoopTypes
  .. cpp:enumerator:: CallTypes

.. cpp:struct:: EntrySite_t

  .. cpp:member:: PatchFunction *func
  .. cpp:member:: PatchBlock *block

  .. cpp:function:: EntrySite_t(PatchFunction *f, PatchBlock *b)

.. cpp:struct:: CallSite_t

  .. cpp:member:: PatchFunction *func
  .. cpp:member:: PatchBlock *block

  .. cpp:function:: CallSite_t(PatchFunction *f, PatchBlock *b)

.. cpp:struct:: ExitSite_t

  .. cpp:member:: PatchFunction *func
  .. cpp:member:: PatchBlock *block

  .. cpp:function:: ExitSite_t(PatchFunction *f, PatchBlock *b)

.. cpp:struct:: InsnLoc_t

  .. cpp:member:: PatchBlock *block
  .. cpp:member:: Dyninst::Address addr
  .. cpp:member:: InstructionAPI::Instruction insn


.. cpp:class:: Location

  Uniquely identifies the location of a :cpp:class:`Point`.

  .. cpp:member:: PatchFunction *func
  .. cpp:member:: PatchBlock *block
  .. cpp:member:: Dyninst::Address addr
  .. cpp:member:: InstructionAPI::Instruction insn
  .. cpp:member:: PatchEdge *edge
  .. cpp:member:: bool trusted
  .. cpp:member:: type_t type

  .. cpp:function:: static Location Function(PatchFunction* f)
  .. cpp:function:: static Location Block(PatchBlock* b)
  .. cpp:function:: static Location BlockInstance(PatchFunction* f, PatchBlock* b, bool trusted = false)
  .. cpp:function:: static Location Instruction(InsnLoc_t l)
  .. cpp:function:: static Location Instruction(PatchBlock* b, Dyninst::Address a)
  .. cpp:function:: static Location InstructionInstance(PatchFunction* f, InsnLoc_t l, bool trusted = false)
  .. cpp:function:: static Location InstructionInstance(PatchFunction* f, PatchBlock* b, Dyninst::Address a)
  .. cpp:function:: static Location InstructionInstance(PatchFunction* f, PatchBlock* b, Dyninst::Address a, InstructionAPI::Instruction i, bool trusted = false)
  .. cpp:function:: static Location Edge(PatchEdge* e)
  .. cpp:function:: static Location EdgeInstance(PatchFunction* f, PatchEdge* e)
  .. cpp:function:: static Location EntrySite(EntrySite_t e)
  .. cpp:function:: static Location EntrySite(PatchFunction* f, PatchBlock* b, bool trusted = false)
  .. cpp:function:: static Location CallSite(CallSite_t c)
  .. cpp:function:: static Location CallSite(PatchFunction* f, PatchBlock* b)
  .. cpp:function:: static Location ExitSite(ExitSite_t e)
  .. cpp:function:: static Location ExitSite(PatchFunction* f, PatchBlock* b)

  .. cpp:function:: bool legal(type_t t)

      Checks if this location is the same type as ``t``.

  .. cpp:function:: InsnLoc_t insnLoc()

.. cpp:enum:: Location::type_t

  .. cpp:enumerator:: Function_
  .. cpp:enumerator:: Block_
  .. cpp:enumerator:: BlockInstance_
  .. cpp:enumerator:: Instruction_
  .. cpp:enumerator:: InstructionInstance_
  .. cpp:enumerator:: Edge_
  .. cpp:enumerator:: EdgeInstance_
  .. cpp:enumerator:: Entry_
  .. cpp:enumerator:: Call_
  .. cpp:enumerator:: Exit_
  .. cpp:enumerator:: Illegal_


.. cpp:class:: Instance : public boost::enable_shared_from_this<Instance>

  **A snippet inserted at a Point**

  A unique instance is created for every snippet insertion, even if the same snippet
  is inserted multiple times at the same point.

  .. cpp:type:: boost::shared_ptr<Instance> Ptr

  .. cpp:function:: Instance(Point* point, SnippetPtr snippet)

      Creates an instrumentation instance containing the code in ``snippet`` at the point ``point``.

  .. cpp:function:: static InstancePtr create(Point* p, SnippetPtr s, SnippetType type = SYSTEM, \
                                              SnippetState state = PENDING)

      Creates an instrumentation instance containing the code in ``s`` at the point ``p`` of type
      ``type`` in the starting state ``state``.

  .. cpp:function:: SnippetState state() const

      Returns the current state of the contained snippet.

  .. cpp:function:: SnippetType type() const

      Returns the type of the contained snippet.

  .. cpp:function:: Point* point() const

      Returns the point where the instance is inserted.

  .. cpp:function:: SnippetPtr snippet() const

      Returns the code snippet for this instance.

  .. cpp:function:: bool destroy()

      Destroys the underlying snippet.


.. cpp:class:: PointMaker

  **A factory for creating points**

  .. cpp:function:: PointMaker(PatchMgrPtr mgr)

      Creates a factory to create points owned by ``mgr``.

  .. cpp:function:: Point *createPoint(Location loc, Point::Type type)

      Creates a point at location ``loc`` of type ``type``.


Notes
=====

The Location structure uniquely identifies the physical location of a
point. A Location object plus a Point::Type value uniquely identifies a
point, because multiple Points with different types can exist at the
same physical location. The Location structure provides a set of static
functions to create an object of Location, where each function takes the
corresponding CFG structures to identify a physical location. In
addition, some functions above (e.g., InstructionInstance) takes input
the ``trusted`` parameter that is to indicate PatchAPI whether the CFG
structures passed in is trusted. If the ``trusted`` parameter is false,
then PatchAPI would have additional checking to verify the CFG
structures passed by users, which causes nontrivial overhead.

A location on the :cpp:class:`CFG` that acts as a container of inserted instances.  Points
of different types are distinct even the underlying code relocation and
generation engine happens to put instrumentation from them at the same place

:cpp:enum:`Point::Type` specifies the logical point type. Multiple enum
values can be OR-ed to form a composite type. For example, the composite
type of ``PreCall | BlockEntry | FuncExit`` is to specify a set of
points with the type PreCall, or BlockEntry, or FuncExit.
