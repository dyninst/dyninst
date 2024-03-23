.. _`sec:StackMod.h`:

StackMod.h
##########

.. cpp:class:: StackMod
   
  **Modifications to the stack frame layout of a function**

  Stack modifications are basd on the abstraction of stack
  locations, not the contents of these locations. All stack offsets are
  with respect to the original stack frame, even if
  :cpp:func:`BPatch_fuction::addMods` is called multiple times for a single function.

  .. note:: Only implemented on x86 and x86-64

  .. cpp:function:: StackMod()
  .. cpp:function:: MType type() const
  .. cpp:function:: MOrder order() const
  .. cpp:function:: virtual ~StackMod() = default
  .. cpp:function:: virtual std::string format() const
  .. cpp:member:: protected MOrder _order{}
  .. cpp:member:: protected MType _type{}


.. cpp:class:: Insert : public StackMod

  **Insert additional space into the stack**

  .. cpp:function:: Insert(int low, int high)

    This constructor creates a stack modification that inserts stack space in the range
    ``[low, high)``, where low and high are stack offsets.

    :cpp:func:`BPatch_function::addMods` will find this modification unsafe if any
    instructions in the function access memory that will be non-contiguous
    after ``[low,high)`` is inserted.

  .. cpp:function:: int low() const
  .. cpp:function:: int high() const
  .. cpp:function:: unsigned int size() const
  .. cpp:function:: std::string format() const


.. cpp:class:: Remove : public StackMod

  **Remove stack space**

  .. cpp:function::   Remove(int low, int high)

    This constructor creates a stack modification that removes stack space in the range
    ``[low, high)``, where low and high are stack offsets.

    :cpp:func:`BPatch_function::addMods` will find this modification unsafe if any
    instructions in the function access stack memory in ``[low,high)``.

  .. cpp:function:: int low() const
  .. cpp:function:: int high() const
  .. cpp:function:: unsigned int size() const
  .. cpp:function:: std::string format() const


.. cpp:class:: Move : public StackMod

  **Move stack space from one location to another**

  .. warning:: The source and destination must not overlap

  .. cpp:function:: Move(int srcLow, int srcHigh, int destLow)

    Move stack space from ``[srcLow, srcHigh)`` to ``[destLow, destLow+(srcHigh-srcLow))``.

  .. cpp:function:: int srcLow() const
  .. cpp:function:: int srcHigh() const
  .. cpp:function:: int destLow() const
  .. cpp:function:: int destHigh() const
  .. cpp:function:: int size() const
  .. cpp:function:: std::string format() const


.. cpp:class:: Canary : public StackMod

  **Insert a stack canary**

  .. cpp:function:: Canary()
  .. cpp:function:: Canary(BPatch_function* failFunc)

    Use ``failFunc`` as the canary check failure function provided by libc's ``__stack_chk_fail``.

    ``failFunc`` cannot take any arguments.

    This uses the same canary as GCC’s ``-fstack-protector``. If the canary
    check at function exit fails, failFunc is called. failFunc must be
    non-returning and take no arguments. If no failFunc is provided,
    ``__stack_chk_fail`` from libc is called; libc must be open in the
    corresponding BPatch_addressSpace.

    This modification will have no effect on functions in which the entry
    and exit point(s) are the same.

    :cpp:func:`BPatch_function::addMods` will find this modification unsafe if another
    Canary has already been added to the function. Note, however, that this
    modification can be applied to code compiled with ``-fstack-protector``.

    .. note:: Only valid on Linux, and libc must be present in the address space.

  .. cpp:function:: int low() const
  .. cpp:function:: int high() const
  .. cpp:function:: void init(int l, int h)
  .. cpp:function:: BPatch_function* failFunc() const
  .. cpp:function:: std::string format() const


.. cpp:class:: Randomize : public StackMod

  **Randomize the locations of the DWARF-specified local variables**

  This modification has no effect on functions without DWARF information.

  .. cpp:function:: Randomize()

  .. cpp:function:: Randomize(int seed)

    This constructor creates a stack modification that rearranges the
    stack-stored local variables of a function. This modification requires
    symbol information (e.g., DWARF), and only local variables specified by
    the symbols will be randomized. If DyninstAPI finds a stack access that
    is not consistent with a symbol-specified local, that local will not be
    randomized. Contiguous ranges of local variables are randomized; if
    there are two or more contiguous ranges of locals within the stack
    frame, each is randomized separately. More than one local variable is
    required for randomization.

    :cpp:func:`BPatch_function::addMods` will return false if Randomize is added to a
    function without local variable information, without local variables on
    the stack, or with only a single local variable.

    srand is used to generate a new ordering of local variables; if seed is
    provided, this value is provided to srand as its seed.

    :cpp:func:`BPatch_function::addMods` will find this modification unsafe if any other
    modifications have been applied.

  .. cpp:function:: bool isSeeded() const
  .. cpp:function:: int seed() const
  .. cpp:function:: std::string format() const


.. cpp:enum:: StackMod::MType

  .. cpp:enumerator:: INSERT
  .. cpp:enumerator:: REMOVE
  .. cpp:enumerator:: MOVE
  .. cpp:enumerator:: CANARY
  .. cpp:enumerator:: RANDOMIZE


.. cpp:enum:: StackMod::MOrder

  .. cpp:enumerator:: NEW
  .. cpp:enumerator:: CLEANUP


