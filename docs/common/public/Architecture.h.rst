.. _`sec:Architecture.h`:

Architecture.h
##############

.. cpp:namespace:: Dyninst

.. cpp:enum:: Architecture

  .. cpp:enumerator:: Arch_none

    Default state


  .. cpp:enumerator:: Arch_x86

    32-bit x86

  .. cpp:enumerator:: Arch_x86_64

    64-bit x86

  .. cpp:enumerator:: Arch_ppc32

    32-bit PowerPC

  .. cpp:enumerator:: Arch_ppc64

    64-bit PowerPC

  .. cpp:enumerator:: Arch_aarch32

    32-bit ARM (unsupported - reserved for future use)

  .. cpp:enumerator:: Arch_aarch64

    64-bit ARM

  .. cpp:enumerator:: Arch_cuda

    NVIDIA CUDA

  .. cpp:enumerator:: Arch_amdgpu_gfx908

    AMD GPU gfx908

  .. cpp:enumerator:: Arch_amdgpu_gfx90a

    AMD GPU gfx90a

  .. cpp:enumerator:: Arch_amdgpu_gfx940

    AMD GPU gfx940

  .. cpp:enumerator:: Arch_intelGen9

    Intel 9th Generation Graphics Technology (Apollo Lake)

.. cpp:function:: inline unsigned getArchAddressWidth(Architecture arch)

    Returns the number of bits used to make a memory address
