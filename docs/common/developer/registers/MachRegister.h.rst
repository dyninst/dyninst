.. _`sec-dev:MachRegister.h`:

MachRegister.h
##############

.. cpp:namespace:: Dyninst::dev

.. cpp:type:: unsigned long MachRegisterVal

  A value that holds the contents of a register.

  A MachRegister names a specific register, while a MachRegisterVal represents
  the value that may be in that register.

.. cpp:class:: MachRegister

  **A representation of a machine register**

  .. cpp:function:: MachRegister()

    Creates an empty, invalid register.

  .. cpp:function:: explicit MachRegister(int32_t r)
  .. cpp:function:: explicit MachRegister(int32_t r, std::string n)


.. _`sec-dev:MachRegister-DWARF-encodings`:

DWARf encodings
===============

The DWARF Encodings for each platform are taken from the documents below.

::

  x86:
   System V Application Binary Interface
   Intel386 Architecture Processor Supplement
   Version 1.0 February 3, 2015
   Table 2.14: DWARF Register Number Mapping
   https://gitlab.com/x86-psABIs/i386-ABI

  x86_64:
    System V Application Binary Interface
    AMD64 Architecture Processor Supplement
    Version 1.0 June 21, 2022
    Table 3.36: DWARF Register Number Mapping
    https://gitlab.com/x86-psABIs/x86-64-ABI


.. admonition:: TODO

    The other architectures have not been updated to the latest ABI docs.
