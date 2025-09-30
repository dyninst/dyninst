#include "common/h/registers/MachRegister.h"
#include "registers/MachRegisterCache.h"
#include "debug_common.h"
#include "dyn_regs.h"

#include <cassert>
#include <unordered_map>
#include <map>
#include <vector>

namespace {
  const std::string invalid_reg_name{"<INVALID_REG>"};

  int32_t getID(Dyninst::MachRegister r) {
    return r.val() & 0x000000ff;
  }
}

namespace Dyninst { namespace registers {
  // These are defined in dyn_regs.C to ensure global constructor initialization ordering
  extern name_cache names;
  extern register_cache all_regs;
}}

namespace Dyninst {

  MachRegister::MachRegister() : reg(0) {}

  MachRegister::MachRegister(signed int r) : reg(r) {}

  MachRegister::MachRegister(signed int r, std::string n) : MachRegister(r) {
    registers::names.emplace(r, std::move(n));
    registers::all_regs[getArchitecture()].push_back(*this);
  }

  unsigned int MachRegister::regClass() const { return reg & 0x00ff0000; }

  int32_t MachRegister::getLengthID() const {
    return val() & 0x0000ff00;
  }

  MachRegister MachRegister::getBaseRegister() const {
    signed int category = (reg & 0x00ff0000);
    switch(getArchitecture()) {
      case Arch_x86:
        if(isGeneralPurpose())
          return MachRegister(reg & 0xffff00ff);
        else if(category == x86::FLAG)
          return x86::flags;
        else if(category == x86::MMX) {
          // MMX<N> -> st<N>
          auto const id = val() & 0xff;
          auto const arch = getArchitecture();
          return MachRegister(id | x86::FPDBL | x86::X87 | arch);
        }
        else if(category == x86::XMM || category == x86::YMM) {
          // assume CPU is new enough that it always has AVX-512 registers
          // XMM<N> -> ZMM<N> and YMM<N> -> ZMM<N>
          auto const id = val() & 0xff;
          auto const arch = getArchitecture();
          return MachRegister(id | x86::ZMMS | x86::ZMM | arch);
        }
        else
          return *this;
      case Arch_x86_64:
        if(isGeneralPurpose())
          return MachRegister(reg & 0xffff00ff);
        else if(category == x86_64::FLAG)
          return x86_64::flags;
        else if(category == x86_64::MMX) {
          // MMX<N> -> st<N>
          auto const id = val() & 0xff;
          auto const arch = getArchitecture();
          return MachRegister(id | x86_64::FPDBL | x86_64::X87 | arch);
        }
        else if(category == x86_64::XMM || category == x86_64::YMM) {
          // assume CPU is new enough that it always has AVX-512 registers
          // XMM<N> -> ZMM<N> and YMM<N> -> ZMM<N>
          auto const id = val() & 0xff;
          auto const arch = getArchitecture();
          return MachRegister(id | x86_64::ZMMS | x86_64::ZMM | arch);
        }
        else if(*this == x86_64::eip) {
          return x86_64::rip;
        }
        return *this;

      case Arch_aarch64: {
        if(isGeneralPurpose()) {
          auto const lengthID = getLengthID();

          // For GPRs, the most-basal registers are 64-bits
          if(lengthID == aarch64::FULL)
            return *this;

          // This is a w<N> register
          auto const offset = getID(*this) - getID(aarch64::w0);
          auto const id = offset + getID(aarch64::x0);
          auto const r = id | aarch64::FULL | aarch64::GPR | Arch_aarch64;
          return MachRegister(r);
        }

        if(isFloatingPoint()) {
          if(isControlStatus()) {
            return *this;
          }

          auto const lengthID = getLengthID();

          // The standard FPRs are aliases of the SVE registers. However, Dyninst
          // doesn't handle them, so we just consider the standard FPRs.
          if(lengthID == aarch64::Q_REG) {
            return *this;
          }

          // This is an 8-bit b<N>, 16-bit h<N>, 32-bit s<N>, or 64-bit d<N> register
          auto const first_of_len = [&]() -> MachRegister {
            switch(lengthID) {
              case aarch64::B_REG: return aarch64::b0;    // 8-bit
              case aarch64::W_REG: return aarch64::h0;    // 16-bit
              case aarch64::D_REG: return aarch64::s0;    // 32-bit
              case aarch64::FULL: return aarch64::d0;     // 64-bit
              case aarch64::HQ_REG: return aarch64::hq0;  // Upper 64 bits in 128-bit reg
            }
            return aarch64::q0;
          }();

          auto const first_seq_num = getID(first_of_len);
          auto const cur_seq_num = getID(*this);
          auto const offset = cur_seq_num - first_seq_num;
          auto const new_seq_num = getID(aarch64::q0) + offset;
          auto const new_len_type = aarch64::Q_REG;
          auto const r = new_seq_num | new_len_type | aarch64::FPR | Arch_aarch64;
          return MachRegister(r);
        }

        if(category == aarch64::SPR) {
          if(*this == aarch64::wsp) {
            return aarch64::sp;
          }
          if(*this == aarch64::wzr) {
            return aarch64::xzr;
          }
        }

        if(category == aarch64::FLAG) {
          switch(val()) {
            case aarch64::in:
            case aarch64::iz:
            case aarch64::ic:
            case aarch64::iv:
              return aarch64::nzcv;
            case aarch64::id:
            case aarch64::ia:
            case aarch64::ii:
            case aarch64::if_:
              return aarch64::daif;
            // We don't track sub-fields for fpcr
          }
        }

        if(category == aarch64::PSTATE) {
          return aarch64::pstate;
        }

        return *this;
      }
      case Arch_riscv64: {
        if(*this == riscv64::fflags || *this == riscv64::frm) {
          return riscv64::fcsr;
        }
        return *this;
      }

      case Arch_amdgpu_gfx908: {
        if(category == amdgpu_gfx908::MISC) {
          switch(val()) {
            case amdgpu_gfx908::ivcc_lo:
            case amdgpu_gfx908::ivcc_hi:
              return amdgpu_gfx908::vcc;
            case amdgpu_gfx908::iexec_lo:
            case amdgpu_gfx908::iexec_hi:
              return amdgpu_gfx908::exec;
            case amdgpu_gfx908::iflat_scratch_lo:
            case amdgpu_gfx908::iflat_scratch_hi:
              return amdgpu_gfx908::flat_scratch_all;
          }
        }
        return *this;
      }

      case Arch_amdgpu_gfx90a: {
        if(category == amdgpu_gfx90a::MISC) {
          switch(val()) {
            case amdgpu_gfx90a::ivcc_lo:
            case amdgpu_gfx90a::ivcc_hi:
              return amdgpu_gfx90a::vcc;
            case amdgpu_gfx90a::iexec_lo:
            case amdgpu_gfx90a::iexec_hi:
              return amdgpu_gfx90a::exec;
            case amdgpu_gfx90a::iflat_scratch_lo:
            case amdgpu_gfx90a::iflat_scratch_hi:
              return amdgpu_gfx90a::flat_scratch_all;
          }
        }
        return *this;
      }

      case Arch_amdgpu_gfx940: {
        if(category == amdgpu_gfx940::MISC) {
          switch(val()) {
            case amdgpu_gfx940::ivcc_lo:
            case amdgpu_gfx940::ivcc_hi:
              return amdgpu_gfx940::vcc;
            case amdgpu_gfx940::iexec_lo:
            case amdgpu_gfx940::iexec_hi:
              return amdgpu_gfx940::exec;
            case amdgpu_gfx940::iflat_scratch_lo:
            case amdgpu_gfx940::iflat_scratch_hi:
              return amdgpu_gfx940::flat_scratch_all;
          }
        }
        return *this;
      }

      case Arch_ppc32: {
        auto ppc_id = [](MachRegister r) {
          return r.val() & 0x0000FFFF;
        };

        auto const id = ppc_id(*this);

        if(category == ppc32::FSR) {
          // fsr<N> -> fpr<N>[0:31]
          auto const base_id = ppc_id(ppc32::fsr0);
          auto const offset = id - base_id;
          auto const new_id = ppc_id(ppc32::fpr0) + offset;
          auto const r = new_id | ppc32::FPR | Arch_ppc32;
          return MachRegister(r);
        }

        // Condition register (cr<N>, cr<N><len>)
        if((id >= 621 && id <= 628) || (id >= 700 && id <= 731)) {
          return ppc32::cr;
        }

        // Floating-point Control/Status
        if(id >= 602 && id <= 609) {
          return ppc32::fpscw;
        }
        return *this;
      }

      case Arch_ppc64: {
        auto ppc_id = [](MachRegister r) {
          return r.val() & 0x0000FFFF;
        };

        /* Power ISA
         *  Version 3.1C
         *  May 26, 2024
         *  7.2.1.2 Vector Registers
         */
        auto to_vsr = [ppc_id](int32_t offset) {
          auto const new_id = ppc_id(ppc64::vsr32) + offset;
          auto const r = new_id | ppc64::VSR | Arch_ppc64;
          return MachRegister(r);
        };

        auto const id = ppc_id(*this);

        if(category == ppc64::FPR) {
          // fpr<N> -> vsr<32+N>[0:63]
          return to_vsr(id - ppc_id(ppc64::fpr0));
        }

        if(category == ppc64::FSR) {
          // fsr<N> -> vsr<32+N>[0:31]
          return to_vsr(id - ppc_id(ppc64::fsr0));
        }

        /* Power ISA
         *  Version 3.1C
         *  May 26, 2024
         *  2.3.1 Condition Register
         */
        // Condition register (cr<N>, cr<N><len>) -> cr
        if((id >= 621 && id <= 628) || (id >= 700 && id <= 731)) {
          return ppc64::cr;
        }

        // Floating-point Control/Status
        if(id >= 602 && id <= 609) {
          return ppc64::fpscw;
        }
        return *this;
      }

      case Arch_aarch32:
      case Arch_intelGen9:
      case Arch_cuda:
      case Arch_none:
        return *this;
      default: return InvalidReg;
    }
    return InvalidReg;
  }

