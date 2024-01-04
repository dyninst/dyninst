.. _`sec:PatchMgr.h`:

PatchMgr.h
##########

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:class:: PatchMgr : public boost::enable_shared_from_this<PatchMgr>

  **Interface for finding instrumentation Points, inserting or deleting Snippets, and registering user-provided plugins**

  .. cpp:type:: boost::shared_ptr<PatchMgr> Ptr
  .. cpp:type:: std::pair<Location, Point::Type> Candidate
  .. cpp:type:: std::vector<Candidate> Candidates

  .. cpp:function:: PatchMgr(AddrSpace* as, Instrumenter* inst, PointMaker* pf)

      Creates a patch manager spanning the address space ``as`` using the factories
      ``inst`` and ``pf`` for creating instrumentation and points, respectively.

  .. cpp:function:: static PatchMgrPtr create(AddrSpace* as, Instrumenter* inst = NULL, PointMaker* pm = NULL)

      A helper for creating a ``PatchMgr``.

  .. cpp:function:: Point* findPoint(Location loc, Point::Type type, bool create = true)

      Returns a unique Point at ``loc`` and type ``type``. The point is created when
      ``create`` is ``true``.

      The Location specifies a physical location of a Point (e.g., at function entry,
      at block entry, etc.). If the Point already exists, it is always used, regardless
      of the value of ``create``.

      Returns ``NULL`` on failure.

  .. cpp:function:: template <class OutputIterator> \
                    bool findPoint(Location loc, Point::Type type, \
                    OutputIterator outputIter, bool create = true)

      Writes a unique Point at ``loc`` and type ``type`` into ``outputIter``. The point is created when
      ``create`` is ``true``.

      .. Note:: ``OutputIterator`` must support the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

      The Location specifies a physical location of a Point (e.g., at function entry,
      at block entry, etc.). If the Point already exists, it is always used, regardless
      of the value of ``create``.

      Returns ``false`` on failure.

  .. cpp:function:: template <class OutputIterator> \
                    bool findPoints(Location loc, Point::Type types, \
                    OutputIterator outputIter, bool create = true)

      Writes all Points at ``loc`` with composite types ``type`` into ``outputIter``. The point is created when
      ``create`` is ``true``.

      .. Note:: ``OutputIterator`` must support the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

      The Location specifies a physical location of a Point (e.g., at function entry,
      at block entry, etc.). If the Point already exists, it is always used, regardless
      of the value of ``create``. It is assumed ``types`` is some number of ``Point::Type``\ s
      combined using ``operator |``.

      Returns ``false`` on failure.

  .. cpp:function:: template <class FilterFunc, class FilterArgument, class OutputIterator> \
                    bool findPoints(Location loc, Point::Type types, FilterFunc filter_func, \
                    FilterArgument filter_arg, OutputIterator outputIter, bool create = true)

      Writes all Points at ``loc`` with composite types ``type`` into ``outputIter`` satisfying the filter
      predicate ``fulter_func(filter_arg)``. The point is created when ``create`` is ``true``.

      .. Note:: ``OutputIterator`` must support the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

      The Location specifies a physical location of a Point (e.g., at function entry,
      at block entry, etc.). If the Point already exists, it is always used, regardless
      of the value of ``create``. It is assumed ``types`` is some number of ``Point::Type``\ s
      combined using ``operator |``.

      Returns ``false`` on failure.

  .. cpp:function:: template <class FilterFunc, class FilterArgument, class OutputIterator> \
                    bool findPoints(Scope scope, Point::Type types, FilterFunc filter_func, \
                    FilterArgument filter_arg, OutputIterator output_iter, bool create = true)

      Writes all Points in the scope ``scope`` with composite types ``type`` into ``outputIter`` satisfying the filter
      predicate ``fulter_func(filter_arg)``. The point is created when ``create`` is ``true``.

      .. Note:: ``OutputIterator`` must support the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

      If the Point already exists, it is always used, regardless
      of the value of ``create``. It is assumed ``types`` is some number of ``Point::Type``\ s
      combined using ``operator |``.

      Returns ``false`` if no point is created.

  .. cpp:function:: template <class OutputIterator> \
                    bool findPoints(Scope scope, Point::Type types, OutputIterator output_iter, \
                    bool create = true)

      Writes all Points in the scope ``scope`` with composite types ``types``. The point is created when ``create`` is ``true``.

      .. Note:: ``OutputIterator`` must support the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

      If the Point already exists, it is always used, regardless
      of the value of ``create``. It is assumed ``types`` is some number of ``Point::Type``\ s
      combined using ``operator |``.

      Returns ``false`` if no point is created.

  .. cpp:function:: bool removeSnippet(InstancePtr i)

      Removes the snippet ``i``.

      Returns ``false`` on error.

  .. cpp:function:: template <class FilterFunc, class FilterArgument> \
                    bool removeSnippets(Scope scope, Point::Type types, FilterFunc filter_func, \
                    FilterArgument filter_arg)

      Deletes **ALL** snippets in the scope ``scope`` with composite types ``types`` satisfying the filter
      predicate ``fulter_func(filter_arg)``.

      It is assumed ``types`` is some number of ``Point::Type``\ s combined using ``operator |``.

      Returns ``false`` if no point is found.

  .. cpp:function:: bool removeSnippets(Scope scope, Point::Type types)

      Deletes **ALL** snippets in the scope ``scope`` with composite types ``types``

      Returns ``false`` if no point is found.

  .. cpp:function:: void destroy(Point *point)

      Destroys the point ``point``.

  .. cpp:function:: AddrSpace* as() const

      Returns the underlying address space.

  .. cpp:function:: PointMaker* pointMaker() const

      Returns the underlying point factory.

  .. cpp:function:: Instrumenter* instrumenter() const

      Returns the underlying instrumentation factory.

  .. cpp:function:: bool getCandidates(Scope &s, Point::Type types, Candidates& ret)

      Fills ``ret`` with the points from the scope ``s`` having composite type ``types``.

      It is assumed ``types`` is some number of ``Point::Type``\ s combined using ``operator |``.

      | Mapping order: Scope -> Type -> Point Set
      |   The Scope x Type provides us a list of matching locations;
      |   we then filter those locations. Points are stored in
      |   their contexts (e.g., Functions or Blocks).

      Returns ``false`` if no point is found.

  .. cpp:function:: bool consistency() const


.. cpp:class:: template <class T> PatchMgr::IdentityFilterFunc

  .. cpp:function:: bool operator()(Point::Type, Location l, T t)


.. cpp:struct:: Scope

  **The scope to find points**

  A scope could be a function or a basic block. In the case that the exact location
  is not known, then a Scope can be used as a wildcard.

  .. cpp:function:: Scope(PatchBlock *b)

      Creates a scope for the block ``b``.

      A basic block can be contained in multiple functions.

  .. cpp:function:: Scope(PatchFunction *f, PatchBlock *b)

      Creates a scope for the block ``b`` in the function ``f``.

      A basic block can be contained in multiple functions.

  .. cpp:function:: Scope(PatchFunction *f)

      Creates a scope for the function ``f``; including all basic blocks.

  .. cpp:member:: PatchObject *obj
  .. cpp:member:: PatchFunction *func
  .. cpp:member:: PatchBlock *block
  .. cpp:member:: bool wholeProgram
