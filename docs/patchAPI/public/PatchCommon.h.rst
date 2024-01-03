PatchCommon.h
=============

.. cpp:namespace:: Dyninst::patchAPI

.. cpp:type:: std::map<PatchFunction*, PatchFunction*> FuncModMap

    The type FuncModMap contains mappings from an PatchFunction to another
    PatchFunction.

map<PatchBlock*, // B : A call block map<PatchFunction*, // F_c:
Function context PatchFunction*> // F : The function to be replaced >
CallModMap

The type CallModMap maps from B -> F\ :math:`_c` -> F, where B
identifies a call block, and F\ :math:`_c` identifies an (optional)
function context for the replacement. If F\ :math:`_c` is not specified,
we use NULL. F specifies the replacement callee if we want to remove
the call entirely, we use NULL.
