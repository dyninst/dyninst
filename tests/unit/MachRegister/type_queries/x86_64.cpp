#include "registers/MachRegister.h"
#include "registers/x86_64_regs.h"
#include "type_query_check.h"

constexpr auto arch = Dyninst::Arch_x86;

static bool is_vec(Dyninst::MachRegister);
static bool is_x87(Dyninst::MachRegister);
static bool is_fpctl(Dyninst::MachRegister);

int main() {
  TYPE_QUERIES_CHECK(Dyninst::x86_64::eip, isPC);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::rip, isPC);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getPC(arch), isPC);

  TYPE_QUERIES_CHECK(Dyninst::x86_64::bp, isFramePointer);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ebp, isFramePointer);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::rbp, isFramePointer);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getFramePointer(arch), isFramePointer);

  TYPE_QUERIES_CHECK(Dyninst::x86_64::sp, isStackPointer);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::esp, isStackPointer);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::rsp, isStackPointer);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getStackPointer(arch), isStackPointer);

  TYPE_QUERIES_CHECK(Dyninst::x86_64::al, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ah, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ax, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::eax, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::rax, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getSyscallNumberReg(arch), isSyscallNumberReg);

  TYPE_QUERIES_CHECK(Dyninst::x86_64::al, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ah, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ax, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::eax, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::rax, isSyscallReturnValueReg);

  /*********************************************************************
   *      Flags
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::x86_64::cf, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::flag1, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::pf, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::flag3, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::af, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::flag5, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::zf, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::sf, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::tf, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::if_, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::df, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::of, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::flagc, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::flagd, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::nt_, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::flagf, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::rf, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::vm, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ac, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::vif, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::vip, isFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::id, isFlag);

  /*********************************************************************
   *      Zero Flag
  *********************************************************************/
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::flags, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::zf, isZeroFlag);
  TYPE_QUERIES_CHECK(Dyninst::MachRegister::getZeroFlag(arch), isZeroFlag);


  /*********************************************************************
   *      General Purpose
   *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::x86_64::al, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ah, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ax, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::eax, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::rax, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::bl, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::bh, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::bx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ebx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::rbx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::cl, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ch, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::cx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ecx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::rcx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::dl, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::dh, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::dx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::edx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::rdx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::si, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::esi, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::rsi, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::sil, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::di, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::edi, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::rdi, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::dil, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r8, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r8b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r8w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r8d, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r9, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r9b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r9w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r9d, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r10, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r10b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r10w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r10d, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r11, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r11b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r11w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r11d, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r12, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r12b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r12w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r12d, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r13, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r13b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r13w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r13d, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r14, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r14b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r14w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r14d, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r15, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r15b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r15w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::r15d, isGeneralPurpose);

  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::mm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::mm7, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::xmm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::xmm31, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::ymm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::ymm31, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::zmm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::zmm31, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::k0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::k7, isGeneralPurpose);

  /*********************************************************************
   *      Control/Status
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::x86_64::fcw, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::fsw, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::mxcsr, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::cr0, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::cr7, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::k0, isControlStatus);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::k7, isControlStatus);

  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::flags, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::zf, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::zf.getBaseRegister(), isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::mm0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::mm7, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::xmm0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::xmm31, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::ymm0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::ymm31, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::zmm0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::zmm31, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rax, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rbx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rcx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rdx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rip, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rsp, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rax, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rbx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rcx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rdx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rip, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rsp, isControlStatus);

  /*********************************************************************
   *      Floating-Point
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::x86_64::st0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::st7, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::mm0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::mm7, isFloatingPoint);

  TYPE_QUERIES_CHECK(Dyninst::x86_64::fcw, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::fsw, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::mxcsr, isFloatingPoint);

  TYPE_QUERIES_CHECK(Dyninst::x86_64::xmm0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::xmm31, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ymm0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ymm31, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::zmm0, isFloatingPoint);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::zmm31, isFloatingPoint);

  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::k0, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::k7, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rax, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rbx, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rcx, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rdx, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rip, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rsp, isFloatingPoint);

  /*********************************************************************
   *      Vector
  *********************************************************************/
  TYPE_QUERIES_CHECK(Dyninst::x86_64::xmm0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::xmm31, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ymm0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::ymm31, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::zmm0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::zmm31, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::mm0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::mm7, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::k0, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::k7, isVector);
  TYPE_QUERIES_CHECK(Dyninst::x86_64::mxcsr, isVector);

  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::st0, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::st7, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::fcw, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::fsw, isVector);

  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::flags, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::zf, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::zf.getBaseRegister(), isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rax, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rbx, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rcx, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rdx, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rip, isVector);
  TYPE_QUERIES_CHECK_FALSE(Dyninst::x86_64::rsp, isVector);

  /*********************************************************************
   *      Filtering
  *********************************************************************/
  {
    for(auto reg : Dyninst::MachRegister::getAllRegistersForArch(Dyninst::Arch_x86_64)) {
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
    case Dyninst::x86_64::imxcsr:
    case Dyninst::x86_64::ifcw:
    case Dyninst::x86_64::ifsw:
      return true;
  }
  return false;
}

static bool is_x87(Dyninst::MachRegister reg) {
  switch(reg) {
    case Dyninst::x86_64::ist0:
    case Dyninst::x86_64::ist1:
    case Dyninst::x86_64::ist2:
    case Dyninst::x86_64::ist3:
    case Dyninst::x86_64::ist4:
    case Dyninst::x86_64::ist5:
    case Dyninst::x86_64::ist6:
    case Dyninst::x86_64::ist7:
      return true;
  }
  return false;
}

