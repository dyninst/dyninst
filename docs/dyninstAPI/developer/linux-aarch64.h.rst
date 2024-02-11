.. _`sec:linux-aarch64.h`:

linux-aarch64.h
###############

.. cpp:namespace:: dev::aarch64

.. cpp:function:: extern Dyninst::Address region_lo(const Dyninst::Address x)

  floor of inferior malloc address range within a single branch of x for 32-bit ELF aarch64 mutatees

.. cpp:function:: extern Dyninst::Address region_lo_64(const Dyninst::Address x)

  floor of inferior malloc address range within a single branch of x for 64-bit ELF aarch64 mutatees

.. cpp:function:: extern Dyninst::Address region_hi(const Dyninst::Address x)

  ceiling of inferior malloc address range within a single branch of x for 32-bit ELF aarch64 mutatees

.. cpp:function:: extern Dyninst::Address region_hi_64(const Dyninst::Address x)

  ceiling of inferior malloc address range within a single branch of x for 64-bit ELF aarch64 mutatees
