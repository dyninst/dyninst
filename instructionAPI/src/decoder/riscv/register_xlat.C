#include "decoder/riscv/register_xlat.h"
#include "registers/abstract_regs.h"
#include "registers/riscv64_regs.h"

namespace Dyninst {
namespace InstructionAPI {
namespace riscv {

// clang-format off
  MachRegister translate_register(riscv_reg cap_reg, cs_mode mode) {
    if(mode & CS_MODE_RISCV64) {
      switch(cap_reg) {
        case RISCV_REG_X0: return riscv64::x0;
        case RISCV_REG_X1: return riscv64::x1;
        case RISCV_REG_X2: return riscv64::x2;
        case RISCV_REG_X3: return riscv64::x3;
        case RISCV_REG_X4: return riscv64::x4;
        case RISCV_REG_X5: return riscv64::x5;
        case RISCV_REG_X6: return riscv64::x6;
        case RISCV_REG_X7: return riscv64::x7;
        case RISCV_REG_X8: return riscv64::x8;
        case RISCV_REG_X9: return riscv64::x9;
        case RISCV_REG_X10: return riscv64::x10;
        case RISCV_REG_X11: return riscv64::x11;
        case RISCV_REG_X12: return riscv64::x12;
        case RISCV_REG_X13: return riscv64::x13;
        case RISCV_REG_X14: return riscv64::x14;
        case RISCV_REG_X15: return riscv64::x15;
        case RISCV_REG_X16: return riscv64::x16;
        case RISCV_REG_X17: return riscv64::x17;
        case RISCV_REG_X18: return riscv64::x18;
        case RISCV_REG_X19: return riscv64::x19;
        case RISCV_REG_X20: return riscv64::x20;
        case RISCV_REG_X21: return riscv64::x21;
        case RISCV_REG_X22: return riscv64::x22;
        case RISCV_REG_X23: return riscv64::x23;
        case RISCV_REG_X24: return riscv64::x24;
        case RISCV_REG_X25: return riscv64::x25;
        case RISCV_REG_X26: return riscv64::x26;
        case RISCV_REG_X27: return riscv64::x27;
        case RISCV_REG_X28: return riscv64::x28;
        case RISCV_REG_X29: return riscv64::x29;
        case RISCV_REG_X30: return riscv64::x30;
        case RISCV_REG_X31: return riscv64::x31;
        case RISCV_REG_F0_32: return riscv64::f0_32;
        case RISCV_REG_F0_64: return riscv64::f0_64;
        case RISCV_REG_F1_32: return riscv64::f1_32;
        case RISCV_REG_F1_64: return riscv64::f1_64;
        case RISCV_REG_F2_32: return riscv64::f2_32;
        case RISCV_REG_F2_64: return riscv64::f2_64;
        case RISCV_REG_F3_32: return riscv64::f3_32;
        case RISCV_REG_F3_64: return riscv64::f3_64;
        case RISCV_REG_F4_32: return riscv64::f4_32;
        case RISCV_REG_F4_64: return riscv64::f4_64;
        case RISCV_REG_F5_32: return riscv64::f5_32;
        case RISCV_REG_F5_64: return riscv64::f5_64;
        case RISCV_REG_F6_32: return riscv64::f6_32;
        case RISCV_REG_F6_64: return riscv64::f6_64;
        case RISCV_REG_F7_32: return riscv64::f7_32;
        case RISCV_REG_F7_64: return riscv64::f7_64;
        case RISCV_REG_F8_32: return riscv64::f8_32;
        case RISCV_REG_F8_64: return riscv64::f8_64;
        case RISCV_REG_F9_32: return riscv64::f9_32;
        case RISCV_REG_F9_64: return riscv64::f9_64;
        case RISCV_REG_F10_32: return riscv64::f10_32;
        case RISCV_REG_F10_64: return riscv64::f10_64;
        case RISCV_REG_F11_32: return riscv64::f11_32;
        case RISCV_REG_F11_64: return riscv64::f11_64;
        case RISCV_REG_F12_32: return riscv64::f12_32;
        case RISCV_REG_F12_64: return riscv64::f12_64;
        case RISCV_REG_F13_32: return riscv64::f13_32;
        case RISCV_REG_F13_64: return riscv64::f13_64;
        case RISCV_REG_F14_32: return riscv64::f14_32;
        case RISCV_REG_F14_64: return riscv64::f14_64;
        case RISCV_REG_F15_32: return riscv64::f15_32;
        case RISCV_REG_F15_64: return riscv64::f15_64;
        case RISCV_REG_F16_32: return riscv64::f16_32;
        case RISCV_REG_F16_64: return riscv64::f16_64;
        case RISCV_REG_F17_32: return riscv64::f17_32;
        case RISCV_REG_F17_64: return riscv64::f17_64;
        case RISCV_REG_F18_32: return riscv64::f18_32;
        case RISCV_REG_F18_64: return riscv64::f18_64;
        case RISCV_REG_F19_32: return riscv64::f19_32;
        case RISCV_REG_F19_64: return riscv64::f19_64;
        case RISCV_REG_F20_32: return riscv64::f20_32;
        case RISCV_REG_F20_64: return riscv64::f20_64;
        case RISCV_REG_F21_32: return riscv64::f21_32;
        case RISCV_REG_F21_64: return riscv64::f21_64;
        case RISCV_REG_F22_32: return riscv64::f22_32;
        case RISCV_REG_F22_64: return riscv64::f22_64;
        case RISCV_REG_F23_32: return riscv64::f23_32;
        case RISCV_REG_F23_64: return riscv64::f23_64;
        case RISCV_REG_F24_32: return riscv64::f24_32;
        case RISCV_REG_F24_64: return riscv64::f24_64;
        case RISCV_REG_F25_32: return riscv64::f25_32;
        case RISCV_REG_F25_64: return riscv64::f25_64;
        case RISCV_REG_F26_32: return riscv64::f26_32;
        case RISCV_REG_F26_64: return riscv64::f26_64;
        case RISCV_REG_F27_32: return riscv64::f27_32;
        case RISCV_REG_F27_64: return riscv64::f27_64;
        case RISCV_REG_F28_32: return riscv64::f28_32;
        case RISCV_REG_F28_64: return riscv64::f28_64;
        case RISCV_REG_F29_32: return riscv64::f29_32;
        case RISCV_REG_F29_64: return riscv64::f29_64;
        case RISCV_REG_F30_32: return riscv64::f30_32;
        case RISCV_REG_F30_64: return riscv64::f30_64;
        case RISCV_REG_F31_32: return riscv64::f31_32;
        case RISCV_REG_F31_64: return riscv64::f31_64;
        default: return InvalidReg;
      }
    }
    return InvalidReg;
  }
  // clang-format off

}}}
