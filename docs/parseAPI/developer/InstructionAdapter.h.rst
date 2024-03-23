.. _`sec-dev:InstructionAdapter.h`:

InstructionAdapter.h
####################

.. cpp:namespace:: Dyninst::InsnAdapter::dev

.. cpp:class:: InstructionAdapter

  .. cpp:function:: protected virtual bool isReturn(Dyninst::ParseAPI::Function * context, Dyninst::ParseAPI::Block* currBlk) const = 0;

      Uses pattern heuristics or backward slicing to determine if a blr instruction is a return or jump table

  .. cpp:function:: protected virtual bool isRealCall() const = 0;

  .. cpp:member:: protected Address current
  .. cpp:member:: protected Address previous
  .. cpp:member:: protected mutable bool parsedJumpTable
  .. cpp:member:: protected mutable bool successfullyParsedJumpTable
  .. cpp:member:: protected mutable bool isDynamicCall_
  .. cpp:member:: protected mutable bool checkedDynamicCall_
  .. cpp:member:: protected mutable bool isInvalidCallTarget_
  .. cpp:member:: protected mutable bool checkedInvalidCallTarget_
  .. cpp:member:: protected ParseAPI::CodeObject * _obj
  .. cpp:member:: protected ParseAPI::CodeRegion * _cr
  .. cpp:member:: protected InstructionSource * _isrc
  .. cpp:member:: protected ParseAPI::Block * _curBlk

      Block associated with the instruction adapter. This is required for powerpc slicing
      to determine the return address of a function.
