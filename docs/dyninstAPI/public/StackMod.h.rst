StackMod.h
==========

``StackMod``
------------
.. cpp:namespace:: StackMod

.. cpp:class:: StackMod
   
   This class defines modifications to the stack frame layout of a
   function. Stack modifications are basd on the abstraction of stack
   locations, not the contents of these locations. All stack offsets are
   with respect to the original stack frame, even if
   BPatch_fuction::addMods is called multiple times for a single function.
   
   implemented on x86 and x86-64
   
   Insert(int low, int high)
   
   | This constructor creates a stack modification that inserts stack space
     in the range
   | [low, high), where low and high are stack offsets.
   
   BPatch_function::addMods will find this modification unsafe if any
   instructions in the function access memory that will be non-contiguous
   after [low,high) is inserted.
   
   Remove(int low, int high)
   
   | This constructor creates a stack modification that removes stack space
     in the range
   | [low, high), where low and high are stack offsets.
   
   BPatch_function::addMods will find this modification unsafe if any
   instructions in the function access stack memory in [low,high).
   
   Move(int sLow, int sHigh, int dLow)
   
   This constructor creates a stack modification that moves stack space
   [sLow, sHigh) to [dLow, dLow+(sHigh-sLow)).
   
   | BPatch_function::addMods will find this modification unsafe if
   | Insert(dLow, dLow+(sHigh-sLow)) or Remove(sLow, sHigh) are unsafe.
   
   Canary()implemented on Linux, GCC only
   
   Canary(BPatch_function* failFunc) implemented on Linux, GCC only
   
   This constructor creates a stack modification that inserts a stack
   canary at function entry and a corresponding canary check at function
   exit(s).
   
   This uses the same canary as GCC’s –fstack-protector. If the canary
   check at function exit fails, failFunc is called. failFunc must be
   non-returning and take no arguments. If no failFunc is provided,
   \__stack_chk_fail from libc is called; libc must be open in the
   corresponding BPatch_addressSpace.
   
   This modification will have no effect on functions in which the entry
   and exit point(s) are the same.
   
   BPatch_function::addMods will find this modification unsafe if another
   Canary has already been added to the function. Note, however, that this
   modification can be applied to code compiled with –fstack-protector.
   
   Randomize()
   
   Randomize(int seed)
   
   This constructor creates a stack modification that rearranges the
   stack-stored local variables of a function. This modification requires
   symbol information (e.g., DWARF), and only local variables specified by
   the symbols will be randomized. If DyninstAPI finds a stack access that
   is not consistent with a symbol-specified local, that local will not be
   randomized. Contiguous ranges of local variables are randomized; if
   there are two or more contiguous ranges of locals within the stack
   frame, each is randomized separately. More than one local variable is
   required for randomization.
   
   BPatch_function::addMods will return false if Randomize is added to a
   function without local variable information, without local variables on
   the stack, or with only a single local variable.
   
   srand is used to generate a new ordering of local variables; if seed is
   provided, this value is provided to srand as its seed.
   
   BPatch_function::addMods will find this modification unsafe if any other
   modifications have been applied.
   
   26. .. rubric:: Container Classes
          :name: container-classes
   
       1. .. rubric:: Class std::vector
             :name: class-stdvector
   
   The **std::vector** class is a container used to hold other objects used
   by the API. As of Dyninst 5.0, std::vector is an alias for the C++
   Standard Template Library (STL) std::vector.