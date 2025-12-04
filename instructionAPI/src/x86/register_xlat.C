#include "registers/abstract_regs.h"
#include "registers/x86_regs.h"
#include "registers/x86_64_regs.h"
#include "x86/register_xlat.h"

namespace Dyninst { namespace InstructionAPI { namespace x86 {

  // clang-format off
  MachRegister translate_register(x86_reg cap_reg, cs_mode mode) {
    switch(mode) {
      case CS_MODE_32: {
        switch (cap_reg) {
          case X86_REG_AH: return Dyninst::x86::ah;
          case X86_REG_AL: return Dyninst::x86::al;
          case X86_REG_AX: return Dyninst::x86::ax;
          case X86_REG_BH: return Dyninst::x86::bh;
          case X86_REG_BL: return Dyninst::x86::bl;
          case X86_REG_BP: return Dyninst::x86::bp;
          case X86_REG_BX: return Dyninst::x86::bx;
          case X86_REG_CH: return Dyninst::x86::ch;
          case X86_REG_CL: return Dyninst::x86::cl;
          case X86_REG_CS: return Dyninst::x86::cs;
          case X86_REG_CX: return Dyninst::x86::cx;
          case X86_REG_DH: return Dyninst::x86::dh;
          case X86_REG_DI: return Dyninst::x86::di;
          case X86_REG_DL: return Dyninst::x86::dl;
          case X86_REG_DS: return Dyninst::x86::ds;
          case X86_REG_DX: return Dyninst::x86::dx;
          case X86_REG_EAX: return Dyninst::x86::eax;
          case X86_REG_EBP: return Dyninst::x86::ebp;
          case X86_REG_EBX: return Dyninst::x86::ebx;
          case X86_REG_ECX: return Dyninst::x86::ecx;
          case X86_REG_EDI: return Dyninst::x86::edi;
          case X86_REG_EDX: return Dyninst::x86::edx;
          case X86_REG_EFLAGS: return Dyninst::x86::flags;
          case X86_REG_EIP: return Dyninst::x86::eip;
          case X86_REG_ES: return Dyninst::x86::es;
          case X86_REG_ESI: return Dyninst::x86::esi;
          case X86_REG_ESP: return Dyninst::x86::esp;
          case X86_REG_FS: return Dyninst::x86::fs;
          case X86_REG_FPSW: return Dyninst::x86::fsw;
          case X86_REG_IP: return Dyninst::x86::eip;
          case X86_REG_GS: return Dyninst::x86::gs;
          case X86_REG_SI: return Dyninst::x86::si;
          case X86_REG_SP: return Dyninst::x86::sp;
          case X86_REG_SS: return Dyninst::x86::ss;
          case X86_REG_CR0: return Dyninst::x86::cr0;
          case X86_REG_CR1: return Dyninst::x86::cr1;
          case X86_REG_CR2: return Dyninst::x86::cr2;
          case X86_REG_CR3: return Dyninst::x86::cr3;
          case X86_REG_CR4: return Dyninst::x86::cr4;
          case X86_REG_CR5: return Dyninst::x86::cr5;
          case X86_REG_CR6: return Dyninst::x86::cr6;
          case X86_REG_CR7: return Dyninst::x86::cr7;
          case X86_REG_DR0: return Dyninst::x86::dr0;
          case X86_REG_DR1: return Dyninst::x86::dr1;
          case X86_REG_DR2: return Dyninst::x86::dr2;
          case X86_REG_DR3: return Dyninst::x86::dr3;
          case X86_REG_DR4: return Dyninst::x86::dr4;
          case X86_REG_DR5: return Dyninst::x86::dr5;
          case X86_REG_DR6: return Dyninst::x86::dr6;
          case X86_REG_DR7: return Dyninst::x86::dr7;
          case X86_REG_K0: return Dyninst::x86::k0;
          case X86_REG_K1: return Dyninst::x86::k1;
          case X86_REG_K2: return Dyninst::x86::k2;
          case X86_REG_K3: return Dyninst::x86::k3;
          case X86_REG_K4: return Dyninst::x86::k4;
          case X86_REG_K5: return Dyninst::x86::k5;
          case X86_REG_K6: return Dyninst::x86::k6;
          case X86_REG_K7: return Dyninst::x86::k7;
          case X86_REG_MM0: return Dyninst::x86::mm0;
          case X86_REG_MM1: return Dyninst::x86::mm1;
          case X86_REG_MM2: return Dyninst::x86::mm2;
          case X86_REG_MM3: return Dyninst::x86::mm3;
          case X86_REG_MM4: return Dyninst::x86::mm4;
          case X86_REG_MM5: return Dyninst::x86::mm5;
          case X86_REG_MM6: return Dyninst::x86::mm6;
          case X86_REG_MM7: return Dyninst::x86::mm7;
          case X86_REG_ST0: return Dyninst::x86::st0;
          case X86_REG_ST1: return Dyninst::x86::st1;
          case X86_REG_ST2: return Dyninst::x86::st2;
          case X86_REG_ST3: return Dyninst::x86::st3;
          case X86_REG_ST4: return Dyninst::x86::st4;
          case X86_REG_ST5: return Dyninst::x86::st5;
          case X86_REG_ST6: return Dyninst::x86::st6;
          case X86_REG_ST7: return Dyninst::x86::st7;
          case X86_REG_XMM0: return Dyninst::x86::xmm0;
          case X86_REG_XMM1: return Dyninst::x86::xmm1;
          case X86_REG_XMM2: return Dyninst::x86::xmm2;
          case X86_REG_XMM3: return Dyninst::x86::xmm3;
          case X86_REG_XMM4: return Dyninst::x86::xmm4;
          case X86_REG_XMM5: return Dyninst::x86::xmm5;
          case X86_REG_XMM6: return Dyninst::x86::xmm6;
          case X86_REG_XMM7: return Dyninst::x86::xmm7;
          case X86_REG_YMM0: return Dyninst::x86::ymm0;
          case X86_REG_YMM1: return Dyninst::x86::ymm1;
          case X86_REG_YMM2: return Dyninst::x86::ymm2;
          case X86_REG_YMM3: return Dyninst::x86::ymm3;
          case X86_REG_YMM4: return Dyninst::x86::ymm4;
          case X86_REG_YMM5: return Dyninst::x86::ymm5;
          case X86_REG_YMM6: return Dyninst::x86::ymm6;
          case X86_REG_YMM7: return Dyninst::x86::ymm7;
          case X86_REG_ZMM0: return Dyninst::x86::zmm0;
          case X86_REG_ZMM1: return Dyninst::x86::zmm1;
          case X86_REG_ZMM2: return Dyninst::x86::zmm2;
          case X86_REG_ZMM3: return Dyninst::x86::zmm3;
          case X86_REG_ZMM4: return Dyninst::x86::zmm4;
          case X86_REG_ZMM5: return Dyninst::x86::zmm5;
          case X86_REG_ZMM6: return Dyninst::x86::zmm6;
          case X86_REG_ZMM7: return Dyninst::x86::zmm7;
          default: return Dyninst::InvalidReg;
        }
      } break;
      case CS_MODE_64: {
        switch(cap_reg) {
          case X86_REG_AH: return Dyninst::x86_64::ah;
          case X86_REG_AL: return Dyninst::x86_64::al;
          case X86_REG_AX: return Dyninst::x86_64::ax;
          case X86_REG_BH: return Dyninst::x86_64::bh;
          case X86_REG_BL: return Dyninst::x86_64::bl;
          case X86_REG_BP: return Dyninst::x86_64::bp;
          case X86_REG_BPL: return Dyninst::x86_64::bpl;
          case X86_REG_BX: return Dyninst::x86_64::bx;
          case X86_REG_CH: return Dyninst::x86_64::ch;
          case X86_REG_CL: return Dyninst::x86_64::cl;
          case X86_REG_CS: return Dyninst::x86_64::cs;
          case X86_REG_CX: return Dyninst::x86_64::cx;
          case X86_REG_DH: return Dyninst::x86_64::dh;
          case X86_REG_DI: return Dyninst::x86_64::di;
          case X86_REG_DIL: return Dyninst::x86_64::dil;
          case X86_REG_DL: return Dyninst::x86_64::dl;
          case X86_REG_DS: return Dyninst::x86_64::ds;
          case X86_REG_DX: return Dyninst::x86_64::dx;
          case X86_REG_EAX: return Dyninst::x86_64::eax;
          case X86_REG_EBP: return Dyninst::x86_64::ebp;
          case X86_REG_EBX: return Dyninst::x86_64::ebx;
          case X86_REG_ECX: return Dyninst::x86_64::ecx;
          case X86_REG_EDI: return Dyninst::x86_64::edi;
          case X86_REG_EDX: return Dyninst::x86_64::edx;
          case X86_REG_EFLAGS: return Dyninst::x86_64::flags;
          case X86_REG_EIP: return Dyninst::x86_64::eip;
          case X86_REG_ES: return Dyninst::x86_64::es;
          case X86_REG_ESI: return Dyninst::x86_64::esi;
          case X86_REG_ESP: return Dyninst::x86_64::esp;
          case X86_REG_FPSW: return Dyninst::x86_64::fsw;
          case X86_REG_FS: return Dyninst::x86_64::fs;
          case X86_REG_GS: return Dyninst::x86_64::gs;
          case X86_REG_IP: return Dyninst::x86_64::rip;
          case X86_REG_RAX: return Dyninst::x86_64::rax;
          case X86_REG_RBP: return Dyninst::x86_64::rbp;
          case X86_REG_RBX: return Dyninst::x86_64::rbx;
          case X86_REG_RCX: return Dyninst::x86_64::rcx;
          case X86_REG_RDI: return Dyninst::x86_64::rdi;
          case X86_REG_RDX: return Dyninst::x86_64::rdx;
          case X86_REG_RIP: return Dyninst::x86_64::rip;
          case X86_REG_RSI: return Dyninst::x86_64::rsi;
          case X86_REG_RSP: return Dyninst::x86_64::rsp;
          case X86_REG_SI: return Dyninst::x86_64::si;
          case X86_REG_SIL: return Dyninst::x86_64::sil;
          case X86_REG_SP: return Dyninst::x86_64::sp;
          case X86_REG_SPL: return Dyninst::x86_64::spl;
          case X86_REG_SS: return Dyninst::x86_64::ss;
          case X86_REG_CR0: return Dyninst::x86_64::cr0;
          case X86_REG_CR1: return Dyninst::x86_64::cr1;
          case X86_REG_CR2: return Dyninst::x86_64::cr2;
          case X86_REG_CR3: return Dyninst::x86_64::cr3;
          case X86_REG_CR4: return Dyninst::x86_64::cr4;
          case X86_REG_CR5: return Dyninst::x86_64::cr5;
          case X86_REG_CR6: return Dyninst::x86_64::cr6;
          case X86_REG_CR7: return Dyninst::x86_64::cr7;
          case X86_REG_CR8: return Dyninst::x86_64::cr8;
          case X86_REG_CR9: return Dyninst::x86_64::cr9;
          case X86_REG_CR10: return Dyninst::x86_64::cr10;
          case X86_REG_CR11: return Dyninst::x86_64::cr11;
          case X86_REG_CR12: return Dyninst::x86_64::cr12;
          case X86_REG_CR13: return Dyninst::x86_64::cr13;
          case X86_REG_CR14: return Dyninst::x86_64::cr14;
          case X86_REG_CR15: return Dyninst::x86_64::cr15;
          case X86_REG_DR0: return Dyninst::x86_64::dr0;
          case X86_REG_DR1: return Dyninst::x86_64::dr1;
          case X86_REG_DR2: return Dyninst::x86_64::dr2;
          case X86_REG_DR3: return Dyninst::x86_64::dr3;
          case X86_REG_DR4: return Dyninst::x86_64::dr4;
          case X86_REG_DR5: return Dyninst::x86_64::dr5;
          case X86_REG_DR6: return Dyninst::x86_64::dr6;
          case X86_REG_DR7: return Dyninst::x86_64::dr7;
          case X86_REG_DR8: return Dyninst::x86_64::dr8;
          case X86_REG_DR9: return Dyninst::x86_64::dr9;
          case X86_REG_DR10: return Dyninst::x86_64::dr10;
          case X86_REG_DR11: return Dyninst::x86_64::dr11;
          case X86_REG_DR12: return Dyninst::x86_64::dr12;
          case X86_REG_DR13: return Dyninst::x86_64::dr13;
          case X86_REG_DR14: return Dyninst::x86_64::dr14;
          case X86_REG_DR15: return Dyninst::x86_64::dr15;
          case X86_REG_K0: return Dyninst::x86_64::k0;
          case X86_REG_K1: return Dyninst::x86_64::k1;
          case X86_REG_K2: return Dyninst::x86_64::k2;
          case X86_REG_K3: return Dyninst::x86_64::k3;
          case X86_REG_K4: return Dyninst::x86_64::k4;
          case X86_REG_K5: return Dyninst::x86_64::k5;
          case X86_REG_K6: return Dyninst::x86_64::k6;
          case X86_REG_K7: return Dyninst::x86_64::k7;
          case X86_REG_MM0: return Dyninst::x86_64::mm0;
          case X86_REG_MM1: return Dyninst::x86_64::mm1;
          case X86_REG_MM2: return Dyninst::x86_64::mm2;
          case X86_REG_MM3: return Dyninst::x86_64::mm3;
          case X86_REG_MM4: return Dyninst::x86_64::mm4;
          case X86_REG_MM5: return Dyninst::x86_64::mm5;
          case X86_REG_MM6: return Dyninst::x86_64::mm6;
          case X86_REG_MM7: return Dyninst::x86_64::mm7;
          case X86_REG_R8: return Dyninst::x86_64::r8;
          case X86_REG_R9: return Dyninst::x86_64::r9;
          case X86_REG_R10: return Dyninst::x86_64::r10;
          case X86_REG_R11: return Dyninst::x86_64::r11;
          case X86_REG_R12: return Dyninst::x86_64::r12;
          case X86_REG_R13: return Dyninst::x86_64::r13;
          case X86_REG_R14: return Dyninst::x86_64::r14;
          case X86_REG_R15: return Dyninst::x86_64::r15;
          case X86_REG_ST0: return Dyninst::x86_64::st0;
          case X86_REG_ST1: return Dyninst::x86_64::st1;
          case X86_REG_ST2: return Dyninst::x86_64::st2;
          case X86_REG_ST3: return Dyninst::x86_64::st3;
          case X86_REG_ST4: return Dyninst::x86_64::st4;
          case X86_REG_ST5: return Dyninst::x86_64::st5;
          case X86_REG_ST6: return Dyninst::x86_64::st6;
          case X86_REG_ST7: return Dyninst::x86_64::st7;
          case X86_REG_XMM0: return Dyninst::x86_64::xmm0;
          case X86_REG_XMM1: return Dyninst::x86_64::xmm1;
          case X86_REG_XMM2: return Dyninst::x86_64::xmm2;
          case X86_REG_XMM3: return Dyninst::x86_64::xmm3;
          case X86_REG_XMM4: return Dyninst::x86_64::xmm4;
          case X86_REG_XMM5: return Dyninst::x86_64::xmm5;
          case X86_REG_XMM6: return Dyninst::x86_64::xmm6;
          case X86_REG_XMM7: return Dyninst::x86_64::xmm7;
          case X86_REG_XMM8: return Dyninst::x86_64::xmm8;
          case X86_REG_XMM9: return Dyninst::x86_64::xmm9;
          case X86_REG_XMM10: return Dyninst::x86_64::xmm10;
          case X86_REG_XMM11: return Dyninst::x86_64::xmm11;
          case X86_REG_XMM12: return Dyninst::x86_64::xmm12;
          case X86_REG_XMM13: return Dyninst::x86_64::xmm13;
          case X86_REG_XMM14: return Dyninst::x86_64::xmm14;
          case X86_REG_XMM15: return Dyninst::x86_64::xmm15;
          case X86_REG_XMM16: return Dyninst::x86_64::xmm16;
          case X86_REG_XMM17: return Dyninst::x86_64::xmm17;
          case X86_REG_XMM18: return Dyninst::x86_64::xmm18;
          case X86_REG_XMM19: return Dyninst::x86_64::xmm19;
          case X86_REG_XMM20: return Dyninst::x86_64::xmm20;
          case X86_REG_XMM21: return Dyninst::x86_64::xmm21;
          case X86_REG_XMM22: return Dyninst::x86_64::xmm22;
          case X86_REG_XMM23: return Dyninst::x86_64::xmm23;
          case X86_REG_XMM24: return Dyninst::x86_64::xmm24;
          case X86_REG_XMM25: return Dyninst::x86_64::xmm25;
          case X86_REG_XMM26: return Dyninst::x86_64::xmm26;
          case X86_REG_XMM27: return Dyninst::x86_64::xmm27;
          case X86_REG_XMM28: return Dyninst::x86_64::xmm28;
          case X86_REG_XMM29: return Dyninst::x86_64::xmm29;
          case X86_REG_XMM30: return Dyninst::x86_64::xmm30;
          case X86_REG_XMM31: return Dyninst::x86_64::xmm31;
          case X86_REG_YMM0: return Dyninst::x86_64::ymm0;
          case X86_REG_YMM1: return Dyninst::x86_64::ymm1;
          case X86_REG_YMM2: return Dyninst::x86_64::ymm2;
          case X86_REG_YMM3: return Dyninst::x86_64::ymm3;
          case X86_REG_YMM4: return Dyninst::x86_64::ymm4;
          case X86_REG_YMM5: return Dyninst::x86_64::ymm5;
          case X86_REG_YMM6: return Dyninst::x86_64::ymm6;
          case X86_REG_YMM7: return Dyninst::x86_64::ymm7;
          case X86_REG_YMM8: return Dyninst::x86_64::ymm8;
          case X86_REG_YMM9: return Dyninst::x86_64::ymm9;
          case X86_REG_YMM10: return Dyninst::x86_64::ymm10;
          case X86_REG_YMM11: return Dyninst::x86_64::ymm11;
          case X86_REG_YMM12: return Dyninst::x86_64::ymm12;
          case X86_REG_YMM13: return Dyninst::x86_64::ymm13;
          case X86_REG_YMM14: return Dyninst::x86_64::ymm14;
          case X86_REG_YMM15: return Dyninst::x86_64::ymm15;
          case X86_REG_YMM16: return Dyninst::x86_64::ymm16;
          case X86_REG_YMM17: return Dyninst::x86_64::ymm17;
          case X86_REG_YMM18: return Dyninst::x86_64::ymm18;
          case X86_REG_YMM19: return Dyninst::x86_64::ymm19;
          case X86_REG_YMM20: return Dyninst::x86_64::ymm20;
          case X86_REG_YMM21: return Dyninst::x86_64::ymm21;
          case X86_REG_YMM22: return Dyninst::x86_64::ymm22;
          case X86_REG_YMM23: return Dyninst::x86_64::ymm23;
          case X86_REG_YMM24: return Dyninst::x86_64::ymm24;
          case X86_REG_YMM25: return Dyninst::x86_64::ymm25;
          case X86_REG_YMM26: return Dyninst::x86_64::ymm26;
          case X86_REG_YMM27: return Dyninst::x86_64::ymm27;
          case X86_REG_YMM28: return Dyninst::x86_64::ymm28;
          case X86_REG_YMM29: return Dyninst::x86_64::ymm29;
          case X86_REG_YMM30: return Dyninst::x86_64::ymm30;
          case X86_REG_YMM31: return Dyninst::x86_64::ymm31;
          case X86_REG_ZMM0: return Dyninst::x86_64::zmm0;
          case X86_REG_ZMM1: return Dyninst::x86_64::zmm1;
          case X86_REG_ZMM2: return Dyninst::x86_64::zmm2;
          case X86_REG_ZMM3: return Dyninst::x86_64::zmm3;
          case X86_REG_ZMM4: return Dyninst::x86_64::zmm4;
          case X86_REG_ZMM5: return Dyninst::x86_64::zmm5;
          case X86_REG_ZMM6: return Dyninst::x86_64::zmm6;
          case X86_REG_ZMM7: return Dyninst::x86_64::zmm7;
          case X86_REG_ZMM8: return Dyninst::x86_64::zmm8;
          case X86_REG_ZMM9: return Dyninst::x86_64::zmm9;
          case X86_REG_ZMM10: return Dyninst::x86_64::zmm10;
          case X86_REG_ZMM11: return Dyninst::x86_64::zmm11;
          case X86_REG_ZMM12: return Dyninst::x86_64::zmm12;
          case X86_REG_ZMM13: return Dyninst::x86_64::zmm13;
          case X86_REG_ZMM14: return Dyninst::x86_64::zmm14;
          case X86_REG_ZMM15: return Dyninst::x86_64::zmm15;
          case X86_REG_ZMM16: return Dyninst::x86_64::zmm16;
          case X86_REG_ZMM17: return Dyninst::x86_64::zmm17;
          case X86_REG_ZMM18: return Dyninst::x86_64::zmm18;
          case X86_REG_ZMM19: return Dyninst::x86_64::zmm19;
          case X86_REG_ZMM20: return Dyninst::x86_64::zmm20;
          case X86_REG_ZMM21: return Dyninst::x86_64::zmm21;
          case X86_REG_ZMM22: return Dyninst::x86_64::zmm22;
          case X86_REG_ZMM23: return Dyninst::x86_64::zmm23;
          case X86_REG_ZMM24: return Dyninst::x86_64::zmm24;
          case X86_REG_ZMM25: return Dyninst::x86_64::zmm25;
          case X86_REG_ZMM26: return Dyninst::x86_64::zmm26;
          case X86_REG_ZMM27: return Dyninst::x86_64::zmm27;
          case X86_REG_ZMM28: return Dyninst::x86_64::zmm28;
          case X86_REG_ZMM29: return Dyninst::x86_64::zmm29;
          case X86_REG_ZMM30: return Dyninst::x86_64::zmm30;
          case X86_REG_ZMM31: return Dyninst::x86_64::zmm31;
          case X86_REG_R8B: return Dyninst::x86_64::r8b;
          case X86_REG_R9B: return Dyninst::x86_64::r9b;
          case X86_REG_R10B: return Dyninst::x86_64::r10b;
          case X86_REG_R11B: return Dyninst::x86_64::r11b;
          case X86_REG_R12B: return Dyninst::x86_64::r12b;
          case X86_REG_R13B: return Dyninst::x86_64::r13b;
          case X86_REG_R14B: return Dyninst::x86_64::r14b;
          case X86_REG_R15B: return Dyninst::x86_64::r15b;
          case X86_REG_R8D: return Dyninst::x86_64::r8d;
          case X86_REG_R9D: return Dyninst::x86_64::r9d;
          case X86_REG_R10D: return Dyninst::x86_64::r10d;
          case X86_REG_R11D: return Dyninst::x86_64::r11d;
          case X86_REG_R12D: return Dyninst::x86_64::r12d;
          case X86_REG_R13D: return Dyninst::x86_64::r13d;
          case X86_REG_R14D: return Dyninst::x86_64::r14d;
          case X86_REG_R15D: return Dyninst::x86_64::r15d;
          case X86_REG_R8W: return Dyninst::x86_64::r8w;
          case X86_REG_R9W: return Dyninst::x86_64::r9w;
          case X86_REG_R10W: return Dyninst::x86_64::r10w;
          case X86_REG_R11W: return Dyninst::x86_64::r11w;
          case X86_REG_R12W: return Dyninst::x86_64::r12w;
          case X86_REG_R13W: return Dyninst::x86_64::r13w;
          case X86_REG_R14W: return Dyninst::x86_64::r14w;
          case X86_REG_R15W: return Dyninst::x86_64::r15w;

          /* pseudo registers used by LLVM */
          case X86_REG_EIZ:
          case X86_REG_RIZ:
          case X86_REG_FP0:
          case X86_REG_FP1:
          case X86_REG_FP2:
          case X86_REG_FP3:
          case X86_REG_FP4:
          case X86_REG_FP5:
          case X86_REG_FP6:
          case X86_REG_FP7:

          /* AMX registers */
          case X86_REG_BND0:
          case X86_REG_BND1:
          case X86_REG_BND2:
          case X86_REG_BND3:
          default: return Dyninst::InvalidReg;
        }
      } break;

      /* Invalid mode */
      default: return Dyninst::InvalidReg;
    }
  }
  // clang-format off

}}}
