.. _`sec:entryIDs.h`:

entryIDs.h
##########
.. cpp:namespace:: Dyninst

Every instruction supported by InstructionAPI has a unique name in the :cpp:type:`entryID` enumeration.
Give the very large instruction set sizes, we do not list them here.

.. cpp:enum:: prefixEntryID

.. cpp:enum:: entryID

.. cpp:namespace:: NS_x86

.. cpp:var:: dyn_hash_map<entryID, std::string> entryNames_IAPI
.. cpp:var:: dyn_hash_map<prefixEntryID, std::string> prefixEntryNames_IAPI
