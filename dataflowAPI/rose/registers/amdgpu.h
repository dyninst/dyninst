/*
 * See the dyninst/COPYRIGHT file for copyright information.
 *
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 *
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef DYNINST_COMMON_REGISTERS_ROSE_AMDGPU_H
#define DYNINST_COMMON_REGISTERS_ROSE_AMDGPU_H

#include "external/rose/amdgpuInstructionEnum.h"
#include "registers/AMDGPU/amdgpu_gfx908_regs.h"
#include "registers/AMDGPU/amdgpu_gfx90a_regs.h"
#include "registers/AMDGPU/amdgpu_gfx940_regs.h"
#include "dataflowAPI/src/debug_dataflow.h"

#include <tuple>

namespace {

  std::tuple<AMDGPURegisterClass, int, int, int>
  AmdgpuGfx908Rose(int32_t category, int32_t baseID, int32_t, int32_t size) {
    constexpr auto pos = 0;

    switch(category) {
      case Dyninst::amdgpu_gfx908::SGPR: {
        auto const reg_idx = static_cast<AMDGPUScalarGeneralPurposeRegister>(baseID);
        return std::make_tuple(amdgpu_regclass_sgpr, reg_idx, pos, size);
      }

      case Dyninst::amdgpu_gfx908::VGPR: {
        auto const reg_idx = static_cast<AMDGPUVectorGeneralPurposeRegister>(baseID);
        return std::make_tuple(amdgpu_regclass_vgpr, reg_idx, pos, size);
      }

      case Dyninst::amdgpu_gfx908::PC: {
        return std::make_tuple(amdgpu_regclass_pc, amdgpu_pc, pos, size);
      }

      case Dyninst::amdgpu_gfx908::MISC: {
        auto const reg_idx = static_cast<AMDGPUMiscRegister>(baseID);
        return std::make_tuple(amdgpu_regclass_misc, reg_idx, pos, size);
      }

    }
    convert_printf("Unknown AmdgpuGfx908 category '%d'\n", category);
    return std::make_tuple(amdgpu_regclass_unknown, baseID, pos, 0);
  }

  std::tuple<AMDGPURegisterClass, int, int, int>
  AmdgpuGfx90aRose(int32_t category, int32_t baseID, int32_t, int32_t size) {
    constexpr auto pos = 0;

    switch(category) {
      case Dyninst::amdgpu_gfx90a::SGPR: {
        auto const reg_idx = static_cast<AMDGPUScalarGeneralPurposeRegister>(baseID);
        return std::make_tuple(amdgpu_regclass_sgpr, reg_idx, pos, size);
      }

      case Dyninst::amdgpu_gfx90a::VGPR: {
        auto const reg_idx = static_cast<AMDGPUVectorGeneralPurposeRegister>(baseID);
        return std::make_tuple(amdgpu_regclass_vgpr, reg_idx, pos, size);
      }

      case Dyninst::amdgpu_gfx90a::PC: {
        return std::make_tuple(amdgpu_regclass_pc, amdgpu_pc, pos, size);
      }

      case Dyninst::amdgpu_gfx90a::MISC: {
        auto const reg_idx = static_cast<AMDGPUMiscRegister>(baseID);
        return std::make_tuple(amdgpu_regclass_misc, reg_idx, pos, size);
      }
    }
    convert_printf("Unknown AmdgpuGfx90a category '%d'\n", category);
    return std::make_tuple(amdgpu_regclass_unknown, baseID, pos, 0);
  }

  std::tuple<AMDGPURegisterClass, int, int, int>
  AmdgpuGfx940Rose(int32_t category, int32_t baseID, int32_t, int32_t size) {
    constexpr auto pos = 0;

    switch(category) {
      case Dyninst::amdgpu_gfx940::SGPR: {
        auto const reg_idx = static_cast<AMDGPUScalarGeneralPurposeRegister>(baseID);
        return std::make_tuple(amdgpu_regclass_sgpr, reg_idx, pos, size);
        break;
      }

      case Dyninst::amdgpu_gfx940::VGPR: {
        auto const reg_idx = static_cast<AMDGPUVectorGeneralPurposeRegister>(baseID);
        return std::make_tuple(amdgpu_regclass_vgpr, reg_idx, pos, size);
      }

      case Dyninst::amdgpu_gfx940::PC: {
        return std::make_tuple(amdgpu_regclass_pc, amdgpu_pc, pos, size);
      }

      case Dyninst::amdgpu_gfx940::MISC: {
        auto const reg_idx = static_cast<AMDGPUMiscRegister>(baseID);
        return std::make_tuple(amdgpu_regclass_misc, reg_idx, pos, size);
      }
    }
    convert_printf("Unknown AmdgpuGfx940 category '%d'\n", category);
    return std::make_tuple(amdgpu_regclass_unknown, baseID, pos, 0);
  }

}

#endif
