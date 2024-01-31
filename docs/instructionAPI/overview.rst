.. _`sec:instructionapi-intro`:

.. cpp:namespace:: Dyninst::InstructionAPI

InstructionAPI
##############

When analyzing and modifying binary code, it is necessary to translate
between raw binary instructions and an abstract form that describes the
semantics of the instructions.

Decoding
  Decoding interprets a sequence of bytes as a machine instruction in a
  specific machine language.

Abstract Syntax
  Each component of an instruction is represented by an AST
  that models its behavior and that data it operates on.

.. _`sec:instructionapi-abstractions`:

Abstractions
************

The InstructionAPI toolkit provides a representation
of machine instructions that can be manipulated by higher-level
algorithms with minimal knowledge of platform-specific details.
This allows for disassembling machine instructions, extracting operations
and their operands, and converting the operands to abstract syntax trees.
For advanced users, platform-specific information is accessible, as well.

.. _`sec:instructionapi-instructions`:

Instructions
  An :cpp:class:`Instruction` contains an :cpp:class:`Operation` and a collection of
  :cpp:class:`Operand`\ s. The operation contains the following:

    -  The mnemonic for the machine language instruction represented by its
       associated Instruction

    -  The number of operands accepted by the Operation

    -  Which Operands are read and/or written by the associated machine
       operation

    -  What other registers (if any) are affected by the underlying machine
       operation

  Each operand contains flags indicate which parts are read from or written to.
  It also contains an :cpp:class:`Expression` abstract syntax tree representing the
  operations required to compute the value of the operand.
  The :ref:`ownership <fig:ownership-graph>` figure below depicts these relationships.

  .. figure:: fig/ownership_graph.png
     :alt: An Instruction and the objects it owns
     :name: fig:ownership-graph
     :align: center

     An Instruction and the objects it owns

  Instruction objects provide two types of interfaces: direct read access
  to their components and common summary operations on those components.
  The first interface allows access to the Operation and Operand data
  members, and each Operand object in turn allows traversal of its
  abstract syntax tree. This interface would be used, for example, in a data
  flow analysis where a user wants to evaluate the results of an effective
  address computation given a known register state.

  The second interface allows a user to get the sets of registers read and
  written by the instruction, information about how the instruction
  accesses memory, and information about how the instruction affects
  control flow, without having to manipulate the Operands directly.

.. _`sec:instructionapi-decoding`:

Decoding
  An :cpp:class:`InstructionDecoder` interprets a sequence of bytes according to a
  given machine language and transforms them into an instruction
  representation. It determines the opcode of the machine instruction,
  translates that opcode to an Operation object, uses that Operation to
  determine how to decode the instruction’s Operands, and produces a
  decoded Instruction.

  .. figure:: fig/decoder_use.png
     :alt: The InstructionDecoder’s inputs and outputs
     :name: fig:decoder-use
     :align: center

     The InstructionDecoder’s inputs and outputs

  It is possible to generalize the construction of
  Instructions from Operations and Operands to an entirely
  platform-independent algorithm. Likewise, much of the construction of
  the ASTs representing each operand can be performed in a
  platform-independent manner.

.. _`sec:instructionapi-abstract-syntax`:

Abstract Syntax
  The :cpp:class:`AST` representation of an operand encapsulates the operations
  performed on registers and immediates to produce an operand for the
  machine language instruction.

  The inheritance hierarchy of the AST classes is shown below.

  .. figure:: fig/full_inheritance_graph.png
     :alt: The InstructionAST inheritance hierarchy
     :name: fig:inheritance
     :align: center

     The InstructionAST inheritance hierarchy

  The grammar for these AST representations is simple: all leaves must be
  RegisterAST or Immediate nodes. These nodes may be combined using a
  BinaryFunction node, which may be constructed as either an addition or a
  multiplication. Also, a single node may descend from a Dereference node,
  which treats its child as a memory address.

  .. figure:: fig/ast_ownership.png
     :alt: InstructionAST intermediate node types and the objects they own
     :name: fig:ownership
     :align: center

     InstructionAST intermediate node types and the objects they own

  .. figure:: fig/instruction_representation.png
     :alt: The decomposition of mov %eax, (%esi)
     :name: fig:representation
     :align: center

     An example of how an IA32 instruction is represented with InstrucationAST.

  These ASTs may be searched for leaf elements or subtrees using
  :cpp:func:`InstructionAST::getUses` and :cpp:func:`InstructionAST::isUsed`.
  The full AST can be traversed breadth- or depth-first using
  :cpp:func:`InstructionAST::getChildren`.

  Any node in these ASTs may be evaluated. Evaluation attempts to
  determine the value represented by a node. If successful, it will return
  that value and cache it in the node. The tree structure, combined with
  the evaluation mechanism, allows the substitution of known register and
  memory values into an operand, regardless of whether those values are
  known at the time an instruction is decoded.

Visitor Paradigm
  An alternative to the bind/eval mechanism is to use a :cpp:class:`Visitor`
  over an expression tree. The visitor paradigm can be used as a more efficient
  replacement for bind/eval to identify whether an expression has a
  desired pattern or to locate children of an expression tree.

  A user provides implementations of the four ``visit`` methods. When
  applied to an expression (via :cpp:func:`Expression::apply`), a
  post-order traversal of the tree is performed, calling the appropriate
  ``visit`` method at each node.

Usage
*****

Basic Disassembly
=================

Using the :cpp:class:`InstructionDecoder`, it's possible to build a very basic disassembler.
See the :ref:`example:instructionapi-func-disassem` example.


Unknown Instructions
====================

For very new binaries that have an ISA Dyninst does not yet support, it is possible to hook in
a user-level decoder to provide information to Dyninst about these instructions.

..  rli:: https://raw.githubusercontent.com/dyninst/examples/master/disassemble/unknown_instruction.cpp
    :language: cpp
    :linenos:


Visitor
=======

The following code prints out the name of each type of :cpp:class:`AST` visited.

..  rli:: https://raw.githubusercontent.com/dyninst/examples/master/instructionAPI/stateless_visitor.cpp
    :language: cpp
    :linenos:

Visitors may also set and use internal state. For example, the following
tracks which type of AST it has most-recently seen.

..  rli:: https://raw.githubusercontent.com/dyninst/examples/master/instructionAPI/statefull_visitor.cpp
    :language: cpp
    :linenos:
