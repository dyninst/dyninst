#include "registers/MachRegister.h"
#include "registers/x86_64_regs.h"
#include "type_query_check.h"

static bool is_vec(Dyninst::MachRegister);
static bool is_x87(Dyninst::MachRegister);
static bool is_fpctl(Dyninst::MachRegister);

namespace dreg = Dyninst::x86_64;
constexpr auto arch = Dyninst::Arch_x86;
using mreg = Dyninst::MachRegister;

int main() {
  TYPE_QUERIES_CHECK(dreg::eip, isPC);
  TYPE_QUERIES_CHECK(dreg::rip, isPC);
  TYPE_QUERIES_CHECK(mreg::getPC(arch), isPC);

  TYPE_QUERIES_CHECK(dreg::bp, isFramePointer);
  TYPE_QUERIES_CHECK(dreg::ebp, isFramePointer);
  TYPE_QUERIES_CHECK(dreg::rbp, isFramePointer);
  TYPE_QUERIES_CHECK(mreg::getFramePointer(arch), isFramePointer);

  TYPE_QUERIES_CHECK(dreg::sp, isStackPointer);
  TYPE_QUERIES_CHECK(dreg::esp, isStackPointer);
  TYPE_QUERIES_CHECK(dreg::rsp, isStackPointer);
  TYPE_QUERIES_CHECK(mreg::getStackPointer(arch), isStackPointer);

  TYPE_QUERIES_CHECK(dreg::al, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(dreg::ah, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(dreg::ax, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(dreg::eax, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(dreg::rax, isSyscallNumberReg);
  TYPE_QUERIES_CHECK(mreg::getSyscallNumberReg(arch), isSyscallNumberReg);

  TYPE_QUERIES_CHECK(dreg::al, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(dreg::ah, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(dreg::ax, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(dreg::eax, isSyscallReturnValueReg);
  TYPE_QUERIES_CHECK(dreg::rax, isSyscallReturnValueReg);

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
   *      General Purpose
   *********************************************************************/
  TYPE_QUERIES_CHECK(dreg::al, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::ah, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::ax, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::eax, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::rax, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::bl, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::bh, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::bx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::ebx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::rbx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::cl, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::ch, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::cx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::ecx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::rcx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::dl, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::dh, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::dx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::edx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::rdx, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::si, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::esi, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::rsi, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::sil, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::di, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::edi, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::rdi, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::dil, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r8, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r8b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r8w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r8d, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r9, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r9b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r9w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r9d, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r10, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r10b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r10w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r10d, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r11, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r11b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r11w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r11d, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r12, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r12b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r12w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r12d, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r13, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r13b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r13w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r13d, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r14, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r14b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r14w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r14d, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r15, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r15b, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r15w, isGeneralPurpose);
  TYPE_QUERIES_CHECK(dreg::r15d, isGeneralPurpose);

  TYPE_QUERIES_CHECK_FALSE(dreg::mm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::mm7, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::xmm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::xmm31, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::ymm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::ymm31, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::zmm0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::zmm31, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::k0, isGeneralPurpose);
  TYPE_QUERIES_CHECK_FALSE(dreg::k7, isGeneralPurpose);

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
  TYPE_QUERIES_CHECK_FALSE(dreg::xmm31, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::ymm0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::ymm31, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::zmm0, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::zmm31, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::rax, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::rbx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::rcx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::rdx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::rip, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::rsp, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::rax, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::rbx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::rcx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::rdx, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::rip, isControlStatus);
  TYPE_QUERIES_CHECK_FALSE(dreg::rsp, isControlStatus);

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
  TYPE_QUERIES_CHECK(dreg::xmm31, isFloatingPoint);
  TYPE_QUERIES_CHECK(dreg::ymm0, isFloatingPoint);
  TYPE_QUERIES_CHECK(dreg::ymm31, isFloatingPoint);
  TYPE_QUERIES_CHECK(dreg::zmm0, isFloatingPoint);
  TYPE_QUERIES_CHECK(dreg::zmm31, isFloatingPoint);

  TYPE_QUERIES_CHECK_FALSE(dreg::k0, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(dreg::k7, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(dreg::rax, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(dreg::rbx, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(dreg::rcx, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(dreg::rdx, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(dreg::rip, isFloatingPoint);
  TYPE_QUERIES_CHECK_FALSE(dreg::rsp, isFloatingPoint);

  /*********************************************************************
   *      Vector
  *********************************************************************/
  TYPE_QUERIES_CHECK(dreg::xmm0, isVector);
  TYPE_QUERIES_CHECK(dreg::xmm31, isVector);
  TYPE_QUERIES_CHECK(dreg::ymm0, isVector);
  TYPE_QUERIES_CHECK(dreg::ymm31, isVector);
  TYPE_QUERIES_CHECK(dreg::zmm0, isVector);
  TYPE_QUERIES_CHECK(dreg::zmm31, isVector);
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
  TYPE_QUERIES_CHECK_FALSE(dreg::rax, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::rbx, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::rcx, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::rdx, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::rip, isVector);
  TYPE_QUERIES_CHECK_FALSE(dreg::rsp, isVector);

  /*********************************************************************
   *      Filtering
  *********************************************************************/
  {
    for(auto reg : mreg::getAllRegistersForArch(Dyninst::Arch_x86_64)) {
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
    case dreg::ixmm8:
    case dreg::ixmm9:
    case dreg::ixmm10:
    case dreg::ixmm11:
    case dreg::ixmm12:
    case dreg::ixmm13:
    case dreg::ixmm14:
    case dreg::ixmm15:
    case dreg::ixmm16:
    case dreg::ixmm17:
    case dreg::ixmm18:
    case dreg::ixmm19:
    case dreg::ixmm20:
    case dreg::ixmm21:
    case dreg::ixmm22:
    case dreg::ixmm23:
    case dreg::ixmm24:
    case dreg::ixmm25:
    case dreg::ixmm26:
    case dreg::ixmm27:
    case dreg::ixmm28:
    case dreg::ixmm29:
    case dreg::ixmm30:
    case dreg::ixmm31:
    case dreg::iymm0:
    case dreg::iymm1:
    case dreg::iymm2:
    case dreg::iymm3:
    case dreg::iymm4:
    case dreg::iymm5:
    case dreg::iymm6:
    case dreg::iymm7:
    case dreg::iymm8:
    case dreg::iymm9:
    case dreg::iymm10:
    case dreg::iymm11:
    case dreg::iymm12:
    case dreg::iymm13:
    case dreg::iymm14:
    case dreg::iymm15:
    case dreg::iymm16:
    case dreg::iymm17:
    case dreg::iymm18:
    case dreg::iymm19:
    case dreg::iymm20:
    case dreg::iymm21:
    case dreg::iymm22:
    case dreg::iymm23:
    case dreg::iymm24:
    case dreg::iymm25:
    case dreg::iymm26:
    case dreg::iymm27:
    case dreg::iymm28:
    case dreg::iymm29:
    case dreg::iymm30:
    case dreg::iymm31:
    case dreg::izmm0:
    case dreg::izmm1:
    case dreg::izmm2:
    case dreg::izmm3:
    case dreg::izmm4:
    case dreg::izmm5:
    case dreg::izmm6:
    case dreg::izmm7:
    case dreg::izmm8:
    case dreg::izmm9:
    case dreg::izmm10:
    case dreg::izmm11:
    case dreg::izmm12:
    case dreg::izmm13:
    case dreg::izmm14:
    case dreg::izmm15:
    case dreg::izmm16:
    case dreg::izmm17:
    case dreg::izmm18:
    case dreg::izmm19:
    case dreg::izmm20:
    case dreg::izmm21:
    case dreg::izmm22:
    case dreg::izmm23:
    case dreg::izmm24:
    case dreg::izmm25:
    case dreg::izmm26:
    case dreg::izmm27:
    case dreg::izmm28:
    case dreg::izmm29:
    case dreg::izmm30:
    case dreg::izmm31:
      return true;
  }
  return false;
}

