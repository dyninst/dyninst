#include "registers/MachRegister.h"
#include "registers/x86_regs.h"
#include "type_query_check.h"

static bool is_vec(Dyninst::MachRegister);
static bool is_x87(Dyninst::MachRegister);
static bool is_fpctl(Dyninst::MachRegister);

constexpr auto arch = Dyninst::Arch_x86;

int main() {
  TYPE_QUERIES_CHECK(Dyninst::x86::eip, isPC);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getPC(arch), isPC);

  TYPE_QUERIES_CHECK(Dyninst::x86::bp, isFramePointer);
  TYPE_QUERIES_CHECK(Dyninst::x86::ebp, isFramePointer);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getFramePointer(arch), isFramePointer);


  TYPE_QUERIES_CHECK(Dyninst::x86::sp, isStackPointer);
  TYPE_QUERIES_CHECK(Dyninst::x86::esp, isStackPointer);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getStackPointer(arch), isStackPointer);

  TYPE_QUERIES_CHECK(Dyninst::x86::al, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(Dyninst::x86::ah, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(Dyninst::x86::ax, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(Dyninst::x86::eax, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getSyscallNumberReg(arch), isSyscallNumberReg);

  TYPE_QUERIES_CHECK(Dyninst::x86::al, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(Dyninst::x86::ah, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(Dyninst::x86::ax, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(Dyninst::x86::eax, isSyscallReturnValueReg);

  /*********************************************************************
   *      General Purpose
   *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::x86::al, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::ah, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::ax, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::eax, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::bl, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::bh, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::bx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::ebx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::cl, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::ch, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::cx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::ecx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::dl, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::dh, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::dx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::edx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::si, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::esi, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::di, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86::edi, isGeneralPurpose);

  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::mm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::mm7, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::xmm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::xmm7, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::ymm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::ymm7, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::zmm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::zmm7, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::k0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::k7, isGeneralPurpose);

  /*********************************************************************
   *      Flags
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::x86::cf, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::flag1, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::pf, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::flag3, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::af, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::flag5, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::zf, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::sf, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::tf, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::if_, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::df, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::of, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::flagc, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::flagd, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::nt_, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::flagf, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::rf, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::vm, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::ac, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::vif, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::vip, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::id, isFlag);

  /*********************************************************************
   *      Zero Flag
  *********************************************************************/
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::flags, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86::zf, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getZeroFlag(arch), isZeroFlag);

  /*********************************************************************
   *      Control/Status
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::x86::fcw, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::x86::fsw, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::x86::mxcsr, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::x86::cr0, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::x86::cr7, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::x86::k0, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::x86::k7, isControlStatus);

  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::flags, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::zf, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::zf.getBaseRegister(), isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::mm0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::mm7, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::xmm0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::xmm7, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::ymm0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::ymm7, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::zmm0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::zmm7, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::eax, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::ebx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::ecx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::edx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::eip, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::esp, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::eax, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::ebx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::ecx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::edx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::eip, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::esp, isControlStatus);

  /*********************************************************************
   *      Floating-Point
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::x86::st0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86::st7, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86::mm0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86::mm7, isFloatingPoint);

  TYPE_QUERIES_CHECK(Dyninst::x86::fcw, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86::fsw, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86::mxcsr, isFloatingPoint);

  TYPE_QUERIES_CHECK(Dyninst::x86::xmm0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86::xmm7, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86::ymm0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86::ymm7, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86::zmm0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86::zmm7, isFloatingPoint);

  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::k0, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::k7, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::eax, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::ebx, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::ecx, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::edx, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::eip, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::esp, isFloatingPoint);

  /*********************************************************************
   *      Vector
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::x86::xmm0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86::xmm7, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86::ymm0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86::ymm7, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86::zmm0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86::zmm7, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86::mm0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86::mm7, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86::k0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86::k7, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86::mxcsr, isVector);

  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::st0, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::st7, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::fcw, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::fsw, isVector);

  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::flags, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::zf, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::zf.getBaseRegister(), isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::eax, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::ebx, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::ecx, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::edx, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::eip, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86::esp, isVector);

  /*********************************************************************
   *      Filtering
  *********************************************************************/
  {
    for(auto reg : Dyninst::MachRegister::getAllRegistersForArch(arch)) {
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
    case Dyninst::x86::imxcsr:
    case Dyninst::x86::ifcw:
    case Dyninst::x86::ifsw:
      return true;
  }
  return false;
}

static bool is_x87(Dyninst::MachRegister reg) {
  switch(reg) {
    case Dyninst::x86::ist0:
    case Dyninst::x86::ist1:
    case Dyninst::x86::ist2:
    case Dyninst::x86::ist3:
    case Dyninst::x86::ist4:
    case Dyninst::x86::ist5:
    case Dyninst::x86::ist6:
    case Dyninst::x86::ist7:
      return true;
  }
  return false;
}

static bool is_vec(Dyninst::MachRegister reg) {
  switch(reg) {
    case Dyninst::x86::imm0:
    case Dyninst::x86::imm1:
    case Dyninst::x86::imm2:
    case Dyninst::x86::imm3:
    case Dyninst::x86::imm4:
    case Dyninst::x86::imm5:
    case Dyninst::x86::imm6:
    case Dyninst::x86::imm7:
    case Dyninst::x86::ixmm0:
    case Dyninst::x86::ixmm1:
    case Dyninst::x86::ixmm2:
    case Dyninst::x86::ixmm3:
    case Dyninst::x86::ixmm4:
    case Dyninst::x86::ixmm5:
    case Dyninst::x86::ixmm6:
    case Dyninst::x86::ixmm7:
    case Dyninst::x86::iymm0:
    case Dyninst::x86::iymm1:
    case Dyninst::x86::iymm2:
    case Dyninst::x86::iymm3:
    case Dyninst::x86::iymm4:
    case Dyninst::x86::iymm5:
    case Dyninst::x86::iymm6:
    case Dyninst::x86::iymm7:
    case Dyninst::x86::izmm0:
    case Dyninst::x86::izmm1:
    case Dyninst::x86::izmm2:
    case Dyninst::x86::izmm3:
    case Dyninst::x86::izmm4:
    case Dyninst::x86::izmm5:
    case Dyninst::x86::izmm6:
    case Dyninst::x86::izmm7:
      return true;
  }
  return false;
}

