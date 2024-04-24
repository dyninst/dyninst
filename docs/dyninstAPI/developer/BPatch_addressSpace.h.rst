.. _`sec-dev:BPatch_addressSpace.h`:

BPatch_addressSpace.h
#####################

.. cpp:namespace:: dev

.. cpp:class:: BPatch_addressSpace

  .. cpp:function:: BPatch_module* findModuleByAddr(Dyninst::Address addr)

      Doesn't cause parsing

  .. cpp:function:: bool findFuncsByRange(Dyninst::Address startAddr, Dyninst::Address endAddr, \
                                          std::set<BPatch_function*>& funcs)

  .. cpp:function:: bool replaceCode(BPatch_point *point, BPatch_snippet *snippet)

    Replace an instruction at ``point`` with a ::cpp:class:`BPatch_snippet` ``snippet``.

    .. warning:: UNIMPLEMENTED - DO NOT USE

    Need to reevaluate how this code works. I don't think it should be point-based, though.

  .. cpp:function:: BPatch_variableExpr* malloc(const BPatch_type& type, std::string name = std::string(""))

   .. note::
    Should return ``NULL`` on failure, but the function which it calls, :cpp:func:`inferiorMalloc`, calls
    ``exit`` rather than returning an error, so this is not currently possible.


.. cpp:class BPatchSnippetHandle

  .. cpp:function:: ~BPatchSnippetHandle()

    Don't delete inst instances since they are might have been copied