  Architecture MachRegister::getArchitecture() const { return (Architecture)(reg & 0xff000000); }

  bool MachRegister::isValid() const { return (reg != InvalidReg.reg); }

  std::string const& MachRegister::name() const {
    auto iter = registers::names.find(reg);
    if(iter != registers::names.end()) {
	return iter->second;
    }
    common_parsing_printf("No MachRegister found with value %x\n", static_cast<unsigned int>(reg));
    return invalid_reg_name;
  }

  unsigned int MachRegister::size() const {
    switch(getArchitecture()) {
      case Arch_x86:
        switch(reg & 0x0000ff00) {
          case x86::FULL: return 4;
          case x86::BIT: return 0;
          case x86::L_REG:
          case x86::H_REG: return 1;
          case x86::W_REG: return 2;
          case x86::FPDBL: return 10;
          case x86::MMS: return 8;
          case x86::XMMS: return 16;
          case x86::YMMS: return 32;
          case x86::ZMMS: return 64;
          case x86::KMSKS: return 8;
          default:
            return 0;
        }
        break;
      case Arch_x86_64:
        switch(reg & 0x0000ff00) {
          case x86_64::L_REG:
          case x86_64::H_REG: return 1;
          case x86_64::W_REG: return 2;
          case x86_64::D_REG: return 4;
          case x86_64::FULL: return 8;
          case x86_64::MMS: return 8;
          case x86_64::XMMS: return 16;
          case x86_64::FPDBL: return 10;
          case x86_64::BIT: return 0;
          case x86_64::YMMS: return 32;
          case x86_64::ZMMS: return 64;
          case x86_64::KMSKS: return 8;
          default:
            return 0;  // Xiaozhu: return 0 as an indication of parsing junk.
        }
        break;
      case Arch_ppc32: {
        int reg_class = reg & 0x00ff0000;
        if(reg_class == ppc32::FPR || reg_class == ppc32::FSR)
          return 8;
        return 4;
      }
      case Arch_ppc64: {
        if((reg & 0x00ff0000) == ppc64::FPR)
          return 16;
        return 8;
      }
      case Arch_aarch32: assert(0); break;
      case Arch_riscv64: {
        return 8;
      }

      case Arch_cuda: return 8;
      case Arch_amdgpu_gfx908: {
        int reg_class = (reg & 0x00ff0000);
        if(reg_class == amdgpu_gfx908::SGPR || reg_class == amdgpu_gfx908::VGPR) {
          return 4;
        } else {
          switch(reg & 0x00007f00) {
            case amdgpu_gfx908::BITS_1:
            case amdgpu_gfx908::BITS_2:
            case amdgpu_gfx908::BITS_3:
            case amdgpu_gfx908::BITS_4:
            case amdgpu_gfx908::BITS_6:
            case amdgpu_gfx908::BITS_7:
            case amdgpu_gfx908::BITS_8: return 1;
            case amdgpu_gfx908::BITS_9:
            case amdgpu_gfx908::BITS_15:
            case amdgpu_gfx908::BITS_16: return 2;
            case amdgpu_gfx908::BITS_32: return 4;
            case amdgpu_gfx908::BITS_48: return 6;
            case amdgpu_gfx908::BITS_64: return 8;
            case amdgpu_gfx908::BITS_128: return 16;
            case amdgpu_gfx908::BITS_256: return 32;
            case amdgpu_gfx908::BITS_512: return 64;
            default:
        	common_parsing_printf(" unknown reg size %x\n", (unsigned int)reg);
        	assert(0);
          }
        }
      }
      break;
      case Arch_amdgpu_gfx90a: {
        int reg_class = (reg & 0x00ff0000);
        if(reg_class == amdgpu_gfx90a::SGPR || reg_class == amdgpu_gfx90a::VGPR) {
          return 4;
        } else {
          switch(reg & 0x00007f00) {
            case amdgpu_gfx90a::BITS_1:
            case amdgpu_gfx90a::BITS_2:
            case amdgpu_gfx90a::BITS_3:
            case amdgpu_gfx90a::BITS_4:
            case amdgpu_gfx90a::BITS_6:
            case amdgpu_gfx90a::BITS_7:
            case amdgpu_gfx90a::BITS_8: return 1;
            case amdgpu_gfx90a::BITS_9:
            case amdgpu_gfx90a::BITS_15:
            case amdgpu_gfx90a::BITS_16: return 2;
            case amdgpu_gfx90a::BITS_32: return 4;
            case amdgpu_gfx90a::BITS_48: return 6;
            case amdgpu_gfx90a::BITS_64: return 8;
            case amdgpu_gfx90a::BITS_128: return 16;
            case amdgpu_gfx90a::BITS_256: return 32;
            case amdgpu_gfx90a::BITS_512: return 64;
            default:
              common_parsing_printf(" unknown reg size %x\n", (unsigned int)reg);
              assert(0);
          }
        }
      }
      break;
      case Arch_amdgpu_gfx940: {
        int reg_class = (reg & 0x00ff0000);
        if(reg_class == amdgpu_gfx940::SGPR || reg_class == amdgpu_gfx940::VGPR) {
          return 4;
        } else {
          switch(reg & 0x00007f00) {
            case amdgpu_gfx940::BITS_1:
            case amdgpu_gfx940::BITS_2:
            case amdgpu_gfx940::BITS_3:
            case amdgpu_gfx940::BITS_4:
            case amdgpu_gfx940::BITS_6:
            case amdgpu_gfx940::BITS_7:
            case amdgpu_gfx940::BITS_8: return 1;
            case amdgpu_gfx940::BITS_9:
            case amdgpu_gfx940::BITS_15:
            case amdgpu_gfx940::BITS_16: return 2;
            case amdgpu_gfx940::BITS_32: return 4;
            case amdgpu_gfx940::BITS_48: return 6;
            case amdgpu_gfx940::BITS_64: return 8;
            case amdgpu_gfx940::BITS_128: return 16;
            case amdgpu_gfx940::BITS_256: return 32;
            case amdgpu_gfx940::BITS_512: return 64;
            default:
              common_parsing_printf(" unknown reg size %x\n", (unsigned int)reg);
              assert(0);
          }
        }
      }
      break;
      case Arch_aarch64: {
        if(isFloatingPoint()) {
          switch(reg & 0x0000ff00) {
            case aarch64::B_REG: return 1;
            case aarch64::W_REG: return 2;
            case aarch64::D_REG: return 4;
            case aarch64::FULL:
            case aarch64::HQ_REG: return 8;
            case aarch64::Q_REG: return 16;
            case aarch64::SVE2S: return 512;   // 512-bit Scalable Vector Extension
            case aarch64::SVES:
            case aarch64::PREDS:               // 2048-bit Scalable Vector Extension (SVE) vector length
            case aarch64::SVLS:
            case aarch64::SMEZAS: return 2048; // 2048-bit SME Effective Streaming SVE vector length (SVL)
            default: assert(0); return 0;
          }
        } else if((reg & 0x00ff0000) == aarch64::GPR || (reg & 0x00ff0000) == aarch64::SPR ||
                  (reg & 0x00ff0000) == aarch64::SYSREG || (reg & 0x00ff0000) == aarch64::FLAG)
          switch(reg & 0x0000ff00) {
            case aarch64::FULL: return 8;
            case aarch64::D_REG: return 4;
            case aarch64::BIT: return 0;
            default: return 0;
          }
        else
          return 4;
        break;
      }
      case Arch_intelGen9: {
        assert(0);
        break;
      }

      case Arch_none: return 0;
    }
    return 0; // Unreachable, but disable warnings
  }

