.. _`sec:PatchObject.h`:

PatchObject.h
#############

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:class:: PatchObject

  **Wrapper around a ParseAPI::CodeObject**

  Represents an individual binary code object, such as an executable or a library.

  .. cpp:type:: std::vector<PatchFunction*> funclist
  .. cpp:type:: std::map<const ParseAPI::Function*, PatchFunction*> FuncMap
  .. cpp:type:: std::map<const ParseAPI::Block*, PatchBlock*> BlockMap
  .. cpp:type:: std::map<const ParseAPI::Edge*, PatchEdge*> EdgeMap

  .. cpp:function:: static PatchObject* create(ParseAPI::CodeObject* co, Address base, CFGMaker* cm = NULL, PatchCallback* cb = NULL)

      Creates patch in the on-disk object ``co`` loaded in mmeory at address ``base`` using the CFG factory ``cm`` and
      the callback ``cb``.

      For binary rewriting, ``base`` should be 0.

  .. cpp:function:: static PatchObject* clone(PatchObject* par_obj, Address base, CFGMaker* cm = NULL, PatchCallback* cb = NULL)

      Creates a deep copy of the patch ``par_obj`` in the on-disk object ``co`` loaded in mmeory at address ``base`` using
      the CFG factory ``cm`` and the callback ``cb``.

      For binary rewriting, ``base`` should be 0.

  .. cpp:function:: Address codeBase()

      Returns the base address where this object is loaded in memory.

  .. cpp:function:: Address codeOffsetToAddr(Address offset) const
  .. cpp:function:: Address addrMask() const
  .. cpp:function:: ParseAPI::CodeObject* co() const
  .. cpp:function:: PatchMgrPtr mgr() const

  .. cpp:function:: PatchFunction* getFunc(ParseAPI::Function* func, bool create = true)

      Returns the patch function that wraps the ParseAPI function ``func``. The function is
      created when it's not found and ``create`` is ``true``.

      Returns ``NULL`` in two cases: 1) ``func`` is not in this object, 2) ``func`` does not exist
      in *any* object and ``create`` is ``false``.

  .. cpp:function:: template <class Iter> void funcs(Iter iter)

      Writes all functions in this object into ``iter``.

      .. Note:: ``Iter`` must support the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

  .. cpp:function:: PatchBlock* getBlock(ParseAPI::Block* blk, bool create = true)

      Returns the patch block that wraps the ParseAPI block ``blk``. The function is
      created when it's not found and ``create`` is ``true``.


      Returns ``NULL`` in two cases: 1) ``blk`` is not in this object, 2) ``blk`` does not exist
      in *any* object and ``create`` is ``false``.

  .. cpp:function:: template <class Iter> void blocks(Iter iter)

      Writes all blocks in this object into ``iter``.

      .. Note:: ``Iter`` must support the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

  .. cpp:function:: PatchEdge* getEdge(ParseAPI::Edge* edge, PatchBlock* src, PatchBlock* trg, bool create = true)

      Returns the patch edge that wraps the ParseAPI edge ``edge`` between ``src`` and ``trg``. The function is
      created when it's not found and ``create`` is ``true``.

      Returns ``NULL`` in two cases: 1) ``func`` is not in this object, 2) ``func`` does not exist
      in *any* object and ``create`` is ``false``.

  .. cpp:function:: template <class Iter> void edges(Iter iter)

      Writes all edges in this object into ``iter``.

      .. Note:: ``Iter`` must support the C++ `LegacyForwardIterator <https://en.cppreference.com/w/cpp/named_req/ForwardIterator>`_ concept.

  .. cpp:function:: PatchCallback* cb() const;

      Returns the PatchCallback object associated with this PatchObject.

  .. cpp:function:: std::string format() const

      Returns a string representation of this object.
