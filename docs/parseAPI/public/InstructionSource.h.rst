.. _`sec:InstructionSource.h`:

InstructionSource.h
###################

.. cpp:namespace:: Dyninst

.. cpp:class:: InstructionSource

  .. cpp:function:: InstructionSource()

  .. cpp:function:: virtual bool isValidAddress(Address const addr)

      Checks if ``addr`` is a valid code location.

  .. cpp:function:: virtual void* getPtrToInstruction(Address const addr) = 0

      Returns a pointer to the raw memory in the binary at ``addr``, assuming
      that location is code.

  .. cpp:function:: virtual void* getPtrToData(Address const addr) = 0

      Returns a pointer to the raw memory in the binary at ``addr``, assuming
      that location is data.

  .. cpp:function:: virtual unsigned int getAddressWidth() = 0

      Returns the address width for the underlying architecture.

  .. cpp:function:: virtual bool isCode(Address const addr) = 0

      Checks if ``addr`` is in the code portion of the binary.

  .. cpp:function:: virtual bool isData(Address const addr) = 0

      Checks if ``addr`` is in the data portion of the binary.

  .. cpp:function:: virtual bool isReadOnly(Address const addr) = 0

      Checks if ``addr`` is in a read-only portion of the binary.

  .. cpp:function:: virtual Address offset() = 0

      The start of the region covered by this instruction source.

  .. cpp:function:: virtual Address length() = 0

      The size of the region.

  .. cpp:function:: virtual Architecture getArch() = 0

      The architecture of the instruction source.

  .. cpp:function:: virtual bool isAligned(Address const addr)

      Checks if the address ``addr`` is aligned according to the architecture
      returned by :cpp:func:`getArch`.
