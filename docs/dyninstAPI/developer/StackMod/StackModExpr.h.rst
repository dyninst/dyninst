.. _`sec:StackModExpr.h`:

StackModExpr.h
##############

.. cpp:class:: BPatch_stackInsertExpr : public BPatch_snippet

  .. cpp:function:: BPatch_stackInsertExpr(int size)

    Creates a stack shift (insertion) of size


.. cpp:class:: BPatch_stackRemoveExpr : public BPatch_snippet

  .. cpp:function:: BPatch_stackRemoveExpr(int size)

    Creates a stack shift (removal) of size


.. cpp:class:: BPatch_stackMoveExpr : public BPatch_snippet

  .. cpp:function:: BPatch_stackMoveExpr()

    Generates no new code, but triggers relocation and sensitivty analysis


.. cpp:class:: BPatch_canaryExpr : public BPatch_snippet

  .. cpp:function:: BPatch_canaryExpr()

    Creates the placement of a per-thread canary value on the stack


.. cpp:class:: BPatch_canaryCheckExpr : public BPatch_snippet

  .. cpp:function:: BPatch_canaryCheckExpr(BPatch_function* failureFunc, bool canaryAfterPrologue, long canaryHeight)

    Checks the integrity of a per-thread canary value on the stack

    ``failureFunc`` is the function called if the canary check at function exit fails.
    ``canaryAfterPrologue`` indicates if the canary is inserted at entry (false) or after the
    function prologue (true).
    ``canaryHeight`` is the height of the canary relative to the stack height at which
    the canary is being referenced; in the common case, this is 0.
    If the canary is inserted after the prologue, there may not be
    an instruction whose stack height is the intended canary location,
    in this case, canaryHeight must be set to the difference.


.. cpp:class:: BPatch_stackRandomizeExpr : public BPatch_snippet

  .. cpp:function:: BPatch_stackRandomizeExpr()

    Generates no new code, but triggers relocation and sensitivty analysis