  bool MachRegister::operator<(const MachRegister& a) const { return (reg < a.reg); }

  bool MachRegister::operator==(const MachRegister& a) const { return (reg == a.reg); }

  MachRegister::operator signed int() const { return reg; }

  signed int MachRegister::val() const { return reg; }

  MachRegister MachRegister::getPC(Dyninst::Architecture arch) {
    switch(arch) {
      case Arch_x86: return x86::eip;
      case Arch_x86_64: return x86_64::rip;
      case Arch_ppc32: return ppc32::pc;
      case Arch_ppc64: return ppc64::pc;
      case Arch_aarch64: // aarch64: pc is not writable
        return aarch64::pc;
      case Arch_aarch32: return InvalidReg;
      case Arch_riscv64: return riscv64::pc;
      case Arch_cuda: return cuda::pc;
      case Arch_intelGen9: return InvalidReg;
      case Arch_amdgpu_gfx908: return amdgpu_gfx908::pc_all;
      case Arch_amdgpu_gfx90a: return amdgpu_gfx90a::pc_all;
      case Arch_amdgpu_gfx940: return amdgpu_gfx940::pc_all;
      case Arch_none: return InvalidReg;
    }
    return InvalidReg;
  }

  MachRegister MachRegister::getReturnAddress(Dyninst::Architecture arch) {
    switch(arch) {
      case Arch_x86: assert(0); break;    // not implemented
      case Arch_x86_64: assert(0); break; // not implemented
      case Arch_ppc32: assert(0); break;  // not implemented
      case Arch_ppc64: assert(0); break;  // not implemented
      case Arch_aarch64:           // aarch64: x30 stores the RA for current frame
        return aarch64::x30;
      case Arch_riscv64:
        return riscv64::ra;
      case Arch_aarch32:
      case Arch_cuda:
      case Arch_amdgpu_gfx908:
      case Arch_amdgpu_gfx90a:
      case Arch_amdgpu_gfx940:
      case Arch_intelGen9: assert(0); break;
      case Arch_none: return InvalidReg;
    }
    return InvalidReg;
  }

