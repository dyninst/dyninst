InstructionDecoder.h
====================

.. cpp:namespace:: Dyninst::instructionAPI

InstructionDecoder Class
------------------------

The ``InstructionDecoder`` class decodes instructions, given a buffer of
bytes and a length, and constructs an Instruction.

.. code-block:: cpp

    InstructionDecoder(const unsigned char *buffer, size_t size, Architecture arch) InstructionDecoder(const void *buffer, size_t size, Architecture arch)

Construct an ``InstructionDecoder`` over the provided ``buffer`` and
``size``. We consider the buffer to contain instructions from the
provided ``arch``, which must be from the set
``{Arch_x86, Arch_x86_64, Arch_ppc32, Arch_ppc64}``.

.. code-block:: cpp

    Instruction::Ptr decode();

Decode the next address in the buffer provided at construction time,
returning either an instruction pointer or ``NULL`` if the buffer
contains no undecoded instructions.