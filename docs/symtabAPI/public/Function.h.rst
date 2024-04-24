.. _`sec:Function.h`:

Function.h
##########

.. cpp:namespace:: Dyninst::SymtabAPI

.. cpp:class:: FunctionBase

  **A common representation of a function**

  .. cpp:function:: boost::shared_ptr<Type> getReturnType(Type::do_share_t) const

      Returns the return type of the function.

  .. cpp:function:: Type* getReturnType() const

      Returns the return type of the function.

  .. cpp:function:: bool findLocalVariable(vector<localVar*> &vars, string name)

      Saves in ``vars`` all local variables in this function with name, ``name``.

      Returns ``true`` if at least one variable was found.

  .. cpp:function:: bool getLocalVariables(vector<localVar*> &vars)

      Saves in ``vars`` all local variables in this function.

      Returns ``false`` if there is no debugging information present.

  .. cpp:function:: bool getParams(vector<localVar*> &params)

      Saves in ``params`` all parameters for this function.

      Returns ``false`` if there is no debugging information present.

  .. cpp:function:: bool operator==(const FunctionBase &f)

      Checks if this function is equal to ``f``.

      Two functions are considered equal if they have the same return type and
      their underlying :cpp:class:`Aggregate`\ s are equal.

  .. cpp:function:: FunctionBase* getInlinedParent()

      Returns the function that this function is inlined into.

      Returns ``NULL`` if this function is not inlined.

  .. cpp:function:: const InlineCollection& getInlines()

      Gets the set of functions inlined into this one (possibly empty).

  .. cpp:function:: const FuncRangeCollection &getRanges()

      Returns the code ranges to which this function belongs.

  .. cpp:function:: std::vector<VariableLocation> &getFramePtr()

      Returns the frame pointer offsets (abstract top of the stack) for this function.

  .. cpp:function:: void* getData()

      Retrieves the user-defined data.

      See :cpp:func:`setData`.

  .. cpp:function:: void setData(void* d)

      Adds arbitrary user-defined data ``d`` to this function.

  .. cpp:function:: virtual std::string getName() const = 0

      Returns primary name of the function (first mangled name or DWARF name).

  .. cpp:function:: virtual Offset getOffset() const = 0

      Returns the starting position of this function.

  .. cpp:function:: virtual unsigned getSize() const = 0

      Returns the size in *bytes* encoded in the symbol table; may not be actual function size.

  .. cpp:function:: virtual Module* getModule() const = 0

      Returns the module this function belongs to.

  .. cpp:function:: virtual ~FunctionBase()


.. cpp:class:: FuncRange

  .. cpp:type:: Dyninst::Offset type

  .. cpp:member:: FunctionBase *container

      The function to which this range belongs.

  .. cpp:member:: Dyninst::Offset off

      The starting position of this range.

  .. cpp:member:: unsigned long size

      The size in *bytes* of this range.

  .. cpp:function:: FuncRange(Dyninst::Offset off_, size_t size_, FunctionBase *cont_)

      Creates a range starting at ``off_`` of ``size_`` bytes owned by ``cont``.

  .. cpp:function:: Dyninst::Offset low() const

      Returns the :cpp:member:`lower bound <off>` of the code region of this function.

  .. cpp:function:: Dyninst::Offset high() const

      Returns the upper bound of the code region of this function.

      This is :cpp:member:`off` ``+`` :cpp:member:`size`.


.. cpp:class:: Function : public FunctionBase, public Aggregate

  **A collection of symbols that have the same address and a type of ST_FUNCTION**

  When appropriate, use this representation instead of the underlying :cpp:class:`Symbol`\ s.

  .. note::

    This class can be derived from (e.g., ParseAPI::PLTFunction), but does not create an
    interface separate from FunctionBase.

  .. cpp:function:: Function()

      Creates a skeleton function with no return type, variables, or associated code.

  .. cpp:function:: Function(Symbol *sym)

      Creates a function representation of ``sym``.

  .. cpp:function:: virtual ~Function()

  .. cpp:function:: unsigned getSymbolSize() const

      Returns the size of the underlying :cpp:class:`Symbol`.

  .. cpp:function:: unsigned getSize() const

      See :cpp:func:`FunctionBase::getSize`.

  .. cpp:function:: std::string getName() const

      See :cpp:func:`FunctionBase::getName`.

  .. cpp:function:: Offset getOffset() const

      See :cpp:func:`FunctionBase::getOffset`.

  .. cpp:function:: Module* getModule() const

      See :cpp:func:`FunctionBase::getModule`.


.. cpp:class:: InlinedFunction : public FunctionBase

  **An inlined function as found in DWARF information**

  .. cpp:function:: InlinedFunction(FunctionBase *parent)

      Creates a function inlined into ``parent``.

  .. cpp:function:: virtual ~InlinedFunction()

  .. cpp:function:: std::pair<std::string, Dyninst::Offset> getCallsite()

      Returns the file and line corresponding to the call site of an inlined function.

  .. cpp:function:: unsigned getSize() const

      See :cpp:func:`FunctionBase::getSize`.

  .. cpp:function:: std::string getName() const

      See :cpp:func:`FunctionBase::getName`.

  .. cpp:function:: Offset getOffset() const

      See :cpp:func:`FunctionBase::getOffset`.

  .. cpp:function:: Module* getModule() const

      See :cpp:func:`FunctionBase::getModule`.
