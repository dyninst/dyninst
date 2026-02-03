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

#include "external/amdgpu/AMDGPUEFlags.h"
#include "AmdgpuKernelDescriptor.h"
#include "unaligned_memory_access.h"

#include <cassert>
#include <cstring>
#include <iomanip>

using namespace llvm;
using namespace amdhsa;

namespace Dyninst {

bool AmdgpuKernelDescriptor::isGfx6() const {
  return (amdgpuMach >= EF_AMDGPU_MACH_AMDGCN_GFX600 &&
          amdgpuMach <= EF_AMDGPU_MACH_AMDGCN_GFX601) ||
         amdgpuMach == EF_AMDGPU_MACH_AMDGCN_GFX602;
}

bool AmdgpuKernelDescriptor::isGfx7() const {
  return (amdgpuMach >= EF_AMDGPU_MACH_AMDGCN_GFX700 &&
          amdgpuMach <= EF_AMDGPU_MACH_AMDGCN_GFX704) ||
         amdgpuMach == EF_AMDGPU_MACH_AMDGCN_GFX705;
}

bool AmdgpuKernelDescriptor::isGfx8() const {
  return (amdgpuMach >= EF_AMDGPU_MACH_AMDGCN_GFX801 &&
          amdgpuMach <= EF_AMDGPU_MACH_AMDGCN_GFX810) ||
         amdgpuMach == EF_AMDGPU_MACH_AMDGCN_GFX805;
}

bool AmdgpuKernelDescriptor::isGfx9() const {
  return isGfx90aOr942() ||
         (amdgpuMach >= EF_AMDGPU_MACH_AMDGCN_GFX900 && amdgpuMach <= EF_AMDGPU_MACH_AMDGCN_GFX90C);
}

bool AmdgpuKernelDescriptor::isGfx90aOr942() const {
  return amdgpuMach >= EF_AMDGPU_MACH_AMDGCN_GFX90A && amdgpuMach <= EF_AMDGPU_MACH_AMDGCN_GFX942;
}

bool AmdgpuKernelDescriptor::isGfx9Plus() const { return isGfx9() || isGfx10Plus(); }

bool AmdgpuKernelDescriptor::isGfx10() const {
  return (amdgpuMach >= EF_AMDGPU_MACH_AMDGCN_GFX1010 &&
          amdgpuMach <= EF_AMDGPU_MACH_AMDGCN_GFX1033) ||
         amdgpuMach == EF_AMDGPU_MACH_AMDGCN_GFX1013 ||
         amdgpuMach == EF_AMDGPU_MACH_AMDGCN_GFX1034 ||
         amdgpuMach == EF_AMDGPU_MACH_AMDGCN_GFX1035 || amdgpuMach == EF_AMDGPU_MACH_AMDGCN_GFX1036;
}

bool AmdgpuKernelDescriptor::isGfx10Plus() const { return isGfx10() || isGfx11(); }

bool AmdgpuKernelDescriptor::isGfx11() const {
  return amdgpuMach == EF_AMDGPU_MACH_AMDGCN_GFX1100 ||
         amdgpuMach == EF_AMDGPU_MACH_AMDGCN_GFX1103 ||
         (amdgpuMach >= EF_AMDGPU_MACH_AMDGCN_GFX1101 &&
          amdgpuMach >= EF_AMDGPU_MACH_AMDGCN_GFX1102);
}

AmdgpuKernelDescriptor::AmdgpuKernelDescriptor(uint8_t *kdBytes, size_t kdSize, unsigned amdgpuMachine) {
  assert(kdSize == sizeof(kernel_descriptor_t));
  kdRepr = read_memory_as<kernel_descriptor_t>(kdBytes);
  amdgpuMach = amdgpuMachine;
}

uint32_t AmdgpuKernelDescriptor::getGroupSegmentFixedSize() const {
  return kdRepr.group_segment_fixed_size;
}

void AmdgpuKernelDescriptor::setGroupSegmentFixedSize(uint32_t value) {
  kdRepr.group_segment_fixed_size = value;
}

uint32_t AmdgpuKernelDescriptor::getPrivateSegmentFixedSize() const {
  return kdRepr.private_segment_fixed_size;
}

void AmdgpuKernelDescriptor::setPrivateSegmentFixedSize(uint32_t value) {
  kdRepr.private_segment_fixed_size = value;
}

uint32_t AmdgpuKernelDescriptor::getKernargSize() const { return kdRepr.kernarg_size; }

void AmdgpuKernelDescriptor::setKernargSize(uint32_t value) { kdRepr.kernarg_size = value; }

int64_t AmdgpuKernelDescriptor::getKernelCodeEntryByteOffset() const {
  return kdRepr.kernel_code_entry_byte_offset;
}

void AmdgpuKernelDescriptor::setKernelCodeEntryByteOffset(int64_t value) {
  kdRepr.kernel_code_entry_byte_offset = value;
}

#define GET_VALUE(MASK) ((fourByteBuffer & MASK) >> (MASK##_SHIFT))

static inline uint32_t insertField32(uint32_t op, uint32_t value, uint32_t mask, uint32_t width, uint32_t shift) {
  assert(((value >> width) == 0) && "value contains more bits than specified");
  op &= ~mask;
  op |= value << shift;
  return op;
}

#define INSERT_FIELD_32(op, value, mask) insertField32(op, value, mask, mask##_WIDTH, mask##_SHIFT)

// ----- COMPUTE_PGM_RSRC3 begin -----
//
//
uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC3() const { return kdRepr.compute_pgm_rsrc3; }

// GFX90A, GFX942 begin
uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC3_AccumOffset() const {
  assert(isGfx90aOr942());
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc3;
  return GET_VALUE(COMPUTE_PGM_RSRC3_GFX90A_ACCUM_OFFSET);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC3_AccumOffset(uint32_t value) {
  assert(isGfx90aOr942());
  kdRepr.compute_pgm_rsrc3 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc3, value, COMPUTE_PGM_RSRC3_GFX90A_ACCUM_OFFSET);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC3_TgSplit() const {
  assert(isGfx90aOr942());
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc3;
  return GET_VALUE(COMPUTE_PGM_RSRC3_GFX90A_TG_SPLIT);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC3_TgSplit(bool value) {
  assert(isGfx90aOr942());
  kdRepr.compute_pgm_rsrc3 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc3, value, COMPUTE_PGM_RSRC3_GFX90A_TG_SPLIT);
}
//
// GFX90A, GFX942 end

// GFX10, GFX11 begin
uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC3_SharedVgprCount() const {
  assert(isGfx10Plus());
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc3;
  return GET_VALUE(COMPUTE_PGM_RSRC3_GFX10_PLUS_SHARED_VGPR_COUNT);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC3_SharedVgprCount(uint32_t value) {
  assert(isGfx10Plus());
  kdRepr.compute_pgm_rsrc3 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc3, value, COMPUTE_PGM_RSRC3_GFX10_PLUS_SHARED_VGPR_COUNT);
}

uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC3_InstPrefSize() const {
  assert(isGfx10Plus());
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc3;
  return GET_VALUE(COMPUTE_PGM_RSRC3_GFX10_PLUS_INST_PREF_SIZE);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC3_InstPrefSize(uint32_t value) {
  assert(isGfx11());
  kdRepr.compute_pgm_rsrc3 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc3, value, COMPUTE_PGM_RSRC3_GFX10_PLUS_INST_PREF_SIZE);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC3_TrapOnStart() const {
  assert(isGfx10Plus());
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc3;
  return GET_VALUE(COMPUTE_PGM_RSRC3_GFX10_PLUS_TRAP_ON_START);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC3_TrapOnStart(bool value) {
  assert(isGfx11());
  kdRepr.compute_pgm_rsrc3 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc3, value, COMPUTE_PGM_RSRC3_GFX10_PLUS_TRAP_ON_START);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC3_TrapOnEnd() const {
  assert(isGfx10Plus());
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc3;
  return GET_VALUE(COMPUTE_PGM_RSRC3_GFX10_PLUS_TRAP_ON_END);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC3_TrapOnEnd(bool value) {
  assert(isGfx11());
  kdRepr.compute_pgm_rsrc3 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc3, value, COMPUTE_PGM_RSRC3_GFX10_PLUS_TRAP_ON_END);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC3_ImageOp() const {
  assert(isGfx10Plus());
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc3;
  return GET_VALUE(COMPUTE_PGM_RSRC3_GFX10_PLUS_IMAGE_OP);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC3_ImageOp(bool value) {
  assert(isGfx11());
  kdRepr.compute_pgm_rsrc3 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc3, value, COMPUTE_PGM_RSRC3_GFX10_PLUS_IMAGE_OP);
}
//
// GFX10, GFX11 end
//
// ----- COMPUTE_PGM_RSRC3 end -----

// ----- COMPUTE_PGM_RSRC1 begin -----
//
//
uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1() const { return kdRepr.compute_pgm_rsrc1; }

uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_GranulatedWorkitemVgprCount() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_GRANULATED_WORKITEM_VGPR_COUNT);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC1_GranulatedWorkitemVgprCount(uint32_t value) {
  kdRepr.compute_pgm_rsrc1 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc1, value, COMPUTE_PGM_RSRC1_GRANULATED_WORKITEM_VGPR_COUNT);
}

uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_GranulatedWavefrontSgprCount() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_GRANULATED_WAVEFRONT_SGPR_COUNT);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC1_GranulatedWavefrontSgprCount(uint32_t value) {
  kdRepr.compute_pgm_rsrc1 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc1, value, COMPUTE_PGM_RSRC1_GRANULATED_WAVEFRONT_SGPR_COUNT);
}

uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_Priority() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_PRIORITY);
}

uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_FloatRoundMode32() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_FLOAT_ROUND_MODE_32);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC1_FloatRoundMode32(uint32_t value) {
  kdRepr.compute_pgm_rsrc1 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc1, value, COMPUTE_PGM_RSRC1_FLOAT_ROUND_MODE_32);
}

uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_FloatRoundMode1664() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_FLOAT_ROUND_MODE_16_64);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC1_FloatRoundMode1664(uint32_t value) {
  kdRepr.compute_pgm_rsrc1 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc1, value, COMPUTE_PGM_RSRC1_FLOAT_ROUND_MODE_16_64);
}

uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_FloatDenormMode32() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_FLOAT_DENORM_MODE_32);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC1_FloatDenormMode32(uint32_t value) {
  kdRepr.compute_pgm_rsrc1 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc1, value, COMPUTE_PGM_RSRC1_FLOAT_DENORM_MODE_32);
}

uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_FloatDenormMode1664() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_FLOAT_DENORM_MODE_16_64);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC1_FloatDenormMode1664(uint32_t value) {
  kdRepr.compute_pgm_rsrc1 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc1, value, COMPUTE_PGM_RSRC1_FLOAT_DENORM_MODE_16_64);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_Priv() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_PRIORITY);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_EnableDx10Clamp() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_ENABLE_DX10_CLAMP);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC1_EnableDx10Clamp(bool value) {
  kdRepr.compute_pgm_rsrc1 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc1, value, COMPUTE_PGM_RSRC1_ENABLE_DX10_CLAMP);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_DebugMode() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_DEBUG_MODE);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_EnableIeeeMode() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_ENABLE_IEEE_MODE);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC1_EnableIeeeMode(bool value) {
  kdRepr.compute_pgm_rsrc1 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc1, value, COMPUTE_PGM_RSRC1_ENABLE_IEEE_MODE);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_Bulky() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_BULKY);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_CdbgUser() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_CDBG_USER);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_Fp16Ovfl() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_FP16_OVFL);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC1_Fp16Ovfl(bool value) {
  assert(isGfx9Plus());
  kdRepr.compute_pgm_rsrc1 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc1, value, COMPUTE_PGM_RSRC1_FP16_OVFL);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_WgpMode() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_WGP_MODE);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC1_WgpMode(bool value) {
  assert(isGfx10Plus());
  kdRepr.compute_pgm_rsrc1 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc1, value, COMPUTE_PGM_RSRC1_WGP_MODE);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_MemOrdered() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_MEM_ORDERED);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC1_MemOrdered(bool value) {
  assert(isGfx10Plus());
  kdRepr.compute_pgm_rsrc1 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc1, value, COMPUTE_PGM_RSRC1_MEM_ORDERED);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC1_FwdProgress() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc1;
  return GET_VALUE(COMPUTE_PGM_RSRC1_FWD_PROGRESS);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC1_FwdProgress(bool value) {
  assert(isGfx10Plus());
  kdRepr.compute_pgm_rsrc1 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc1, value, COMPUTE_PGM_RSRC1_FWD_PROGRESS);
}

