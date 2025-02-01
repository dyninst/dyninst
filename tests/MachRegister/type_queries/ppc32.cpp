#include "registers/MachRegister.h"
#include "registers/ppc32_regs.h"
#include "type_query_check.h"
#include <iostream>

constexpr auto arch = Dyninst::Arch_ppc32;

static bool is_arithmetic_float(Dyninst::MachRegister);

int main() {
  TYPE_QUERIES_CHECK(Dyninst::ppc32::pc, isPC);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getPC(arch), isPC);

  TYPE_QUERIES_CHECK(Dyninst::ppc32::r1, isFramePointer);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getFramePointer(arch), isFramePointer);

  TYPE_QUERIES_CHECK(Dyninst::ppc32::r1, isStackPointer);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getStackPointer(arch), isStackPointer);

  TYPE_QUERIES_CHECK(Dyninst::ppc32::r0, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getSyscallNumberReg(arch), isSyscallNumberReg);

  TYPE_QUERIES_CHECK(Dyninst::ppc32::r3, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getSyscallReturnValueReg(arch), isSyscallReturnValueReg);

  /*********************************************************************
   *      General Purpose
   *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::ppc32::r0, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::r31, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::ppc32::fpr0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::ppc32::fpr1, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::ppc32::fsr0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::ppc32::fsr1, isGeneralPurpose);

  /*********************************************************************
   *      Flags
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr0l, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr0g, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr0e, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr0s, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr7l, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr7g, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr7e, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr7s, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr0, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr7, isFlag);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::ppc32::fpscw, isFlag);

  /*********************************************************************
   *      Zero Flag
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr0e, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr1e, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr2e, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr3e, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr4e, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr5e, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr6e, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::cr7e, isZeroFlag);

  /*********************************************************************
   *      Control/Status
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::ppc32::ctr, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::fpscw, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::fpscw0, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::fpscw1, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::fpscw2, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::fpscw3, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::fpscw4, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::fpscw5, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::fpscw6, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::fpscw7, isControlStatus);

  /*********************************************************************
   *      Floating-Point
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::ppc32::fpr0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::fpr31, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::fsr0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::ppc32::fsr31, isFloatingPoint);

  /*********************************************************************
   *      Filtering
  *********************************************************************/
  {
    for(auto reg : Dyninst::MachRegister::getAllRegistersForArch(arch)) {
      if(reg.isFloatingPoint() && !reg.isVector() && !reg.isControlStatus()) {
        TYPE_QUERIES_ASSERT_TRUE(reg, is_arithmetic_float(reg));
      }
      if(is_arithmetic_float(reg)) {
        TYPE_QUERIES_ASSERT_TRUE(reg, reg.isFloatingPoint() && !reg.isVector() && !reg.isControlStatus());
      }
    }
  }
  return EXIT_SUCCESS;
}

static bool is_arithmetic_float(Dyninst::MachRegister reg) {
  switch(reg) {
    case Dyninst::ppc32::ifsr0:
    case Dyninst::ppc32::ifsr1:
    case Dyninst::ppc32::ifsr2:
    case Dyninst::ppc32::ifsr3:
    case Dyninst::ppc32::ifsr4:
    case Dyninst::ppc32::ifsr5:
    case Dyninst::ppc32::ifsr6:
    case Dyninst::ppc32::ifsr7:
    case Dyninst::ppc32::ifsr8:
    case Dyninst::ppc32::ifsr9:
    case Dyninst::ppc32::ifsr10:
    case Dyninst::ppc32::ifsr11:
    case Dyninst::ppc32::ifsr12:
    case Dyninst::ppc32::ifsr13:
    case Dyninst::ppc32::ifsr14:
    case Dyninst::ppc32::ifsr15:
    case Dyninst::ppc32::ifsr16:
    case Dyninst::ppc32::ifsr17:
    case Dyninst::ppc32::ifsr18:
    case Dyninst::ppc32::ifsr19:
    case Dyninst::ppc32::ifsr20:
    case Dyninst::ppc32::ifsr21:
    case Dyninst::ppc32::ifsr22:
    case Dyninst::ppc32::ifsr23:
    case Dyninst::ppc32::ifsr24:
    case Dyninst::ppc32::ifsr25:
    case Dyninst::ppc32::ifsr26:
    case Dyninst::ppc32::ifsr27:
    case Dyninst::ppc32::ifsr28:
    case Dyninst::ppc32::ifsr29:
    case Dyninst::ppc32::ifsr30:
    case Dyninst::ppc32::ifsr31:

    case Dyninst::ppc32::ifpr0:
    case Dyninst::ppc32::ifpr1:
    case Dyninst::ppc32::ifpr2:
    case Dyninst::ppc32::ifpr3:
    case Dyninst::ppc32::ifpr4:
    case Dyninst::ppc32::ifpr5:
    case Dyninst::ppc32::ifpr6:
    case Dyninst::ppc32::ifpr7:
    case Dyninst::ppc32::ifpr8:
    case Dyninst::ppc32::ifpr9:
    case Dyninst::ppc32::ifpr10:
    case Dyninst::ppc32::ifpr11:
    case Dyninst::ppc32::ifpr12:
    case Dyninst::ppc32::ifpr13:
    case Dyninst::ppc32::ifpr14:
    case Dyninst::ppc32::ifpr15:
    case Dyninst::ppc32::ifpr16:
    case Dyninst::ppc32::ifpr17:
    case Dyninst::ppc32::ifpr18:
    case Dyninst::ppc32::ifpr19:
    case Dyninst::ppc32::ifpr20:
    case Dyninst::ppc32::ifpr21:
    case Dyninst::ppc32::ifpr22:
    case Dyninst::ppc32::ifpr23:
    case Dyninst::ppc32::ifpr24:
    case Dyninst::ppc32::ifpr25:
    case Dyninst::ppc32::ifpr26:
    case Dyninst::ppc32::ifpr27:
    case Dyninst::ppc32::ifpr28:
    case Dyninst::ppc32::ifpr29:
    case Dyninst::ppc32::ifpr30:
    case Dyninst::ppc32::ifpr31:
      return true;
  }
  return false;
}





