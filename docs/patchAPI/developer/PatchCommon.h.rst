.. _`sec:PatchCommon.h`:

PatchCommon.h
#############

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:type:: std::set<Point*> PointSet
.. cpp:type:: PointSet::iterator PointIter
.. cpp:type:: boost::shared_ptr<Instance> InstancePtr
.. cpp:type:: std::set<InstancePtr> InstanceSet
.. cpp:type:: std::list<InstancePtr> InstanceList
.. cpp:type:: boost::shared_ptr<PatchMgr> PatchMgrPtr
.. cpp:type:: boost::shared_ptr<Snippet> SnippetPtr
.. cpp:type:: std::map<PatchFunction*, PatchFunction*> FuncModMap

  Maps a PatchFunction to another PatchFunction.

.. cpp:type:: std::map<PatchFunction*, std::pair<PatchFunction*, std::string> > FuncWrapMap

.. cpp:type:: std::map<PatchBlock*, std::map<PatchFunction*, PatchFunction*> > CallModMap

  | This is a little complex, so let me explain my logic.
  |
  | Map from B -> F_c -> F
  |
  |   B identifies a call site
  |
  |   F_c identifies an (optional) function context for the replacement
  |     If F_c is not specified, we use NULL
  |
  |   F specifies the replacement callee
  |     If we want to remove the call entirely, also use NULL

.. cpp:type:: std::set<ParseAPI::CodeObject*> CodeObjectSet

.. cpp:type:: std::set<ParseAPI::CodeSource*> CodeSourceSet

.. c:macro:: patchapi_debug

  Set ``DYNINST_DEBUG_PATCHAPI`` in your environment to enable.