//
// ----- COMPUTE_PGM_RSRC1 end -----

// ----- COMPUTE_PGM_RSRC2 begin -----
//
uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2() const { return kdRepr.compute_pgm_rsrc2; }

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnablePrivateSegment() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_PRIVATE_SEGMENT);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC2_EnablePrivateSegment(bool value) {
  kdRepr.compute_pgm_rsrc2 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc2, value, COMPUTE_PGM_RSRC2_ENABLE_PRIVATE_SEGMENT);
}

uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_UserSgprCount() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_USER_SGPR_COUNT);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC2_UserSgprCount(uint32_t value) {
  kdRepr.compute_pgm_rsrc2 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc2, value, COMPUTE_PGM_RSRC2_USER_SGPR_COUNT);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableTrapHandler() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_TRAP_HANDLER);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdX() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_SGPR_WORKGROUP_ID_X);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdX(bool value) {
  kdRepr.compute_pgm_rsrc2 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc2, value, COMPUTE_PGM_RSRC2_ENABLE_SGPR_WORKGROUP_ID_X);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdY() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_SGPR_WORKGROUP_ID_Y);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdY(bool value) {
  kdRepr.compute_pgm_rsrc2 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc2, value, COMPUTE_PGM_RSRC2_ENABLE_SGPR_WORKGROUP_ID_Y);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdZ() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_SGPR_WORKGROUP_ID_Z);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdZ(bool value) {
  kdRepr.compute_pgm_rsrc2 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc2, value, COMPUTE_PGM_RSRC2_ENABLE_SGPR_WORKGROUP_ID_Z);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupInfo() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_SGPR_WORKGROUP_INFO);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupInfo(bool value) {
  kdRepr.compute_pgm_rsrc2 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc2, value, COMPUTE_PGM_RSRC2_ENABLE_SGPR_WORKGROUP_INFO);
}

uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableVgprWorkitemId() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_VGPR_WORKITEM_ID);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC2_EnableVgprWorkitemId(uint32_t value) {
  kdRepr.compute_pgm_rsrc2 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc2, value, COMPUTE_PGM_RSRC2_ENABLE_VGPR_WORKITEM_ID);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableExceptionAddressWatch() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_ADDRESS_WATCH);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableExceptionMemory() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_MEMORY);
}

uint32_t AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_GranulatedLdsSize() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_GRANULATED_LDS_SIZE);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpInvalidOperation() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_IEEE_754_FP_INVALID_OPERATION);
}

void AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpInvalidOperation(
    bool value) {
  kdRepr.compute_pgm_rsrc2 = INSERT_FIELD_32(kdRepr.compute_pgm_rsrc2, value,
                                             COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_IEEE_754_FP_INVALID_OPERATION);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableExceptionFpDenormalSource() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_FP_DENORMAL_SOURCE);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC2_EnableExceptionFpDenormalSource(bool value) {
  kdRepr.compute_pgm_rsrc2 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc2, value, COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_FP_DENORMAL_SOURCE);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpDivisionByZero() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_IEEE_754_FP_DIVISION_BY_ZERO);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpDivisionByZero(
    bool value) {
  kdRepr.compute_pgm_rsrc2 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc2, value, COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_IEEE_754_FP_DIVISION_BY_ZERO);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpOverflow() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_IEEE_754_FP_OVERFLOW);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpOverflow(bool value) {
  kdRepr.compute_pgm_rsrc2 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc2, value, COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_IEEE_754_FP_OVERFLOW);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpUnderflow() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_IEEE_754_FP_UNDERFLOW);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpUnderflow(bool value) {
  kdRepr.compute_pgm_rsrc2 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc2, value, COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_IEEE_754_FP_UNDERFLOW);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpInexact() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_IEEE_754_FP_INEXACT);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpInexact(bool value) {
  kdRepr.compute_pgm_rsrc2 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc2, value, COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_IEEE_754_FP_INEXACT);
}

bool AmdgpuKernelDescriptor::getCOMPUTE_PGM_RSRC2_EnableExceptionIntDivideByZero() const {
  const uint32_t fourByteBuffer = kdRepr.compute_pgm_rsrc2;
  return GET_VALUE(COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_INT_DIVIDE_BY_ZERO);
}

void AmdgpuKernelDescriptor::setCOMPUTE_PGM_RSRC2_EnableExceptionIntDivideByZero(bool value) {
  kdRepr.compute_pgm_rsrc2 =
      INSERT_FIELD_32(kdRepr.compute_pgm_rsrc2, value, COMPUTE_PGM_RSRC2_ENABLE_EXCEPTION_INT_DIVIDE_BY_ZERO);
}
//
// ----- COMPUTE_PGM_RSRC2 end -----
//

#undef GET_VALUE

#define GET_VALUE(MASK) ((twoByteBuffer & (MASK)) >> (MASK##_SHIFT))

static inline uint16_t insertField16(uint16_t op, uint16_t value, uint16_t mask, uint16_t width, uint16_t shift) {
  assert(((value >> width) == 0) && "value contains more bits than specified");
  op &= ~mask;
  op |= value << shift;
  return op;
}

#define INSERT_FIELD_16(op, value, mask) insertField16(op, value, mask, mask##_WIDTH, mask##_SHIFT)

// ----- KERNEL_CODE_PROPERTIES begin -----
//
bool AmdgpuKernelDescriptor::getKernelCodeProperty_EnableSgprPrivateSegmentBuffer() const {
  uint16_t twoByteBuffer = kdRepr.kernel_code_properties;
  return GET_VALUE(KERNEL_CODE_PROPERTY_ENABLE_SGPR_PRIVATE_SEGMENT_BUFFER);
}

void AmdgpuKernelDescriptor::setKernelCodeProperty_EnableSgprPrivateSegmentBuffer(bool value) {
  kdRepr.kernel_code_properties =
      INSERT_FIELD_16(kdRepr.kernel_code_properties, value, KERNEL_CODE_PROPERTY_ENABLE_SGPR_PRIVATE_SEGMENT_BUFFER);
}

