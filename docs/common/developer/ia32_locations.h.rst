.. _`sec:ia32_locations.h`:

ia32_locations.h
################

.. cpp:class:: ia32_locations

  .. cpp:function:: ia32_locations()
  .. cpp:function:: void reinit()

  .. cpp:member:: int num_prefixes
  .. cpp:member:: unsigned int opcode_size
  .. cpp:member:: int opcode_position
  .. cpp:member:: unsigned disp_size
  .. cpp:member:: int disp_position
  .. cpp:member:: int imm_cnt
  .. cpp:member:: int modrm_position
  .. cpp:member:: int modrm_operand
  .. cpp:member:: unsigned char modrm_byte
  .. cpp:member:: unsigned char modrm_mod
  .. cpp:member:: unsigned char modrm_rm
  .. cpp:member:: unsigned char modrm_reg
  .. cpp:member:: unsigned char sib_byte
  .. cpp:member:: int sib_position
  .. cpp:member:: int rex_position
  .. cpp:member:: unsigned char rex_byte
  .. cpp:member:: unsigned char rex_w
  .. cpp:member:: unsigned char rex_r
  .. cpp:member:: unsigned char rex_x
  .. cpp:member:: unsigned char rex_b
  .. cpp:member:: int address_size
  .. cpp:member:: int imm_position[2]
  .. cpp:member:: unsigned int imm_size[2]
