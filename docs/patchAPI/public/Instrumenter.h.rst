Instrumenter.h
==============

.. cpp:namespace:: Dyninst::patchAPI

Instrumenter
============

**Declared in**: Command.h

The Instrumenter class inherits BatchCommand to encapsulate the core
code patching logic, which includes binary code generation. Instrumenter
would contain several logical steps that are individual Commands.

    ``CommandList user_commands_``

This class has a protected data member *user_commands\_* that contains
all Commands issued by users, e.g., snippet insertion. This is to
facilitate the implementation of the instrumentation engine.

.. code-block:: cpp
    
    static InstrumenterPtr create(AddrSpacePtr as)

Returns an instance of Instrumenter, and it takes input the address
space *as* that is going to be instrumented.

.. code-block:: cpp
    
    virtual bool replaceFunction(PatchFunction* oldfunc, PatchFunction* newfunc)

Replaces a function *oldfunc* with a new function *newfunc*.

It returns true on success otherwise, it returns false.

.. code-block:: cpp
    
    virtual bool revertReplacedFunction(PatchFunction* oldfunc)

Undoes the function replacement for *oldfunc*.

It returns true on success otherwise, it returns false.

.. code-block:: cpp
    
    typedef std::map<PatchFunction*, PatchFunction*> FuncModMap

The type FuncModMap contains mappings from an PatchFunction to another
PatchFunction.

.. code-block:: cpp
    
    virtual FuncModMap& funcRepMap()

Returns the FuncModMap that contains a set of mappings from an old
function to a new function, where the old function is replaced by the
new function.

.. code-block:: cpp
    
    virtual bool wrapFunction(PatchFunction* oldfunc, PatchFunction* newfunc, string name)

Replaces all calls to *oldfunc* with calls to wrapper *newfunc* (similar
to function replacement). However, we create a copy of original using
the *name* that can be used to call the original. The wrapper code would
look like follows:

.. code-block:: cpp

   void *malloc_wrapper(int size) {
     // do stuff
     void *ret = malloc_clone(size)
     // do more stuff
     return ret
   }

This interface requires the user to give us a name (as represented by
clone) for the original function. This matches current techniques and
allows users to use indirect calls (function pointers).

.. code-block:: cpp
    
    virtual bool revertWrappedFunction(PatchFunction* oldfunc)

Undoes the function wrapping for *oldfunc*.

It returns true on success otherwise, it returns false.

.. code-block:: cpp
    
    virtual FuncModMap& funcWrapMap()

The type FuncModMap contains mappings from the original PatchFunction to
the wrapper PatchFunction.

.. code-block:: cpp
    
    bool modifyCall(PatchBlock *callBlock, PatchFunction *newCallee, PatchFunction *context = NULL)

Replaces the function that is invoked in the basic block *callBlock*
with the function *newCallee*. There may be multiple functions
containing the same *callBlock*, so the *context* parameter specifies in
which function the *callBlock* should be modified. If *context* is NULL,
then the *callBlock* would be modified in all PatchFunctions that
contain it. If the *newCallee* is NULL, then the *callBlock* is removed.

It returns true on success otherwise, it returns false.

.. code-block:: cpp
    
    bool revertModifiedCall(PatchBlock *callBlock, PatchFunction *context = NULL)

Undoes the function call modification for *oldfunc*. There may be
multiple functions containing the same *callBlock*, so the *context*
parameter specifies in which function the *callBlock* should be
modified. If *context* is NULL, then the *callBlock* would be modified
in all PatchFunctions that contain it.

It returns true on success otherwise, it returns false.

.. code-block:: cpp
    
    bool removeCall(PatchBlock *callBlock, PatchFunction *context = NULL)

Removes the *callBlock*, where a function is invoked. There may be
multiple functions containing the same *callBlock*, so the *context*
parameter specifies in which function the *callBlock* should be
modified. If *context* is NULL, then the *callBlock* would be modified
in all PatchFunctions that contain it.

It returns true on success otherwise, it returns false.

.. code-block:: cpp
    
    typedef map<PatchBlock*, // B : A call block map<PatchFunction*, // F_c:
    Function context PatchFunction*> // F : The function to be replaced >
    CallModMap

The type CallModMap maps from B -> F\ :math:`_c` -> F, where B
identifies a call block, and F\ :math:`_c` identifies an (optional)
function context for the replacement. If F\ :math:`_c` is not specified,
we use NULL. F specifies the replacement callee if we want to remove
the call entirely, we use NULL.

.. code-block:: cpp
    
    CallModMap& callModMap()

Returns the CallModMap for function call replacement / removal.

.. code-block:: cpp
    
    AddrSpacePtr as() const

Returns the address space associated with this Instrumenter.

.. _sec-3.2.6: