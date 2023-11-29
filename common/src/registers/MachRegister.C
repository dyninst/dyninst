#include "common/h/registers/MachRegister.h"
#include "debug_common.h"
#include "dyn_regs.h"
#include "external/rose/amdgpuInstructionEnum.h"
#include "external/rose/armv8InstructionEnum.h"
#include "external/rose/powerpcInstructionEnum.h"
#include "external/rose/rose-compat.h"

#include <cassert>
#include <unordered_map>

namespace {
  std::unordered_map<signed int, std::string> names;
  const std::string invalid_reg_name{"<INVALID_REG>"};
}

namespace Dyninst {

  MachRegister::MachRegister() : reg(0) {}

  MachRegister::MachRegister(signed int r) : reg(r) {}

  MachRegister::MachRegister(signed int r, std::string n) : reg(r) { names.emplace(r, std::move(n)); }

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
      case Arch_ppc32:
      case Arch_ppc64:
      case Arch_none: return *this;
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

      case Arch_aarch32:
      case Arch_aarch64:
      case Arch_intelGen9:
      case Arch_cuda:
        // not verified
        return *this;
      default: return InvalidReg;
    }
    return InvalidReg;
  }

  Architecture MachRegister::getArchitecture() const { return (Architecture)(reg & 0xff000000); }

  bool MachRegister::isValid() const { return (reg != InvalidReg.reg); }

  std::string const& MachRegister::name() const {
    auto iter = names.find(reg);
    if(iter != names.end()) {
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
        if((reg & 0x00ff0000) == aarch64::FPR)
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

  // reg_idx needs to be set as the offset from base register
  // offset needs to be set as the offset inside the register

  static void getAmdgpuGfx908RoseRegister(int& reg_class, int& reg_idx, int& offset,
                                          const int& reg) {
    signed int category = (reg & 0x00ff0000);
    signed int baseID = (reg & 0x000000ff);

    offset = 0;
    reg_idx = baseID;
    switch(category) {
      case amdgpu_gfx908::SGPR: {
        reg_class = amdgpu_regclass_sgpr;
        break;
      }

      case amdgpu_gfx908::VGPR: {
        reg_class = amdgpu_regclass_vgpr;
        break;
      }

      case amdgpu_gfx908::PC: {
        reg_class = amdgpu_regclass_pc;
        reg_idx = amdgpu_pc;
        break;
      }

      case amdgpu_gfx908::HWR: {
        reg_class = amdgpu_regclass_pc;
        reg_idx = amdgpu_pc;
        break;
      }

      default: {
        assert(0 && "unsupported register type for amdgpu gfx908");
      }
    }
    return;
  }

  static void getAmdgpuGfx90aRoseRegister(int& reg_class, int& reg_idx, int& offset,
                                          const int& reg) {
    signed int category = (reg & 0x00ff0000);
    signed int baseID = (reg & 0x000000ff);

    offset = 0;
    reg_idx = baseID;
    switch(category) {
      case amdgpu_gfx90a::SGPR: {
        reg_class = amdgpu_regclass_sgpr;
        break;
      }

      case amdgpu_gfx90a::VGPR: {
        reg_class = amdgpu_regclass_vgpr;
        break;
      }

      case amdgpu_gfx90a::PC: {
        reg_class = amdgpu_regclass_pc;
        reg_idx = amdgpu_pc;
        break;
      }

      case amdgpu_gfx90a::HWR: {
        reg_class = amdgpu_regclass_pc;
        reg_idx = amdgpu_pc;
        break;
      }

      default: {
        assert(0 && "unsupported register type for amdgpu gfx90a");
      }
    }
    return;
  }

  static void getAmdgpuGfx940RoseRegister(int& reg_class, int& reg_idx, int& offset,
                                          const int& reg) {
    signed int category = (reg & 0x00ff0000);
    signed int baseID = (reg & 0x000000ff);

    offset = 0;
    reg_idx = baseID;
    switch(category) {
      case amdgpu_gfx940::SGPR: {
        reg_class = amdgpu_regclass_sgpr;
        break;
      }

      case amdgpu_gfx940::VGPR: {
        reg_class = amdgpu_regclass_vgpr;
        break;
      }

      case amdgpu_gfx940::PC: {
        reg_class = amdgpu_regclass_pc;
        reg_idx = amdgpu_pc;
        break;
      }

      case amdgpu_gfx940::HWR: {
        reg_class = amdgpu_regclass_pc;
        reg_idx = amdgpu_pc;
        break;
      }

      default: {
        assert(0 && "unsupported register type for amdgpu gfx940");
      }
    }
    return;
  }

  /* This function should has a boolean return value
   * to indicate whether there is a corresponding
   * ROSE register.
   *
   * Since historically, this function does not
   * have a return value. We set c to -1 to represent
   * error cases
   * c is set to regClass
   * n is set to regNum
   * p is set to regPosition
   * see dataflowAPI/src/ExpressionConversionVisitor.C
   */

  void MachRegister::getROSERegister(int& c, int& n, int& p) {
    // Rose: class, number, position
    // Dyninst: category, base id, subrange

    signed int category = (reg & 0x00ff0000);
    signed int subrange = (reg & 0x0000ff00);
    signed int baseID = (reg & 0x000000ff);

    switch(getArchitecture()) {
      case Arch_amdgpu_gfx908: {
        getAmdgpuGfx908RoseRegister(c, n, p, reg);
        return;
      }
      case Arch_amdgpu_gfx90a: {
        getAmdgpuGfx90aRoseRegister(c, n, p, reg);
        return;
      }
      case Arch_amdgpu_gfx940: {
        getAmdgpuGfx940RoseRegister(c, n, p, reg);
        return;
      }
      case Arch_x86:
        switch(category) {
          case x86::GPR:
            c = x86_regclass_gpr;
            switch(baseID) {
              case x86::BASEA: n = x86_gpr_ax; break;
              case x86::BASEC: n = x86_gpr_cx; break;
              case x86::BASED: n = x86_gpr_dx; break;
              case x86::BASEB: n = x86_gpr_bx; break;
              case x86::BASESP: n = x86_gpr_sp; break;
              case x86::BASEBP: n = x86_gpr_bp; break;
              case x86::BASESI: n = x86_gpr_si; break;
              case x86::BASEDI: n = x86_gpr_di; break;
              default: n = 0; break;
            }
            break;
          case x86::SEG:
            c = x86_regclass_segment;
            switch(baseID) {
              case x86::BASEDS: n = x86_segreg_ds; break;
              case x86::BASEES: n = x86_segreg_es; break;
              case x86::BASEFS: n = x86_segreg_fs; break;
              case x86::BASEGS: n = x86_segreg_gs; break;
              case x86::BASECS: n = x86_segreg_cs; break;
              case x86::BASESS: n = x86_segreg_ss; break;
              default: n = 0; break;
            }
            break;
          case x86::FLAG:
            c = x86_regclass_flags;
            switch(baseID) {
              case x86::CF: n = x86_flag_cf; break;
              case x86::PF: n = x86_flag_pf; break;
              case x86::AF: n = x86_flag_af; break;
              case x86::ZF: n = x86_flag_zf; break;
              case x86::SF: n = x86_flag_sf; break;
              case x86::TF: n = x86_flag_tf; break;
              case x86::IF: n = x86_flag_if; break;
              case x86::DF: n = x86_flag_df; break;
              case x86::OF: n = x86_flag_of; break;
              case x86::FLAGC: n= x86_flag_iopl0; break;
              case x86::FLAGD: n= x86_flag_iopl1; break;
              case x86::NT: n = x86_flag_nt; break;
              case x86::RF: n = x86_flag_rf; break;
              case x86::VM: n= x86_flag_vm; break;
              case x86::AC: n = x86_flag_ac; break;
              case x86::VIF: n = x86_flag_vif; break;
              case x86::VIP: n = x86_flag_vip; break;
              case x86::ID: n = x86_flag_id; break;
              default: assert(0); break;
            }
            break;
          case x86::MISC: c = x86_regclass_unknown; break;
          case x86::XMM:
            c = x86_regclass_xmm;
            n = baseID;
            break;
          case x86::MMX:
            c = x86_regclass_mm;
            n = baseID;
            break;
          case x86::X87:
            c = x86_regclass_st_top;
            n = baseID;
            break;
          case x86::YMM:
            c = x86_regclass_ymm;
            n = baseID;
            break;
          case x86::ZMM:
            c = x86_regclass_zmm;
            n = baseID;
            break;
          case x86::CTL:
            c = x86_regclass_cr;
            n = baseID;
            break;
          case x86::DBG:
            c = x86_regclass_dr;
            n = baseID;
            break;
          case x86::TST: c = x86_regclass_unknown; break;
          case 0:
            switch(baseID) {
              case 0x10:
                c = x86_regclass_ip;
                n = 0;
                break;
              default: c = x86_regclass_unknown; break;
            }
            break;
          default:
              common_parsing_printf("Unknown category '%d' for Arch_x86\n", category);
              break;
        }
        break;
      case Arch_x86_64:
        switch(category) {
          case x86_64::GPR:
            c = x86_regclass_gpr;
            switch(baseID) {
              case x86_64::BASEA: n = x86_gpr_ax; break;
              case x86_64::BASEC: n = x86_gpr_cx; break;
              case x86_64::BASED: n = x86_gpr_dx; break;
              case x86_64::BASEB: n = x86_gpr_bx; break;
              case x86_64::BASESP: n = x86_gpr_sp; break;
              case x86_64::BASEBP: n = x86_gpr_bp; break;
              case x86_64::BASESI: n = x86_gpr_si; break;
              case x86_64::BASEDI: n = x86_gpr_di; break;
              case x86_64::BASE8: n = x86_gpr_r8; break;
              case x86_64::BASE9: n = x86_gpr_r9; break;
              case x86_64::BASE10: n = x86_gpr_r10; break;
              case x86_64::BASE11: n = x86_gpr_r11; break;
              case x86_64::BASE12: n = x86_gpr_r12; break;
              case x86_64::BASE13: n = x86_gpr_r13; break;
              case x86_64::BASE14: n = x86_gpr_r14; break;
              case x86_64::BASE15: n = x86_gpr_r15; break;
              default: n = 0; break;
            }
            break;
          case x86_64::SEG:
            c = x86_regclass_segment;
            switch(baseID) {
              case x86_64::BASEDS: n = x86_segreg_ds; break;
              case x86_64::BASEES: n = x86_segreg_es; break;
              case x86_64::BASEFS: n = x86_segreg_fs; break;
              case x86_64::BASEGS: n = x86_segreg_gs; break;
              case x86_64::BASECS: n = x86_segreg_cs; break;
              case x86_64::BASESS: n = x86_segreg_ss; break;
              default: n = 0; break;
            }
            break;
          case x86_64::FLAG:
            c = x86_regclass_flags;
            switch(baseID) {
              case x86_64::CF: n = x86_flag_cf; break;
              case x86_64::FLAG1: n = x86_flag_1; break;
              case x86_64::PF: n = x86_flag_pf; break;
              case x86_64::FLAG3: n = x86_flag_3; break;
              case x86_64::AF: n = x86_flag_af; break;
              case x86_64::FLAG5: n = x86_flag_5; break;
              case x86_64::ZF: n = x86_flag_zf; break;
              case x86_64::SF: n = x86_flag_sf; break;
              case x86_64::TF: n = x86_flag_tf; break;
              case x86_64::IF: n = x86_flag_if; break;
              case x86_64::DF: n = x86_flag_df; break;
              case x86_64::OF: n = x86_flag_of; break;
              case x86_64::FLAGC: n = x86_flag_iopl0; break;
              case x86_64::FLAGD: n = x86_flag_iopl1; break;
              case x86_64::NT: n = x86_flag_nt; break;
              case x86_64::FLAGF: n = x86_flag_15; break;
              case x86_64::VM: n = x86_flag_vm; break;
              case x86_64::RF: n = x86_flag_rf; break;
              case x86_64::AC: n = x86_flag_ac; break;
              case x86_64::VIF: n = x86_flag_vif; break;
              case x86_64::VIP: n = x86_flag_vip; break;
              case x86_64::ID: n = x86_flag_id; break;
              default:
                c = -1;
                return;
                break;
            }
            break;
          case x86_64::MISC: c = x86_regclass_unknown; break;
          case x86_64::KMASK:
            c = x86_regclass_kmask;
            n = baseID;
            break;
          case x86_64::ZMM:
            c = x86_regclass_zmm;
            n = baseID;
            break;
          case x86_64::YMM:
            c = x86_regclass_ymm;
            n = baseID;
            break;
          case x86_64::XMM:
            c = x86_regclass_xmm;
            n = baseID;
            break;
          case x86_64::MMX:
            c = x86_regclass_mm;
            n = baseID;
            break;
          case x86_64::X87:
            c = x86_regclass_st_top;
            n = baseID;
            break;
          case x86_64::CTL:
            c = x86_regclass_cr;
            n = baseID;
            break;
          case x86_64::DBG:
            c = x86_regclass_dr;
            n = baseID;
            break;
          case x86_64::TST: c = x86_regclass_unknown; break;
          case 0:
            switch(baseID) {
              case 0x10:
                c = x86_regclass_ip;
                n = 0;
                break;
              default: c = x86_regclass_unknown; break;
            }
            break;
          default:
	    common_parsing_printf("Unknown category '%d' for Arch_x86_64\n", category);
	    break;
        }
        break;
      case Arch_ppc32: {
        baseID = reg & 0x0000FFFF;
        n = baseID;
        switch(category) {
          case ppc32::GPR: c = powerpc_regclass_gpr; break;
          case ppc32::FPR:
          case ppc32::FSR: c = powerpc_regclass_fpr; break;
          case ppc32::SPR: {
            if(baseID < 613) {
              c = powerpc_regclass_spr;
            } else if(baseID < 621) {
              c = powerpc_regclass_sr;
            } else {
              c = powerpc_regclass_cr;
              n = 0;
              p = baseID - 621;
            }
          } break;
          default: c = -1; return;
        }
        return;
      } break;
      case Arch_ppc64: {
        baseID = reg & 0x0000FFFF;
        n = baseID;
        switch(category) {
          case ppc64::GPR: c = powerpc_regclass_gpr; break;
          case ppc64::FPR:
          case ppc64::FSR: c = powerpc_regclass_fpr; break;
          case ppc64::SPR: {
            if(baseID < 613) {
              c = powerpc_regclass_spr;
            } else if(baseID < 621) {
              c = powerpc_regclass_sr;
            } else {
              c = powerpc_regclass_cr;
              n = 0;
              p = baseID - 621;
            }
          } break;
          default: c = -1; return;
        }
        return;
      } break;
      case Arch_aarch64: {
        p = 0;
        switch(category) {
          case aarch64::GPR: {
            c = armv8_regclass_gpr;
            int regnum = baseID - (aarch64::x0 & 0xFF);
            n = armv8_gpr_r0 + regnum;
          } break;
          case aarch64::SPR: {
            n = 0;
            if(baseID == (aarch64::pstate & 0xFF)) {
              c = armv8_regclass_pstate;
            } else if(baseID == (aarch64::xzr & 0xFF) || baseID == (aarch64::wzr & 0xFF)) {
              c = armv8_regclass_gpr;
              n = armv8_gpr_zr;
            } else if(baseID == (aarch64::pc & 0xFF)) {
              c = armv8_regclass_pc;
            } else if(baseID == (aarch64::sp & 0xFF) || baseID == (aarch64::wsp & 0xFF)) {
              c = armv8_regclass_sp;
            }
          } break;
          case aarch64::FPR: {
            c = armv8_regclass_simd_fpr;

            int firstRegId;
            switch(reg & 0xFF00) {
              case aarch64::Q_REG: firstRegId = (aarch64::q0 & 0xFF); break;
              case aarch64::HQ_REG:
                firstRegId = (aarch64::hq0 & 0xFF);
                p = 64;
                break;
              case aarch64::FULL: firstRegId = (aarch64::d0 & 0xFF); break;
              case aarch64::D_REG: firstRegId = (aarch64::s0 & 0xFF); break;
              case aarch64::W_REG: firstRegId = (aarch64::h0 & 0xFF); break;
              case aarch64::B_REG: firstRegId = (aarch64::b0 & 0xFF); break;
              default: assert(!"invalid register subcategory for ARM64!"); break;
            }
            n = armv8_simdfpr_v0 + (baseID - firstRegId);
          } break;
          case aarch64::FLAG: {
            c = armv8_regclass_pstate;
            n = 0;
            switch(baseID) {
              case aarch64::N_FLAG: p = armv8_pstatefield_n; break;
              case aarch64::Z_FLAG: p = armv8_pstatefield_z; break;
              case aarch64::V_FLAG: p = armv8_pstatefield_v; break;
              case aarch64::C_FLAG: p = armv8_pstatefield_c; break;
              default: c = -1; return;
            }
          } break;
          default:
            // We do not want to assert here.
            // Set these output variable to invalid values and let the
            // semantics code to throw exceptions
            p = -1;
            c = -1;
            n = -1;
            break;
        }
        return;
      } break;
      default:
        c = x86_regclass_unknown;
        n = 0;
        break;
    }

    switch(getArchitecture()) {
      case Arch_x86:
        switch(subrange) {
          case x86::XMMS:
          case x86::YMMS:
          case x86::ZMMS:
          case x86::KMSKS:
          case x86::FPDBL: p = x86_regpos_qword; break;
          case x86::MMS: p = x86_regpos_qword; break;
          case x86::H_REG: p = x86_regpos_high_byte; break;
          case x86::L_REG: p = x86_regpos_low_byte; break;
          case x86::W_REG: p = x86_regpos_word; break;
          case x86::FULL:
          case x86::BIT: p = x86_regpos_all; break;
          default:
              common_parsing_printf("Unknown subrange value '%d' for Arch_x86\n", subrange);
              break;
        }
        break;

      case Arch_x86_64:
        switch(subrange) {
          case x86_64::FULL:
          case x86_64::XMMS:
          case x86_64::MMS:
          case x86_64::KMSKS:
          case x86_64::YMMS:
          case x86_64::ZMMS:
          case x86_64::FPDBL: p = x86_regpos_qword; break;
          case x86_64::H_REG: p = x86_regpos_high_byte; break;
          case x86_64::L_REG: p = x86_regpos_low_byte; break;
          case x86_64::W_REG: p = x86_regpos_word; break;
          case x86_64::D_REG: p = x86_regpos_dword; break;
          case x86_64::BIT: p = x86_regpos_all; break;
          default:
              common_parsing_printf("Unknown subrange value '%d' for Arch_x86_64\n", subrange);
              break;
        }
        break;
      case Arch_aarch64: {
        c = -1;
        return;
      }
      default: p = x86_regpos_unknown;
    }
  }

  /*
   * DWARF Encodings
   *
   * x86:
   *  System V Application Binary Interface
   *  Intel386 Architecture Processor Supplement
   *  Version 1.0 February 3, 2015
   *  Table 2.14: DWARF Register Number Mapping
   *  https://gitlab.com/x86-psABIs/i386-ABI
   *
   * x86_64:
   *   System V Application Binary Interface
   *   AMD64 Architecture Processor Supplement
   *   Version 1.0 June 21, 2022
   *   Table 3.36: DWARF Register Number Mapping
   *   https://gitlab.com/x86-psABIs/x86-64-ABI
   */
  MachRegister MachRegister::DwarfEncToReg(int encoding, Dyninst::Architecture arch) {
    switch(arch) {
      case Arch_x86:
        switch(encoding) {
          case 0: return Dyninst::x86::eax;
          case 1: return Dyninst::x86::ecx;
          case 2: return Dyninst::x86::edx;
          case 3: return Dyninst::x86::ebx;
          case 4: return Dyninst::x86::esp;
          case 5: return Dyninst::x86::ebp;
          case 6: return Dyninst::x86::esi;
          case 7: return Dyninst::x86::edi;
          case 8: return Dyninst::x86::eip;
          case 9: return Dyninst::x86::flags;
          case 10: return Dyninst::InvalidReg;
          case 11: return Dyninst::x86::st0;
          case 12: return Dyninst::x86::st1;
          case 13: return Dyninst::x86::st2;
          case 14: return Dyninst::x86::st3;
          case 15: return Dyninst::x86::st4;
          case 16: return Dyninst::x86::st5;
          case 17: return Dyninst::x86::st6;
          case 18: return Dyninst::x86::st7;
          case 19: return Dyninst::InvalidReg;
          case 20: return Dyninst::InvalidReg;
          case 21: return Dyninst::x86::xmm0;
          case 22: return Dyninst::x86::xmm1;
          case 23: return Dyninst::x86::xmm2;
          case 24: return Dyninst::x86::xmm3;
          case 25: return Dyninst::x86::xmm4;
          case 26: return Dyninst::x86::xmm5;
          case 27: return Dyninst::x86::xmm6;
          case 28: return Dyninst::x86::xmm7;
          case 29: return Dyninst::x86::mm0;
          case 30: return Dyninst::x86::mm1;
          case 31: return Dyninst::x86::mm2;
          case 32: return Dyninst::x86::mm3;
          case 33: return Dyninst::x86::mm4;
          case 34: return Dyninst::x86::mm5;
          case 35: return Dyninst::x86::mm6;
          case 36: return Dyninst::x86::mm7;
          case 37: return Dyninst::InvalidReg;
          case 38: return Dyninst::InvalidReg;
          case 39: return Dyninst::x86::mxcsr;
          case 40: return Dyninst::x86::es;
          case 41: return Dyninst::x86::cs;
          case 42: return Dyninst::x86::ss;
          case 43: return Dyninst::x86::ds;
          case 44: return Dyninst::x86::fs;
          case 45: return Dyninst::x86::gs;
          case 46: return Dyninst::InvalidReg;
          case 47: return Dyninst::InvalidReg;
          case 48: return Dyninst::x86::tr;
          case 49: return Dyninst::x86::ldtr;
          case 50: return Dyninst::InvalidReg;
          case 51: return Dyninst::InvalidReg;
          case 52: return Dyninst::InvalidReg;
          case 53: return Dyninst::InvalidReg;
          case 54: return Dyninst::InvalidReg;
          case 55: return Dyninst::InvalidReg;
          case 56: return Dyninst::InvalidReg;
          case 57: return Dyninst::InvalidReg;
          case 58: return Dyninst::InvalidReg;
          case 59: return Dyninst::InvalidReg;
          case 60: return Dyninst::InvalidReg;
          case 61: return Dyninst::InvalidReg;
          case 62: return Dyninst::InvalidReg;
          case 63: return Dyninst::InvalidReg;
          case 64: return Dyninst::InvalidReg;
          case 65: return Dyninst::InvalidReg;
          case 66: return Dyninst::InvalidReg;
          case 67: return Dyninst::InvalidReg;
          case 68: return Dyninst::InvalidReg;
          case 69: return Dyninst::InvalidReg;
          case 70: return Dyninst::InvalidReg;
          case 71: return Dyninst::InvalidReg;
          case 72: return Dyninst::InvalidReg;
          case 73: return Dyninst::InvalidReg;
          case 74: return Dyninst::InvalidReg;
          case 75: return Dyninst::InvalidReg;
          case 76: return Dyninst::InvalidReg;
          case 77: return Dyninst::InvalidReg;
          case 78: return Dyninst::InvalidReg;
          case 79: return Dyninst::InvalidReg;
          case 80: return Dyninst::InvalidReg;
          case 81: return Dyninst::InvalidReg;
          case 82: return Dyninst::InvalidReg;
          case 83: return Dyninst::InvalidReg;
          case 84: return Dyninst::InvalidReg;
          case 85: return Dyninst::InvalidReg;
          case 86: return Dyninst::InvalidReg;
          case 87: return Dyninst::InvalidReg;
          case 88: return Dyninst::InvalidReg;
          case 89: return Dyninst::InvalidReg;
          case 90: return Dyninst::InvalidReg;
          case 91: return Dyninst::InvalidReg;
          case 92: return Dyninst::InvalidReg;

          /* End of documented registers */
          /* The rest of these are assigned arbitrary values for internal Dyninst use. */
          case 1024: return Dyninst::x86::ax;
          case 1025: return Dyninst::x86::ah;
          case 1026: return Dyninst::x86::al;
          case 1027: return Dyninst::x86::cx;
          case 1028: return Dyninst::x86::ch;
          case 1029: return Dyninst::x86::cl;
          case 1030: return Dyninst::x86::dx;
          case 1031: return Dyninst::x86::dh;
          case 1032: return Dyninst::x86::dl;
          case 1033: return Dyninst::x86::bx;
          case 1034: return Dyninst::x86::bh;
          case 1035: return Dyninst::x86::bl;
          case 1036: return Dyninst::x86::sp;
          case 1037: return Dyninst::x86::bp;
          case 1038: return Dyninst::x86::si;
          case 1039: return Dyninst::x86::di;
          case 1040: return Dyninst::x86::gdtr;
          case 1041: return Dyninst::x86::idtr;
          case 1042: return Dyninst::x86::cf;
          case 1043: return Dyninst::x86::flag1;
          case 1044: return Dyninst::x86::pf;
          case 1045: return Dyninst::x86::flag3;
          case 1046: return Dyninst::x86::af;
          case 1047: return Dyninst::x86::flag5;
          case 1048: return Dyninst::x86::zf;
          case 1049: return Dyninst::x86::sf;
          case 1050: return Dyninst::x86::tf;
          case 1051: return Dyninst::x86::if_;
          case 1052: return Dyninst::x86::df;
          case 1053: return Dyninst::x86::of;
          case 1054: return Dyninst::x86::flagc;
          case 1055: return Dyninst::x86::flagd;
          case 1056: return Dyninst::x86::nt_;
          case 1057: return Dyninst::x86::flagf;
          case 1058: return Dyninst::x86::rf;
          case 1059: return Dyninst::x86::vm;
          case 1060: return Dyninst::x86::ac;
          case 1061: return Dyninst::x86::vif;
          case 1062: return Dyninst::x86::vip;
          case 1063: return Dyninst::x86::id;
          case 1064: return Dyninst::x86::cr0;
          case 1065: return Dyninst::x86::cr1;
          case 1066: return Dyninst::x86::cr2;
          case 1067: return Dyninst::x86::cr3;
          case 1068: return Dyninst::x86::cr4;
          case 1069: return Dyninst::x86::cr5;
          case 1070: return Dyninst::x86::cr6;
          case 1071: return Dyninst::x86::cr7;
          case 1072: return Dyninst::x86::dr0;
          case 1073: return Dyninst::x86::dr1;
          case 1074: return Dyninst::x86::dr2;
          case 1075: return Dyninst::x86::dr3;
          case 1076: return Dyninst::x86::dr4;
          case 1077: return Dyninst::x86::dr5;
          case 1078: return Dyninst::x86::dr6;
          case 1079: return Dyninst::x86::dr7;
          case 1080: return Dyninst::x86::fcw;
          case 1081: return Dyninst::x86::fsw;
          case 1082: return Dyninst::x86::ymm0;
          case 1083: return Dyninst::x86::ymm1;
          case 1084: return Dyninst::x86::ymm2;
          case 1085: return Dyninst::x86::ymm3;
          case 1086: return Dyninst::x86::ymm4;
          case 1087: return Dyninst::x86::ymm5;
          case 1088: return Dyninst::x86::ymm6;
          case 1089: return Dyninst::x86::ymm7;
          case 1090: return Dyninst::x86::zmm0;
          case 1091: return Dyninst::x86::zmm1;
          case 1092: return Dyninst::x86::zmm2;
          case 1093: return Dyninst::x86::zmm3;
          case 1094: return Dyninst::x86::zmm4;
          case 1095: return Dyninst::x86::zmm5;
          case 1096: return Dyninst::x86::zmm6;
          case 1097: return Dyninst::x86::zmm7;
          case 1098: return Dyninst::x86::k0;
          case 1099: return Dyninst::x86::k1;
          case 1100: return Dyninst::x86::k2;
          case 1101: return Dyninst::x86::k3;
          case 1102: return Dyninst::x86::k4;
          case 1103: return Dyninst::x86::k5;
          case 1104: return Dyninst::x86::k6;
          case 1105: return Dyninst::x86::k7;
          case 1106: return Dyninst::x86::oeax;
          case 1107: return Dyninst::x86::fsbase;
          case 1108: return Dyninst::x86::gsbase;
          case 1109: return Dyninst::x86::tr0;
          case 1110: return Dyninst::x86::tr1;
          case 1111: return Dyninst::x86::tr2;
          case 1112: return Dyninst::x86::tr3;
          case 1113: return Dyninst::x86::tr4;
          case 1114: return Dyninst::x86::tr5;
          case 1115: return Dyninst::x86::tr6;
          case 1116: return Dyninst::x86::tr7;
          default: return Dyninst::InvalidReg;
        }
        break;
      case Arch_x86_64:
        switch(encoding) {
          case 0: return Dyninst::x86_64::rax;
          case 1: return Dyninst::x86_64::rdx;
          case 2: return Dyninst::x86_64::rcx;
          case 3: return Dyninst::x86_64::rbx;
          case 4: return Dyninst::x86_64::rsi;
          case 5: return Dyninst::x86_64::rdi;
          case 6: return Dyninst::x86_64::rbp;
          case 7: return Dyninst::x86_64::rsp;
          case 8: return Dyninst::x86_64::r8;
          case 9: return Dyninst::x86_64::r9;
          case 10: return Dyninst::x86_64::r10;
          case 11: return Dyninst::x86_64::r11;
          case 12: return Dyninst::x86_64::r12;
          case 13: return Dyninst::x86_64::r13;
          case 14: return Dyninst::x86_64::r14;
          case 15: return Dyninst::x86_64::r15;
          case 16: return Dyninst::x86_64::rip;
          case 17: return Dyninst::x86_64::xmm0;
          case 18: return Dyninst::x86_64::xmm1;
          case 19: return Dyninst::x86_64::xmm2;
          case 20: return Dyninst::x86_64::xmm3;
          case 21: return Dyninst::x86_64::xmm4;
          case 22: return Dyninst::x86_64::xmm5;
          case 23: return Dyninst::x86_64::xmm6;
          case 24: return Dyninst::x86_64::xmm7;
          case 25: return Dyninst::x86_64::xmm8;
          case 26: return Dyninst::x86_64::xmm9;
          case 27: return Dyninst::x86_64::xmm10;
          case 28: return Dyninst::x86_64::xmm11;
          case 29: return Dyninst::x86_64::xmm12;
          case 30: return Dyninst::x86_64::xmm13;
          case 31: return Dyninst::x86_64::xmm14;
          case 32: return Dyninst::x86_64::xmm15;
          case 33: return Dyninst::x86_64::st0;
          case 34: return Dyninst::x86_64::st1;
          case 35: return Dyninst::x86_64::st2;
          case 36: return Dyninst::x86_64::st3;
          case 37: return Dyninst::x86_64::st4;
          case 38: return Dyninst::x86_64::st5;
          case 39: return Dyninst::x86_64::st6;
          case 40: return Dyninst::x86_64::st7;
          case 41: return Dyninst::x86_64::mm0;
          case 42: return Dyninst::x86_64::mm1;
          case 43: return Dyninst::x86_64::mm2;
          case 44: return Dyninst::x86_64::mm3;
          case 45: return Dyninst::x86_64::mm4;
          case 46: return Dyninst::x86_64::mm5;
          case 47: return Dyninst::x86_64::mm6;
          case 48: return Dyninst::x86_64::mm7;
          case 49: return Dyninst::x86_64::flags;
          case 50: return Dyninst::x86_64::es;
          case 51: return Dyninst::x86_64::cs;
          case 52: return Dyninst::x86_64::ss;
          case 53: return Dyninst::x86_64::ds;
          case 54: return Dyninst::x86_64::fs;
          case 55: return Dyninst::x86_64::gs;
          case 56: return Dyninst::InvalidReg;
          case 57: return Dyninst::InvalidReg;
          case 58: return Dyninst::x86_64::fsbase;
          case 59: return Dyninst::x86_64::gsbase;
          case 60: return Dyninst::InvalidReg;
          case 61: return Dyninst::InvalidReg;
          case 62: return Dyninst::x86_64::tr;
          case 63: return Dyninst::x86_64::ldtr;
          case 64: return Dyninst::x86_64::mxcsr;
          case 65: return Dyninst::x86_64::fcw;
          case 66: return Dyninst::x86_64::fsw;
          case 67: return Dyninst::x86_64::xmm16;
          case 68: return Dyninst::x86_64::xmm17;
          case 69: return Dyninst::x86_64::xmm18;
          case 70: return Dyninst::x86_64::xmm19;
          case 71: return Dyninst::x86_64::xmm20;
          case 72: return Dyninst::x86_64::xmm21;
          case 73: return Dyninst::x86_64::xmm22;
          case 74: return Dyninst::x86_64::xmm23;
          case 75: return Dyninst::x86_64::xmm24;
          case 76: return Dyninst::x86_64::xmm25;
          case 77: return Dyninst::x86_64::xmm26;
          case 78: return Dyninst::x86_64::xmm27;
          case 79: return Dyninst::x86_64::xmm28;
          case 80: return Dyninst::x86_64::xmm29;
          case 81: return Dyninst::x86_64::xmm30;
          case 82: return Dyninst::x86_64::xmm31;
          case 83: return Dyninst::InvalidReg;
          case 84: return Dyninst::InvalidReg;
          case 85: return Dyninst::InvalidReg;
          case 86: return Dyninst::InvalidReg;
          case 87: return Dyninst::InvalidReg;
          case 88: return Dyninst::InvalidReg;
          case 89: return Dyninst::InvalidReg;
          case 90: return Dyninst::InvalidReg;
          case 91: return Dyninst::InvalidReg;
          case 92: return Dyninst::InvalidReg;
          case 93: return Dyninst::InvalidReg;
          case 94: return Dyninst::InvalidReg;
          case 95: return Dyninst::InvalidReg;
          case 96: return Dyninst::InvalidReg;
          case 97: return Dyninst::InvalidReg;
          case 98: return Dyninst::InvalidReg;
          case 99: return Dyninst::InvalidReg;
          case 100: return Dyninst::InvalidReg;
          case 101: return Dyninst::InvalidReg;
          case 102: return Dyninst::InvalidReg;
          case 103: return Dyninst::InvalidReg;
          case 104: return Dyninst::InvalidReg;
          case 105: return Dyninst::InvalidReg;
          case 106: return Dyninst::InvalidReg;
          case 107: return Dyninst::InvalidReg;
          case 108: return Dyninst::InvalidReg;
          case 109: return Dyninst::InvalidReg;
          case 110: return Dyninst::InvalidReg;
          case 111: return Dyninst::InvalidReg;
          case 112: return Dyninst::InvalidReg;
          case 113: return Dyninst::InvalidReg;
          case 114: return Dyninst::InvalidReg;
          case 115: return Dyninst::InvalidReg;
          case 116: return Dyninst::InvalidReg;
          case 117: return Dyninst::InvalidReg;
          case 118: return Dyninst::x86_64::k0;
          case 119: return Dyninst::x86_64::k1;
          case 120: return Dyninst::x86_64::k2;
          case 121: return Dyninst::x86_64::k3;
          case 122: return Dyninst::x86_64::k4;
          case 123: return Dyninst::x86_64::k5;
          case 124: return Dyninst::x86_64::k6;
          case 125: return Dyninst::x86_64::k7;
          case 126: return Dyninst::InvalidReg;
          case 127: return Dyninst::InvalidReg;
          case 128: return Dyninst::InvalidReg;
          case 129: return Dyninst::InvalidReg;

          /* End of documented registers */
          /* The rest of these are assigned arbitrary values for internal Dyninst use. */
          case 1024: return Dyninst::x86_64::eax;
          case 1025: return Dyninst::x86_64::ax;
          case 1026: return Dyninst::x86_64::ah;
          case 1027: return Dyninst::x86_64::al;
          case 1028: return Dyninst::x86_64::ecx;
          case 1029: return Dyninst::x86_64::cx;
          case 1030: return Dyninst::x86_64::ch;
          case 1031: return Dyninst::x86_64::cl;
          case 1032: return Dyninst::x86_64::edx;
          case 1033: return Dyninst::x86_64::dx;
          case 1034: return Dyninst::x86_64::dh;
          case 1035: return Dyninst::x86_64::dl;
          case 1036: return Dyninst::x86_64::ebx;
          case 1037: return Dyninst::x86_64::bx;
          case 1038: return Dyninst::x86_64::bh;
          case 1039: return Dyninst::x86_64::bl;
          case 1040: return Dyninst::x86_64::esp;
          case 1041: return Dyninst::x86_64::sp;
          case 1042: return Dyninst::x86_64::spl;
          case 1043: return Dyninst::x86_64::ebp;
          case 1044: return Dyninst::x86_64::bp;
          case 1045: return Dyninst::x86_64::bpl;
          case 1046: return Dyninst::x86_64::esi;
          case 1047: return Dyninst::x86_64::si;
          case 1048: return Dyninst::x86_64::sil;
          case 1049: return Dyninst::x86_64::edi;
          case 1050: return Dyninst::x86_64::di;
          case 1051: return Dyninst::x86_64::dil;
          case 1052: return Dyninst::x86_64::r8b;
          case 1053: return Dyninst::x86_64::r8w;
          case 1054: return Dyninst::x86_64::r8d;
          case 1055: return Dyninst::x86_64::r9b;
          case 1056: return Dyninst::x86_64::r9w;
          case 1057: return Dyninst::x86_64::r9d;
          case 1058: return Dyninst::x86_64::r10b;
          case 1059: return Dyninst::x86_64::r10w;
          case 1060: return Dyninst::x86_64::r10d;
          case 1061: return Dyninst::x86_64::r11b;
          case 1062: return Dyninst::x86_64::r11w;
          case 1063: return Dyninst::x86_64::r11d;
          case 1064: return Dyninst::x86_64::r12b;
          case 1065: return Dyninst::x86_64::r12w;
          case 1066: return Dyninst::x86_64::r12d;
          case 1067: return Dyninst::x86_64::r13b;
          case 1068: return Dyninst::x86_64::r13w;
          case 1069: return Dyninst::x86_64::r13d;
          case 1070: return Dyninst::x86_64::r14b;
          case 1071: return Dyninst::x86_64::r14w;
          case 1072: return Dyninst::x86_64::r14d;
          case 1073: return Dyninst::x86_64::r15b;
          case 1074: return Dyninst::x86_64::r15w;
          case 1075: return Dyninst::x86_64::r15d;
          case 1076: return Dyninst::x86_64::eip;
          case 1077: return Dyninst::x86_64::cf;
          case 1078: return Dyninst::x86_64::flag1;
          case 1079: return Dyninst::x86_64::pf;
          case 1080: return Dyninst::x86_64::flag3;
          case 1081: return Dyninst::x86_64::af;
          case 1082: return Dyninst::x86_64::flag5;
          case 1083: return Dyninst::x86_64::zf;
          case 1084: return Dyninst::x86_64::sf;
          case 1085: return Dyninst::x86_64::tf;
          case 1086: return Dyninst::x86_64::if_;
          case 1087: return Dyninst::x86_64::df;
          case 1088: return Dyninst::x86_64::of;
          case 1089: return Dyninst::x86_64::flagc;
          case 1090: return Dyninst::x86_64::flagd;
          case 1091: return Dyninst::x86_64::nt_;
          case 1092: return Dyninst::x86_64::flagf;
          case 1093: return Dyninst::x86_64::rf;
          case 1094: return Dyninst::x86_64::vm;
          case 1095: return Dyninst::x86_64::ac;
          case 1096: return Dyninst::x86_64::vif;
          case 1097: return Dyninst::x86_64::vip;
          case 1098: return Dyninst::x86_64::id;
          case 1099: return Dyninst::x86_64::gdtr;
          case 1100: return Dyninst::x86_64::idtr;
          case 1101: return Dyninst::x86_64::cr0;
          case 1102: return Dyninst::x86_64::cr1;
          case 1103: return Dyninst::x86_64::cr2;
          case 1104: return Dyninst::x86_64::cr3;
          case 1105: return Dyninst::x86_64::cr4;
          case 1106: return Dyninst::x86_64::cr5;
          case 1107: return Dyninst::x86_64::cr6;
          case 1108: return Dyninst::x86_64::cr7;
          case 1109: return Dyninst::x86_64::cr8;
          case 1110: return Dyninst::x86_64::cr9;
          case 1111: return Dyninst::x86_64::cr10;
          case 1112: return Dyninst::x86_64::cr11;
          case 1113: return Dyninst::x86_64::cr12;
          case 1114: return Dyninst::x86_64::cr13;
          case 1115: return Dyninst::x86_64::cr14;
          case 1116: return Dyninst::x86_64::cr15;
          case 1117: return Dyninst::x86_64::dr0;
          case 1118: return Dyninst::x86_64::dr1;
          case 1119: return Dyninst::x86_64::dr2;
          case 1120: return Dyninst::x86_64::dr3;
          case 1121: return Dyninst::x86_64::dr4;
          case 1122: return Dyninst::x86_64::dr5;
          case 1123: return Dyninst::x86_64::dr6;
          case 1124: return Dyninst::x86_64::dr7;
          case 1125: return Dyninst::x86_64::dr8;
          case 1126: return Dyninst::x86_64::dr9;
          case 1127: return Dyninst::x86_64::dr10;
          case 1128: return Dyninst::x86_64::dr11;
          case 1129: return Dyninst::x86_64::dr12;
          case 1130: return Dyninst::x86_64::dr13;
          case 1131: return Dyninst::x86_64::dr14;
          case 1132: return Dyninst::x86_64::dr15;
          case 1133: return Dyninst::x86_64::ymm0;
          case 1134: return Dyninst::x86_64::ymm1;
          case 1135: return Dyninst::x86_64::ymm2;
          case 1136: return Dyninst::x86_64::ymm3;
          case 1137: return Dyninst::x86_64::ymm4;
          case 1138: return Dyninst::x86_64::ymm5;
          case 1139: return Dyninst::x86_64::ymm6;
          case 1140: return Dyninst::x86_64::ymm7;
          case 1141: return Dyninst::x86_64::ymm8;
          case 1142: return Dyninst::x86_64::ymm9;
          case 1143: return Dyninst::x86_64::ymm10;
          case 1144: return Dyninst::x86_64::ymm11;
          case 1145: return Dyninst::x86_64::ymm12;
          case 1146: return Dyninst::x86_64::ymm13;
          case 1147: return Dyninst::x86_64::ymm14;
          case 1148: return Dyninst::x86_64::ymm15;
          case 1149: return Dyninst::x86_64::ymm16;
          case 1150: return Dyninst::x86_64::ymm17;
          case 1151: return Dyninst::x86_64::ymm18;
          case 1152: return Dyninst::x86_64::ymm19;
          case 1153: return Dyninst::x86_64::ymm20;
          case 1154: return Dyninst::x86_64::ymm21;
          case 1155: return Dyninst::x86_64::ymm22;
          case 1156: return Dyninst::x86_64::ymm23;
          case 1157: return Dyninst::x86_64::ymm24;
          case 1158: return Dyninst::x86_64::ymm25;
          case 1159: return Dyninst::x86_64::ymm26;
          case 1160: return Dyninst::x86_64::ymm27;
          case 1161: return Dyninst::x86_64::ymm28;
          case 1162: return Dyninst::x86_64::ymm29;
          case 1163: return Dyninst::x86_64::ymm30;
          case 1164: return Dyninst::x86_64::ymm31;
          case 1165: return Dyninst::x86_64::zmm0;
          case 1166: return Dyninst::x86_64::zmm1;
          case 1167: return Dyninst::x86_64::zmm2;
          case 1168: return Dyninst::x86_64::zmm3;
          case 1169: return Dyninst::x86_64::zmm4;
          case 1170: return Dyninst::x86_64::zmm5;
          case 1171: return Dyninst::x86_64::zmm6;
          case 1172: return Dyninst::x86_64::zmm7;
          case 1173: return Dyninst::x86_64::zmm8;
          case 1174: return Dyninst::x86_64::zmm9;
          case 1175: return Dyninst::x86_64::zmm10;
          case 1176: return Dyninst::x86_64::zmm11;
          case 1177: return Dyninst::x86_64::zmm12;
          case 1178: return Dyninst::x86_64::zmm13;
          case 1179: return Dyninst::x86_64::zmm14;
          case 1180: return Dyninst::x86_64::zmm15;
          case 1181: return Dyninst::x86_64::zmm16;
          case 1182: return Dyninst::x86_64::zmm17;
          case 1183: return Dyninst::x86_64::zmm18;
          case 1184: return Dyninst::x86_64::zmm19;
          case 1185: return Dyninst::x86_64::zmm20;
          case 1186: return Dyninst::x86_64::zmm21;
          case 1187: return Dyninst::x86_64::zmm22;
          case 1188: return Dyninst::x86_64::zmm23;
          case 1189: return Dyninst::x86_64::zmm24;
          case 1190: return Dyninst::x86_64::zmm25;
          case 1191: return Dyninst::x86_64::zmm26;
          case 1192: return Dyninst::x86_64::zmm27;
          case 1193: return Dyninst::x86_64::zmm28;
          case 1194: return Dyninst::x86_64::zmm29;
          case 1195: return Dyninst::x86_64::zmm30;
          case 1196: return Dyninst::x86_64::zmm31;
          case 1197: return Dyninst::x86_64::orax;
          case 1198: return Dyninst::x86_64::tr0;
          case 1199: return Dyninst::x86_64::tr1;
          case 1200: return Dyninst::x86_64::tr2;
          case 1201: return Dyninst::x86_64::tr3;
          case 1202: return Dyninst::x86_64::tr4;
          case 1203: return Dyninst::x86_64::tr5;
          case 1204: return Dyninst::x86_64::tr6;
          case 1205: return Dyninst::x86_64::tr7;
          default: return Dyninst::InvalidReg;
        }
        break;
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
        // this info can be found in
        // DWARF for the ARM ® 64-bit Architecture (AArch64)
        switch(encoding) {
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
          case 31: return Dyninst::aarch64::sp;
          case 32: return Dyninst::InvalidReg;
          default: return Dyninst::InvalidReg;
        }
        switch(encoding) {
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
      case Arch_x86:
        switch(val()) {
          case Dyninst::x86::ieax: return 0;
          case Dyninst::x86::iecx: return 1;
          case Dyninst::x86::iedx: return 2;
          case Dyninst::x86::iebx: return 3;
          case Dyninst::x86::iesp: return 4;
          case Dyninst::x86::iebp: return 5;
          case Dyninst::x86::iesi: return 6;
          case Dyninst::x86::iedi: return 7;
          case Dyninst::x86::ieip: return 8;
          case Dyninst::x86::iflags: return 9;
          /*[10] Reserved */
          case Dyninst::x86::ist0: return 11;
          case Dyninst::x86::ist1: return 12;
          case Dyninst::x86::ist2: return 13;
          case Dyninst::x86::ist3: return 14;
          case Dyninst::x86::ist4: return 15;
          case Dyninst::x86::ist5: return 16;
          case Dyninst::x86::ist6: return 17;
          case Dyninst::x86::ist7: return 18;
          /*[19] Reserved */
          /*[20] Reserved */
          case Dyninst::x86::ixmm0: return 21;
          case Dyninst::x86::ixmm1: return 22;
          case Dyninst::x86::ixmm2: return 23;
          case Dyninst::x86::ixmm3: return 24;
          case Dyninst::x86::ixmm4: return 25;
          case Dyninst::x86::ixmm5: return 26;
          case Dyninst::x86::ixmm6: return 27;
          case Dyninst::x86::ixmm7: return 28;
          case Dyninst::x86::imm0: return 29;
          case Dyninst::x86::imm1: return 30;
          case Dyninst::x86::imm2: return 31;
          case Dyninst::x86::imm3: return 32;
          case Dyninst::x86::imm4: return 33;
          case Dyninst::x86::imm5: return 34;
          case Dyninst::x86::imm6: return 35;
          case Dyninst::x86::imm7: return 36;
          /*[37] Reserved */
          /*[38] Reserved */
          case Dyninst::x86::imxcsr: return 39;
          case Dyninst::x86::ies: return 40;
          case Dyninst::x86::ics: return 41;
          case Dyninst::x86::iss: return 42;
          case Dyninst::x86::ids: return 43;
          case Dyninst::x86::ifs: return 44;
          case Dyninst::x86::igs: return 45;
          /*[46] Reserved */
          /*[47] Reserved */
          case Dyninst::x86::itr: return 48;
          case Dyninst::x86::ildtr: return 49;
          /*[50] Reserved */
          /*[51] Reserved */
          /*[52] Reserved */
          /*[53] Reserved */
          /*[54] Reserved */
          /*[55] Reserved */
          /*[56] Reserved */
          /*[57] Reserved */
          /*[58] Reserved */
          /*[59] Reserved */
          /*[60] Reserved */
          /*[61] Reserved */
          /*[62] Reserved */
          /*[63] Reserved */
          /*[64] Reserved */
          /*[65] Reserved */
          /*[66] Reserved */
          /*[67] Reserved */
          /*[68] Reserved */
          /*[69] Reserved */
          /*[70] Reserved */
          /*[71] Reserved */
          /*[72] Reserved */
          /*[73] Reserved */
          /*[74] Reserved */
          /*[75] Reserved */
          /*[76] Reserved */
          /*[77] Reserved */
          /*[78] Reserved */
          /*[79] Reserved */
          /*[80] Reserved */
          /*[81] Reserved */
          /*[82] Reserved */
          /*[83] Reserved */
          /*[84] Reserved */
          /*[85] Reserved */
          /*[86] Reserved */
          /*[87] Reserved */
          /*[88] Reserved */
          /*[89] Reserved */
          /*[90] Reserved */
          /*[91] Reserved */
          /*[92] Reserved */

          /* End of documented registers */
          /* The rest of these are assigned arbitrary values for internal Dyninst use. */
          case Dyninst::x86::iax: return 1024;
          case Dyninst::x86::iah: return 1025;
          case Dyninst::x86::ial: return 1026;
          case Dyninst::x86::icx: return 1027;
          case Dyninst::x86::ich: return 1028;
          case Dyninst::x86::icl: return 1029;
          case Dyninst::x86::idx: return 1030;
          case Dyninst::x86::idh: return 1031;
          case Dyninst::x86::idl: return 1032;
          case Dyninst::x86::ibx: return 1033;
          case Dyninst::x86::ibh: return 1034;
          case Dyninst::x86::ibl: return 1035;
          case Dyninst::x86::isp: return 1036;
          case Dyninst::x86::ibp: return 1037;
          case Dyninst::x86::isi: return 1038;
          case Dyninst::x86::idi: return 1039;
          case Dyninst::x86::igdtr: return 1040;
          case Dyninst::x86::iidtr: return 1041;
          case Dyninst::x86::icf: return 1042;
          case Dyninst::x86::iflag1: return 1043;
          case Dyninst::x86::ipf: return 1044;
          case Dyninst::x86::iflag3: return 1045;
          case Dyninst::x86::iaf: return 1046;
          case Dyninst::x86::iflag5: return 1047;
          case Dyninst::x86::izf: return 1048;
          case Dyninst::x86::isf: return 1049;
          case Dyninst::x86::itf: return 1050;
          case Dyninst::x86::iif_: return 1051;
          case Dyninst::x86::idf: return 1052;
          case Dyninst::x86::iof: return 1053;
          case Dyninst::x86::iflagc: return 1054;
          case Dyninst::x86::iflagd: return 1055;
          case Dyninst::x86::int_: return 1056;
          case Dyninst::x86::iflagf: return 1057;
          case Dyninst::x86::irf: return 1058;
          case Dyninst::x86::ivm: return 1059;
          case Dyninst::x86::iac: return 1060;
          case Dyninst::x86::ivif: return 1061;
          case Dyninst::x86::ivip: return 1062;
          case Dyninst::x86::iid: return 1063;
          case Dyninst::x86::icr0: return 1064;
          case Dyninst::x86::icr1: return 1065;
          case Dyninst::x86::icr2: return 1066;
          case Dyninst::x86::icr3: return 1067;
          case Dyninst::x86::icr4: return 1068;
          case Dyninst::x86::icr5: return 1069;
          case Dyninst::x86::icr6: return 1070;
          case Dyninst::x86::icr7: return 1071;
          case Dyninst::x86::idr0: return 1072;
          case Dyninst::x86::idr1: return 1073;
          case Dyninst::x86::idr2: return 1074;
          case Dyninst::x86::idr3: return 1075;
          case Dyninst::x86::idr4: return 1076;
          case Dyninst::x86::idr5: return 1077;
          case Dyninst::x86::idr6: return 1078;
          case Dyninst::x86::idr7: return 1079;
          case Dyninst::x86::ifcw: return 1080;
          case Dyninst::x86::ifsw: return 1081;
          case Dyninst::x86::iymm0: return 1082;
          case Dyninst::x86::iymm1: return 1083;
          case Dyninst::x86::iymm2: return 1084;
          case Dyninst::x86::iymm3: return 1085;
          case Dyninst::x86::iymm4: return 1086;
          case Dyninst::x86::iymm5: return 1087;
          case Dyninst::x86::iymm6: return 1088;
          case Dyninst::x86::iymm7: return 1089;
          case Dyninst::x86::izmm0: return 1090;
          case Dyninst::x86::izmm1: return 1091;
          case Dyninst::x86::izmm2: return 1092;
          case Dyninst::x86::izmm3: return 1093;
          case Dyninst::x86::izmm4: return 1094;
          case Dyninst::x86::izmm5: return 1095;
          case Dyninst::x86::izmm6: return 1096;
          case Dyninst::x86::izmm7: return 1097;
          case Dyninst::x86::ik0: return 1098;
          case Dyninst::x86::ik1: return 1099;
          case Dyninst::x86::ik2: return 1100;
          case Dyninst::x86::ik3: return 1101;
          case Dyninst::x86::ik4: return 1102;
          case Dyninst::x86::ik5: return 1103;
          case Dyninst::x86::ik6: return 1104;
          case Dyninst::x86::ik7: return 1105;
          case Dyninst::x86::ioeax: return 1106;
          case Dyninst::x86::ifsbase: return 1107;
          case Dyninst::x86::igsbase: return 1108;
          case Dyninst::x86::itr0: return 1109;
          case Dyninst::x86::itr1: return 1110;
          case Dyninst::x86::itr2: return 1111;
          case Dyninst::x86::itr3: return 1112;
          case Dyninst::x86::itr4: return 1113;
          case Dyninst::x86::itr5: return 1114;
          case Dyninst::x86::itr6: return 1115;
          case Dyninst::x86::itr7: return 1116;
          default: return -1;
        }
        break;
      case Arch_x86_64:
        switch(val()) {
          case Dyninst::x86_64::irax: return 0;
          case Dyninst::x86_64::irdx: return 1;
          case Dyninst::x86_64::ircx: return 2;
          case Dyninst::x86_64::irbx: return 3;
          case Dyninst::x86_64::irsi: return 4;
          case Dyninst::x86_64::irdi: return 5;
          case Dyninst::x86_64::irbp: return 6;
          case Dyninst::x86_64::irsp: return 7;
          case Dyninst::x86_64::ir8: return 8;
          case Dyninst::x86_64::ir9: return 9;
          case Dyninst::x86_64::ir10: return 10;
          case Dyninst::x86_64::ir11: return 11;
          case Dyninst::x86_64::ir12: return 12;
          case Dyninst::x86_64::ir13: return 13;
          case Dyninst::x86_64::ir14: return 14;
          case Dyninst::x86_64::ir15: return 15;
          case Dyninst::x86_64::irip: return 16;
          case Dyninst::x86_64::ixmm0: return 17;
          case Dyninst::x86_64::ixmm1: return 18;
          case Dyninst::x86_64::ixmm2: return 19;
          case Dyninst::x86_64::ixmm3: return 20;
          case Dyninst::x86_64::ixmm4: return 21;
          case Dyninst::x86_64::ixmm5: return 22;
          case Dyninst::x86_64::ixmm6: return 23;
          case Dyninst::x86_64::ixmm7: return 24;
          case Dyninst::x86_64::ixmm8: return 25;
          case Dyninst::x86_64::ixmm9: return 26;
          case Dyninst::x86_64::ixmm10: return 27;
          case Dyninst::x86_64::ixmm11: return 28;
          case Dyninst::x86_64::ixmm12: return 29;
          case Dyninst::x86_64::ixmm13: return 30;
          case Dyninst::x86_64::ixmm14: return 31;
          case Dyninst::x86_64::ixmm15: return 32;
          case Dyninst::x86_64::ist0: return 33;
          case Dyninst::x86_64::ist1: return 34;
          case Dyninst::x86_64::ist2: return 35;
          case Dyninst::x86_64::ist3: return 36;
          case Dyninst::x86_64::ist4: return 37;
          case Dyninst::x86_64::ist5: return 38;
          case Dyninst::x86_64::ist6: return 39;
          case Dyninst::x86_64::ist7: return 40;
          case Dyninst::x86_64::imm0: return 41;
          case Dyninst::x86_64::imm1: return 42;
          case Dyninst::x86_64::imm2: return 43;
          case Dyninst::x86_64::imm3: return 44;
          case Dyninst::x86_64::imm4: return 45;
          case Dyninst::x86_64::imm5: return 46;
          case Dyninst::x86_64::imm6: return 47;
          case Dyninst::x86_64::imm7: return 48;
          case Dyninst::x86_64::iflags: return 49;
          case Dyninst::x86_64::ies: return 50;
          case Dyninst::x86_64::ics: return 51;
          case Dyninst::x86_64::iss: return 52;
          case Dyninst::x86_64::ids: return 53;
          case Dyninst::x86_64::ifs: return 54;
          case Dyninst::x86_64::igs: return 55;
          /*[56] Reserved */
          /*[57] Reserved */
          case Dyninst::x86_64::ifsbase: return 58;
          case Dyninst::x86_64::igsbase: return 59;
          /*[60] Reserved */
          /*[61] Reserved */
          case Dyninst::x86_64::itr: return 62;
          case Dyninst::x86_64::ildtr: return 63;
          case Dyninst::x86_64::imxcsr: return 64;
          case Dyninst::x86_64::ifcw: return 65;
          case Dyninst::x86_64::ifsw: return 66;
          case Dyninst::x86_64::ixmm16: return 67;
          case Dyninst::x86_64::ixmm17: return 68;
          case Dyninst::x86_64::ixmm18: return 69;
          case Dyninst::x86_64::ixmm19: return 70;
          case Dyninst::x86_64::ixmm20: return 71;
          case Dyninst::x86_64::ixmm21: return 72;
          case Dyninst::x86_64::ixmm22: return 73;
          case Dyninst::x86_64::ixmm23: return 74;
          case Dyninst::x86_64::ixmm24: return 75;
          case Dyninst::x86_64::ixmm25: return 76;
          case Dyninst::x86_64::ixmm26: return 77;
          case Dyninst::x86_64::ixmm27: return 78;
          case Dyninst::x86_64::ixmm28: return 79;
          case Dyninst::x86_64::ixmm29: return 80;
          case Dyninst::x86_64::ixmm30: return 81;
          case Dyninst::x86_64::ixmm31: return 82;
          /*[83] Reserved */
          /*[84] Reserved */
          /*[85] Reserved */
          /*[86] Reserved */
          /*[87] Reserved */
          /*[88] Reserved */
          /*[89] Reserved */
          /*[90] Reserved */
          /*[91] Reserved */
          /*[92] Reserved */
          /*[93] Reserved */
          /*[94] Reserved */
          /*[95] Reserved */
          /*[96] Reserved */
          /*[97] Reserved */
          /*[98] Reserved */
          /*[99] Reserved */
          /*[100] Reserved */
          /*[101] Reserved */
          /*[102] Reserved */
          /*[103] Reserved */
          /*[104] Reserved */
          /*[105] Reserved */
          /*[106] Reserved */
          /*[107] Reserved */
          /*[108] Reserved */
          /*[109] Reserved */
          /*[110] Reserved */
          /*[111] Reserved */
          /*[112] Reserved */
          /*[113] Reserved */
          /*[114] Reserved */
          /*[115] Reserved */
          /*[116] Reserved */
          /*[117] Reserved */
          case Dyninst::x86_64::ik0: return 118;
          case Dyninst::x86_64::ik1: return 119;
          case Dyninst::x86_64::ik2: return 120;
          case Dyninst::x86_64::ik3: return 121;
          case Dyninst::x86_64::ik4: return 122;
          case Dyninst::x86_64::ik5: return 123;
          case Dyninst::x86_64::ik6: return 124;
          case Dyninst::x86_64::ik7: return 125;
          /*[126] Reserved */
          /*[127] Reserved */
          /*[128] Reserved */
          /*[129] Reserved */

          /* End of documented registers */
          /* The rest of these are assigned arbitrary values for internal Dyninst use. */
          case Dyninst::x86_64::ieax: return 1024;
          case Dyninst::x86_64::iax: return 1025;
          case Dyninst::x86_64::iah: return 1026;
          case Dyninst::x86_64::ial: return 1027;
          case Dyninst::x86_64::iecx: return 1028;
          case Dyninst::x86_64::icx: return 1029;
          case Dyninst::x86_64::ich: return 1030;
          case Dyninst::x86_64::icl: return 1031;
          case Dyninst::x86_64::iedx: return 1032;
          case Dyninst::x86_64::idx: return 1033;
          case Dyninst::x86_64::idh: return 1034;
          case Dyninst::x86_64::idl: return 1035;
          case Dyninst::x86_64::iebx: return 1036;
          case Dyninst::x86_64::ibx: return 1037;
          case Dyninst::x86_64::ibh: return 1038;
          case Dyninst::x86_64::ibl: return 1039;
          case Dyninst::x86_64::iesp: return 1040;
          case Dyninst::x86_64::isp: return 1041;
          case Dyninst::x86_64::ispl: return 1042;
          case Dyninst::x86_64::iebp: return 1043;
          case Dyninst::x86_64::ibp: return 1044;
          case Dyninst::x86_64::ibpl: return 1045;
          case Dyninst::x86_64::iesi: return 1046;
          case Dyninst::x86_64::isi: return 1047;
          case Dyninst::x86_64::isil: return 1048;
          case Dyninst::x86_64::iedi: return 1049;
          case Dyninst::x86_64::idi: return 1050;
          case Dyninst::x86_64::idil: return 1051;
          case Dyninst::x86_64::ir8b: return 1052;
          case Dyninst::x86_64::ir8w: return 1053;
          case Dyninst::x86_64::ir8d: return 1054;
          case Dyninst::x86_64::ir9b: return 1055;
          case Dyninst::x86_64::ir9w: return 1056;
          case Dyninst::x86_64::ir9d: return 1057;
          case Dyninst::x86_64::ir10b: return 1058;
          case Dyninst::x86_64::ir10w: return 1059;
          case Dyninst::x86_64::ir10d: return 1060;
          case Dyninst::x86_64::ir11b: return 1061;
          case Dyninst::x86_64::ir11w: return 1062;
          case Dyninst::x86_64::ir11d: return 1063;
          case Dyninst::x86_64::ir12b: return 1064;
          case Dyninst::x86_64::ir12w: return 1065;
          case Dyninst::x86_64::ir12d: return 1066;
          case Dyninst::x86_64::ir13b: return 1067;
          case Dyninst::x86_64::ir13w: return 1068;
          case Dyninst::x86_64::ir13d: return 1069;
          case Dyninst::x86_64::ir14b: return 1070;
          case Dyninst::x86_64::ir14w: return 1071;
          case Dyninst::x86_64::ir14d: return 1072;
          case Dyninst::x86_64::ir15b: return 1073;
          case Dyninst::x86_64::ir15w: return 1074;
          case Dyninst::x86_64::ir15d: return 1075;
          case Dyninst::x86_64::ieip: return 1076;
          case Dyninst::x86_64::icf: return 1077;
          case Dyninst::x86_64::iflag1: return 1078;
          case Dyninst::x86_64::ipf: return 1079;
          case Dyninst::x86_64::iflag3: return 1080;
          case Dyninst::x86_64::iaf: return 1081;
          case Dyninst::x86_64::iflag5: return 1082;
          case Dyninst::x86_64::izf: return 1083;
          case Dyninst::x86_64::isf: return 1084;
          case Dyninst::x86_64::itf: return 1085;
          case Dyninst::x86_64::iif_: return 1086;
          case Dyninst::x86_64::idf: return 1087;
          case Dyninst::x86_64::iof: return 1088;
          case Dyninst::x86_64::iflagc: return 1089;
          case Dyninst::x86_64::iflagd: return 1090;
          case Dyninst::x86_64::int_: return 1091;
          case Dyninst::x86_64::iflagf: return 1092;
          case Dyninst::x86_64::irf: return 1093;
          case Dyninst::x86_64::ivm: return 1094;
          case Dyninst::x86_64::iac: return 1095;
          case Dyninst::x86_64::ivif: return 1096;
          case Dyninst::x86_64::ivip: return 1097;
          case Dyninst::x86_64::iid: return 1098;
          case Dyninst::x86_64::igdtr: return 1099;
          case Dyninst::x86_64::iidtr: return 1100;
          case Dyninst::x86_64::icr0: return 1101;
          case Dyninst::x86_64::icr1: return 1102;
          case Dyninst::x86_64::icr2: return 1103;
          case Dyninst::x86_64::icr3: return 1104;
          case Dyninst::x86_64::icr4: return 1105;
          case Dyninst::x86_64::icr5: return 1106;
          case Dyninst::x86_64::icr6: return 1107;
          case Dyninst::x86_64::icr7: return 1108;
          case Dyninst::x86_64::icr8: return 1109;
          case Dyninst::x86_64::icr9: return 1110;
          case Dyninst::x86_64::icr10: return 1111;
          case Dyninst::x86_64::icr11: return 1112;
          case Dyninst::x86_64::icr12: return 1113;
          case Dyninst::x86_64::icr13: return 1114;
          case Dyninst::x86_64::icr14: return 1115;
          case Dyninst::x86_64::icr15: return 1116;
          case Dyninst::x86_64::idr0: return 1117;
          case Dyninst::x86_64::idr1: return 1118;
          case Dyninst::x86_64::idr2: return 1119;
          case Dyninst::x86_64::idr3: return 1120;
          case Dyninst::x86_64::idr4: return 1121;
          case Dyninst::x86_64::idr5: return 1122;
          case Dyninst::x86_64::idr6: return 1123;
          case Dyninst::x86_64::idr7: return 1124;
          case Dyninst::x86_64::idr8: return 1125;
          case Dyninst::x86_64::idr9: return 1126;
          case Dyninst::x86_64::idr10: return 1127;
          case Dyninst::x86_64::idr11: return 1128;
          case Dyninst::x86_64::idr12: return 1129;
          case Dyninst::x86_64::idr13: return 1130;
          case Dyninst::x86_64::idr14: return 1131;
          case Dyninst::x86_64::idr15: return 1132;
          case Dyninst::x86_64::iymm0: return 1133;
          case Dyninst::x86_64::iymm1: return 1134;
          case Dyninst::x86_64::iymm2: return 1135;
          case Dyninst::x86_64::iymm3: return 1136;
          case Dyninst::x86_64::iymm4: return 1137;
          case Dyninst::x86_64::iymm5: return 1138;
          case Dyninst::x86_64::iymm6: return 1139;
          case Dyninst::x86_64::iymm7: return 1140;
          case Dyninst::x86_64::iymm8: return 1141;
          case Dyninst::x86_64::iymm9: return 1142;
          case Dyninst::x86_64::iymm10: return 1143;
          case Dyninst::x86_64::iymm11: return 1144;
          case Dyninst::x86_64::iymm12: return 1145;
          case Dyninst::x86_64::iymm13: return 1146;
          case Dyninst::x86_64::iymm14: return 1147;
          case Dyninst::x86_64::iymm15: return 1148;
          case Dyninst::x86_64::iymm16: return 1149;
          case Dyninst::x86_64::iymm17: return 1150;
          case Dyninst::x86_64::iymm18: return 1151;
          case Dyninst::x86_64::iymm19: return 1152;
          case Dyninst::x86_64::iymm20: return 1153;
          case Dyninst::x86_64::iymm21: return 1154;
          case Dyninst::x86_64::iymm22: return 1155;
          case Dyninst::x86_64::iymm23: return 1156;
          case Dyninst::x86_64::iymm24: return 1157;
          case Dyninst::x86_64::iymm25: return 1158;
          case Dyninst::x86_64::iymm26: return 1159;
          case Dyninst::x86_64::iymm27: return 1160;
          case Dyninst::x86_64::iymm28: return 1161;
          case Dyninst::x86_64::iymm29: return 1162;
          case Dyninst::x86_64::iymm30: return 1163;
          case Dyninst::x86_64::iymm31: return 1164;
          case Dyninst::x86_64::izmm0: return 1165;
          case Dyninst::x86_64::izmm1: return 1166;
          case Dyninst::x86_64::izmm2: return 1167;
          case Dyninst::x86_64::izmm3: return 1168;
          case Dyninst::x86_64::izmm4: return 1169;
          case Dyninst::x86_64::izmm5: return 1170;
          case Dyninst::x86_64::izmm6: return 1171;
          case Dyninst::x86_64::izmm7: return 1172;
          case Dyninst::x86_64::izmm8: return 1173;
          case Dyninst::x86_64::izmm9: return 1174;
          case Dyninst::x86_64::izmm10: return 1175;
          case Dyninst::x86_64::izmm11: return 1176;
          case Dyninst::x86_64::izmm12: return 1177;
          case Dyninst::x86_64::izmm13: return 1178;
          case Dyninst::x86_64::izmm14: return 1179;
          case Dyninst::x86_64::izmm15: return 1180;
          case Dyninst::x86_64::izmm16: return 1181;
          case Dyninst::x86_64::izmm17: return 1182;
          case Dyninst::x86_64::izmm18: return 1183;
          case Dyninst::x86_64::izmm19: return 1184;
          case Dyninst::x86_64::izmm20: return 1185;
          case Dyninst::x86_64::izmm21: return 1186;
          case Dyninst::x86_64::izmm22: return 1187;
          case Dyninst::x86_64::izmm23: return 1188;
          case Dyninst::x86_64::izmm24: return 1189;
          case Dyninst::x86_64::izmm25: return 1190;
          case Dyninst::x86_64::izmm26: return 1191;
          case Dyninst::x86_64::izmm27: return 1192;
          case Dyninst::x86_64::izmm28: return 1193;
          case Dyninst::x86_64::izmm29: return 1194;
          case Dyninst::x86_64::izmm30: return 1195;
          case Dyninst::x86_64::izmm31: return 1196;
          case Dyninst::x86_64::iorax: return 1197;
          case Dyninst::x86_64::itr0: return 1198;
          case Dyninst::x86_64::itr1: return 1199;
          case Dyninst::x86_64::itr2: return 1200;
          case Dyninst::x86_64::itr3: return 1201;
          case Dyninst::x86_64::itr4: return 1202;
          case Dyninst::x86_64::itr5: return 1203;
          case Dyninst::x86_64::itr6: return 1204;
          case Dyninst::x86_64::itr7: return 1205;
          default: return -1;
        }
        break;
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
          case Dyninst::aarch64::isp: return 31;

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

}
