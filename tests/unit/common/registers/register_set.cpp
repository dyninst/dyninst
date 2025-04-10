#include "registers/x86_regs.h"
#include "registers/aarch64_regs.h"
#include "registers/register_set.h"

#include <cstdlib>
#include <iostream>


int main() {

  {
    Dyninst::register_set x{Dyninst::x86::ebp, Dyninst::aarch64::w0};
    Dyninst::register_set y{Dyninst::aarch64::w0};
    Dyninst::register_set z{Dyninst::x86::ebp};

    if((x-y) != z) {
      std::cerr << "Failed\n";
      return EXIT_FAILURE;
    }

    if((y|z) != x) {
      std::cerr << "Failed\n";
      return EXIT_FAILURE;
    }

    if((x^z) != y) {
      std::cerr << "Failed\n";
      return EXIT_FAILURE;
    }
  }

  return EXIT_SUCCESS;
}