static bool is_vec(Dyninst::MachRegister reg) {
  switch(reg) {
    case Dyninst::x86_64::imm0:
    case Dyninst::x86_64::imm1:
    case Dyninst::x86_64::imm2:
    case Dyninst::x86_64::imm3:
    case Dyninst::x86_64::imm4:
    case Dyninst::x86_64::imm5:
    case Dyninst::x86_64::imm6:
    case Dyninst::x86_64::imm7:
    case Dyninst::x86_64::ixmm0:
    case Dyninst::x86_64::ixmm1:
    case Dyninst::x86_64::ixmm2:
    case Dyninst::x86_64::ixmm3:
    case Dyninst::x86_64::ixmm4:
    case Dyninst::x86_64::ixmm5:
    case Dyninst::x86_64::ixmm6:
    case Dyninst::x86_64::ixmm7:
    case Dyninst::x86_64::ixmm8:
    case Dyninst::x86_64::ixmm9:
    case Dyninst::x86_64::ixmm10:
    case Dyninst::x86_64::ixmm11:
    case Dyninst::x86_64::ixmm12:
    case Dyninst::x86_64::ixmm13:
    case Dyninst::x86_64::ixmm14:
    case Dyninst::x86_64::ixmm15:
    case Dyninst::x86_64::ixmm16:
    case Dyninst::x86_64::ixmm17:
    case Dyninst::x86_64::ixmm18:
    case Dyninst::x86_64::ixmm19:
    case Dyninst::x86_64::ixmm20:
    case Dyninst::x86_64::ixmm21:
    case Dyninst::x86_64::ixmm22:
    case Dyninst::x86_64::ixmm23:
    case Dyninst::x86_64::ixmm24:
    case Dyninst::x86_64::ixmm25:
    case Dyninst::x86_64::ixmm26:
    case Dyninst::x86_64::ixmm27:
    case Dyninst::x86_64::ixmm28:
    case Dyninst::x86_64::ixmm29:
    case Dyninst::x86_64::ixmm30:
    case Dyninst::x86_64::ixmm31:
    case Dyninst::x86_64::iymm0:
    case Dyninst::x86_64::iymm1:
    case Dyninst::x86_64::iymm2:
    case Dyninst::x86_64::iymm3:
    case Dyninst::x86_64::iymm4:
    case Dyninst::x86_64::iymm5:
    case Dyninst::x86_64::iymm6:
    case Dyninst::x86_64::iymm7:
    case Dyninst::x86_64::iymm8:
    case Dyninst::x86_64::iymm9:
    case Dyninst::x86_64::iymm10:
    case Dyninst::x86_64::iymm11:
    case Dyninst::x86_64::iymm12:
    case Dyninst::x86_64::iymm13:
    case Dyninst::x86_64::iymm14:
    case Dyninst::x86_64::iymm15:
    case Dyninst::x86_64::iymm16:
    case Dyninst::x86_64::iymm17:
    case Dyninst::x86_64::iymm18:
    case Dyninst::x86_64::iymm19:
    case Dyninst::x86_64::iymm20:
    case Dyninst::x86_64::iymm21:
    case Dyninst::x86_64::iymm22:
    case Dyninst::x86_64::iymm23:
    case Dyninst::x86_64::iymm24:
    case Dyninst::x86_64::iymm25:
    case Dyninst::x86_64::iymm26:
    case Dyninst::x86_64::iymm27:
    case Dyninst::x86_64::iymm28:
    case Dyninst::x86_64::iymm29:
    case Dyninst::x86_64::iymm30:
    case Dyninst::x86_64::iymm31:
    case Dyninst::x86_64::izmm0:
    case Dyninst::x86_64::izmm1:
    case Dyninst::x86_64::izmm2:
    case Dyninst::x86_64::izmm3:
    case Dyninst::x86_64::izmm4:
    case Dyninst::x86_64::izmm5:
    case Dyninst::x86_64::izmm6:
    case Dyninst::x86_64::izmm7:
    case Dyninst::x86_64::izmm8:
    case Dyninst::x86_64::izmm9:
    case Dyninst::x86_64::izmm10:
    case Dyninst::x86_64::izmm11:
    case Dyninst::x86_64::izmm12:
    case Dyninst::x86_64::izmm13:
    case Dyninst::x86_64::izmm14:
    case Dyninst::x86_64::izmm15:
    case Dyninst::x86_64::izmm16:
    case Dyninst::x86_64::izmm17:
    case Dyninst::x86_64::izmm18:
    case Dyninst::x86_64::izmm19:
    case Dyninst::x86_64::izmm20:
    case Dyninst::x86_64::izmm21:
    case Dyninst::x86_64::izmm22:
    case Dyninst::x86_64::izmm23:
    case Dyninst::x86_64::izmm24:
    case Dyninst::x86_64::izmm25:
    case Dyninst::x86_64::izmm26:
    case Dyninst::x86_64::izmm27:
    case Dyninst::x86_64::izmm28:
    case Dyninst::x86_64::izmm29:
    case Dyninst::x86_64::izmm30:
    case Dyninst::x86_64::izmm31:
      return true;
  }
  return false;
}

