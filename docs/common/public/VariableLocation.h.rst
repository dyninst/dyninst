.. _`sec:VariableLocation.h`:

VariableLocation.h
##################

.. cpp:namespace:: Dyninst

.. cpp:class:: VariableLocation

  An encoding of the location of a variable in memory or registers.

  A VariableLocation is valid within the address range represented by :cpp:member:`lowPC`
  and :cpp:member:`hiPC`. If these are 0 and -1, respectively, it is always valid.

  .. cpp:member:: storageClass stClass
  .. cpp:member:: storageRefClass refClass
  .. cpp:member:: MachRegister mr_reg
  .. cpp:member:: long frameOffset
  .. cpp:member:: long frameOffsetAbs
  .. cpp:member:: Address lowPC
  .. cpp:member:: Address hiPC

  .. cpp:enum:: storageClass

    Encodes how a variable is stored.

    .. cpp:enumerator:: storageClass::storageUnset
    .. cpp:enumerator:: storageClass::storageAddr

      Absolute address of variable.

    .. cpp:enumerator:: storageClass::storageReg

      Register which holds variable value.

    .. cpp:enumerator:: storageClass::storageRegOffset

      Address of variable = $reg + address.

  .. cpp:enum:: storageRefClass

    Encodes if a variable can be accessed through a register/address.

    .. cpp:enumerator:: storageRefClass::storageRefUnset
    .. cpp:enumerator:: storageRefClass::storageRef

      There is a pointer to variable.

    .. cpp:enumerator:: storageRefClass::storageNoRef

      No reference. Value can be obtained using storageClass.

