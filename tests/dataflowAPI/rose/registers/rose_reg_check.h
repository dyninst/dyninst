#ifndef DYNINST_UNIT_TESTS_DATAFLOWAPI_ROSE_REGISTERS_REGCHECK_H
#define DYNINST_UNIT_TESTS_DATAFLOWAPI_ROSE_REGISTERS_REGCHECK_H

#include "dataflowAPI/rose/registers/convert.h"

#include <iostream>

#define ROSEREG_CHECK(reg, mjr, mnr, p)                              \
{                                                                    \
    int major{}, minor{}, pos{}, size{};                             \
    namespace dd = Dyninst::DataflowAPI;                             \
    std::tie(major,minor,pos,size) = dd::convertToROSERegister(reg); \
    if(mjr != major) {                                               \
      std::cerr << "FAILED " #reg " major " #mjr "(" << mjr          \
                << "): got " << major << '\n';                       \
      return EXIT_FAILURE;                                           \
    }                                                                \
    if(mnr != minor) {                                               \
      std::cerr << "FAILED " #reg " minor " #mnr "(" << mnr          \
                << "): got " << minor << '\n';                       \
      return EXIT_FAILURE;                                           \
    }                                                                \
    if(p != pos) {                                                   \
      std::cerr << "FAILED " #reg " position " #p "(" << p           \
                << "): got " << pos << '\n';                         \
      return EXIT_FAILURE;                                           \
    }                                                                \
  }

#endif
