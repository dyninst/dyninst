#ifndef DYNINST_UNIT_TESTS_MACHREGISTER_BASE_REGISTERS_H
#define DYNINST_UNIT_TESTS_MACHREGISTER_BASE_REGISTERS_H

#include "registers/MachRegister.h"
#include <iostream>
#include <cstdlib>

std::ostream& operator<<(std::ostream &os, Dyninst::MachRegister const& r) {
  os << r.name() << "[" << r.size() << "] (0x" << std::hex << r.val() << ")\n";
  return os;
}

#define BASEREG_CHECK(r1, r2)                     \
  {                                               \
    auto const rr1 = Dyninst::r1;                 \
    auto const rr2 = Dyninst::r2;                 \
    if(rr1.getBaseRegister() != rr2) {            \
      std::cerr << "FAILED " #r1 " -> " #r2 "\n"; \
      return EXIT_FAILURE;                        \
    }                                             \
  }

#endif
