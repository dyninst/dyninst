.. _`sec:dev-stackanalysis.h`:

stackanalysis.h
###############

.. cpp:namespace:: Dyninst::dev

The types described here are in the public header file, but are intended for interna use only.

.. cpp:class:: StackAnalysis

  .. cpp:member:: static const unsigned DEF_LIMIT=2

    This constant limits the number of definitions we track per register. If
    more than this many definitions are found, the register is considered to
    be BOTTOM, and the definitions tracked so far are dropped.

  .. cpp:member:: std::set<Address> toppableFunctions

    Functions whose return values should be topped rather than bottomed.  This
    is used when evaluating a cycle in the call graph via fixed-point
    analysis.  Note that if functionSummaries contains a summary for the
    function, it will be used instead.

  .. cpp:member:: bool retop

  Distinguish between default-constructed transfer functions and explicitly-retopped transfer functions.

  .. cpp:member:: bool topBottom

    Annotate transfer functions that have the following characteristic:

    if target is TOP, keep as TOP
    else, target must be set to BOTTOM
    e.g., sign-extending a register:

      if the register had an uninitialized stack height (TOP), the sign-extension has no effect
      if the register had a valid or notunique (BOTTOM) stack height, the sign-extension must result in a BOTTOM stack height


.. cpp:class:: StackAnalysis::Definition

  This class represents a stack pointer definition by recording the block
  and address of the definition, as well as the original absloc that was
  defined by the definition.

.. cpp:class:: StackAnalysis::DefHeight

  During stack pointer analysis, we keep track of any stack pointers in registers or
  memory, as well as the instruction addresses at which those pointers were
  defined. This is useful for :cpp:class:`StackMod`, where we sometimes want to modify
  pointer definitions to adjust the locations of variables on the stack.
  Thus, it makes sense to associate each stack pointer (:cpp:class:`StackAnalysis::Height`) to the point
  at which it was defined (:cpp:class:`StackAnalysis::Definition`).

.. cpp:class:: StackAnalysis::DefHeightSet

  In some programs, it is possible for a register or memory location to
  contain different stack pointers depending on the path taken to the
  current instruction.  When this happens, our stack pointer analysis tries
  to keep track of the different possible stack pointers, up to a maximum
  number per instruction (specified by the DEF_LIMIT constant).  As a
  result, we need a structure to hold sets of DefHeights.

.. cpp:class:: StackAnalysis::TransferFunc

  We need to represent the effects of instructions. We do this in terms of
  transfer functions. We recognize the following effects on the stack.

    1) Offset by known amount: push/pop/etc.
    2) Set to known value: leave
    3) Copy the stack pointer to/from some Absloc.

  There are also:

    1) Offset by unknown amount expressible in a range [l, h]
    2) Set to unknown value expressible in a range [l, h] which we don't handle yet.

  This gives us the following transfer functions.

    1) Delta(RV, f, t, v) -> RV[f] += v;
    2) Abs(RV, f, t, v) -> RV[f] = v;
    3) Copy(RV, f, t, v) -> RV[t] = RV[f];

  In the implementations below, we provide f, t, v at construction time (as
  they are fixed) and RV as a parameter. Note that a transfer function is a
  function T : (RegisterVector, RegisterID, RegisterID, value) -> (RegisterVector).

.. cpp:class:: StackAnalysis::SummaryFunc

  Summarize the effects of a series (list!) of transfer functions.
  Intended to summarize a block. We may want to do a better job of
  summarizing, but this works...

Intervals
*********

  The results of the stack analysis is a series of intervals. For each interval we have the following information:

    a) Whether the function has a well-defined stack frame. This is defined as follows:

         1. x86/AMD-64: a frame pointer
         2. POWER: an allocated frame pointed to by GPR1

    b) The "depth" of the stack; the distance between the stack pointer and the caller's stack pointer.

    c) The "depth" of any copies of the stack pointer.

