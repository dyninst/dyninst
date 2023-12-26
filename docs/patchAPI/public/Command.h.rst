Command.h
=========

.. cpp:namespace:: Dyninst::patchAPI

Command
=======

**Declared in**: Command.h

The Command class represents an instrumentation request (e.g., snippet
insertion or removal), or an internal logical step in the code patching
(e.g., install instrumentation).

.. code-block:: cpp
    
    virtual bool run() = 0;

Executes the normal operation of this Command.

It returns true on success; otherwise, it returns false.

.. code-block:: cpp
    
    virtual bool undo() = 0;

Undoes the operation of this Command.

.. code-block:: cpp
    
    virtual bool commit();

Implements the transactional semantics: all succeed, or all fail.
Basically, it performs such logic:

.. code-block:: cpp
   
   if (run()) {
     return true;
   } else {
     undo();
     return false;
   }

BatchCommand
============

**Declared in**: Command.h

The BatchCommand class inherits from the Command class. It is actually a
container of a list of Commands that will be executed in a transaction:
all Commands will succeed, or all will fail.

.. code-block:: cpp
    
    typedef std::list<CommandPtr> CommandList;
    CommandList to_do_; CommandList done_;

This class has two protected members *to_do\_* and *done\_*, where
*to_do\_* is a list of Commands to execute, and *done\_* is a list of
Commands that are executed.

.. code-block:: cpp
    
    virtual bool run(); virtual bool undo();

The method run() of BatchCommand invokes the run() method of each
Command in *to_do\_* in order, and puts the finished Commands in
*done\_*. The method undo() of BatchCommand invokes the undo() method of
each Command in *done \_* in order.

.. code-block:: cpp
    
    void add(CommandPtr command);

This method adds a Command into *to_do\_*.

.. code-block:: cpp
    
    void remove(CommandList::iterator iter);

This method removes a Command from *to_do\_*.

Instrumenter
============

**Declared in**: Command.h

The Instrumenter class inherits BatchCommand to encapsulate the core
code patching logic, which includes binary code generation. Instrumenter
would contain several logical steps that are individual Commands.

    ``CommandList user_commands_;``

This class has a protected data member *user_commands\_* that contains
all Commands issued by users, e.g., snippet insertion. This is to
facilitate the implementation of the instrumentation engine.

.. code-block:: cpp
    
    static InstrumenterPtr create(AddrSpacePtr as);

Returns an instance of Instrumenter, and it takes input the address
space *as* that is going to be instrumented.

.. code-block:: cpp
    
    virtual bool replaceFunction(PatchFunction* oldfunc, PatchFunction* newfunc);

Replaces a function *oldfunc* with a new function *newfunc*.

It returns true on success; otherwise, it returns false.

.. code-block:: cpp
    
    virtual bool revertReplacedFunction(PatchFunction* oldfunc);

Undoes the function replacement for *oldfunc*.

It returns true on success; otherwise, it returns false.

.. code-block:: cpp
    
    typedef std::map<PatchFunction*, PatchFunction*> FuncModMap;

The type FuncModMap contains mappings from an PatchFunction to another
PatchFunction.

.. code-block:: cpp
    
    virtual FuncModMap& funcRepMap();

Returns the FuncModMap that contains a set of mappings from an old
function to a new function, where the old function is replaced by the
new function.

.. code-block:: cpp
    
    virtual bool wrapFunction(PatchFunction* oldfunc, PatchFunction* newfunc, string name);

Replaces all calls to *oldfunc* with calls to wrapper *newfunc* (similar
to function replacement). However, we create a copy of original using
the *name* that can be used to call the original. The wrapper code would
look like follows:

.. code-block:: cpp

   void *malloc_wrapper(int size) {
     // do stuff
     void *ret = malloc_clone(size);
     // do more stuff
     return ret;
   }

This interface requires the user to give us a name (as represented by
clone) for the original function. This matches current techniques and
allows users to use indirect calls (function pointers).

.. code-block:: cpp
    
    virtual bool revertWrappedFunction(PatchFunction* oldfunc);

Undoes the function wrapping for *oldfunc*.

It returns true on success; otherwise, it returns false.

.. code-block:: cpp
    
    virtual FuncModMap& funcWrapMap();

The type FuncModMap contains mappings from the original PatchFunction to
the wrapper PatchFunction.

.. code-block:: cpp
    
    bool modifyCall(PatchBlock *callBlock, PatchFunction *newCallee, PatchFunction *context = NULL);

Replaces the function that is invoked in the basic block *callBlock*
with the function *newCallee*. There may be multiple functions
containing the same *callBlock*, so the *context* parameter specifies in
which function the *callBlock* should be modified. If *context* is NULL,
then the *callBlock* would be modified in all PatchFunctions that
contain it. If the *newCallee* is NULL, then the *callBlock* is removed.

It returns true on success; otherwise, it returns false.

.. code-block:: cpp
    
    bool revertModifiedCall(PatchBlock *callBlock, PatchFunction *context = NULL);

