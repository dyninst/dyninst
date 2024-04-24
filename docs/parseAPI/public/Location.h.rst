Location.h
==========

.. cpp:namespace:: Dyninst::ParseAPI

.. cpp:struct:: Location

  .. cpp:member:: Function *const func
  .. cpp:member:: Block *const block
  .. cpp:member:: const Offset offset
  .. cpp:member:: InstructionAPI::Instruction insn
  .. cpp:member:: Edge *const edge
  .. cpp:member:: const bool untrusted
  .. cpp:member:: const type_t type

  .. cpp:function:: Location()
  .. cpp:function:: Location(Function *f)
  .. cpp:function:: Location(EntrySite e)
  .. cpp:function:: Location(CallSite c)
  .. cpp:function:: Location(ExitSite e)
  .. cpp:function:: Location(Function *f, Block *b)

      A block in a particular function

  .. cpp:function:: Location(BlockSite b)

      A block of a function

  .. cpp:function:: Location(Function *f, InsnLoc l)

      A trusted instruction (in a particular function)

  .. cpp:function:: Location(Function *f, Block *b, Offset o, InstructionAPI::Instruction i)

      An untrusted (raw) instruction (in a particular function)

  .. cpp:function:: Location(Function *f, Edge *e)

      An edge (in a particular function)

  .. cpp:function:: Location(EdgeLoc e)
  .. cpp:function:: Location(Block *b)

      A block in general

  .. cpp:function:: Location(InsnLoc l)

      A trusted instruction in general

  .. cpp:function:: Location(Block *b, Offset o)

      An untrusted (raw) instruction

  .. cpp:function:: Location(Edge *e)

  .. cpp:function:: bool legal(type_t t)

  .. cpp:function:: InsnLoc insnLoc()

  .. cpp:function:: bool isValid()


.. cpp:struct:: EntrySite

  .. cpp:member:: Function *func
  .. cpp:member:: Block *block
  .. cpp:function:: EntrySite(Function *f, Block *b)

.. cpp:struct:: CallSite

  .. cpp:member:: Function *func
  .. cpp:member:: Block *block
  .. cpp:function:: CallSite(Function *f, Block *b)

.. cpp:struct:: ExitSite

  .. cpp:member:: Function *func
  .. cpp:member:: Block *block
  .. cpp:function:: ExitSite(Function *f, Block *b)

.. cpp:struct:: EdgeLoc

  .. cpp:member:: Function *func
  .. cpp:member:: Edge *edge
  .. cpp:function:: EdgeLoc(Function *f, Edge *e)

.. cpp:struct:: BlockSite

  .. cpp:member:: Function *func
  .. cpp:member:: Block *block
  .. cpp:function:: BlockSite(Function *f, Block *b)

.. cpp:type:: std::vector<std::pair<InstructionAPI::Instruction, Offset>> InsnVec

.. cpp:function:: static void getInsnInstances(Block* block, InsnVec& insns)

.. cpp:struct:: InsnLoc

  .. cpp:member:: Block *const block
  .. cpp:member:: const Offset offset
  .. cpp:member:: InstructionAPI::Instruction insn

  .. cpp:function:: InsnLoc(Block *const b,  Offset o, const InstructionAPI::Instruction& i)

.. cpp:enum:: Location::type_t

  .. cpp:enumerator:: function_
  .. cpp:enumerator:: block_
  .. cpp:enumerator:: blockInstance_
  .. cpp:enumerator:: instruction_
  .. cpp:enumerator:: instructionInstance_
  .. cpp:enumerator:: edge_
  .. cpp:enumerator:: entry_
  .. cpp:enumerator:: call_
  .. cpp:enumerator:: exit_
  .. cpp:enumerator:: illegal_