  MachRegister MachRegister::getFramePointer(Dyninst::Architecture arch) {
    switch(arch) {
      case Arch_x86: return x86::ebp;
      case Arch_x86_64: return x86_64::rbp;
      case Arch_ppc32: return ppc32::r1;
      case Arch_ppc64: return ppc64::r1;
      case Arch_aarch64: return aarch64::x29; // aarch64: frame pointer is X29 by convention
      case Arch_riscv64: return riscv64::fp;
      case Arch_aarch32:
      case Arch_cuda:
      case Arch_intelGen9:
      case Arch_amdgpu_gfx908:
      case Arch_amdgpu_gfx90a:
      case Arch_amdgpu_gfx940:
      case Arch_none: return InvalidReg;
    }
    return InvalidReg;
  }

  MachRegister MachRegister::getStackPointer(Dyninst::Architecture arch) {
    switch(arch) {
      case Arch_x86: return x86::esp;
      case Arch_x86_64: return x86_64::rsp;
      case Arch_ppc32: return ppc32::r1;
      case Arch_ppc64: return ppc64::r1;
      case Arch_aarch64: return aarch64::sp; // aarch64: stack pointer is an independent register
      case Arch_riscv64: return riscv64::sp;
      case Arch_aarch32:
      case Arch_cuda:
      case Arch_intelGen9:
      case Arch_amdgpu_gfx908:
      case Arch_amdgpu_gfx90a:
      case Arch_amdgpu_gfx940:
      case Arch_none: return InvalidReg;
    }
    return InvalidReg;
  }

