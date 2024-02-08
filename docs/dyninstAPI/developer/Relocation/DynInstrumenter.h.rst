.. _`sec:DynInstrumenter.h`:

DynInstrumenter.h
#################

.. cpp:namespace:: Dyninst::Relocation

.. cpp:class:: DynInstrumenter : public Dyninst::PatchAPI::Instrumenter

  .. cpp:function:: DynInstrumenter()
  .. cpp:function:: virtual ~DynInstrumenter()
  .. cpp:function:: virtual bool run()
  .. cpp:function:: virtual bool undo()
  .. cpp:function:: virtual bool isInstrumentable(PatchFunction*)

.. cpp:class:: DynInsertSnipCommand : public Command

  Dyninst-specific Insert Snippet Command

  .. cpp:function:: DynInsertSnipCommand(instPoint* pt, callOrder order, AstNodePtr ast, bool recursive)
  .. cpp:function:: static DynInsertSnipCommand* create(instPoint* pt, callOrder order, AstNodePtr ast, bool recursive)
  .. cpp:function:: virtual ~DynInsertSnipCommand()
  .. cpp:function:: Instance::Ptr inst()
  .. cpp:function:: virtual bool run()
  .. cpp:function:: virtual bool undo()
  .. cpp:member:: protected Instance::Ptr inst_

.. cpp:class:: DynRemoveSnipCommand : public Command

  Dyninst-specific Remove Snippet Command

  .. cpp:function:: DynRemoveSnipCommand(Instance::Ptr inst)
  .. cpp:function:: static DynRemoveSnipCommand* create(Instance::Ptr inst)
  .. cpp:function:: virtual ~DynRemoveSnipCommand()
  .. cpp:function:: virtual bool run()
  .. cpp:function:: virtual bool undo()
  .. cpp:member:: protected Instance::Ptr inst_

.. cpp:class:: DynReplaceFuncCommand : public Command

  Dyninst-specific Function Replacement

  .. cpp:function:: DynReplaceFuncCommand(AddressSpace* as, func_instance* old_func, func_instance* new_func)
  .. cpp:function:: static DynReplaceFuncCommand* create(AddressSpace* as, func_instance* old_func, func_instance* new_func)
  .. cpp:function:: virtual ~DynReplaceFuncCommand()
  .. cpp:function:: virtual bool run()
  .. cpp:function:: virtual bool undo()
  .. cpp:member:: protected AddressSpace* as_
  .. cpp:member:: protected func_instance *old_func_
  .. cpp:member:: protected func_instance *new_func_

.. cpp:class:: DynModifyCallCommand : public Command

  Dyninst-specific Modify Function call

  .. cpp:function:: DynModifyCallCommand(AddressSpace* as, block_instance* block, func_instance* new_func, func_instance* context)
  .. cpp:function:: static DynModifyCallCommand* create(AddressSpace* as, block_instance* block, func_instance* new_func, func_instance* context)
  .. cpp:function:: virtual ~DynModifyCallCommand()
  .. cpp:function:: virtual bool run()
  .. cpp:function:: virtual bool undo()
  .. cpp:member:: protected AddressSpace* as_
  .. cpp:member:: protected block_instance *block_
  .. cpp:member:: protected func_instance *new_func_
  .. cpp:member:: protected func_instance *context_

.. cpp:class:: DynRemoveCallCommand : public Command

  Dyninst-specific Remove Function call

  .. cpp:function:: DynRemoveCallCommand(AddressSpace* as, block_instance* block, func_instance* context)
  .. cpp:function:: static DynRemoveCallCommand* create(AddressSpace* as, block_instance* block, func_instance* context)
  .. cpp:function:: virtual ~DynRemoveCallCommand()
  .. cpp:function:: virtual bool run()
  .. cpp:function:: virtual bool undo()
  .. cpp:member:: protected AddressSpace* as_
  .. cpp:member:: protected block_instance *block_
  .. cpp:member:: protected func_instance *context_
