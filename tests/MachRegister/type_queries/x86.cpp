#include "registers/MachRegister.h"
#include "registers/x86_regs.h"
#include "type_query_check.h"

static bool is_vec(Dyninst::MachRegister);
static bool is_x87(Dyninst::MachRegister);
static bool is_fpctl(Dyninst::MachRegister);

namespace dreg = Dyninst::x86;
constexpr auto arch = Dyninst::Arch_x86;
using mreg = Dyninst::MachRegister;

int main() {
  TYPE_QUERIES_CHECK(dreg::eip, isPC);
  TYPE_QUERIES_CHECK(mreg::getPC(arch), isPC);

  TYPE_QUERIES_CHECK(dreg::bp, isFramePointer);
  TYPE_QUERIES_CHECK(dreg::ebp, isFramePointer);
  TYPE_QUERIES_CHECK(mreg::getFramePointer(arch), isFramePointer);


  TYPE_QUERIES_CHECK(dreg::sp, isStackPointer);
  TYPE_QUERIES_CHECK(dreg::esp, isStackPointer);
  TYPE_QUERIES_CHECK(mreg::getStackPointer(arch), isStackPointer);

  TYPE_QUERIES_CHECK(dreg::al, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(dreg::ah, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(dreg::ax, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(dreg::eax, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(mreg::getSyscallNumberReg(arch), isSyscallNumberReg);

  TYPE_QUERIES_CHECK(dreg::al, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(dreg::ah, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(dreg::ax, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(dreg::eax, isSyscallReturnValueReg);

  /*********************************************************************
   *      General Purpose
   *********************************************************************/
  TYPE_QUERIES_CHECK(dreg::al, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::ah, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::ax, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::eax, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::bl, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::bh, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::bx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::ebx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::cl, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::ch, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::cx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::ecx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::dl, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::dh, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::dx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::edx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::si, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::esi, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::di, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::edi, isGeneralPurpose);

  TYPE_QUERIES_CHECK_FALSE(dreg::mm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::mm7, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::xmm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::xmm7, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::ymm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::ymm7, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::zmm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::zmm7, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::k0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::k7, isGeneralPurpose);

  /*********************************************************************
   *      Flags
  *********************************************************************/
  TYPE_QUERIES_CHECK(dreg::cf, isFlag);
  TYPE_QUERIES_CHECK(dreg::flag1, isFlag);
  TYPE_QUERIES_CHECK(dreg::pf, isFlag);
  TYPE_QUERIES_CHECK(dreg::flag3, isFlag);
  TYPE_QUERIES_CHECK(dreg::af, isFlag);
  TYPE_QUERIES_CHECK(dreg::flag5, isFlag);
  TYPE_QUERIES_CHECK(dreg::zf, isFlag);
  TYPE_QUERIES_CHECK(dreg::sf, isFlag);
  TYPE_QUERIES_CHECK(dreg::tf, isFlag);
  TYPE_QUERIES_CHECK(dreg::if_, isFlag);
  TYPE_QUERIES_CHECK(dreg::df, isFlag);
  TYPE_QUERIES_CHECK(dreg::of, isFlag);
  TYPE_QUERIES_CHECK(dreg::flagc, isFlag);
  TYPE_QUERIES_CHECK(dreg::flagd, isFlag);
  TYPE_QUERIES_CHECK(dreg::nt_, isFlag);
  TYPE_QUERIES_CHECK(dreg::flagf, isFlag);
  TYPE_QUERIES_CHECK(dreg::rf, isFlag);
  TYPE_QUERIES_CHECK(dreg::vm, isFlag);
  TYPE_QUERIES_CHECK(dreg::ac, isFlag);
  TYPE_QUERIES_CHECK(dreg::vif, isFlag);
  TYPE_QUERIES_CHECK(dreg::vip, isFlag);
  TYPE_QUERIES_CHECK(dreg::id, isFlag);

  /*********************************************************************
   *      Zero Flag
  *********************************************************************/
  TYPE_QUERIES_CHECK_FALSE(dreg::flags, isZeroFlag);
  TYPE_QUERIES_CHECK(dreg::zf, isZeroFlag);
  TYPE_QUERIES_CHECK(mreg::getZeroFlag(arch), isZeroFlag);

  /*********************************************************************
   *      Control/Status
  *********************************************************************/
  TYPE_QUERIES_CHECK(dreg::fcw, isControlStatus);
  TYPE_QUERIES_CHECK(dreg::fsw, isControlStatus);
  TYPE_QUERIES_CHECK(dreg::mxcsr, isControlStatus);
  TYPE_QUERIES_CHECK(dreg::cr0, isControlStatus);
  TYPE_QUERIES_CHECK(dreg::cr7, isControlStatus);
  TYPE_QUERIES_CHECK(dreg::k0, isControlStatus);
  TYPE_QUERIES_CHECK(dreg::k7, isControlStatus);

  TYPE_QUERIES_CHECK_FALSE(dreg::flags, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::zf, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::zf.getBaseRegister(), isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::mm0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::mm7, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::xmm0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::xmm7, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::ymm0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::ymm7, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::zmm0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::zmm7, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::eax, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::ebx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::ecx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::edx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::eip, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::esp, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::eax, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::ebx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::ecx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::edx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::eip, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::esp, isControlStatus);

  /*********************************************************************
   *      Floating-Point
  *********************************************************************/
  TYPE_QUERIES_CHECK(dreg::st0, isFloatingPoint);
  TYPE_QUERIES_CHECK(dreg::st7, isFloatingPoint);
  TYPE_QUERIES_CHECK(dreg::mm0, isFloatingPoint);
  TYPE_QUERIES_CHECK(dreg::mm7, isFloatingPoint);

  TYPE_QUERIES_CHECK(dreg::fcw, isFloatingPoint);
  TYPE_QUERIES_CHECK(dreg::fsw, isFloatingPoint);
  TYPE_QUERIES_CHECK(dreg::mxcsr, isFloatingPoint);

  TYPE_QUERIES_CHECK(dreg::xmm0, isFloatingPoint);
  TYPE_QUERIES_CHECK(dreg::xmm7, isFloatingPoint);
  TYPE_QUERIES_CHECK(dreg::ymm0, isFloatingPoint);
  TYPE_QUERIES_CHECK(dreg::ymm7, isFloatingPoint);
  TYPE_QUERIES_CHECK(dreg::zmm0, isFloatingPoint);
  TYPE_QUERIES_CHECK(dreg::zmm7, isFloatingPoint);

  TYPE_QUERIES_CHECK_FALSE(dreg::k0, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(dreg::k7, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(dreg::eax, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(dreg::ebx, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(dreg::ecx, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(dreg::edx, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(dreg::eip, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(dreg::esp, isFloatingPoint);

  /*********************************************************************
   *      Vector
  *********************************************************************/
  TYPE_QUERIES_CHECK(dreg::xmm0, isVector);
  TYPE_QUERIES_CHECK(dreg::xmm7, isVector);
  TYPE_QUERIES_CHECK(dreg::ymm0, isVector);
  TYPE_QUERIES_CHECK(dreg::ymm7, isVector);
  TYPE_QUERIES_CHECK(dreg::zmm0, isVector);
  TYPE_QUERIES_CHECK(dreg::zmm7, isVector);
  TYPE_QUERIES_CHECK(dreg::mm0, isVector);
  TYPE_QUERIES_CHECK(dreg::mm7, isVector);
  TYPE_QUERIES_CHECK(dreg::k0, isVector);
  TYPE_QUERIES_CHECK(dreg::k7, isVector);
  TYPE_QUERIES_CHECK(dreg::mxcsr, isVector);

  TYPE_QUERIES_CHECK_FALSE(dreg::st0, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::st7, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::fcw, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::fsw, isVector);

  TYPE_QUERIES_CHECK_FALSE(dreg::flags, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::zf, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::zf.getBaseRegister(), isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::eax, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::ebx, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::ecx, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::edx, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::eip, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::esp, isVector);

  /*********************************************************************
   *      Filtering
  *********************************************************************/
  {
    for(auto reg : mreg::getAllRegistersForArch(arch)) {
      if(reg.isFloatingPoint() && !reg.isControlStatus() && !reg.isVector()) {
        TYPE_QUERIES_ASSERT_TRUE(reg, is_x87(reg));
      }
      if(is_x87(reg)) {
        TYPE_QUERIES_ASSERT_TRUE(reg, reg.isFloatingPoint() && !reg.isControlStatus() && !reg.isVector());
      }
      if(reg.isVector() && !reg.isControlStatus()) {
        TYPE_QUERIES_ASSERT_TRUE(reg, is_vec(reg));
      }
      if(is_vec(reg)) {
        TYPE_QUERIES_ASSERT_TRUE(reg, reg.isVector() && !reg.isControlStatus());
      }
      if(reg.isFloatingPoint() && reg.isControlStatus()) {
        TYPE_QUERIES_ASSERT_TRUE(reg, is_fpctl(reg));
      }
      if(is_fpctl(reg)) {
        TYPE_QUERIES_ASSERT_TRUE(reg, reg.isFloatingPoint() && reg.isControlStatus());
      }
    }
  }

  return EXIT_SUCCESS;
}

static bool is_fpctl(Dyninst::MachRegister reg) {
  switch(reg) {
    case dreg::imxcsr:
    case dreg::ifcw:
    case dreg::ifsw:
      return true;
  }
  return false;
}

static bool is_x87(Dyninst::MachRegister reg) {
  switch(reg) {
    case dreg::ist0:
    case dreg::ist1:
    case dreg::ist2:
    case dreg::ist3:
    case dreg::ist4:
    case dreg::ist5:
    case dreg::ist6:
    case dreg::ist7:
      return true;
  }
  return false;
}

static bool is_vec(Dyninst::MachRegister reg) {
  switch(reg) {
    case dreg::imm0:
    case dreg::imm1:
    case dreg::imm2:
    case dreg::imm3:
    case dreg::imm4:
    case dreg::imm5:
    case dreg::imm6:
    case dreg::imm7:
    case dreg::ixmm0:
    case dreg::ixmm1:
    case dreg::ixmm2:
    case dreg::ixmm3:
    case dreg::ixmm4:
    case dreg::ixmm5:
    case dreg::ixmm6:
    case dreg::ixmm7:
    case dreg::iymm0:
    case dreg::iymm1:
    case dreg::iymm2:
    case dreg::iymm3:
    case dreg::iymm4:
    case dreg::iymm5:
    case dreg::iymm6:
    case dreg::iymm7:
    case dreg::izmm0:
    case dreg::izmm1:
    case dreg::izmm2:
    case dreg::izmm3:
    case dreg::izmm4:
    case dreg::izmm5:
    case dreg::izmm6:
    case dreg::izmm7:
      return true;
  }
  return false;
}

