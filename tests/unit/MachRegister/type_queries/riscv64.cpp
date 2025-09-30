#include "registers/MachRegister.h"
#include "registers/riscv64_regs.h"
#include "type_query_check.h"
#include <iostream>

constexpr auto arch = Dyninst::Arch_riscv64;

int main() {
  TYPE_QUERIES_CHECK(Dyninst::riscv64::pc, isPC);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getPC(arch), isPC);

  TYPE_QUERIES_CHECK(Dyninst::riscv64::fp, isFramePointer);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getFramePointer(arch), isFramePointer);

  TYPE_QUERIES_CHECK(Dyninst::riscv64::sp, isStackPointer);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getStackPointer(arch), isStackPointer);

  TYPE_QUERIES_CHECK(Dyninst::riscv64::a7, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getSyscallNumberReg(arch), isSyscallNumberReg);

  TYPE_QUERIES_CHECK(Dyninst::riscv64::a0, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getSyscallReturnValueReg(arch), isSyscallReturnValueReg);

  /*********************************************************************
   *      General Purpose
   *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::riscv64::x0, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::riscv64::x31, isGeneralPurpose);

  /*********************************************************************
   *      Floating-Point
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::riscv64::f0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::riscv64::f31, isFloatingPoint);

  return EXIT_SUCCESS;
}
