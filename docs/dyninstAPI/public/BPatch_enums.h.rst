.. _`sec:BPatch_enums.h`:

BPatch_enums.h
##############

.. cpp:enum::  BPatch_snippetOrder

  Used to specify whether a snippet should be installed before other snippets
  that have previously been inserted at the same point, or after.

  .. cpp:enumerator:: BPatch_firstSnippet
  .. cpp:enumerator:: BPatch_lastSnippet

.. cpp:enum:: eBPatch_procedureLocation

  Used with BPatch_function::findPoint to specify which of the possible
  instrumentation points within a procedure should be returned.

  .. cpp:enumerator:: BPatch_locEntry
  .. cpp:enumerator:: BPatch_locExit
  .. cpp:enumerator:: BPatch_locSubroutine
  .. cpp:enumerator:: BPatch_locLongJump
  .. cpp:enumerator:: BPatch_locAllLocations
  .. cpp:enumerator:: BPatch_locInstruction
  .. cpp:enumerator:: BPatch_locUnknownLocation
  .. cpp:enumerator:: BPatch_locSourceBlockEntry

    not yet used

  .. cpp:enumerator:: BPatch_locSourceBlockExit

    not yet used

  .. cpp:enumerator:: BPatch_locSourceLoopEntry

    not yet used

  .. cpp:enumerator:: BPatch_locSourceLoopExit

    not yet used

  .. cpp:enumerator:: BPatch_locBasicBlockEntry
  .. cpp:enumerator:: BPatch_locBasicBlockExit

    not yet used

  .. cpp:enumerator:: BPatch_locSourceLoop

    not yet used

  .. cpp:enumerator:: BPatch_locLoopEntry
  .. cpp:enumerator:: BPatch_locLoopExit
  .. cpp:enumerator:: BPatch_locLoopStartIter
  .. cpp:enumerator:: BPatch_locLoopEndIter
  .. cpp:enumerator:: BPatch_locVarInitStart

    not yet used

  .. cpp:enumerator:: BPatch_locVarInitEnd

    not yet used

  .. cpp:enumerator:: BPatch_locStatement

    not yet used


.. cpp:enum:: BPatch_callWhen
  
  Used to specify whether a snippet is to be called before the instructions
  at the point where it is inserted, or after.

  .. cpp:enumerator:: BPatch_callBefore
  .. cpp:enumerator:: BPatch_callAfter
  .. cpp:enumerator:: BPatch_callUnset


.. cpp:enum:: BPatch_opCode

  .. cpp:enumerator:: BPatch_opLoad
  .. cpp:enumerator:: BPatch_opStore
  .. cpp:enumerator:: BPatch_opPrefetch


.. cpp:enum:: BPatch_ploc

  **instrumentation locations for BPatch_paramExpr's**

  .. cpp:enumerator:: BPatch_ploc_guess
  .. cpp:enumerator:: BPatch_ploc_call
  .. cpp:enumerator:: BPatch_ploc_entry

.. cpp:enum:: BPatch_hybridMode

  **program analysis styles**

  .. cpp:enumerator:: BPatch_heuristicMode
  .. cpp:enumerator:: BPatch_normalMode
  .. cpp:enumerator:: BPatch_exploratoryMode
  .. cpp:enumerator:: BPatch_defensiveMode