bool AmdgpuKernelDescriptor::getKernelCodeProperty_EnableSgprDispatchPtr() const {
  const uint16_t twoByteBuffer = kdRepr.kernel_code_properties;
  return GET_VALUE(KERNEL_CODE_PROPERTY_ENABLE_SGPR_DISPATCH_PTR);
}

void AmdgpuKernelDescriptor::setKernelCodeProperty_EnableSgprDispatchPtr(bool value) {
  kdRepr.kernel_code_properties =
      INSERT_FIELD_16(kdRepr.kernel_code_properties, value, KERNEL_CODE_PROPERTY_ENABLE_SGPR_DISPATCH_PTR);
}

bool AmdgpuKernelDescriptor::getKernelCodeProperty_EnableSgprQueuePtr() const {
  const uint16_t twoByteBuffer = kdRepr.kernel_code_properties;
  return GET_VALUE(KERNEL_CODE_PROPERTY_ENABLE_SGPR_QUEUE_PTR);
}

void AmdgpuKernelDescriptor::setKernelCodeProperty_EnableSgprQueuePtr(bool value) {
  kdRepr.kernel_code_properties =
      INSERT_FIELD_16(kdRepr.kernel_code_properties, value, KERNEL_CODE_PROPERTY_ENABLE_SGPR_QUEUE_PTR);
}

bool AmdgpuKernelDescriptor::getKernelCodeProperty_EnableSgprKernargSegmentPtr() const {
  const uint16_t twoByteBuffer = kdRepr.kernel_code_properties;
  return GET_VALUE(KERNEL_CODE_PROPERTY_ENABLE_SGPR_KERNARG_SEGMENT_PTR);
}

void AmdgpuKernelDescriptor::setKernelCodeProperty_EnableSgprKernargSegmentPtr(bool value) {
  kdRepr.kernel_code_properties =
      INSERT_FIELD_16(kdRepr.kernel_code_properties, value, KERNEL_CODE_PROPERTY_ENABLE_SGPR_KERNARG_SEGMENT_PTR);
}

bool AmdgpuKernelDescriptor::getKernelCodeProperty_EnableSgprDispatchId() const {
  const uint16_t twoByteBuffer = kdRepr.kernel_code_properties;
  return GET_VALUE(KERNEL_CODE_PROPERTY_ENABLE_SGPR_DISPATCH_ID);
}

void AmdgpuKernelDescriptor::setKernelCodeProperty_EnableSgprDispatchId(bool value) {
  kdRepr.kernel_code_properties =
      INSERT_FIELD_16(kdRepr.kernel_code_properties, value, KERNEL_CODE_PROPERTY_ENABLE_SGPR_DISPATCH_ID);
}

bool AmdgpuKernelDescriptor::getKernelCodeProperty_EnableSgprFlatScratchInit() const {
  const uint16_t twoByteBuffer = kdRepr.kernel_code_properties;
  return GET_VALUE(KERNEL_CODE_PROPERTY_ENABLE_SGPR_FLAT_SCRATCH_INIT);
}

void AmdgpuKernelDescriptor::setKernelCodeProperty_EnableSgprFlatScratchInit(bool value) {
  kdRepr.kernel_code_properties =
      INSERT_FIELD_16(kdRepr.kernel_code_properties, value, KERNEL_CODE_PROPERTY_ENABLE_SGPR_FLAT_SCRATCH_INIT);
}

bool AmdgpuKernelDescriptor::getKernelCodeProperty_EnablePrivateSegmentSize() const {
  const uint16_t twoByteBuffer = kdRepr.kernel_code_properties;
  return GET_VALUE(KERNEL_CODE_PROPERTY_ENABLE_SGPR_PRIVATE_SEGMENT_SIZE);
}

void AmdgpuKernelDescriptor::setKernelCodeProperty_EnablePrivateSegmentSize(bool value) {
  kdRepr.kernel_code_properties =
      INSERT_FIELD_16(kdRepr.kernel_code_properties, value, KERNEL_CODE_PROPERTY_ENABLE_SGPR_PRIVATE_SEGMENT_SIZE);
}

bool AmdgpuKernelDescriptor::getKernelCodeProperty_EnableWavefrontSize32() const {
  const uint16_t twoByteBuffer = kdRepr.kernel_code_properties;
  return GET_VALUE(KERNEL_CODE_PROPERTY_ENABLE_WAVEFRONT_SIZE32);
}

void AmdgpuKernelDescriptor::setKernelCodeProperty_EnableWavefrontSize32(bool value) {
  assert(isGfx10Plus());
  kdRepr.kernel_code_properties =
      INSERT_FIELD_16(kdRepr.kernel_code_properties, value, KERNEL_CODE_PROPERTY_ENABLE_WAVEFRONT_SIZE32);
}

