BPatch_instruction.h
====================

      
``BPatch_register``
-------------------
.. cpp:namespace:: BPatch_register

.. cpp:class:: BPatch_register
   
   A **BPatch_register** represents a single register of the mutatee. The
   list of BPatch_registers can be retrieved with the
   BPatch_addressSpace::getRegisters method.
   
   .. cpp:function:: std::string name()
      
      This function returns the canonical name of the register.