Undoes the function call modification for *oldfunc*. There may be
multiple functions containing the same *callBlock*, so the *context*
parameter specifies in which function the *callBlock* should be
modified. If *context* is NULL, then the *callBlock* would be modified
in all PatchFunctions that contain it.

It returns true on success; otherwise, it returns false.

.. code-block:: cpp
    
    bool removeCall(PatchBlock *callBlock, PatchFunction *context = NULL);

Removes the *callBlock*, where a function is invoked. There may be
multiple functions containing the same *callBlock*, so the *context*
parameter specifies in which function the *callBlock* should be
modified. If *context* is NULL, then the *callBlock* would be modified
in all PatchFunctions that contain it.

It returns true on success; otherwise, it returns false.

.. code-block:: cpp
    
    typedef map<PatchBlock*, // B : A call block map<PatchFunction*, // F_c:
    Function context PatchFunction*> // F : The function to be replaced >
    CallModMap;

The type CallModMap maps from B -> F\ :math:`_c` -> F, where B
identifies a call block, and F\ :math:`_c` identifies an (optional)
function context for the replacement. If F\ :math:`_c` is not specified,
we use NULL. F specifies the replacement callee; if we want to remove
the call entirely, we use NULL.

.. code-block:: cpp
    
    CallModMap& callModMap();

Returns the CallModMap for function call replacement / removal.

.. code-block:: cpp
    
    AddrSpacePtr as() const;

Returns the address space associated with this Instrumenter.

.. _sec-3.2.6:

Patcher
=======

**Declared in**: Command.h

The class Patcher inherits from the class BatchCommand. It accepts
instrumentation requests from users, where these instrumentation
requests are Commands (e.g., snippet insertion). Furthermore, Patcher
implicitly adds an instance of Instrumenter to the end of the Command
list to generate binary code and install the instrumentation.

.. code-block:: cpp
    
    Patcher(PatchMgrPtr mgr)

The constructor of Patcher takes input the relevant PatchMgr *mgr*.

.. code-block:: cpp
    
    virtual bool run();

Performs the same logic as BatchCommand::run(), except that this
function implicitly adds an internal Command – Instrumenter, which is
executed after all other Commands in the *to_do\_*.

PushFrontCommand and PushBackCommand
====================================

**Declared in**: Command.h

The class PushFrontCommand and the class PushBackCommand inherit from
the Command class. They are to insert a snippet to a point. A point
maintains a list of snippet instances. PushFrontCommand would add the
new snippet instance to the front of the list, while PushBackCommand
would add to the end of the list.

.. code-block:: cpp
    
    static Ptr create(Point* pt, SnippetPtr snip);

This static method creates an object of PushFrontCommand or
PushBackCommand.

.. code-block:: cpp
    
    InstancePtr instance();

Returns a snippet instance that is inserted at the point.

.. _sec-3.3.2:

RemoveSnippetCommand
====================

**Declared in**: Command.h

The class RemoveSnippetCommand inherits from the Command class. It is to
delete a snippet Instance.

.. code-block:: cpp
    
    static Ptr create(InstancePtr instance);

This static function creates an instance of RemoveSnippetCommand.

.. _sec-3.3.3:

RemoveCallCommand
=================

**Declared in**: Command.h

The class RemoveCallCommand inherits from the class Command. It is to
remove a function call.

.. code-block:: cpp
    
    static Ptr create(PatchMgrPtr mgr, PatchBlock* call_block,
    PatchFunction* context = NULL);

This static method takes input the relevant PatchMgr *mgr*, the
*call_block* that contains the function call to be removed, and the
PatchFunction *context*. There may be multiple PatchFunctions containing
the same *call_block*. If the *context* is NULL, then the *call_block*
would be deleted from all PatchFunctions that contains it; otherwise,
the *call_block* would be deleted only from the PatchFuncton *context*.

.. _sec-3.3.4:

ReplaceCallCommand
==================

**Declared in**: Command.h

The class ReplaceCallCommand inherits from the class Command. It is to
replace a function call with another function.

.. code-block:: cpp
    
    static Ptr create(PatchMgrPtr mgr, PatchBlock* call_block,
    PatchFunction* new_callee, PatchFunction* context);

This Command replaces the *call_block* with the new PatchFunction
*new_callee*. There may be multiple functions containing the same
*call_block*, so the *context* parameter specifies in which function the
*call_block* should be replaced. If *context* is NULL, then the
*call_block* would be replaced in all PatchFunctions that contains it.

.. _sec-3.3.5:

ReplaceFuncCommand
==================

**Declared in**: Command.h

The class ReplaceFuncCommand inherits from the class Command. It is to
replace an old function with the new one.

.. code-block:: cpp
    
    static Ptr create(PatchMgrPtr mgr, PatchFunction* old_func,
    PatchFunction* new_func);

This Command replaces the old PatchFunction *old_func* with the new
PatchFunction *new_func*.