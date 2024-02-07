.. _`sec:BPatch_point.h`:

BPatch_point.h
##############



.. cpp:class:: BPatch_point

  **A location in an application’s code at which the library can insert instrumentation**

  A :cpp:class:`BPatch_image` object is used to retrieve a BPatch_point representing a desired point in the application.

  .. cpp:function:: BPatch_basicBlockLoop* getLoop()

    Returns the containing BPatch_basicBlockLoop if this point is part of
    loop instrumentation. Returns NULL otherwise.

  .. cpp:function:: BPatch_procedureLocation getPointType()

    Return the type of the point.

  .. cpp:function:: BPatch_function* getFunction()

    Returns a BPatch_function representing the function in which this point
    is contained.

  .. cpp:function:: BPatch_function* getCalledFunction()

    Return a BPatch_function representing the function that is called at the
    point. If the point is not a function call site or the target of the
    call cannot be determined, then this function returns NULL.

  .. cpp:function:: std::string getCalledFunctionName()

    Returns the name of the function called at this point.

    This method is similar to ``getCalledFunction()->getName()``, except in cases where
    DyninstAPI is running in binary rewrit­ing mode and the called function resides in a
    library or object file that DyninstAPI has not opened. In these cases, Dyninst is
    able to determine the name of the called function, but is unable to construct a
    :cpp:class:`BPatch_function` object.

  .. cpp:function:: BPatch_basicBlock* getBlock()

    BPatch_point::getBlock  Returns block to which this point belongs if such a block exists  For example, function entry points do not have blocks associated with them.

  .. cpp:function:: void* getAddress()

    Return the address of the first instruction at this point.

  .. cpp:function:: const BPatch_memoryAccess* getMemoryAccess()

    Returns the memory access object associated with this point.

  .. cpp:function:: Dyninst::InstructionAPI::Instruction getInsnAtPoint()

    Returns the first machine instruction at this point’s address.

  .. cpp:function:: const BPatch_Vector<BPatchSnippetHandle*> getCurrentSnippets()

    Get all current snippets at this point

  .. cpp:function:: const BPatch_Vector<BPatchSnippetHandle*> getCurrentSnippets(BPatch_callWhen when)

    Return the BPatchSnippetHandles for the BPatch_snippets that are associated with the point.

    If argument when is BPatch_callBefore, then
    BPatchSnippetHandles for snippets installed immediately before this
    point will be returned. Alternatively, if when is BPatch_callAfter, then
    BPatchSnippetHandles for snippets installed immediately after this point
    will be returned.

  .. cpp:function:: bool getLiveRegisters(std::vector<BPatch_register> &liveRegs)

    Fill regs with the registers that are live before this point (e.g.,
    BPatch_callBefore). Currently returns only general purpose registers
    (GPRs).

  .. cpp:function:: bool isDynamic()

    Checks if this is a dynamic call site (e.g. a call site where the function call is made via
    a function pointer).

  .. cpp:function:: void* monitorCalls(BPatch_function *f = NULL)

    For a dynamic call site, this call instruments the call site represented
    by this instrumentation point with a function call. If input parameter
    func is not NULL, func is called at the call site as the
    instrumentation. If func is NULL, the callback function registered with
    BPatch::registerDynamicCallCallback is used for instrumentation. Under
    both cases, this call returns a pointer to the called function. If the
    instrumentation point does not represent a dynamic call site, this call
    returns NULL.

  .. cpp:function:: bool stopMonitoring()

    This call returns true if this instrumentation point is a dynamic call
    site and its instrumentation is successfully removed. Otherwise, it
    returns false.

  .. cpp:function:: int getDisplacedInstructions(int maxSize, void *insns)

    BPatch_point::getDisplacedInstructions  Returns the instructions to be relocated when instrumentation is inserted  at this point.  Returns the number of bytes taken up by these instructions.

  .. cpp:function:: bool usesTrap_NP()

    Checks if inserting instrumentation at this point requires using a trap.

    On the x86 architecture, because instructions are of variable
    size, the instruction at a point may be too small for Dyninst to replace
    it with the normal code sequence used to call instrumentation. Also,
    when instrumentation is placed at points other than subroutine entry,
    exit, or call points, traps may be used to ensure the instrumentation
    fits. In this case, Dyninst replaces the instruction with a single-byte
    instruction that generates a trap. A trap handler then calls the
    appropriate instrumentation code. Since this technique is used only on
    some platforms, on other platforms this function always returns false.



Notes
*****

These aliases are provided for backward compatibility only. Do not use.

.. code:: cpp

  #define BPatch_entry BPatch_locEntry
  #define BPatch_exit BPatch_locExit
  #define BPatch_subroutine BPatch_locSubroutine
  #define BPatch_longJump BPatch_locLongJump
  #define BPatch_allLocations BPatch_locAllLocations
  #define BPatch_arbitrary BPatch_locInstruction