  MachRegister MachRegister::getSyscallNumberReg(Dyninst::Architecture arch) {
    // The syscall register for all platforms can be found in the 
    // linux syscall(2) man page.
    switch(arch) {
      case Arch_x86: return x86::eax;
      case Arch_x86_64: return x86_64::rax;
      case Arch_ppc32: return ppc32::r0;
      case Arch_ppc64: return ppc64::r0;
      case Arch_aarch64: return aarch64::x8;
      case Arch_riscv64: return riscv64::a7;
      case Arch_aarch32:
      case Arch_cuda:
      case Arch_intelGen9:
      case Arch_amdgpu_gfx908:
      case Arch_amdgpu_gfx90a:
      case Arch_amdgpu_gfx940:
      case Arch_none: return InvalidReg;
    }
    return InvalidReg;
  }

  MachRegister MachRegister::getSyscallNumberOReg(Dyninst::Architecture arch) {
    switch(arch) {
      case Arch_x86: return x86::oeax;
      case Arch_x86_64: return x86_64::orax;
      case Arch_ppc32: return ppc32::r0;
      case Arch_ppc64: return ppc64::r0;
      case Arch_aarch64: return aarch64::x8;
      case Arch_riscv64: return riscv64::a7;
      case Arch_aarch32:
      case Arch_none:
      case Arch_cuda:
      case Arch_amdgpu_gfx908:
      case Arch_amdgpu_gfx90a:
      case Arch_amdgpu_gfx940:
      case Arch_intelGen9:
        return InvalidReg;
      default: assert(0); return InvalidReg;
    }
    return InvalidReg;
  }

  MachRegister MachRegister::getSyscallReturnValueReg(Dyninst::Architecture arch) {
    switch(arch) {
      case Arch_x86: return x86::eax;
      case Arch_x86_64: return x86_64::rax;
      case Arch_ppc32: return ppc32::r3;
      case Arch_ppc64: return ppc64::r3;
      case Arch_aarch64: return aarch64::x0; // returned value is save in x0
      case Arch_riscv64: return riscv64::a0;
      case Arch_aarch32:
      case Arch_cuda:
      case Arch_intelGen9:
      case Arch_amdgpu_gfx908:
      case Arch_amdgpu_gfx90a:
      case Arch_amdgpu_gfx940:
      case Arch_none: return InvalidReg;
    }
    return InvalidReg;
  }

  MachRegister MachRegister::getZeroFlag(Dyninst::Architecture arch) {
    switch(arch) {
      case Arch_x86: return x86::zf;
      case Arch_x86_64: return x86_64::zf;
      case Arch_ppc32: return ppc32::cr0e;
      case Arch_ppc64: return ppc64::cr0e;
      case Arch_aarch64: return aarch64::z;
      case Arch_aarch32:
      // These Architecture do not have a zero flag:
      case Arch_riscv64:
      case Arch_cuda:
      case Arch_intelGen9:
      case Arch_amdgpu_gfx908:
      case Arch_amdgpu_gfx90a:
      case Arch_amdgpu_gfx940:
      case Arch_none: return InvalidReg;
    }

    return InvalidReg;
  }

  bool MachRegister::isPC() const {
    if(*this == InvalidReg) return false;
    auto const base = getBaseRegister();
    return base == getPC(getArchitecture());
  }

  bool MachRegister::isFramePointer() const {
    if(*this == InvalidReg) return false;
    auto const arch = getArchitecture();
    auto const is_ppc = (arch == Arch_ppc32 || arch == Arch_ppc64);

    // ppc uses a specific GPR as its frame pointer
    auto const base = (is_ppc) ? *this : getBaseRegister();
    return base == FrameBase || base == getFramePointer(getArchitecture());
  }

  bool MachRegister::isStackPointer() const {
    if(*this == InvalidReg) return false;
    auto const arch = getArchitecture();
    auto const is_ppc = (arch == Arch_ppc32 || arch == Arch_ppc64);

    // ppc uses a specific GPR as its stack pointer
    auto const base = (is_ppc) ? *this : getBaseRegister();
    return base == StackTop || base == getStackPointer(getArchitecture());
  }

  bool MachRegister::isSyscallNumberReg() const {
    if(*this == InvalidReg) return false;
    auto const arch = getArchitecture();
    auto const is_ppc = (arch == Arch_ppc32 || arch == Arch_ppc64);

    // ppc uses a specific GPR as its syscall reg
    auto const base = (is_ppc) ? *this : getBaseRegister();
    return base == getSyscallNumberReg(getArchitecture());
  }

