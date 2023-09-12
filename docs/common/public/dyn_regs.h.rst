.. _`sec:dyn_regs.h`:

dyn_regs.h
##########

Representations of hardware registers on each supported architecture. We do not list the names of the registers here as the list is very long. See the file for those.

.. cpp:namespace:: Dyninst
.. cpp:type:: unsigned long MachRegisterVal

.. cpp:enum:: Architecture

   .. cpp:enumerator:: Architecture::Arch_none
   .. cpp:enumerator:: Architecture::Arch_x86
   .. cpp:enumerator:: Architecture::Arch_x86_64
   .. cpp:enumerator:: Architecture::Arch_ppc32
   .. cpp:enumerator:: Architecture::Arch_ppc64
   .. cpp:enumerator:: Architecture::Arch_aarch32
   .. cpp:enumerator:: Architecture::Arch_aarch64
   .. cpp:enumerator:: Architecture::Arch_amdgpu_vega
   .. cpp:enumerator:: Architecture::Arch_cuda
   .. cpp:enumerator:: Architecture::Arch_amdgpu_gfx90a
   .. cpp:enumerator:: Architecture::Arch_amdgpu_gfx908
   .. cpp:enumerator:: Architecture::Arch_intelGen9

.. cpp:function:: bool isSegmentRegister(int regClass)
.. cpp:function:: unsigned getArchAddressWidth(Dyninst::Architecture arch)

    The size of a pointer, in bytes, on the given architecture

.. cpp:class:: MachRegister

  .. cpp:function:: explicit MachRegister(signed int r)
  .. cpp:function:: explicit MachRegister(signed int r, const char *n)
  .. cpp:function:: explicit MachRegister(signed int r, std::string n)
  .. cpp:function:: MachRegister getBaseRegister() const

    This function returns the largest register that may alias with the given register.
    For example, ``x86_64::rax`` aliases ``x86_64::ah``.

  .. cpp:function:: Architecture getArchitecture() const
  .. cpp:function:: bool isValid() const

    Returns ``true`` if this instance represents a valid register. Some API
    functions may return invalid registers upon error.

  .. cpp:function:: MachRegisterVal getSubRegValue(const MachRegister& subreg, MachRegisterVal &orig) const

    Given a value for this register, orig, and a smaller aliased register, subreg, then this function
    returns the value of the aliased register. For example, if this function were called on ``x86::eax``
    with ``subreg`` as ``x86::al`` and an orig value of 0x11223344, then it would return 0x44.

  .. cpp:function:: std::string name() const
  .. cpp:function:: unsigned int size() const

    Number of bytes the register can hold

  .. cpp:function:: unsigned int regClass() const

    Return the category of the MachRegister

  .. cpp:function:: static MachRegister getPC(Dyninst::Architecture arch)
  .. cpp:function:: static MachRegister getReturnAddress(Dyninst::Architecture arch)
  .. cpp:function:: static MachRegister getFramePointer(Dyninst::Architecture arch)
  .. ATTENTION:: If an architecture does not support a frame pointer (e.g., ppc64) then ``getFramePointer`` returns an invalid register.

  .. cpp:function:: static MachRegister getStackPointer(Dyninst::Architecture arch)

  .. cpp:function:: static MachRegister getSyscallNumberReg(Dyninst::Architecture arch)
  .. cpp:function:: static MachRegister getSyscallNumberOReg(Dyninst::Architecture arch)
  .. cpp:function:: static MachRegister getSyscallReturnValueReg(Dyninst::Architecture arch)
  .. cpp:function:: static MachRegister getZeroFlag(Dyninst::Architecture arch)

  .. cpp:function:: bool isPC() const
  .. cpp:function:: bool isFramePointer() const
  .. cpp:function:: bool isStackPointer() const
  .. cpp:function:: bool isSyscallNumberReg() const
  .. cpp:function:: bool isSyscallReturnValueReg() const
  .. cpp:function:: bool isFlag() const
  .. cpp:function:: bool isZeroFlag() const
  .. cpp:function:: void getROSERegister(int &c, int &n, int &p)
  .. cpp:function:: static MachRegister DwarfEncToReg(int encoding, Dyninst::Architecture arch)
  .. cpp:function:: static MachRegister getArchRegFromAbstractReg(MachRegister abstract, Dyninst::Architecture arch)
  .. cpp:function:: static MachRegister getArchReg(unsigned int regNum, Dyninst::Architecture arch)
