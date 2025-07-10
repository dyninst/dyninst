#include "rose/registers/convert.h"

#include "dataflowAPI/rose/registers/aarch64.h"
#include "dataflowAPI/rose/registers/amdgpu.h"
#include "dataflowAPI/rose/registers/ppc32.h"
#include "dataflowAPI/rose/registers/ppc64.h"
#include "dataflowAPI/rose/registers/riscv64.h"
#include "dataflowAPI/rose/registers/x86.h"
#include "dataflowAPI/rose/registers/x86_64.h"

#include "dataflowAPI/src/debug_dataflow.h"

#include <tuple>

namespace Dyninst { namespace DataflowAPI {

  rose_reg_raw_t convertToROSERegister(Dyninst::MachRegister reg) {

    /*
     * A ROSE register has a major version, minor version, position, and size.
     *
     * These map to a MachRegister as follows:
     *
     *  category -> major version
     *  baseID   -> minor version
     *  lengthID -> position
     *
     * The major version, register position, and size (in bits) are based on the
     * user-provided register.
     *
     * On most platforms, ROSE only has minor versions for the most-basal registers.
     *
     *  For example, the aarch64 8-bit FPR b0 is mapped to the 128-bit q0 represented
     *  by armv8_simdfpr_v0.
     *
     *  For x86, ROSE sometimes uses the 16-bit names and sometimes the 64-bit names.
     *
     *  For example, the major version for 'rax' is x86_gpr_ax, and the major version
     *  for 'r15' is x86_gpr_r15.
     *
     */
    auto const category = reg.regClass();
    auto const baseID = reg.getBaseRegister().val() & 0x000000ff;
    auto const lengthID = reg.getLengthID();

    // MachRegister::size is in _bytes_
    auto const num_bits = 8*static_cast<int32_t>(reg.size());

    // A RegisterDescriptor is invalid if it has no bits
    auto const INVALID_REG = std::make_tuple(0,0,0,0);

    switch(reg.getArchitecture()) {
      case Arch_amdgpu_gfx908: {
        return AmdgpuGfx908Rose(category, baseID, lengthID, num_bits);
      }
      case Arch_amdgpu_gfx90a: {
        return AmdgpuGfx90aRose(category, baseID, lengthID, num_bits);
      }
      case Arch_amdgpu_gfx940: {
        return AmdgpuGfx940Rose(category, baseID, lengthID, num_bits);
      }
      case Arch_x86: {
        if(reg.isPC()) {
          // ROSE docs: only minor value allowed is 0
          return std::make_tuple(x86_regclass_ip, 0, x86_regpos_dword, num_bits);
        }
        if(reg.isFlag()) {
          // Split the flag register into its parts
          auto const id = reg.val() & 0x000000ff;
          return x86Rose(category, id, lengthID, num_bits);
        }
        return x86Rose(category, baseID, lengthID, num_bits);
      }
      case Arch_x86_64: {
        if(reg.isPC()) {
          auto const pos = (reg == Dyninst::x86_64::eip) ? x86_regpos_dword : x86_regpos_qword;
          // ROSE docs: only minor value allowed is 0
          return std::make_tuple(x86_regclass_ip, 0, pos, num_bits);
        }
        if(reg.isFlag()) {
          // Split the flag register into its parts
          auto const id = reg.val() & 0x000000ff;
          return x8664Rose(category, id, lengthID, num_bits);
        }
        return x8664Rose(category, baseID, lengthID, num_bits);
      }
      case Arch_ppc32: {
        return ppc32Rose(category, reg, num_bits);
      }
      case Arch_ppc64: {
        return ppc64Rose(category, reg, num_bits);
      }
      case Arch_aarch64: {
        // ROSE doesn't have register numbers for PC
        if(reg.isPC()) {
          // ROSE docs: only minor value allowed is 0
          return std::make_tuple(armv8_regclass_pc, 0, 0, num_bits);
        }
        if(reg.getBaseRegister() == Dyninst::aarch64::xzr) {
          // ROSE treats the zero register as a GPR
          return std::make_tuple(armv8_regclass_gpr, armv8_gpr_zr, 0, num_bits);
        }
        if(reg.isStackPointer()) {
          // ROSE docs: only minor value allowed is 0
          return std::make_tuple(armv8_regclass_sp, 0, 0, num_bits);
        }
        if(reg.isFlag()) {
          // Use the given register rather than the bas to preserve the
          // individual flags as separate registers
          auto const id = reg.val() & 0x000000ff;
          return aarch64Rose(category, id, lengthID, num_bits);
        }
        return aarch64Rose(category, baseID, lengthID, num_bits);
      }
      case Arch_riscv64: {
        if(reg.isPC()) {
          return std::make_tuple(riscv64_regclass_pc, 0, 0, num_bits);
        }
        return riscv64Rose(category, baseID, num_bits);
      }
      case Arch_aarch32:
      case Arch_cuda:
      case Arch_intelGen9:
      case Arch_none:
        // Set these output variable to invalid values and let the
        // semantics code to throw exceptions
        convert_printf("No ROSE register for architecture 0x%X\n", reg.getArchitecture());
        return INVALID_REG;
    }
    convert_printf("Unknown Architecture 0x%X\n", reg.getArchitecture());
    return INVALID_REG;
  }

}}