  bool MachRegister::isSyscallReturnValueReg() const {
    if(*this == InvalidReg) return false;
    auto const arch = getArchitecture();
    auto const is_ppc = (arch == Arch_ppc32 || arch == Arch_ppc64);

    // ppc uses a specific GPR as its return value reg
    auto const base = (is_ppc) ? *this : getBaseRegister();
    return base == getSyscallReturnValueReg(getArchitecture());
  }

  bool MachRegister::isFlag() const {
    auto const base = getBaseRegister();
    auto const regC = base.regClass();
    switch(getArchitecture()) {
      case Arch_x86: return regC == x86::FLAG;
      case Arch_x86_64: return regC == x86_64::FLAG;
      case Arch_aarch64: {
        return regC == aarch64::FLAG ||
               regC == aarch64::PSTATE;
      }
      case Arch_riscv64: return regC == riscv64::CSR;
      case Arch_ppc32:
      case Arch_ppc64: {
        // For power, we have a different register representation.
        // We do not use the subrange field for MachReigsters
        // and all lower 32 bits are base ID
        int baseID = base.val() & 0x0000FFFF;
        return (baseID <= 731 && baseID >= 700) || (baseID <= 629 && baseID >= 621);
      }
      case Arch_amdgpu_gfx908:
      case Arch_amdgpu_gfx90a:
      case Arch_amdgpu_gfx940: {
        return (base.val() & 0x0000F000);
      }
      case Arch_cuda: return false;

      default: assert(!"Not implemented!");
    }
    return false;
  }

  bool MachRegister::isZeroFlag() const {
    if(*this == InvalidReg) return false;
    auto const arch = getArchitecture();
    if(arch == Arch_ppc32) {
      /*Power ISA
       * Version 3.1C, May 26, 2024
       * 2.3.1 Condition Register
       */
      switch(reg) {
        case Dyninst::ppc32::icr0e:
        case Dyninst::ppc32::icr1e:
        case Dyninst::ppc32::icr2e:
        case Dyninst::ppc32::icr3e:
        case Dyninst::ppc32::icr4e:
        case Dyninst::ppc32::icr5e:
        case Dyninst::ppc32::icr6e:
        case Dyninst::ppc32::icr7e:
          return true;
      }
    }
    if(arch == Arch_ppc64) {
      switch(reg) {
        case Dyninst::ppc64::icr0e:
        case Dyninst::ppc64::icr1e:
        case Dyninst::ppc64::icr2e:
        case Dyninst::ppc64::icr3e:
        case Dyninst::ppc64::icr4e:
        case Dyninst::ppc64::icr5e:
        case Dyninst::ppc64::icr6e:
        case Dyninst::ppc64::icr7e:
          return true;
      }
    }
    return *this == getZeroFlag(getArchitecture());
  }

  bool MachRegister::isGeneralPurpose() const {
    // clang-format: off
    auto const category = regClass();
    switch(getArchitecture()) {
      case Arch_x86:
        return category == x86::GPR;

      case Arch_x86_64:
        return category == x86_64::GPR;

      case Arch_aarch64:
        return category == aarch64::GPR;

      case Arch_riscv64:
        return category == riscv64::GPR;

      case Arch_ppc32:
        return category == ppc32::GPR;

      case Arch_ppc64:
        return category == ppc64::GPR;

      case Arch_cuda:
        return category == cuda::GPR;

      case Arch_amdgpu_gfx908: {
        switch(category) {
          case amdgpu_gfx908::SGPR:
          case amdgpu_gfx908::VGPR:
          case amdgpu_gfx908::ACC_VGPR:
          case amdgpu_gfx908::TTMP_SGPR:
            return true;
        }
        return false;
      }

      case Arch_amdgpu_gfx90a: {
        switch(category) {
          case amdgpu_gfx90a::SGPR:
          case amdgpu_gfx90a::VGPR:
          case amdgpu_gfx90a::ACC_VGPR:
          case amdgpu_gfx90a::TTMP_SGPR:
            return true;
        }
        return false;
      }

      case Arch_amdgpu_gfx940: {
        switch(category) {
          case amdgpu_gfx940::SGPR:
          case amdgpu_gfx940::VGPR:
          case amdgpu_gfx940::ACC_VGPR:
          case amdgpu_gfx940::TTMP_SGPR:
            return true;
        }
        return false;
      }

      case Arch_intelGen9:
      case Arch_aarch32:
      case Arch_none:
        return false;
    }
    return false;
    // clang-format: on
  }