bool AmdgpuKernelDescriptor::getKernelCodeProperty_UsesDynamicStack() const {
  const uint16_t twoByteBuffer = kdRepr.kernel_code_properties;
  return GET_VALUE(KERNEL_CODE_PROPERTY_USES_DYNAMIC_STACK);
}

void AmdgpuKernelDescriptor::setKernelCodeProperty_UsesDynamicStack(bool value) {
  kdRepr.kernel_code_properties =
      INSERT_FIELD_16(kdRepr.kernel_code_properties, value, KERNEL_CODE_PROPERTY_USES_DYNAMIC_STACK);
}

bool AmdgpuKernelDescriptor::supportsArchitectedFlatScratch() const {
  switch (amdgpuMach) {
  case EF_AMDGPU_MACH_AMDGCN_GFX942:
  case EF_AMDGPU_MACH_AMDGCN_GFX1100:
  case EF_AMDGPU_MACH_AMDGCN_GFX1101:
  case EF_AMDGPU_MACH_AMDGCN_GFX1102:
  case EF_AMDGPU_MACH_AMDGCN_GFX1103:
    return true;
  default:
    return false;
  }
}

//
// ----- KERNEL_CODE_PROPERTIES end -----
//

#undef GET_VALUE

unsigned AmdgpuKernelDescriptor::getKernargPtrRegister() {
  unsigned kernargPtrReg = 0;
  if (this->getKernelCodeProperty_EnableSgprPrivateSegmentBuffer()) {
    kernargPtrReg += 4;
  }

  if (this->getKernelCodeProperty_EnableSgprDispatchPtr()) {
    kernargPtrReg += 2;
  }

  if (this->getKernelCodeProperty_EnableSgprQueuePtr()) {
    kernargPtrReg += 2;
  }

  return kernargPtrReg;
}

void AmdgpuKernelDescriptor::dump(std::ostream &os) const {
  os << name << '\n';
  os << std::hex;
  os << "GROUP_SEGMENT_FIXED_SIZE : "
     << "0x" << kdRepr.group_segment_fixed_size << '\n';
  os << "PRIVATE_SEGMENT_FIXED_SIZE : "
     << "0x" << kdRepr.private_segment_fixed_size << '\n';
  os << "KERNARG_SIZE : "
     << "0x" << kdRepr.kernarg_size << '\n';
  os << "KERNEL_CODE_ENTRY_BYTE_OFFSET : "
     << "0x" << kdRepr.kernel_code_entry_byte_offset << '\n';
  os << "COMPUTE_PGM_RSRC3 : "
     << "0x" << kdRepr.compute_pgm_rsrc3 << '\n';
  os << "COMPUTE_PGM_RSRC1 : "
     << "0x" << kdRepr.compute_pgm_rsrc1 << '\n';
  os << "COMPUTE_PGM_RSRC2 : "
     << "0x" << kdRepr.compute_pgm_rsrc2 << '\n';
  os << "KERNEL_CODE_PROPERTIES : " << kdRepr.kernel_code_properties << '\n';
  os << std::dec;
}

void AmdgpuKernelDescriptor::dumpDetailed(std::ostream &os) const {
  const char *space = "    ";
  const char *indent = "  ";

  os << "----- detailed dump for " << name << "  begin ---- \n\n";

  os << indent << "-- GROUP_SEGMENT_FIXED_SIZE : " << std::hex << "0x"
     << kdRepr.group_segment_fixed_size << space << std::dec << kdRepr.group_segment_fixed_size
     << '\n'
     << '\n';

  os << indent << "-- PRIVATE_SEGMENT_FIXED_SIZE : " << std::hex << "0x"
     << kdRepr.private_segment_fixed_size << space << std::dec << kdRepr.private_segment_fixed_size
     << '\n'
     << '\n';

  os << indent << "-- KERNARG_SIZE : " << std::hex << "0x" << kdRepr.kernarg_size << space
     << std::dec << kdRepr.kernarg_size << '\n'
     << '\n';

  os << indent << "-- KERNEL_CODE_ENTRY_BYTE_OFFSET : " << std::hex << "0x"
     << kdRepr.kernel_code_entry_byte_offset << space << std::dec
     << kdRepr.kernel_code_entry_byte_offset << '\n'
     << '\n';

  dumpCOMPUTE_PGM_RSRC3(os);
  os << '\n';

  dumpCOMPUTE_PGM_RSRC1(os);
  os << '\n';

  dumpCOMPUTE_PGM_RSRC2(os);
  os << '\n';

  dumpKernelCodeProperties(os);
  os << '\n';

  os << "----- detailed dump for " << name << "  end ---- \n";
}

