.. _`sec:InstructionDecoder.h`:

InstructionDecoder.h
####################

.. cpp:namespace:: Dyninst::InstructionAPI

.. cpp:class:: InstructionDecoder

  **Converts a sequence of bytes into a machine instruction**

  .. cpp:member:: static const unsigned int maxInstructionLength

  .. cpp:function:: InstructionDecoder(const unsigned char *buffer, size_t size, Architecture arch)

      Construct an ``InstructionDecoder`` over the provided ``buffer`` and ``size``.

  .. cpp:function:: InstructionDecoder(const void *buffer, size_t size, Architecture arch)

  .. cpp:function:: Instruction decode();

      Decodes the next instruction in the buffer.

  .. cpp:function:: void doDelayedDecode(const Instruction* insn_to_complete)

.. cpp:struct:: buffer

  .. cpp:member:: const unsigned char* start
  .. cpp:member:: const unsigned char* end

  .. cpp:function:: buffer(const unsigned char* b, unsigned int len)
  .. cpp:function:: buffer(const void* b, unsigned int len)
  .. cpp:function:: buffer(const unsigned char* b, const unsigned char* e)


.. cpp:struct:: unknown_instruction

  This interface allows registering a callback function to be invoked
  when the InstructionDecoder encounters a byte sequence it is not able
  to successfully convert into a known instruction.

  .. cpp:type:: callback_t = Instruction(*)(buffer);
  .. cpp:function:: static void register_callback(callback_t)
  .. cpp:function:: static callback_t unregister_callback()
  .. cpp:function:: unknown_instruction() = delete
  .. cpp:function:: ~unknown_instruction() = delete
