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
        if(category == x86_64::GPR)
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

        if(category == aarch64::SPR) {
          if(*this == aarch64::wsp) {
            return aarch64::sp;
          }
          if(*this == aarch64::wzr) {
            return aarch64::xzr;
          }
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
      case Arch_aarch32:
      case Arch_none:
      case Arch_cuda:
      case Arch_amdgpu_gfx908:
      case Arch_amdgpu_gfx90a:
      case Arch_amdgpu_gfx940:
      case Arch_intelGen9:
        return InvalidReg;
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
      case Arch_aarch64: return regC == aarch64::FLAG;
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
