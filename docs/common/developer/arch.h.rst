.. _`sec:arch.h`:

arch.h
######

.. code:: cpp

  #if defined(arch_power)
  #include "arch-power.h"
  using namespace NS_power;

  #elif defined(i386_unknown_nt4_0) \
     || defined(arch_x86)           \
     || defined(arch_x86_64)
  #include "arch-x86.h"
  using namespace NS_x86;

  #elif defined(arch_aarch64)
  #include "arch-aarch64.h"
  using namespace NS_aarch64;
  #else
  #error "unknown architecture"

  #endif

.. c:macro:: GET_PTR(insn, gen)
.. c:macro:: SET_PTR(insn, gen)
.. c:macro:: REGET_PTR(insn, gen)
