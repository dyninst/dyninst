.. _`sec:opcode.h`:

opcode.h
########

.. cpp:enum:: opCode 

  .. cpp:enumerator:: invalidOp
  .. cpp:enumerator:: plusOp
  .. cpp:enumerator:: minusOp
  .. cpp:enumerator:: timesOp
  .. cpp:enumerator:: divOp
  .. cpp:enumerator:: lessOp
  .. cpp:enumerator:: leOp
  .. cpp:enumerator:: greaterOp
  .. cpp:enumerator:: geOp
  .. cpp:enumerator:: eqOp
  .. cpp:enumerator:: neOp
  .. cpp:enumerator:: loadOp
  .. cpp:enumerator:: loadConstOp
  .. cpp:enumerator:: loadFrameRelativeOp
  .. cpp:enumerator:: loadFrameAddr
  .. cpp:enumerator:: loadRegRelativeOp

    More general form of loadFrameRelativeOp

  .. cpp:enumerator:: loadRegRelativeAddr

    More general form of loadFrameAddr

  .. cpp:enumerator:: storeOp
  .. cpp:enumerator:: storeFrameRelativeOp
  .. cpp:enumerator:: ifOp
  .. cpp:enumerator:: whileOp

    Simple control structures will be useful

  .. cpp:enumerator:: doOp
  .. cpp:enumerator:: callOp
  .. cpp:enumerator:: trampPreamble
  .. cpp:enumerator:: noOp
  .. cpp:enumerator:: orOp
  .. cpp:enumerator:: andOp
  .. cpp:enumerator:: getRetValOp
  .. cpp:enumerator:: getRetAddrOp
  .. cpp:enumerator:: getSysRetValOp
  .. cpp:enumerator:: getParamOp
  .. cpp:enumerator:: getParamAtCallOp
  .. cpp:enumerator:: getParamAtEntryOp
  .. cpp:enumerator:: getSysParamOp
  .. cpp:enumerator:: getAddrOp

    return the address of the operand

  .. cpp:enumerator:: loadIndirOp
  .. cpp:enumerator:: storeIndirOp
  .. cpp:enumerator:: saveRegOp
  .. cpp:enumerator:: loadRegOp
  .. cpp:enumerator:: saveStateOp

    For saving of non-register state (flags reg condition reg)

  .. cpp:enumerator:: loadStateOp

    And the corresponding load

  .. cpp:enumerator:: updateCostOp
  .. cpp:enumerator:: funcJumpOp

    Jump to function without linkage

  .. cpp:enumerator:: funcCallOp

    Call to function with linkage

  .. cpp:enumerator:: branchOp
  .. cpp:enumerator:: ifMCOp
  .. cpp:enumerator:: breakOp
  .. cpp:enumerator:: xorOp
  .. cpp:enumerator:: undefOp
