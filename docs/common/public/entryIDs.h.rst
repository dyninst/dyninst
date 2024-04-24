.. _`sec:entryIDs.h`:

entryIDs.h
##########

.. cpp:namespace:: Dyninst

Every instruction supported by InstructionAPI has a unique name in the :cpp:type:`entryID` enumeration.
Give the very large instruction set sizes, we do not list them here.

.. cpp:enum:: entryID

  .. cpp:enumerator:: _entry_ids_max_


.. cpp:enum:: prefixEntryID

  .. cpp:enumerator:: prefix_none
  .. cpp:enumerator:: prefix_rep
  .. cpp:enumerator:: prefix_repnz
