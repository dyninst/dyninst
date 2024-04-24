.. _`sec:Modification.h`:

Modification.h
##############

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: Modification : public Transformer

  .. cpp:type:: std::list<RelocBlock *> RelocBlockList
  .. cpp:type:: Dyninst::PatchAPI::CallModMap CallModMap

    Block (IDing a call site) -> func

  .. cpp:type:: Dyninst::PatchAPI::FuncModMap FuncModMap
  .. cpp:type:: Dyninst::PatchAPI::FuncWrapMap FuncWrapMap

  .. cpp:function:: virtual bool process(RelocBlock *cur, RelocGraph *)
  .. cpp:function:: Modification(const CallModMap &callRepl, const FuncModMap &funcRepl, const FuncWrapMap &funcWrap)
  .. cpp:function:: virtual ~Modification()
  .. cpp:function:: private bool replaceCall(RelocBlock *trace, RelocGraph *)
  .. cpp:function:: private bool replaceFunction(RelocBlock *trace, RelocGraph *)
  .. cpp:function:: private bool wrapFunction(RelocBlock *trace, RelocGraph *)
  .. cpp:function:: private RelocBlock *makeRelocBlock(block_instance *block, func_instance *func, RelocBlock *cur, RelocGraph *cfg)
  .. cpp:member:: private const CallModMap &callMods_
  .. cpp:member:: private const FuncModMap &funcReps_
  .. cpp:member:: private const FuncWrapMap &funcWraps_


.. cpp:struct:: Modification::WrapperPredicate

  .. cpp:function:: WrapperPredicate(func_instance *f)
  .. cpp:member:: func_instance *f_
  .. cpp:function:: bool operator()(RelocEdge *e)


.. cpp:struct:: Modification::WrapperPatch : public Patch

  .. cpp:function:: WrapperPatch(func_instance *func, std::string name)
  .. cpp:function:: virtual bool apply(codeGen &gen, CodeBuffer *buf)
  .. cpp:function:: virtual unsigned estimate(codeGen &)
  .. cpp:function:: virtual ~WrapperPatch()
  .. cpp:member:: func_instance *func_
  .. cpp:member:: std::string name_
