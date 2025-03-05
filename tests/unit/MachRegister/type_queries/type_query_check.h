#ifndef DYNINST_UNIT_TESTS_MACHREGISTER_TYPE_QUERIES_H
#define DYNINST_UNIT_TESTS_MACHREGISTER_TYPE_QUERIES_H

#include "registers/MachRegister.h"
#include "registers/abstract_regs.h"

#include <iostream>
#include <cstdlib>

std::ostream& operator<<(std::ostream &os, Dyninst::MachRegister const& r) {
  os << r.name() << "[" << r.size() << "] (0x" << std::hex << r.val() << ")\n";
  return os;
}

#define TYPE_QUERIES_CHECK_FALSE(reg, type_func)      \
  if((reg).type_func()) {                             \
    std::cerr << "FAILED " #reg "." #type_func "\n";  \
    return EXIT_FAILURE;                              \
  }


#define TYPE_QUERIES_ASSERT_TRUE(reg, stmt) \
  if(!(stmt)) {                             \
    std::cerr << "FAILED " << reg.name()    \
              << " '" #stmt "'\n";          \
    return EXIT_FAILURE;                    \
  }

#define TYPE_QUERIES_CHECK(reg, type_func)              \
  if(!(reg).type_func()) {                              \
    std::cerr << "FAILED " #reg "." #type_func "\n";    \
    return EXIT_FAILURE;                                \
  }                                                     \

#define TYPE_QUERIES_CHECK_INVALID(type_func, arch)       \
{                                                         \
  auto bad_reg = Dyninst::InvalidReg;                     \
  if(Dyninst::MachRegister::type_func(arch) != bad_reg) { \
    std::cerr << "FAILED " #type_func "(" #arch ")\n";    \
    return EXIT_FAILURE;                                  \
  }                                                       \
}

#endif

#define TYPE_QUERIES_ASSERT_FALSE TYPE_QUERIES_CHECK_FALSE