void AmdgpuKernelDescriptor::dumpCOMPUTE_PGM_RSRC3(std::ostream &os) const {
  const char *indent = "    ";
  os << "  -- COMPUTE_PGM_RSRC3 begin\n";

  if (isGfx90aOr942())
    dumpCOMPUTE_PGM_RSRC3_Gfx90aOr942(os);

  else if (isGfx10Plus())
    dumpCOMPUTE_PGM_RSRC3_Gfx10Plus(os);

  else
    os << indent << getCOMPUTE_PGM_RSRC3() << '\n';

  os << "  -- COMPUTE_PGM_RSRC3 end\n";
}

void AmdgpuKernelDescriptor::dumpCOMPUTE_PGM_RSRC3_Gfx90aOr942(std::ostream &os) const {
  assert(isGfx90aOr942());
  const char *indent = "    ";

  os << indent << "ACCUM_OFFSET : " << getCOMPUTE_PGM_RSRC3_AccumOffset() << '\n';

  os << indent << "TG_SPLIT : " << getCOMPUTE_PGM_RSRC3_TgSplit() << '\n';
}

void AmdgpuKernelDescriptor::dumpCOMPUTE_PGM_RSRC3_Gfx10Plus(std::ostream &os) const {
  assert(isGfx10Plus());
  const char *indent = "    ";

  os << indent << "SHARED_VGPR_COUNT : " << getCOMPUTE_PGM_RSRC3_SharedVgprCount() << '\n';

  os << indent << "INST_PREF_SIZE : " << getCOMPUTE_PGM_RSRC3_InstPrefSize() << '\n';

  os << indent << "TRAP_ON_START : " << getCOMPUTE_PGM_RSRC3_TrapOnStart() << '\n';

  os << indent << "TRAP_ON_END : " << getCOMPUTE_PGM_RSRC3_TrapOnEnd() << '\n';
  os << indent << "IMAGE_OP : " << getCOMPUTE_PGM_RSRC3_ImageOp() << '\n';
}

void AmdgpuKernelDescriptor::dumpCOMPUTE_PGM_RSRC1(std::ostream &os) const {
  const char *indent = "    ";
  os << "  -- COMPUTE_PGM_RSRC1 begin\n";

  os << indent
     << "GRANULATED_WORKITEM_VGPR_COUNT : " << getCOMPUTE_PGM_RSRC1_GranulatedWorkitemVgprCount()
     << '\n';

  os << indent
     << "GRANULATED_WAVEFRONT_SGPR_COUNT : " << getCOMPUTE_PGM_RSRC1_GranulatedWavefrontSgprCount()
     << '\n';

  os << indent << "PRIORITY : " << getCOMPUTE_PGM_RSRC1_Priority() << '\n';

  os << indent << "FLOAT_ROUND_MODE_32 : " << getCOMPUTE_PGM_RSRC1_FloatRoundMode32() << '\n';

  os << indent << "FLOAT_ROUND_MODE_16_64 : " << getCOMPUTE_PGM_RSRC1_FloatRoundMode1664() << '\n';

  os << indent << "FLOAT_DENORM_MODE_32 : " << getCOMPUTE_PGM_RSRC1_FloatDenormMode32() << '\n';

  os << indent << "FLOAT_DENORM_MODE_16_64 : " << getCOMPUTE_PGM_RSRC1_FloatDenormMode1664()
     << '\n';

  os << indent << "PRIV : " << getCOMPUTE_PGM_RSRC1_Priv() << '\n';

  os << indent << "ENABLE_DX10_CLAMP : " << getCOMPUTE_PGM_RSRC1_EnableDx10Clamp() << '\n';

  os << indent << "ENABLE_IEEE_MODE : " << getCOMPUTE_PGM_RSRC1_EnableIeeeMode() << '\n';

  os << indent << "BULKY : " << getCOMPUTE_PGM_RSRC1_Bulky() << '\n';

  os << indent << "CDBG_USER : " << getCOMPUTE_PGM_RSRC1_CdbgUser() << '\n';

  os << indent << "FP16_OVFL : " << getCOMPUTE_PGM_RSRC1_Fp16Ovfl() << '\n';

  os << indent << "WGP_MODE : " << getCOMPUTE_PGM_RSRC1_WgpMode() << '\n';

  os << indent << "MEM_ORDERED : " << getCOMPUTE_PGM_RSRC1_MemOrdered() << '\n';

  os << indent << "FWD_PROGRESS : " << getCOMPUTE_PGM_RSRC1_FwdProgress() << '\n';

  os << "  -- COMPUTE_PGM_RSRC1 end\n";
}

