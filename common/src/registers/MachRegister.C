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
  int32_t getAlias(Dyninst::MachRegister r) {
    return r.val() & 0x0000ff00;
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

  MachRegister MachRegister::getBaseRegister() const {
    signed int category = (reg & 0x00ff0000);
    switch(getArchitecture()) {
      case Arch_x86:
        if(category == x86::GPR)
          return MachRegister(reg & 0xffff00ff);
        else if(category == x86::FLAG)
          return x86::flags;
        else if(category == x86::MMX)
          // Keep the register number, but change the category to X87 (e.g., MMX1 -> st1).
          return MachRegister((reg & ~0x00ff0000) | x86::X87);
        else if(category == x86::XMM)
          // assume CPU is new enough that it always has AVX registers
          // Keep the register number, but change the category to ZMM (e.g., XMM1 -> ZMM1).
          return MachRegister((reg & ~0x00ff0000) | x86::ZMM);
        else if(category == x86::YMM)
            // assume CPU is new enough that it always has AVX-512 registers
            // Keep the register number, but change the category to ZMM (e.g., YMM1 -> ZMM1).
            return MachRegister((reg & ~0x00ff0000) | x86::ZMM);
        else
          return *this;
      case Arch_x86_64:
        if(category == x86_64::GPR)
          return MachRegister(reg & 0xffff00ff);
        else if(category == x86_64::FLAG)
          return x86_64::flags;
        else if(category == x86_64::MMX)
          // Keep the register number, but change the category to X87 (e.g., MMX1 -> st1).
          return MachRegister((reg & ~0x00ff0000) | x86_64::X87);
        else if(category == x86_64::XMM)
            // Keep the register number, but change the category to ZMM (e.g., XMM1 -> ZMM1).
	    return MachRegister((reg & ~0x00ff0000) | x86_64::ZMM);
        else if(category == x86_64::YMM)
            // Keep the register number, but change the category to ZMM (e.g., YMM1 -> ZMM1).
	    return MachRegister((reg & ~0x00ff0000) | x86_64::ZMM);
        else
          return *this;

      case Arch_aarch64: {
        if(category == aarch64::GPR) {
          auto const alias = getAlias(*this);

          // For GPRs, the most-basal registers are 64-bits
          if(alias == aarch64::FULL)
            return *this;

          // This is a w<N> register
          auto const offset = getID(*this) - getID(aarch64::w0);
          auto const id = offset + getID(aarch64::x0);
          auto const r = id | aarch64::FULL | aarch64::GPR | Arch_aarch64;
          return MachRegister(r);
        }

        if(category == aarch64::FPR) {
          auto const alias = getAlias(*this);

          // The standard FPRs are aliases of the SVE registers. However, Dyninst
          // doesn't handle them, so we just consider the standard FPRs.
          if(alias == aarch64::Q_REG) {
            return *this;
          }

          // This is an 8-bit b<N>, 16-bit h<N>, 32-bit s<N>, or 64-bit d<N> register
          auto const first_of_len = [&]() -> MachRegister {
            switch(alias) {
              case aarch64::B_REG: return aarch64::b0;  // 8-bit
              case aarch64::W_REG: return aarch64::h0;  // 16-bit
              case aarch64::D_REG: return aarch64::s0;  // 32-bit
              case aarch64::FULL: return aarch64::d0;   // 64-bit
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
        return *this;
      }

      case Arch_amdgpu_gfx908:
        switch(category) {
          case amdgpu_gfx908::SGPR: return MachRegister((reg & 0x000000ff) | amdgpu_gfx908::s0);
          case amdgpu_gfx908::VGPR: return MachRegister((reg & 0x000000ff) | amdgpu_gfx908::v0);
          case amdgpu_gfx908::HWR: return MachRegister(reg);

          default: return *this;
        }

      case Arch_amdgpu_gfx90a:
        switch(category) {
          case amdgpu_gfx90a::SGPR: return MachRegister((reg & 0x000000ff) | amdgpu_gfx90a::s0);
          case amdgpu_gfx90a::VGPR: return MachRegister((reg & 0x000000ff) | amdgpu_gfx90a::v0);
          case amdgpu_gfx90a::HWR: return MachRegister(reg);

          default: return *this;
        }

      case Arch_amdgpu_gfx940:
        switch(category) {
          case amdgpu_gfx940::SGPR: return MachRegister((reg & 0x000000ff) | amdgpu_gfx940::s0);
          case amdgpu_gfx940::VGPR: return MachRegister((reg & 0x000000ff) | amdgpu_gfx940::v0);
          case amdgpu_gfx940::HWR: return MachRegister(reg);

          default: return *this;
        }

      case Arch_ppc32:
      case Arch_ppc64:
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
        if((reg & 0x00ff0000) == aarch64::FPR) {
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
    switch(arch) {
      case Arch_x86: return x86::eax;
      case Arch_x86_64: return x86_64::rax;
      case Arch_ppc32: return ppc32::r0;
      case Arch_ppc64: return ppc64::r0;
      case Arch_aarch64: return aarch64::x8;
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
      case Arch_none: return InvalidReg;
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

  MachRegister MachRegister::getArchRegFromAbstractReg(MachRegister abstract,
                                                       Dyninst::Architecture arch) {
    switch(arch) {
      case Arch_aarch64:
        if(abstract == ReturnAddr)
          return aarch64::x30;
        if(abstract == FrameBase)
          return aarch64::x29;
        if(abstract == StackTop)
          return aarch64::sp;
        if(abstract == CFA)
          assert(0); // don't know what to do
        // not abstract, return arch reg
        return abstract;
      default: assert(0);
    }

    return Dyninst::InvalidReg;
  }

  MachRegister MachRegister::getZeroFlag(Dyninst::Architecture arch) {
    switch(arch) {
      case Arch_x86: return x86::zf;
      case Arch_x86_64: return x86_64::zf;
      case Arch_ppc32: return ppc32::cr0e;
      case Arch_ppc64: return ppc64::cr0e;
      case Arch_aarch64: return aarch64::z;
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

  bool MachRegister::isPC() const {
    if(*this == InvalidReg) return false;
    return *this == getPC(getArchitecture());
  }

  bool MachRegister::isFramePointer() const {
    if(*this == InvalidReg) return false;
    return *this == FrameBase || *this == getFramePointer(getArchitecture());
  }

  bool MachRegister::isStackPointer() const {
    if(*this == InvalidReg) return false;
    return *this == StackTop || *this == getStackPointer(getArchitecture());
  }

  bool MachRegister::isSyscallNumberReg() const {
    if(*this == InvalidReg) return false;
    return *this == getSyscallNumberReg(getArchitecture());
  }

  bool MachRegister::isSyscallReturnValueReg() const {
    if(*this == InvalidReg) return false;
    return *this == getSyscallReturnValueReg(getArchitecture());
  }

  bool MachRegister::isFlag() const {
    int regC = regClass();
    switch(getArchitecture()) {
      case Arch_x86: return regC == x86::FLAG;
      case Arch_x86_64: return regC == x86_64::FLAG;
      case Arch_aarch64: return regC == aarch64::FLAG;
      case Arch_ppc32:
      case Arch_ppc64: {
        // For power, we have a different register representation.
        // We do not use the subrange field for MachReigsters
        // and all lower 32 bits are base ID
        int baseID = reg & 0x0000FFFF;
        return (baseID <= 731 && baseID >= 700) || (baseID <= 629 && baseID >= 621);
      }
      case Arch_amdgpu_gfx908:
      case Arch_amdgpu_gfx90a:
      case Arch_amdgpu_gfx940: {
        return (reg & 0x0000F000);
      }
      case Arch_cuda: return false;

      default: assert(!"Not implemented!");
    }
    return false;
  }

  bool MachRegister::isZeroFlag() const {
    if(*this == InvalidReg) return false;
    switch(getArchitecture()) {
      case Arch_ppc32:
      case Arch_ppc64: {
        // For power, we have a different register representation.
        // We do not use the subrange field for MachReigsters
        // and all lower 32 bits are base ID
        int baseID = reg & 0x0000FFFF;
        return (baseID <= 731 && baseID >= 700 && baseID % 4 == 2) ||
               (baseID <= 628 && baseID >= 621);
      }
      default:
	return *this == getZeroFlag(getArchitecture());
    }
    return false;
  }

  /*
   * DWARF Encodings
   *
   * Aarch64:
   *  DWARF for the Arm 64-bit Architecture
   *  6th October 2023
   *  4.1 DWARF register names
   *  https://github.com/ARM-software/abi-aa/releases/download/2023Q3/aadwarf64.pdf
   */
  MachRegister MachRegister::DwarfEncToReg(int encoding, Dyninst::Architecture arch) {
    switch(arch) {
      case Arch_ppc32:
        switch(encoding) {
          case 0: return Dyninst::ppc32::r0;
          case 1: return Dyninst::ppc32::r1;
          case 2: return Dyninst::ppc32::r2;
          case 3: return Dyninst::ppc32::r3;
          case 4: return Dyninst::ppc32::r4;
          case 5: return Dyninst::ppc32::r5;
          case 6: return Dyninst::ppc32::r6;
          case 7: return Dyninst::ppc32::r7;
          case 8: return Dyninst::ppc32::r8;
          case 9: return Dyninst::ppc32::r9;
          case 10: return Dyninst::ppc32::r10;
          case 11: return Dyninst::ppc32::r11;
          case 12: return Dyninst::ppc32::r12;
          case 13: return Dyninst::ppc32::r13;
          case 14: return Dyninst::ppc32::r14;
          case 15: return Dyninst::ppc32::r15;
          case 16: return Dyninst::ppc32::r16;
          case 17: return Dyninst::ppc32::r17;
          case 18: return Dyninst::ppc32::r18;
          case 19: return Dyninst::ppc32::r19;
          case 20: return Dyninst::ppc32::r20;
          case 21: return Dyninst::ppc32::r21;
          case 22: return Dyninst::ppc32::r22;
          case 23: return Dyninst::ppc32::r23;
          case 24: return Dyninst::ppc32::r24;
          case 25: return Dyninst::ppc32::r25;
          case 26: return Dyninst::ppc32::r26;
          case 27: return Dyninst::ppc32::r27;
          case 28: return Dyninst::ppc32::r28;
          case 29: return Dyninst::ppc32::r29;
          case 30: return Dyninst::ppc32::r30;
          case 31: return Dyninst::ppc32::r31;
          case 32: return Dyninst::ppc32::fpr0;
          case 33: return Dyninst::ppc32::fpr1;
          case 34: return Dyninst::ppc32::fpr2;
          case 35: return Dyninst::ppc32::fpr3;
          case 36: return Dyninst::ppc32::fpr4;
          case 37: return Dyninst::ppc32::fpr5;
          case 38: return Dyninst::ppc32::fpr6;
          case 39: return Dyninst::ppc32::fpr7;
          case 40: return Dyninst::ppc32::fpr8;
          case 41: return Dyninst::ppc32::fpr9;
          case 42: return Dyninst::ppc32::fpr10;
          case 43: return Dyninst::ppc32::fpr11;
          case 44: return Dyninst::ppc32::fpr12;
          case 45: return Dyninst::ppc32::fpr13;
          case 46: return Dyninst::ppc32::fpr14;
          case 47: return Dyninst::ppc32::fpr15;
          case 48: return Dyninst::ppc32::fpr16;
          case 49: return Dyninst::ppc32::fpr17;
          case 50: return Dyninst::ppc32::fpr18;
          case 51: return Dyninst::ppc32::fpr19;
          case 52: return Dyninst::ppc32::fpr20;
          case 53: return Dyninst::ppc32::fpr21;
          case 54: return Dyninst::ppc32::fpr22;
          case 55: return Dyninst::ppc32::fpr23;
          case 56: return Dyninst::ppc32::fpr24;
          case 57: return Dyninst::ppc32::fpr25;
          case 58: return Dyninst::ppc32::fpr26;
          case 59: return Dyninst::ppc32::fpr27;
          case 60: return Dyninst::ppc32::fpr28;
          case 61: return Dyninst::ppc32::fpr29;
          case 62: return Dyninst::ppc32::fpr30;
          case 63: return Dyninst::ppc32::fpr31;
          case 64: return Dyninst::ppc32::cr;
          case 65: return Dyninst::InvalidReg; // FPSCR
          default: return Dyninst::InvalidReg;
        }
        // Seperate switch statements to give compilers an easier time of
        //  optimizing
        switch(encoding) {
          case 100: return Dyninst::ppc32::mq;
          case 101: return Dyninst::ppc32::xer;
          case 102: return Dyninst::InvalidReg;
          case 103: return Dyninst::InvalidReg;
          case 104: return Dyninst::InvalidReg; // RTCU
          case 105: return Dyninst::InvalidReg; // RTCL
          case 106: return Dyninst::InvalidReg;
          case 107: return Dyninst::InvalidReg;
          case 108: return Dyninst::ppc32::lr;
          case 109: return Dyninst::ppc32::ctr;
          default: return Dyninst::InvalidReg;
        }
        break;
      case Arch_ppc64:
        switch(encoding) {
          case 0: return Dyninst::ppc64::r0;
          case 1: return Dyninst::ppc64::r1;
          case 2: return Dyninst::ppc64::r2;
          case 3: return Dyninst::ppc64::r3;
          case 4: return Dyninst::ppc64::r4;
          case 5: return Dyninst::ppc64::r5;
          case 6: return Dyninst::ppc64::r6;
          case 7: return Dyninst::ppc64::r7;
          case 8: return Dyninst::ppc64::r8;
          case 9: return Dyninst::ppc64::r9;
          case 10: return Dyninst::ppc64::r10;
          case 11: return Dyninst::ppc64::r11;
          case 12: return Dyninst::ppc64::r12;
          case 13: return Dyninst::ppc64::r13;
          case 14: return Dyninst::ppc64::r14;
          case 15: return Dyninst::ppc64::r15;
          case 16: return Dyninst::ppc64::r16;
          case 17: return Dyninst::ppc64::r17;
          case 18: return Dyninst::ppc64::r18;
          case 19: return Dyninst::ppc64::r19;
          case 20: return Dyninst::ppc64::r20;
          case 21: return Dyninst::ppc64::r21;
          case 22: return Dyninst::ppc64::r22;
          case 23: return Dyninst::ppc64::r23;
          case 24: return Dyninst::ppc64::r24;
          case 25: return Dyninst::ppc64::r25;
          case 26: return Dyninst::ppc64::r26;
          case 27: return Dyninst::ppc64::r27;
          case 28: return Dyninst::ppc64::r28;
          case 29: return Dyninst::ppc64::r29;
          case 30: return Dyninst::ppc64::r30;
          case 31: return Dyninst::ppc64::r31;
          case 32: return Dyninst::ppc64::fpr0;
          case 33: return Dyninst::ppc64::fpr1;
          case 34: return Dyninst::ppc64::fpr2;
          case 35: return Dyninst::ppc64::fpr3;
          case 36: return Dyninst::ppc64::fpr4;
          case 37: return Dyninst::ppc64::fpr5;
          case 38: return Dyninst::ppc64::fpr6;
          case 39: return Dyninst::ppc64::fpr7;
          case 40: return Dyninst::ppc64::fpr8;
          case 41: return Dyninst::ppc64::fpr9;
          case 42: return Dyninst::ppc64::fpr10;
          case 43: return Dyninst::ppc64::fpr11;
          case 44: return Dyninst::ppc64::fpr12;
          case 45: return Dyninst::ppc64::fpr13;
          case 46: return Dyninst::ppc64::fpr14;
          case 47: return Dyninst::ppc64::fpr15;
          case 48: return Dyninst::ppc64::fpr16;
          case 49: return Dyninst::ppc64::fpr17;
          case 50: return Dyninst::ppc64::fpr18;
          case 51: return Dyninst::ppc64::fpr19;
          case 52: return Dyninst::ppc64::fpr20;
          case 53: return Dyninst::ppc64::fpr21;
          case 54: return Dyninst::ppc64::fpr22;
          case 55: return Dyninst::ppc64::fpr23;
          case 56: return Dyninst::ppc64::fpr24;
          case 57: return Dyninst::ppc64::fpr25;
          case 58: return Dyninst::ppc64::fpr26;
          case 59: return Dyninst::ppc64::fpr27;
          case 60: return Dyninst::ppc64::fpr28;
          case 61: return Dyninst::ppc64::fpr29;
          case 62: return Dyninst::ppc64::fpr30;
          case 63: return Dyninst::ppc64::fpr31;
          case 64: return Dyninst::ppc64::cr;
          case 65: return Dyninst::InvalidReg; // FPSCR
          default: return Dyninst::InvalidReg;
        }
        // Seperate switch statements to give compilers an easier time of
        //  optimizing
        switch(encoding) {
          case 100: return Dyninst::ppc64::mq;
          case 101: return Dyninst::ppc64::xer;
          case 102: return Dyninst::InvalidReg;
          case 103: return Dyninst::InvalidReg;
          case 104: return Dyninst::InvalidReg; // RTCU
          case 105: return Dyninst::InvalidReg; // RTCL
          case 106: return Dyninst::InvalidReg;
          case 107: return Dyninst::InvalidReg;
          case 108: return Dyninst::ppc64::lr;
          case 109: return Dyninst::ppc64::ctr;
          default: return Dyninst::InvalidReg;
        }
        break;
      case Arch_aarch64: {
        switch(encoding) {

          // 64-bit general-purpose registers
          case 0: return Dyninst::aarch64::x0;
          case 1: return Dyninst::aarch64::x1;
          case 2: return Dyninst::aarch64::x2;
          case 3: return Dyninst::aarch64::x3;
          case 4: return Dyninst::aarch64::x4;
          case 5: return Dyninst::aarch64::x5;
          case 6: return Dyninst::aarch64::x6;
          case 7: return Dyninst::aarch64::x7;
          case 8: return Dyninst::aarch64::x8;
          case 9: return Dyninst::aarch64::x9;
          case 10: return Dyninst::aarch64::x10;
          case 11: return Dyninst::aarch64::x11;
          case 12: return Dyninst::aarch64::x12;
          case 13: return Dyninst::aarch64::x13;
          case 14: return Dyninst::aarch64::x14;
          case 15: return Dyninst::aarch64::x15;
          case 16: return Dyninst::aarch64::x16;
          case 17: return Dyninst::aarch64::x17;
          case 18: return Dyninst::aarch64::x18;
          case 19: return Dyninst::aarch64::x19;
          case 20: return Dyninst::aarch64::x20;
          case 21: return Dyninst::aarch64::x21;
          case 22: return Dyninst::aarch64::x22;
          case 23: return Dyninst::aarch64::x23;
          case 24: return Dyninst::aarch64::x24;
          case 25: return Dyninst::aarch64::x25;
          case 26: return Dyninst::aarch64::x26;
          case 27: return Dyninst::aarch64::x27;
          case 28: return Dyninst::aarch64::x28;
          case 29: return Dyninst::aarch64::x29;
          case 30: return Dyninst::aarch64::x30;
          case 31: return Dyninst::aarch64::sp;  // 64-bit stack pointer
          case 32: return Dyninst::aarch64::pc;  // 64-bit program counter
          case 33: return Dyninst::aarch64::elr_el1;  // Current mode exception link register
          case 34: return Dyninst::InvalidReg;  // RA_SIGN_STATE Return address signed state pseudo-register (beta)
          case 35: return Dyninst::aarch64::tpidrro_el0;  // EL0 Read-Only Software Thread ID register
          case 36: return Dyninst::aarch64::tpidr_el0;  // EL0 Read/Write Software Thread ID register
          case 37: return Dyninst::aarch64::tpidr_el1;  // EL1 Software Thread ID register
          case 38: return Dyninst::aarch64::tpidr_el2;  // EL2 Software Thread ID register
          case 39: return Dyninst::aarch64::tpidr_el3;  // EL3 Software Thread ID register

          // Reserved
          case 40:
          case 41:
          case 42:
          case 43:
          case 44:
          case 45: return Dyninst::InvalidReg;

          case 46: return Dyninst::aarch64::vg;  // 64-bit SVE vector granule pseudo-register (beta state)
          case 47: return Dyninst::aarch64::ffr;  // VG × 8-bit SVE first fault register

          // VG × 8-bit SVE predicate registers
          case 48: return Dyninst::aarch64::p0;
          case 49: return Dyninst::aarch64::p1;
          case 50: return Dyninst::aarch64::p2;
          case 51: return Dyninst::aarch64::p3;
          case 52: return Dyninst::aarch64::p4;
          case 53: return Dyninst::aarch64::p5;
          case 54: return Dyninst::aarch64::p6;
          case 55: return Dyninst::aarch64::p7;
          case 56: return Dyninst::aarch64::p8;
          case 57: return Dyninst::aarch64::p9;
          case 58: return Dyninst::aarch64::p10;
          case 59: return Dyninst::aarch64::p11;
          case 60: return Dyninst::aarch64::p12;
          case 61: return Dyninst::aarch64::p13;
          case 62: return Dyninst::aarch64::p14;
          case 63: return Dyninst::aarch64::p15;

          // 128-bit FP/Advanced SIMD registers
          case 64: return Dyninst::aarch64::q0;
          case 65: return Dyninst::aarch64::q1;
          case 66: return Dyninst::aarch64::q2;
          case 67: return Dyninst::aarch64::q3;
          case 68: return Dyninst::aarch64::q4;
          case 69: return Dyninst::aarch64::q5;
          case 70: return Dyninst::aarch64::q6;
          case 71: return Dyninst::aarch64::q7;
          case 72: return Dyninst::aarch64::q8;
          case 73: return Dyninst::aarch64::q9;
          case 74: return Dyninst::aarch64::q10;
          case 75: return Dyninst::aarch64::q11;
          case 76: return Dyninst::aarch64::q12;
          case 77: return Dyninst::aarch64::q13;
          case 78: return Dyninst::aarch64::q14;
          case 79: return Dyninst::aarch64::q15;
          case 80: return Dyninst::aarch64::q16;
          case 81: return Dyninst::aarch64::q17;
          case 82: return Dyninst::aarch64::q18;
          case 83: return Dyninst::aarch64::q19;
          case 84: return Dyninst::aarch64::q20;
          case 85: return Dyninst::aarch64::q21;
          case 86: return Dyninst::aarch64::q22;
          case 87: return Dyninst::aarch64::q23;
          case 88: return Dyninst::aarch64::q24;
          case 89: return Dyninst::aarch64::q25;
          case 90: return Dyninst::aarch64::q26;
          case 91: return Dyninst::aarch64::q27;
          case 92: return Dyninst::aarch64::q28;
          case 93: return Dyninst::aarch64::q29;
          case 94: return Dyninst::aarch64::q30;
          case 95: return Dyninst::aarch64::q31;

          // VG × 64-bit SVE vector registers (beta state)
          case 96: return Dyninst::aarch64::z0;
          case 97: return Dyninst::aarch64::z1;
          case 98: return Dyninst::aarch64::z2;
          case 99: return Dyninst::aarch64::z3;
          case 100: return Dyninst::aarch64::z4;
          case 101: return Dyninst::aarch64::z5;
          case 102: return Dyninst::aarch64::z6;
          case 103: return Dyninst::aarch64::z7;
          case 104: return Dyninst::aarch64::z8;
          case 105: return Dyninst::aarch64::z9;
          case 106: return Dyninst::aarch64::z10;
          case 107: return Dyninst::aarch64::z11;
          case 108: return Dyninst::aarch64::z12;
          case 109: return Dyninst::aarch64::z13;
          case 110: return Dyninst::aarch64::z14;
          case 111: return Dyninst::aarch64::z15;
          case 112: return Dyninst::aarch64::z16;
          case 113: return Dyninst::aarch64::z17;
          case 114: return Dyninst::aarch64::z18;
          case 115: return Dyninst::aarch64::z19;
          case 116: return Dyninst::aarch64::z20;
          case 117: return Dyninst::aarch64::z21;
          case 118: return Dyninst::aarch64::z22;
          case 119: return Dyninst::aarch64::z23;
          case 120: return Dyninst::aarch64::z24;
          case 121: return Dyninst::aarch64::z25;
          case 122: return Dyninst::aarch64::z26;
          case 123: return Dyninst::aarch64::z27;
          case 124: return Dyninst::aarch64::z28;
          case 125: return Dyninst::aarch64::z29;
          case 126: return Dyninst::aarch64::z30;
          case 127: return Dyninst::aarch64::z31;

          default: return Dyninst::InvalidReg; break;
        }
        return Dyninst::InvalidReg;
      }
      case Arch_cuda:
        // ignore CUDA register encodings for now
        return Dyninst::InvalidReg;
        break;
      case Arch_amdgpu_gfx908:
      case Arch_amdgpu_gfx90a:
      case Arch_amdgpu_gfx940:
        // ignore AMD register encodings for now
        return Dyninst::InvalidReg;
        break;
      case Arch_intelGen9: return Dyninst::InvalidReg; break;
      case Arch_none: return Dyninst::InvalidReg; break;
      default: assert(0); return InvalidReg;
    }
    // Invalid Architecture passed
    return Dyninst::InvalidReg;
  }

  int MachRegister::getDwarfEnc() const {
    switch(getArchitecture()) {
      case Arch_ppc32:
        switch(val()) {
          case Dyninst::ppc32::ir0: return 0;
          case Dyninst::ppc32::ir1: return 1;
          case Dyninst::ppc32::ir2: return 2;
          case Dyninst::ppc32::ir3: return 3;
          case Dyninst::ppc32::ir4: return 4;
          case Dyninst::ppc32::ir5: return 5;
          case Dyninst::ppc32::ir6: return 6;
          case Dyninst::ppc32::ir7: return 7;
          case Dyninst::ppc32::ir8: return 8;
          case Dyninst::ppc32::ir9: return 9;
          case Dyninst::ppc32::ir10: return 10;
          case Dyninst::ppc32::ir11: return 11;
          case Dyninst::ppc32::ir12: return 12;
          case Dyninst::ppc32::ir13: return 13;
          case Dyninst::ppc32::ir14: return 14;
          case Dyninst::ppc32::ir15: return 15;
          case Dyninst::ppc32::ir16: return 16;
          case Dyninst::ppc32::ir17: return 17;
          case Dyninst::ppc32::ir18: return 18;
          case Dyninst::ppc32::ir19: return 19;
          case Dyninst::ppc32::ir20: return 20;
          case Dyninst::ppc32::ir21: return 21;
          case Dyninst::ppc32::ir22: return 22;
          case Dyninst::ppc32::ir23: return 23;
          case Dyninst::ppc32::ir24: return 24;
          case Dyninst::ppc32::ir25: return 25;
          case Dyninst::ppc32::ir26: return 26;
          case Dyninst::ppc32::ir27: return 27;
          case Dyninst::ppc32::ir28: return 28;
          case Dyninst::ppc32::ir29: return 29;
          case Dyninst::ppc32::ir30: return 30;
          case Dyninst::ppc32::ir31: return 31;
          case Dyninst::ppc32::ifpr0: return 32;
          case Dyninst::ppc32::ifpr1: return 33;
          case Dyninst::ppc32::ifpr2: return 34;
          case Dyninst::ppc32::ifpr3: return 35;
          case Dyninst::ppc32::ifpr4: return 36;
          case Dyninst::ppc32::ifpr5: return 37;
          case Dyninst::ppc32::ifpr6: return 38;
          case Dyninst::ppc32::ifpr7: return 39;
          case Dyninst::ppc32::ifpr8: return 40;
          case Dyninst::ppc32::ifpr9: return 41;
          case Dyninst::ppc32::ifpr10: return 42;
          case Dyninst::ppc32::ifpr11: return 43;
          case Dyninst::ppc32::ifpr12: return 44;
          case Dyninst::ppc32::ifpr13: return 45;
          case Dyninst::ppc32::ifpr14: return 46;
          case Dyninst::ppc32::ifpr15: return 47;
          case Dyninst::ppc32::ifpr16: return 48;
          case Dyninst::ppc32::ifpr17: return 49;
          case Dyninst::ppc32::ifpr18: return 50;
          case Dyninst::ppc32::ifpr19: return 51;
          case Dyninst::ppc32::ifpr20: return 52;
          case Dyninst::ppc32::ifpr21: return 53;
          case Dyninst::ppc32::ifpr22: return 54;
          case Dyninst::ppc32::ifpr23: return 55;
          case Dyninst::ppc32::ifpr24: return 56;
          case Dyninst::ppc32::ifpr25: return 57;
          case Dyninst::ppc32::ifpr26: return 58;
          case Dyninst::ppc32::ifpr27: return 59;
          case Dyninst::ppc32::ifpr28: return 60;
          case Dyninst::ppc32::ifpr29: return 61;
          case Dyninst::ppc32::ifpr30: return 62;
          case Dyninst::ppc32::ifpr31: return 63;
          case Dyninst::ppc32::icr: return 64;
          case Dyninst::ppc32::imq: return 100;
          case Dyninst::ppc32::ixer: return 101;
          case Dyninst::ppc32::ilr: return 108;
          case Dyninst::ppc32::ictr: return 109;
          default: return -1;
        }
      case Arch_ppc64:
        switch(val()) {
          case Dyninst::ppc64::ir0: return 0;
          case Dyninst::ppc64::ir1: return 1;
          case Dyninst::ppc64::ir2: return 2;
          case Dyninst::ppc64::ir3: return 3;
          case Dyninst::ppc64::ir4: return 4;
          case Dyninst::ppc64::ir5: return 5;
          case Dyninst::ppc64::ir6: return 6;
          case Dyninst::ppc64::ir7: return 7;
          case Dyninst::ppc64::ir8: return 8;
          case Dyninst::ppc64::ir9: return 9;
          case Dyninst::ppc64::ir10: return 10;
          case Dyninst::ppc64::ir11: return 11;
          case Dyninst::ppc64::ir12: return 12;
          case Dyninst::ppc64::ir13: return 13;
          case Dyninst::ppc64::ir14: return 14;
          case Dyninst::ppc64::ir15: return 15;
          case Dyninst::ppc64::ir16: return 16;
          case Dyninst::ppc64::ir17: return 17;
          case Dyninst::ppc64::ir18: return 18;
          case Dyninst::ppc64::ir19: return 19;
          case Dyninst::ppc64::ir20: return 20;
          case Dyninst::ppc64::ir21: return 21;
          case Dyninst::ppc64::ir22: return 22;
          case Dyninst::ppc64::ir23: return 23;
          case Dyninst::ppc64::ir24: return 24;
          case Dyninst::ppc64::ir25: return 25;
          case Dyninst::ppc64::ir26: return 26;
          case Dyninst::ppc64::ir27: return 27;
          case Dyninst::ppc64::ir28: return 28;
          case Dyninst::ppc64::ir29: return 29;
          case Dyninst::ppc64::ir30: return 30;
          case Dyninst::ppc64::ir31: return 31;
          case Dyninst::ppc64::ifpr0: return 32;
          case Dyninst::ppc64::ifpr1: return 33;
          case Dyninst::ppc64::ifpr2: return 34;
          case Dyninst::ppc64::ifpr3: return 35;
          case Dyninst::ppc64::ifpr4: return 36;
          case Dyninst::ppc64::ifpr5: return 37;
          case Dyninst::ppc64::ifpr6: return 38;
          case Dyninst::ppc64::ifpr7: return 39;
          case Dyninst::ppc64::ifpr8: return 40;
          case Dyninst::ppc64::ifpr9: return 41;
          case Dyninst::ppc64::ifpr10: return 42;
          case Dyninst::ppc64::ifpr11: return 43;
          case Dyninst::ppc64::ifpr12: return 44;
          case Dyninst::ppc64::ifpr13: return 45;
          case Dyninst::ppc64::ifpr14: return 46;
          case Dyninst::ppc64::ifpr15: return 47;
          case Dyninst::ppc64::ifpr16: return 48;
          case Dyninst::ppc64::ifpr17: return 49;
          case Dyninst::ppc64::ifpr18: return 50;
          case Dyninst::ppc64::ifpr19: return 51;
          case Dyninst::ppc64::ifpr20: return 52;
          case Dyninst::ppc64::ifpr21: return 53;
          case Dyninst::ppc64::ifpr22: return 54;
          case Dyninst::ppc64::ifpr23: return 55;
          case Dyninst::ppc64::ifpr24: return 56;
          case Dyninst::ppc64::ifpr25: return 57;
          case Dyninst::ppc64::ifpr26: return 58;
          case Dyninst::ppc64::ifpr27: return 59;
          case Dyninst::ppc64::ifpr28: return 60;
          case Dyninst::ppc64::ifpr29: return 61;
          case Dyninst::ppc64::ifpr30: return 62;
          case Dyninst::ppc64::ifpr31: return 63;
          case Dyninst::ppc64::icr: return 64;
          case Dyninst::ppc64::imq: return 100;
          case Dyninst::ppc64::ixer: return 101;
          case Dyninst::ppc64::ilr: return 108;
          case Dyninst::ppc64::ictr: return 109;
          default: return -1;
        }
        break;
      case Arch_aarch64:
        switch(val()) {
          // 64-bit general-purpose registers
          case Dyninst::aarch64::ix0: return 0;
          case Dyninst::aarch64::ix1: return 1;
          case Dyninst::aarch64::ix2: return 2;
          case Dyninst::aarch64::ix3: return 3;
          case Dyninst::aarch64::ix4: return 4;
          case Dyninst::aarch64::ix5: return 5;
          case Dyninst::aarch64::ix6: return 6;
          case Dyninst::aarch64::ix7: return 7;
          case Dyninst::aarch64::ix8: return 8;
          case Dyninst::aarch64::ix9: return 9;
          case Dyninst::aarch64::ix10: return 10;
          case Dyninst::aarch64::ix11: return 11;
          case Dyninst::aarch64::ix12: return 12;
          case Dyninst::aarch64::ix13: return 13;
          case Dyninst::aarch64::ix14: return 14;
          case Dyninst::aarch64::ix15: return 15;
          case Dyninst::aarch64::ix16: return 16;
          case Dyninst::aarch64::ix17: return 17;
          case Dyninst::aarch64::ix18: return 18;
          case Dyninst::aarch64::ix19: return 19;
          case Dyninst::aarch64::ix20: return 20;
          case Dyninst::aarch64::ix21: return 21;
          case Dyninst::aarch64::ix22: return 22;
          case Dyninst::aarch64::ix23: return 23;
          case Dyninst::aarch64::ix24: return 24;
          case Dyninst::aarch64::ix25: return 25;
          case Dyninst::aarch64::ix26: return 26;
          case Dyninst::aarch64::ix27: return 27;
          case Dyninst::aarch64::ix28: return 28;
          case Dyninst::aarch64::ix29: return 29;
          case Dyninst::aarch64::ix30: return 30;
          case Dyninst::aarch64::isp: return 31;  // 64-bit stack pointer
          case Dyninst::aarch64::ipc: return 32;  // 64-bit program counter
          case Dyninst::aarch64::ielr_el1: return 33;  // Current mode exception link register

          // 34 is invalid -- RA_SIGN_STATE Return address signed state pseudo-register (beta)

          case Dyninst::aarch64::itpidrro_el0: return 35;  // EL0 Read-Only Software Thread ID register
          case Dyninst::aarch64::itpidr_el0: return 36;  // EL0 Read/Write Software Thread ID register
          case Dyninst::aarch64::itpidr_el1: return 37;  // EL1 Software Thread ID register
          case Dyninst::aarch64::itpidr_el2: return 38;  // EL2 Software Thread ID register
          case Dyninst::aarch64::itpidr_el3: return 39;  // EL3 Software Thread ID register

          // 40 to 45 are reserved

          case Dyninst::aarch64::ivg: return 46;  // 64-bit SVE vector granule pseudo-register (beta state)
          case Dyninst::aarch64::iffr: return 47;  // VG × 8-bit SVE first fault register

          // VG × 8-bit SVE predicate registers
          case Dyninst::aarch64::ip0: return 48;
          case Dyninst::aarch64::ip1: return 49;
          case Dyninst::aarch64::ip2: return 50;
          case Dyninst::aarch64::ip3: return 51;
          case Dyninst::aarch64::ip4: return 52;
          case Dyninst::aarch64::ip5: return 53;
          case Dyninst::aarch64::ip6: return 54;
          case Dyninst::aarch64::ip7: return 55;
          case Dyninst::aarch64::ip8: return 56;
          case Dyninst::aarch64::ip9: return 57;
          case Dyninst::aarch64::ip10: return 58;
          case Dyninst::aarch64::ip11: return 59;
          case Dyninst::aarch64::ip12: return 60;
          case Dyninst::aarch64::ip13: return 61;
          case Dyninst::aarch64::ip14: return 62;
          case Dyninst::aarch64::ip15: return 63;

          // 128-bit FP/Advanced SIMD registers
          case Dyninst::aarch64::iq0: return 64;
          case Dyninst::aarch64::iq1: return 65;
          case Dyninst::aarch64::iq2: return 66;
          case Dyninst::aarch64::iq3: return 67;
          case Dyninst::aarch64::iq4: return 68;
          case Dyninst::aarch64::iq5: return 69;
          case Dyninst::aarch64::iq6: return 70;
          case Dyninst::aarch64::iq7: return 71;
          case Dyninst::aarch64::iq8: return 72;
          case Dyninst::aarch64::iq9: return 73;
          case Dyninst::aarch64::iq10: return 74;
          case Dyninst::aarch64::iq11: return 75;
          case Dyninst::aarch64::iq12: return 76;
          case Dyninst::aarch64::iq13: return 77;
          case Dyninst::aarch64::iq14: return 78;
          case Dyninst::aarch64::iq15: return 79;
          case Dyninst::aarch64::iq16: return 80;
          case Dyninst::aarch64::iq17: return 81;
          case Dyninst::aarch64::iq18: return 82;
          case Dyninst::aarch64::iq19: return 83;
          case Dyninst::aarch64::iq20: return 84;
          case Dyninst::aarch64::iq21: return 85;
          case Dyninst::aarch64::iq22: return 86;
          case Dyninst::aarch64::iq23: return 87;
          case Dyninst::aarch64::iq24: return 88;
          case Dyninst::aarch64::iq25: return 89;
          case Dyninst::aarch64::iq26: return 90;
          case Dyninst::aarch64::iq27: return 91;
          case Dyninst::aarch64::iq28: return 92;
          case Dyninst::aarch64::iq29: return 93;
          case Dyninst::aarch64::iq30: return 94;
          case Dyninst::aarch64::iq31: return 95;

          // VG × 64-bit SVE vector registers (beta state)
          case Dyninst::aarch64::iz0: return 96;
          case Dyninst::aarch64::iz1: return 97;
          case Dyninst::aarch64::iz2: return 98;
          case Dyninst::aarch64::iz3: return 99;
          case Dyninst::aarch64::iz4: return 100;
          case Dyninst::aarch64::iz5: return 101;
          case Dyninst::aarch64::iz6: return 102;
          case Dyninst::aarch64::iz7: return 103;
          case Dyninst::aarch64::iz8: return 104;
          case Dyninst::aarch64::iz9: return 105;
          case Dyninst::aarch64::iz10: return 106;
          case Dyninst::aarch64::iz11: return 107;
          case Dyninst::aarch64::iz12: return 108;
          case Dyninst::aarch64::iz13: return 109;
          case Dyninst::aarch64::iz14: return 110;
          case Dyninst::aarch64::iz15: return 111;
          case Dyninst::aarch64::iz16: return 112;
          case Dyninst::aarch64::iz17: return 113;
          case Dyninst::aarch64::iz18: return 114;
          case Dyninst::aarch64::iz19: return 115;
          case Dyninst::aarch64::iz20: return 116;
          case Dyninst::aarch64::iz21: return 117;
          case Dyninst::aarch64::iz22: return 118;
          case Dyninst::aarch64::iz23: return 119;
          case Dyninst::aarch64::iz24: return 120;
          case Dyninst::aarch64::iz25: return 121;
          case Dyninst::aarch64::iz26: return 122;
          case Dyninst::aarch64::iz27: return 123;
          case Dyninst::aarch64::iz28: return 124;
          case Dyninst::aarch64::iz29: return 125;
          case Dyninst::aarch64::iz30: return 126;
          case Dyninst::aarch64::iz31: return 127;


          default: return -1;
        }
        break;
      case Arch_none: assert(0); return -1;
      default: assert(0); return -1;
    }
    // Invalid register passed
    return -1;
  }

  MachRegister MachRegister::getArchReg(unsigned int regNum, Dyninst::Architecture arch) {
    switch(arch) {
      case Arch_aarch64:
        switch(regNum) {
          case 0: return Dyninst::aarch64::x0;
          case 1: return Dyninst::aarch64::x1;
          case 2: return Dyninst::aarch64::x2;
          case 3: return Dyninst::aarch64::x3;
          case 4: return Dyninst::aarch64::x4;
          case 5: return Dyninst::aarch64::x5;
          case 6: return Dyninst::aarch64::x6;
          case 7: return Dyninst::aarch64::x7;
          case 8: return Dyninst::aarch64::x8;
          case 9: return Dyninst::aarch64::x9;
          case 10: return Dyninst::aarch64::x10;
          case 11: return Dyninst::aarch64::x11;
          case 12: return Dyninst::aarch64::x12;
          case 13: return Dyninst::aarch64::x13;
          case 14: return Dyninst::aarch64::x14;
          case 15: return Dyninst::aarch64::x15;
          case 16: return Dyninst::aarch64::x16;
          case 17: return Dyninst::aarch64::x17;
          case 18: return Dyninst::aarch64::x18;
          case 19: return Dyninst::aarch64::x19;
          case 20: return Dyninst::aarch64::x20;
          case 21: return Dyninst::aarch64::x21;
          case 22: return Dyninst::aarch64::x22;
          case 23: return Dyninst::aarch64::x23;
          case 24: return Dyninst::aarch64::x24;
          case 25: return Dyninst::aarch64::x25;
          case 26: return Dyninst::aarch64::x26;
          case 27: return Dyninst::aarch64::x27;
          case 28: return Dyninst::aarch64::x28;
          case 29: return Dyninst::aarch64::x29;
          case 30: return Dyninst::aarch64::x30;

          case 100: return Dyninst::aarch64::sp;
          case 101: return Dyninst::aarch64::pc;
          case 102: return Dyninst::aarch64::pstate;
          case 103: return Dyninst::aarch64::xzr;
          default: return InvalidReg;
        }
      default: return InvalidReg;
    }
    return InvalidReg;
  }

  std::vector<MachRegister> const& MachRegister::getAllRegistersForArch(Dyninst::Architecture arch) {
    return registers::all_regs[arch];
  }
}
