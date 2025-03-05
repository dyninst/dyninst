#include "registers/MachRegister.h"
#include "registers/ppc64_regs.h"
#include "type_query_check.h"
#include <iostream>

constexpr auto arch = Dyninst::Arch_ppc64;

static bool is_arithmetic_float(Dyninst::MachRegister);
static bool is_arithmetic_vector(Dyninst::MachRegister);

int main() {
  TYPE_QUERIES_CHECK(Dyninst::ppc64::pc, isPC);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getPC(arch), isPC);

  TYPE_QUERIES_CHECK(Dyninst::ppc64::r1, isFramePointer);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getFramePointer(arch), isFramePointer);

  TYPE_QUERIES_CHECK(Dyninst::ppc64::r1, isStackPointer);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getStackPointer(arch), isStackPointer);

  TYPE_QUERIES_CHECK(Dyninst::ppc64::r0, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getSyscallNumberReg(arch), isSyscallNumberReg);

  TYPE_QUERIES_CHECK(Dyninst::ppc64::r3, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getSyscallReturnValueReg(arch), isSyscallReturnValueReg);

  /*********************************************************************
   *      General Purpose
   *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::ppc64::r0, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::r31, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::ppc64::fpr0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::ppc64::fpr1, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::ppc64::fsr0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::ppc64::fsr1, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::ppc64::vsr0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::ppc64::vsr63, isGeneralPurpose);

  /*********************************************************************
   *      Flags
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr0l, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr0g, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr0e, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr0s, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr7l, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr7g, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr7e, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr7s, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr0, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr7, isFlag);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::ppc64::fpscw, isFlag);

  /*********************************************************************
   *      Zero Flag
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr0e, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr1e, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr2e, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr3e, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr4e, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr5e, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr6e, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::cr7e, isZeroFlag);

  /*********************************************************************
   *      Control/Status
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::ppc64::ctr, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::fpscw, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::fpscw0, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::fpscw1, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::fpscw2, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::fpscw3, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::fpscw4, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::fpscw5, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::fpscw6, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::fpscw7, isControlStatus);

  /*********************************************************************
   *      Floating-Point
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::ppc64::fpr0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::fpr31, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::fsr0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::fsr31, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::vsr0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::vsr63, isFloatingPoint);

  /*********************************************************************
   *      Vector
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::ppc64::vsr0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::ppc64::vsr63, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::ppc64::fpscw, isVector);

  /*********************************************************************
   *      Filtering
  *********************************************************************/
  {
    for(auto reg : Dyninst::MachRegister::getAllRegistersForArch(arch)) {
      if(reg.isVector() && !reg.isControlStatus()) {
        TYPE_QUERIES_ASSERT_TRUE(reg, is_arithmetic_vector(reg));
      }
      if(is_arithmetic_vector(reg)) {
        TYPE_QUERIES_ASSERT_TRUE(reg, reg.isVector() && !reg.isControlStatus());
      }
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

static bool is_arithmetic_vector(Dyninst::MachRegister reg) {
  switch(reg) {
    case Dyninst::ppc64::ivsr0:
    case Dyninst::ppc64::ivsr1:
    case Dyninst::ppc64::ivsr2:
    case Dyninst::ppc64::ivsr3:
    case Dyninst::ppc64::ivsr4:
    case Dyninst::ppc64::ivsr5:
    case Dyninst::ppc64::ivsr6:
    case Dyninst::ppc64::ivsr7:
    case Dyninst::ppc64::ivsr8:
    case Dyninst::ppc64::ivsr9:
    case Dyninst::ppc64::ivsr10:
    case Dyninst::ppc64::ivsr11:
    case Dyninst::ppc64::ivsr12:
    case Dyninst::ppc64::ivsr13:
    case Dyninst::ppc64::ivsr14:
    case Dyninst::ppc64::ivsr15:
    case Dyninst::ppc64::ivsr16:
    case Dyninst::ppc64::ivsr17:
    case Dyninst::ppc64::ivsr18:
    case Dyninst::ppc64::ivsr19:
    case Dyninst::ppc64::ivsr20:
    case Dyninst::ppc64::ivsr21:
    case Dyninst::ppc64::ivsr22:
    case Dyninst::ppc64::ivsr23:
    case Dyninst::ppc64::ivsr24:
    case Dyninst::ppc64::ivsr25:
    case Dyninst::ppc64::ivsr26:
    case Dyninst::ppc64::ivsr27:
    case Dyninst::ppc64::ivsr28:
    case Dyninst::ppc64::ivsr29:
    case Dyninst::ppc64::ivsr30:
    case Dyninst::ppc64::ivsr31:
    case Dyninst::ppc64::ivsr32:
    case Dyninst::ppc64::ivsr33:
    case Dyninst::ppc64::ivsr34:
    case Dyninst::ppc64::ivsr35:
    case Dyninst::ppc64::ivsr36:
    case Dyninst::ppc64::ivsr37:
    case Dyninst::ppc64::ivsr38:
    case Dyninst::ppc64::ivsr39:
    case Dyninst::ppc64::ivsr40:
    case Dyninst::ppc64::ivsr41:
    case Dyninst::ppc64::ivsr42:
    case Dyninst::ppc64::ivsr43:
    case Dyninst::ppc64::ivsr44:
    case Dyninst::ppc64::ivsr45:
    case Dyninst::ppc64::ivsr46:
    case Dyninst::ppc64::ivsr47:
    case Dyninst::ppc64::ivsr48:
    case Dyninst::ppc64::ivsr49:
    case Dyninst::ppc64::ivsr50:
    case Dyninst::ppc64::ivsr51:
    case Dyninst::ppc64::ivsr52:
    case Dyninst::ppc64::ivsr53:
    case Dyninst::ppc64::ivsr54:
    case Dyninst::ppc64::ivsr55:
    case Dyninst::ppc64::ivsr56:
    case Dyninst::ppc64::ivsr57:
    case Dyninst::ppc64::ivsr58:
    case Dyninst::ppc64::ivsr59:
    case Dyninst::ppc64::ivsr60:
    case Dyninst::ppc64::ivsr61:
    case Dyninst::ppc64::ivsr62:
    case Dyninst::ppc64::ivsr63:
      return true;
  }
  return false;
}

static bool is_arithmetic_float(Dyninst::MachRegister reg) {
  switch(reg) {
    case Dyninst::ppc64::ifsr0:
    case Dyninst::ppc64::ifsr1:
    case Dyninst::ppc64::ifsr2:
    case Dyninst::ppc64::ifsr3:
    case Dyninst::ppc64::ifsr4:
    case Dyninst::ppc64::ifsr5:
    case Dyninst::ppc64::ifsr6:
    case Dyninst::ppc64::ifsr7:
    case Dyninst::ppc64::ifsr8:
    case Dyninst::ppc64::ifsr9:
    case Dyninst::ppc64::ifsr10:
    case Dyninst::ppc64::ifsr11:
    case Dyninst::ppc64::ifsr12:
    case Dyninst::ppc64::ifsr13:
    case Dyninst::ppc64::ifsr14:
    case Dyninst::ppc64::ifsr15:
    case Dyninst::ppc64::ifsr16:
    case Dyninst::ppc64::ifsr17:
    case Dyninst::ppc64::ifsr18:
    case Dyninst::ppc64::ifsr19:
    case Dyninst::ppc64::ifsr20:
    case Dyninst::ppc64::ifsr21:
    case Dyninst::ppc64::ifsr22:
    case Dyninst::ppc64::ifsr23:
    case Dyninst::ppc64::ifsr24:
    case Dyninst::ppc64::ifsr25:
    case Dyninst::ppc64::ifsr26:
    case Dyninst::ppc64::ifsr27:
    case Dyninst::ppc64::ifsr28:
    case Dyninst::ppc64::ifsr29:
    case Dyninst::ppc64::ifsr30:
    case Dyninst::ppc64::ifsr31:

    case Dyninst::ppc64::ifpr0:
    case Dyninst::ppc64::ifpr1:
    case Dyninst::ppc64::ifpr2:
    case Dyninst::ppc64::ifpr3:
    case Dyninst::ppc64::ifpr4:
    case Dyninst::ppc64::ifpr5:
    case Dyninst::ppc64::ifpr6:
    case Dyninst::ppc64::ifpr7:
    case Dyninst::ppc64::ifpr8:
    case Dyninst::ppc64::ifpr9:
    case Dyninst::ppc64::ifpr10:
    case Dyninst::ppc64::ifpr11:
    case Dyninst::ppc64::ifpr12:
    case Dyninst::ppc64::ifpr13:
    case Dyninst::ppc64::ifpr14:
    case Dyninst::ppc64::ifpr15:
    case Dyninst::ppc64::ifpr16:
    case Dyninst::ppc64::ifpr17:
    case Dyninst::ppc64::ifpr18:
    case Dyninst::ppc64::ifpr19:
    case Dyninst::ppc64::ifpr20:
    case Dyninst::ppc64::ifpr21:
    case Dyninst::ppc64::ifpr22:
    case Dyninst::ppc64::ifpr23:
    case Dyninst::ppc64::ifpr24:
    case Dyninst::ppc64::ifpr25:
    case Dyninst::ppc64::ifpr26:
    case Dyninst::ppc64::ifpr27:
    case Dyninst::ppc64::ifpr28:
    case Dyninst::ppc64::ifpr29:
    case Dyninst::ppc64::ifpr30:
    case Dyninst::ppc64::ifpr31:
    return true;
  }
  return false;
}