void AmdgpuKernelDescriptor::dumpCOMPUTE_PGM_RSRC2(std::ostream &os) const {
  const char *indent = "    ";
  os << "  -- COMPUTE_PGM_RSRC2 begin\n";

  os << indent << "ENABLE_PRIVATE_SEGMENT : " << getCOMPUTE_PGM_RSRC2_EnablePrivateSegment()
     << '\n';

  os << indent << "USER_SGPR_COUNT : " << getCOMPUTE_PGM_RSRC2_UserSgprCount() << '\n';

  os << indent << "ENABLE_TRAP_HANDLER : " << getCOMPUTE_PGM_RSRC2_EnableTrapHandler() << '\n';

  os << indent << "ENABLE_SGPR_WORKGROUP_ID_X : " << getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdX()
     << '\n';

  os << indent << "ENABLE_SGPR_WORKGROUP_ID_Y : " << getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdY()
     << '\n';

  os << indent << "ENABLE_SGPR_WORKGROUP_ID_Z : " << getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdZ()
     << '\n';

  os << indent << "ENABLE_SGPR_WORKGROUP_INFO : " << getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupInfo()
     << '\n';

  os << indent << "ENABLE_VGPR_WORKITEM_ID : " << getCOMPUTE_PGM_RSRC2_EnableVgprWorkitemId()
     << '\n';

  os << indent
     << "ENABLE_EXCEPTION_ADDRESS_WATCH : " << getCOMPUTE_PGM_RSRC2_EnableExceptionAddressWatch()
     << '\n';

  os << indent << "ENABLE_EXCEPTION_MEMORY : " << getCOMPUTE_PGM_RSRC2_EnableExceptionMemory()
     << '\n';

  os << indent << "GRANULATED_LDS_SIZE : " << getCOMPUTE_PGM_RSRC2_GranulatedLdsSize() << '\n';

  os << indent << "ENABLE_EXCEPTION_IEEE_754_FP_INVALID_OPERATION : "
     << getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpInvalidOperation() << '\n';

  os << indent << "ENABLE_EXCEPTION_FP_DENORMAL_SOURCE : "
     << getCOMPUTE_PGM_RSRC2_EnableExceptionFpDenormalSource() << '\n';

  os << indent << "ENABLE_EXCEPTION_IEEE_754_FP_DIVISION_BY_ZERO : "
     << getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpDivisionByZero() << '\n';

  os << indent << "ENABLE_EXCEPTION_IEEE_754_FP_OVERFLOW : "
     << getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpOverflow() << '\n';

  os << indent << "ENABLE_EXCEPTION_IEEE_754_FP_UNDERFLOW : "
     << getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpUnderflow() << '\n';

  os << indent << "ENABLE_EXCEPTION_IEEE_754_FP_INEXACT : "
     << getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpInexact() << '\n';

  os << indent << "ENABLE_EXCEPTION_INT_DIVIDE_BY_ZERO : "
     << getCOMPUTE_PGM_RSRC2_EnableExceptionIntDivideByZero() << '\n';

  os << "  -- COMPUTE_PGM_RSRC2 end\n";
}

void AmdgpuKernelDescriptor::dumpKernelCodeProperties(std::ostream &os) const {
  const char *indent = "    ";
  os << "  -- Kernel code properties begin\n";

  os << indent << "ENABLE_SGPR_PRIVATE_SEGMENT_BUFFER : "
     << getKernelCodeProperty_EnableSgprPrivateSegmentBuffer() << '\n';

  os << indent << "ENABLE_SGPR_DISPATCH_PTR : " << getKernelCodeProperty_EnableSgprDispatchPtr()
     << '\n';

  os << indent << "ENABLE_SGPR_QUEUE_PTR : " << getKernelCodeProperty_EnableSgprQueuePtr() << '\n';

  os << indent
     << "ENABLE_SGPR_KERNARG_SEGMENT_PTR : " << getKernelCodeProperty_EnableSgprKernargSegmentPtr()
     << '\n';

  os << indent << "ENABLE_SGPR_DISPATCH_ID : " << getKernelCodeProperty_EnableSgprDispatchId()
     << '\n';

  os << indent
     << "ENABLE_SGPR_FLAT_SCRATCH_INIT : " << getKernelCodeProperty_EnableSgprFlatScratchInit()
     << '\n';

  os << indent
     << "ENABLE_PRIVATE_SEGMENT_SIZE : " << getKernelCodeProperty_EnablePrivateSegmentSize()
     << '\n';

  os << indent << "ENABLE_WAVEFRONT_SIZE_32 : " << getKernelCodeProperty_EnableWavefrontSize32()
     << '\n';

  os << indent << "USES_DYNAMIC_STACK : " << getKernelCodeProperty_UsesDynamicStack() << '\n';

  os << "  -- Kernel code properties end\n";
}

void AmdgpuKernelDescriptor::writeToMemory(uint8_t *memPtr) const {
  write_memory_as(memPtr, kdRepr);
}

} // namespace Dyninst
