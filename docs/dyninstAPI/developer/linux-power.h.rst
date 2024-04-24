.. _`sec:linux-power.h`:

linux-power.h
#############

.. cpp:namespace:: dev::power

.. cpp:function:: extern Dyninst::Address region_lo(const Dyninst::Address x)

  floor of inferior malloc address range within a single branch of x for 32-bit ELF PowerPC mutatees

.. cpp:function:: extern Dyninst::Address region_lo_64(const Dyninst::Address x)

  floor of inferior malloc address range within a single branch of x for 64-bit ELF PowerPC mutatees

.. cpp:function:: extern Dyninst::Address region_hi(const Dyninst::Address x)

  ceiling of inferior malloc address range within a single branch of x for 32-bit ELF PowerPC mutatees

.. cpp:function:: extern Dyninst::Address region_hi_64(const Dyninst::Address x)

  ceiling of inferior malloc address range within a single branch of x for 64-bit ELF PowerPC mutatees

