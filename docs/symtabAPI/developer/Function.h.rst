.. _`sec-dev:Function.h`:

Function.h
##########

.. cpp:namespace:: Dyninst::SymtabAPI::dev

.. cpp:class:: FunctionBase

  .. cpp:function:: bool setReturnType(boost::shared_ptr<Type>)

      Sets the return type for this function.

  .. cpp:function:: bool setModule (Module *module)

      Makes ``module`` the owning module.

      Returns ``false`` on error.

  .. cpp:function:: bool setSize (unsigned size)

      Sets the size of the function to ``size``.

      Returns ``false`` on error.

  .. cpp:function:: bool setOffset (Offset offset)

      Sets the offset of the function to ``offset``.

      Returns ``false`` on error.

  .. cpp:function:: virtual bool addMangledName(std::string name, bool isPrimary, bool isDebug=false) = 0

      Adds the mangled name ``name`` to the function. If ``isPrimary`` is ``true`` then it becomes the
      default name for the function.

      Returns ``false`` on error.

  .. cpp:function:: virtual bool addPrettyName(std::string name, bool isPrimary, bool isDebug=false) = 0

      Adds the pretty name ``name`` to the function. If ``isPrimary`` is ``true`` then it becomes the default
      name for the function.

      Returns ``false`` on error.

  .. cpp:function:: bool addTypedName(string name, bool isPrimary)

      Adds the typed name ``name`` to the function. If ``isPrimary``
      is ``true`` then it becomes the default name for the function.

      Returns ``false`` on error.

  .. cpp:function:: bool setFramePtr(std::vector<VariableLocation>* locs)
  .. cpp:function:: std::vector<VariableLocation> &getFramePtrRefForInit()
  .. cpp:function:: dyn_mutex &getFramePtrLock()

  .. cpp:member:: protected localVarCollection* locals
  .. cpp:member:: protected localVarCollection* params
  .. cpp:member:: protected mutable unsigned functionSize_
  .. cpp:member:: protected boost::shared_ptr<Type> retType_
  .. cpp:member:: protected dyn_mutex inlines_lock
  .. cpp:member:: protected InlineCollection inlines
  .. cpp:member:: protected FunctionBase* inline_parent
  .. cpp:member:: protected FuncRangeCollection ranges
  .. cpp:member:: protected std::vector<VariableLocation> frameBase_
  .. cpp:member:: protected dyn_mutex frameBaseLock_
  .. cpp:member:: protected bool frameBaseExpanded_
  .. cpp:member:: protected void* data

  .. cpp:function:: protected FunctionBase()
  .. cpp:function:: protected void expandLocation(const VariableLocation &loc, std::vector<VariableLocation> &ret)


.. cpp:type:: std::vector<FuncRange> FuncRangeCollection


.. cpp:class:: Function : public FunctionBase, public Aggregate

  .. cpp:function:: bool removeSymbol(Symbol *sym)

  .. cpp:function:: bool setFramePtrRegnum(int regnum)

        IA64-Specific frame pointer information.

  .. cpp:function:: int getFramePtrRegnum() const

      IA64-Specific frame pointer information.

  .. cpp:function:: Offset getPtrOffset() const

      PPC64 Linux specific information.

  .. cpp:function:: Offset getTOCOffset() const

      PPC64 Linux specific information.


.. cpp:class:: InlinedFunction : public FunctionBase

  .. cpp:function:: bool removeSymbol(Symbol *sym)
  .. cpp:function:: void setFile(std::string filename)