  bool MachRegister::isFloatingPoint() const {
    // clang-format: off
    auto const category = regClass();
    switch(getArchitecture()) {
      case Arch_x86: {
        auto const is_vec = isVector();
        auto const is_x87 = (category == x86::X87);
        auto const is_ctl = (category == x86::FPCTL);
        auto const is_msk = (category == x86::KMASK);
        return is_x87 || is_ctl || (is_vec && !is_msk);
      }

      case Arch_x86_64: {
        auto const is_vec = isVector();
        auto const is_x87 = (category == x86_64::X87);
        auto const is_ctl = (category == x86_64::FPCTL);
        auto const is_msk = (category == x86_64::KMASK);
        return is_x87 || is_ctl || (is_vec && !is_msk);
      }

      case Arch_aarch64: {
        auto const is_vec = isVector();
        auto const is_fpr = (category == aarch64::FPR);
        auto const is_ctl = (*this == aarch64::fpcr);
        auto const is_sts = (*this == aarch64::fpsr);
        return is_vec || is_fpr || is_ctl || is_sts;
      }

      case Arch_riscv64: {
        auto const is_vec = isVector();
        auto const is_fpr = (category == riscv64::FPR);
        auto const is_fcsr = (*this == riscv64::fflags) || (*this == riscv64::frm) || (*this == riscv64::fcsr);
        return is_vec || is_fpr || is_fcsr;
      }

      case Arch_ppc32: {
        auto const is_fpr = (category == ppc32::FPR);
        auto const is_fsr = (category == ppc32::FSR);
        auto const is_ctr = (getBaseRegister() == ppc32::fpscw);
        return is_fpr || is_fsr || is_ctr;
      }

      case Arch_ppc64: {
        auto const is_vec = isVector();
        auto const is_fpr = (category == ppc64::FPR);
        auto const is_fsr = (category == ppc64::FSR);
        auto const is_ctr = (getBaseRegister() == ppc64::fpscw);
        return is_vec || is_fpr || is_fsr || is_ctr;
      }

      case Arch_amdgpu_gfx908:
        return category == amdgpu_gfx908::SGPR ||
               category == amdgpu_gfx908::VGPR ||
               category == amdgpu_gfx908::ACC_VGPR;


      case Arch_amdgpu_gfx90a:
        return category == amdgpu_gfx90a::SGPR ||
               category == amdgpu_gfx90a::VGPR ||
               category == amdgpu_gfx90a::ACC_VGPR;

      case Arch_amdgpu_gfx940:
        return category == amdgpu_gfx940::SGPR ||
               category == amdgpu_gfx940::VGPR ||
               category == amdgpu_gfx940::ACC_VGPR;

      case Arch_intelGen9:
      case Arch_aarch32:
      case Arch_cuda:
      case Arch_none:
        return false;
    }
    return false;
    // clang-format: on
  }

  bool MachRegister::isControlStatus() const {
    // clang-format: off
    auto const category = regClass();
    switch(getArchitecture()) {
      case Arch_x86:
        return category == x86::CTL   ||
               category == x86::FPCTL ||
               category == x86::KMASK;

      case Arch_x86_64:
        return category == x86_64::CTL   ||
               category == x86_64::FPCTL ||
               category == x86_64::KMASK;

      case Arch_aarch64: {
        switch(val()) {
          case aarch64::ifpcr:
          case aarch64::ifpsr:
          case aarch64::iffr:
          case aarch64::ip0:
          case aarch64::ip1:
          case aarch64::ip2:
          case aarch64::ip3:
          case aarch64::ip4:
          case aarch64::ip5:
          case aarch64::ip6:
          case aarch64::ip7:
          case aarch64::ip8:
          case aarch64::ip9:
          case aarch64::ip10:
          case aarch64::ip11:
          case aarch64::ip12:
          case aarch64::ip13:
          case aarch64::ip14:
          case aarch64::ip15:
          case aarch64::ivg:
          case aarch64::izt0:
            return true;
        }
        return false;
      }

      case Arch_riscv64: {
        return category == riscv64::CSR;
      }

      case Arch_ppc32: {
        // Most of the current control-like registers aren't part
        // of the ISA after v2.07 (Power8+). In particular, the FSR
        // registers are associated with instructions like 'qvfxxmadds',
        // but those are no longer in the ISA.
        auto const is_ctr = (*this == ppc32::ctr);
        auto const is_scw = (getBaseRegister() == ppc32::fpscw);
        return is_ctr || is_scw;
      }

      case Arch_ppc64: {
        // Most of the current control-like registers aren't part
        // of the ISA after v2.07 (Power8+). In particular, the FSR
        // registers are associated with instructions like 'qvfxxmadds',
        // but those are no longer in the ISA.
        auto const is_ctr = (*this == ppc64::ctr);
        auto const is_scw = (getBaseRegister() == ppc64::fpscw);
        return is_ctr || is_scw;
      }

      case Arch_amdgpu_gfx908: {
        switch(val()) {
        case amdgpu_gfx908::ivcc:
        case amdgpu_gfx908::ivcc_lo:
        case amdgpu_gfx908::ivcc_hi:
        case amdgpu_gfx908::iexec:
        case amdgpu_gfx908::iexec_lo:
        case amdgpu_gfx908::iexec_hi:
        case amdgpu_gfx908::isrc_scc:
        case amdgpu_gfx908::isrc_vccz:
        case amdgpu_gfx908::isrc_execz:
        case amdgpu_gfx908::ixnack_mask_lo:
        case amdgpu_gfx908::ixnack_mask_hi:
            return true;
        }
        return false;
      }

      case Arch_amdgpu_gfx90a: {
        switch(val()) {
        case amdgpu_gfx90a::ivcc:
        case amdgpu_gfx90a::ivcc_lo:
        case amdgpu_gfx90a::ivcc_hi:
        case amdgpu_gfx90a::iexec:
        case amdgpu_gfx90a::iexec_lo:
        case amdgpu_gfx90a::iexec_hi:
        case amdgpu_gfx90a::isrc_scc:
        case amdgpu_gfx90a::isrc_vccz:
        case amdgpu_gfx90a::isrc_execz:
        case amdgpu_gfx90a::ixnack_mask_lo:
        case amdgpu_gfx90a::ixnack_mask_hi:
        case amdgpu_gfx90a::ihw_reg_status:
            return true;
        }
        return false;
      }

      case Arch_amdgpu_gfx940: {
        switch(val()) {
        case amdgpu_gfx940::ivcc:
        case amdgpu_gfx940::ivcc_lo:
        case amdgpu_gfx940::ivcc_hi:
        case amdgpu_gfx940::iexec:
        case amdgpu_gfx940::iexec_lo:
        case amdgpu_gfx940::iexec_hi:
        case amdgpu_gfx940::isrc_scc:
        case amdgpu_gfx940::isrc_vccz:
        case amdgpu_gfx940::isrc_execz:
        case amdgpu_gfx940::ixnack_mask_lo:
        case amdgpu_gfx940::ixnack_mask_hi:
        case amdgpu_gfx940::ihw_reg_status:
            return true;
        }
        return false;
      }

      case Arch_cuda:
      case Arch_intelGen9:
      case Arch_aarch32:
      case Arch_none:
        return false;
    }
    return false;
    // clang-format: on
  }

