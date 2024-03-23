.. _`sec:Instrumenter.h`:

Instrumenter.h
##############

.. cpp:namespace:: Dyninst::PatchAPI

.. cpp:class:: Instrumenter

  **Relocate the original code and generate snippet binary code in mutatee's address space**

  .. cpp:function:: protected explicit Instrumenter(AddrSpace* as)

      Creates an instrumenter for the address space ``as``.

  .. cpp:function:: protected Instrumenter()

      Creates an empty instrumenter.

  .. cpp:member:: protected AddrSpace* as_
  .. cpp:member:: protected CommandList user_commands_
  .. cpp:member:: protected FuncModMap functionReplacements_
  .. cpp:member:: protected FuncWrapMap functionWraps_
  .. cpp:member:: protected CallModMap callModifications_

  .. cpp:function:: static Instrumenter* create(AddrSpace* as)

      Helper for creating a ``Instrumenter``.

  .. cpp:function:: virtual bool replaceFunction(PatchFunction* oldfunc, PatchFunction* newfunc)

      Replaces ``oldfunc`` with a new function ``newfunc``.

      Returns ``false`` on error.

  .. cpp:function:: virtual bool revertReplacedFunction(PatchFunction* oldfunc)

      Undoes the function replacement for ``oldfunc``.

      Returns ``false`` on error.

  .. cpp:function:: virtual FuncModMap& funcRepMap()

      Returns the mappings from an old function to a new one where the old one is replaced by the
      new one.

  .. cpp:function:: virtual bool wrapFunction(PatchFunction* oldfunc, PatchFunction* newfunc, string name)

      Replaces all calls to ``oldfunc`` with calls to wrapper ``newfunc`` (similar
      to function replacement).

      Create a copy of original using the ``name`` that can be used to call the original.

  .. cpp:function:: virtual bool revertWrappedFunction(PatchFunction* oldfunc)

      Undoes the function wrapping for ``oldfunc``.

      Returns ``false`` on error.

  .. cpp:function:: virtual FuncModMap& funcWrapMap()

      Returns the mappings from an old function to a new one where the old one is wrapped by the
      new one.

  .. cpp:function:: bool modifyCall(PatchBlock *callBlock, PatchFunction *newCallee, PatchFunction *context = NULL)

      Replaces the function invoked in the basic block ``callBlock`` with the function ``newCallee``.

      If multiple functions contain the same ``callBlock``, then ``context`` is used to determine
      which function to modify. If ``context`` is ``NULL``, then the ``callBlock`` is modified in all functions
      that contain it. If the ``newCallee`` is NULL, then the ``callBlock`` is removed.

      Returns ``false`` on error.

  .. cpp:function:: bool revertModifiedCall(PatchBlock *callBlock, PatchFunction *context = NULL)

      Undoes the function call modification for ``oldfunc``.

      If multiple functions contain the same ``callBlock``, then ``context`` is used to determine
      which function to modify. If ``context`` is ``NULL``, then the ``callBlock`` is modified
      in all functions that contain it.

      Returns ``false`` on error.

  .. cpp:function:: bool removeCall(PatchBlock *callBlock, PatchFunction *context = NULL)

      Removes the ``callBlock`` where a function is invoked.

      If multiple functions contain the same ``callBlock``, then ``context`` is used to determine
      which function to modify. If ``context`` is ``NULL``, then the ``callBlock`` is modified
      in all functions that contain it.

      Returns ``false`` on error.

  .. cpp:function:: CallModMap& callModMap()

      Returns the mapping of function calls that are to be replaced or removed.

  .. cpp:function:: AddrSpacePtr as() const

      Returns the address space associated with this Instrumenter.

  .. cpp:function:: virtual bool isInstrumentable(PatchFunction *f)

      Checks if ``f`` is instrumentable.
