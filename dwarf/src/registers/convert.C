#include "dwarf/src/registers/convert.h"
#include "dwarf/src/registers/aarch64.h"
#include "dwarf/src/registers/riscv64.h"
#include "dwarf/src/registers/ppc32.h"
#include "dwarf/src/registers/ppc64.h"
#include "dwarf/src/registers/x86.h"
#include "dwarf/src/registers/x86_64.h"

#include "debug_common.h"
#include "registers/abstract_regs.h"

namespace Dyninst {
namespace DwarfDyninst {

  Dyninst::MachRegister encoding_to_reg(int encoding, Dyninst::Architecture arch) {

    switch(arch) {
      case Dyninst::Arch_x86:
        return x86_from_dwarf(encoding);

      case Dyninst::Arch_x86_64:
        return x8664_from_dwarf(encoding);

      case Dyninst::Arch_ppc32:
        return ppc32_from_dwarf(encoding);

      case Dyninst::Arch_ppc64:
        return ppc64_from_dwarf(encoding);

      case Dyninst::Arch_aarch64:
        return aarch64_from_dwarf(encoding);

      case Dyninst::Arch_riscv64:
        return riscv64_from_dwarf(encoding);

      case Dyninst::Arch_aarch32:
      case Dyninst::Arch_none:
      case Dyninst::Arch_cuda:
      case Dyninst::Arch_amdgpu_gfx908:
      case Dyninst::Arch_amdgpu_gfx90a:
      case Dyninst::Arch_amdgpu_gfx940:
      case Dyninst::Arch_intelGen9: {
        dwarf_printf("No known register for '%d' on '%u'\n", encoding, arch);
        return Dyninst::InvalidReg;
      }
    }
    dwarf_printf("No known register for '%d' on '%u'\n", encoding, arch);
    return Dyninst::InvalidReg;
  }

  int register_to_dwarf(Dyninst::MachRegister reg) {

    switch(reg.getArchitecture()) {
      case Dyninst::Arch_x86:
        return x86_to_dwarf(reg);

      case Dyninst::Arch_x86_64:
        return x8664_to_dwarf(reg);

      case Dyninst::Arch_ppc32:
        return ppc32_to_dwarf(reg);

      case Dyninst::Arch_ppc64:
        return ppc64_to_dwarf(reg);

      case Dyninst::Arch_aarch64:
        return aarch64_to_dwarf(reg);

      case Dyninst::Arch_riscv64:
        return riscv64_to_dwarf(reg);

      case Dyninst::Arch_aarch32:
      case Dyninst::Arch_none:
      case Dyninst::Arch_cuda:
      case Dyninst::Arch_amdgpu_gfx908:
      case Dyninst::Arch_amdgpu_gfx90a:
      case Dyninst::Arch_amdgpu_gfx940:
      case Dyninst::Arch_intelGen9: {
        dwarf_printf("No known DWARF encoding for '%s'\n", reg.name().c_str());
        return Dyninst::InvalidReg;
      }
    }
    dwarf_printf("No known DWARF encoding for '%s'\n", reg.name().c_str());
    return -1;
  }

}}