  bool MachRegister::isVector() const {
    // clang-format: off
    auto const category = regClass();
    switch(getArchitecture()) {
      case Arch_x86:
        return category == x86::MMX   ||
               category == x86::XMM   ||
               category == x86::YMM   ||
               category == x86::ZMM   ||
               category == x86::KMASK ||
               (*this == x86::mxcsr);

      case Arch_x86_64:
        return category == x86_64::MMX ||
               category == x86_64::XMM ||
               category == x86_64::YMM ||
               category == x86_64::ZMM ||
               category == x86_64::KMASK ||
               (*this == x86_64::mxcsr);

      case Arch_aarch64:
        return category == aarch64::SVE  ||
               category == aarch64::SVE2 ||
               category == aarch64::SME;

      case Arch_riscv64:
        return false; // vector currently not supported

      case Arch_amdgpu_gfx908: {
        switch(val()) {
        case amdgpu_gfx908::ivcc:
        case amdgpu_gfx908::ivcc_lo:
        case amdgpu_gfx908::ivcc_hi:
        case amdgpu_gfx908::iexec:
        case amdgpu_gfx908::iexec_lo:
        case amdgpu_gfx908::iexec_hi:
        case amdgpu_gfx908::isrc_scc:
        case amdgpu_gfx908::isrc_vccz:
        case amdgpu_gfx908::isrc_execz:
        case amdgpu_gfx908::ixnack_mask_lo:
        case amdgpu_gfx908::ixnack_mask_hi:
            return true;
        }
        return category == amdgpu_gfx908::VGPR ||
               category == amdgpu_gfx908::ACC_VGPR;
      }

      case Arch_amdgpu_gfx90a: {
        switch(val()) {
        case amdgpu_gfx90a::ivcc:
        case amdgpu_gfx90a::ivcc_lo:
        case amdgpu_gfx90a::ivcc_hi:
        case amdgpu_gfx90a::iexec:
        case amdgpu_gfx90a::iexec_lo:
        case amdgpu_gfx90a::iexec_hi:
        case amdgpu_gfx90a::isrc_scc:
        case amdgpu_gfx90a::isrc_vccz:
        case amdgpu_gfx90a::isrc_execz:
        case amdgpu_gfx90a::ixnack_mask_lo:
        case amdgpu_gfx90a::ixnack_mask_hi:
            return true;
        }
        return category == amdgpu_gfx90a::VGPR ||
               category == amdgpu_gfx90a::ACC_VGPR;
      }

      case Arch_amdgpu_gfx940: {
        switch(val()) {
        case amdgpu_gfx940::ivcc:
        case amdgpu_gfx940::ivcc_lo:
        case amdgpu_gfx940::ivcc_hi:
        case amdgpu_gfx940::iexec:
        case amdgpu_gfx940::iexec_lo:
        case amdgpu_gfx940::iexec_hi:
        case amdgpu_gfx940::isrc_scc:
        case amdgpu_gfx940::isrc_vccz:
        case amdgpu_gfx940::isrc_execz:
        case amdgpu_gfx940::ixnack_mask_lo:
        case amdgpu_gfx940::ixnack_mask_hi:
            return true;
        }
        return category == amdgpu_gfx940::VGPR ||
               category == amdgpu_gfx940::ACC_VGPR;
      }

      case Arch_ppc64:
        return category == ppc64::VSR;

      case Arch_intelGen9:
      case Arch_aarch32:
      case Arch_none:
      case Arch_ppc32:
      case Arch_cuda:
        return false;
    }
    return false;
    // clang-format: on
  }

  std::vector<MachRegister> const& MachRegister::getAllRegistersForArch(Dyninst::Architecture arch) {
    return registers::all_regs[arch];
  }
}
