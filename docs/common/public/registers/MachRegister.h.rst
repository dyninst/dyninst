.. _`sec:MachRegister.h`:

MachRegister.h
##############

.. cpp:namespace:: Dyninst

.. cpp:type:: unsigned long MachRegisterVal

.. cpp:class:: MachRegister

  **A representation of a machine register**

  .. cpp:function:: MachRegister getBaseRegister() const

    Returns the largest register that may alias with the given register.

    For example on x86, ``rax`` aliases ``ah``.

  .. cpp:function:: Architecture getArchitecture() const

    Returns the architecture to which this register belongs.

  .. cpp:function:: bool isValid() const

    Checks if the register is valid.

    An invalid register is represented by :ref:`InvalidReg <sec:abstract-registers>`.

    Invalid registers may be returned from member functions of this class when
    an error occurs or if the request is not supported on the specified architecture.

  .. cpp:function:: std::string const& name() const

    Returns a string representation of the register.

  .. cpp:function:: unsigned int regClass() const

    Returns the :ref:`category <sec:register-categories>` of the register.

  .. cpp:function:: static MachRegister getPC(Architecture arch)

    Returns the program counter (PC) register for ``arch``.

    Returns :ref:`InvalidReg <sec:abstract-registers>` if ``arch`` has no PC.

  .. cpp:function:: static MachRegister getReturnAddress(Architecture arch)

    Returns the :ref:`return address <sec:abstract-registers>` register for ``arch``.

    Returns :ref:`InvalidReg <sec:abstract-registers>` if ``arch`` has no return address register.

  .. cpp:function:: static MachRegister getFramePointer(Architecture arch)

    Returns the :ref:`frame pointer <sec:abstract-registers>` (FP) register for ``arch``.

    Returns :ref:`InvalidReg <sec:abstract-registers>` if ``arch`` has no FP.

  .. cpp:function:: static MachRegister getStackPointer(Architecture arch)

    Returns the :ref:`stack pointer <sec:abstract-registers>` register for ``arch``.

    Returns :ref:`InvalidReg <sec:abstract-registers>` if ``arch`` has no SP.

  .. cpp:function:: static MachRegister getSyscallNumberReg(Architecture arch)

    Returns the register used to pass the syscall number for ``arch``.

    Returns :ref:`InvalidReg <sec:abstract-registers>` if ``arch`` has no syscall register.

  .. cpp:function:: static MachRegister getSyscallNumberOReg(Architecture arch)

    Returns the register used to pass the syscall number zero for ``arch``.

    Returns :ref:`InvalidReg <sec:abstract-registers>` if ``arch`` has no syscall number zero register.

  .. cpp:function:: static MachRegister getSyscallReturnValueReg(Architecture arch)

    Returns the return value register for ``arch``.

    Returns :ref:`InvalidReg <sec:abstract-registers>` if ``arch`` has no return value register.

  .. cpp:function:: static MachRegister getZeroFlag(Architecture arch)

    Returns the zero flag (ZF) register for ``arch``.

    Returns :ref:`InvalidReg <sec:abstract-registers>` if ``arch`` has no ZF.

  .. cpp:function:: bool isPC() const

    Checks if the current register is the program counter register for its architecture.

  .. cpp:function:: bool isFramePointer() const

    Checks if the current register is the frame pointer register for its architecture.

  .. cpp:function:: bool isStackPointer() const

    Checks if the current register is the stack pointer register for its architecture.

  .. cpp:function:: bool isSyscallNumberReg() const

    Checks if the current register is the syscall number register for its architecture.

  .. cpp:function:: bool isSyscallReturnValueReg() const

    Checks if the current register is the return value register for its architecture.

  .. cpp:function:: bool isFlag() const

    Checks if the current register is the flag register for its architecture.

  .. cpp:function:: bool isZeroFlag() const

    Checks if the current register is the zero flag register for its architecture.

  .. cpp:function:: void getROSERegister(int& c, int& n, int& p)

    Returns the ROSE class , number, and position in ``c``, ``n``, and ``p``, respectively.

    These correspond to the Dyninst category, base id, and subrange values.

    On error, ``c`` is set to -1.

  .. cpp:function:: static MachRegister DwarfEncToReg(int encoding, Architecture arch)

    Returns the register corresponding to the DWARF encoding as specified by the ABI for architecture ``arch``.

  .. cpp:function:: int getDwarfEnc() const

    Returns the register's DWARF encoding as specified by the ABI for architecture ``arch``.

  .. cpp:function:: static MachRegister getArchRegFromAbstractReg(MachRegister abstract, Architecture arch)

    This is only valid when ``arch`` is :cpp:enumerator:`Arch_aarch64`.

  .. cpp:function:: static MachRegister getArchReg(unsigned int regNum, Architecture arch)

    This is only valid when ``arch`` is :cpp:enumerator:`Arch_aarch64`.
