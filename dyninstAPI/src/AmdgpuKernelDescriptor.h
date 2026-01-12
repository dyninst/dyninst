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

#ifndef KERNEL_DESCRIPTOR_H
#define KERNEL_DESCRIPTOR_H

#include "external/amdgpu/AMDHSAKernelDescriptor.h"

#include <ostream>
#include <string>

namespace Dyninst {

class AmdgpuKernelDescriptor {
public:
  AmdgpuKernelDescriptor(uint8_t *kdBytes, size_t kdSize, unsigned amdgpuMachine);

  AmdgpuKernelDescriptor(llvm::amdhsa::kernel_descriptor_t &rawKd, unsigned _amdgpuMach) : kdRepr(rawKd), amdgpuMach(_amdgpuMach) {}
  uint32_t getGroupSegmentFixedSize() const;
  void setGroupSegmentFixedSize(uint32_t value);

  uint32_t getPrivateSegmentFixedSize() const;
  void setPrivateSegmentFixedSize(uint32_t value);

  uint32_t getKernargSize() const;
  void setKernargSize(uint32_t value);

  // 4 reserved bytes

  int64_t getKernelCodeEntryByteOffset() const;
  void setKernelCodeEntryByteOffset(int64_t value);

  // 20 reserved bytes

  // ----- COMPUTE_PGM_RSRC3 begin -----
  //
  // work with the entire register
  uint32_t getCOMPUTE_PGM_RSRC3() const;
  //
  // work with individual contents in the COMPUTE_PGM_RSRC3
  //
  // GFX90A, GFX942 begin
  //
  // NOTE : THIS IS A 6 BIT VALUE
  uint32_t getCOMPUTE_PGM_RSRC3_AccumOffset() const;
  void setCOMPUTE_PGM_RSRC3_AccumOffset(uint32_t value);

  // 10 reserved bits

  bool getCOMPUTE_PGM_RSRC3_TgSplit() const;
  void setCOMPUTE_PGM_RSRC3_TgSplit(bool value);

  // 15 reserved bits
  // GFX90A, GFX942 end
  //
  // GFX10, GFX11 begin
  //
  // NOTE: THIS IS A 4 BIT VALUE
  uint32_t getCOMPUTE_PGM_RSRC3_SharedVgprCount() const;
  void setCOMPUTE_PGM_RSRC3_SharedVgprCount(uint32_t value);

  // NOTE: THIS IS A 6 BIT VALUE
  uint32_t getCOMPUTE_PGM_RSRC3_InstPrefSize() const;
  void setCOMPUTE_PGM_RSRC3_InstPrefSize(uint32_t value);

  bool getCOMPUTE_PGM_RSRC3_TrapOnStart() const;
  void setCOMPUTE_PGM_RSRC3_TrapOnStart(bool value);

  bool getCOMPUTE_PGM_RSRC3_TrapOnEnd() const;
  void setCOMPUTE_PGM_RSRC3_TrapOnEnd(bool value);

  // 19 reserved bits

  bool getCOMPUTE_PGM_RSRC3_ImageOp() const;
  void setCOMPUTE_PGM_RSRC3_ImageOp(bool value);
  //
  // ----- COMPUTE_PGM_RSRC3 end -----

  // ----- COMPUTE_PGM_RSRC1 begin -----
  //
  // work with the entire register
  uint32_t getCOMPUTE_PGM_RSRC1() const;
  //
  // work with individual contents in the COMPUTE_PGM_RSRC1
  //
  // NOTE : THIS IS A 6 BIT VALUE
  uint32_t getCOMPUTE_PGM_RSRC1_GranulatedWorkitemVgprCount() const;
  void setCOMPUTE_PGM_RSRC1_GranulatedWorkitemVgprCount(uint32_t value);

  // NOTE : THIS IS A 4 BIT VALUE
  uint32_t getCOMPUTE_PGM_RSRC1_GranulatedWavefrontSgprCount() const;
  void setCOMPUTE_PGM_RSRC1_GranulatedWavefrontSgprCount(uint32_t value);

  // NOTE : THIS IS A 2 BIT VALUE MUST BE 0 AND CANT BE SET BY USER.
  uint32_t getCOMPUTE_PGM_RSRC1_Priority() const;

  // NOTE : THIS IS A 2 BIT VALUE
  uint32_t getCOMPUTE_PGM_RSRC1_FloatRoundMode32() const;
  void setCOMPUTE_PGM_RSRC1_FloatRoundMode32(uint32_t value);

  // NOTE : THIS IS A 2 BIT VALUE
  uint32_t getCOMPUTE_PGM_RSRC1_FloatRoundMode1664() const;
  void setCOMPUTE_PGM_RSRC1_FloatRoundMode1664(uint32_t value);

  // NOTE : THIS IS A 2 BIT VALUE
  uint32_t getCOMPUTE_PGM_RSRC1_FloatDenormMode32() const;
  void setCOMPUTE_PGM_RSRC1_FloatDenormMode32(uint32_t value);

  // NOTE : THIS IS A 2 BIT VALUE
  uint32_t getCOMPUTE_PGM_RSRC1_FloatDenormMode1664() const;
  void setCOMPUTE_PGM_RSRC1_FloatDenormMode1664(uint32_t value);

  // NOTE : THIS BIT MUST BE 0 AND CANT BE SET BY USER.
  bool getCOMPUTE_PGM_RSRC1_Priv() const;

  bool getCOMPUTE_PGM_RSRC1_EnableDx10Clamp() const;
  void setCOMPUTE_PGM_RSRC1_EnableDx10Clamp(bool value);

  // NOTE : THIS BIT MUST BE 0 AND CANT BE SET BY USER.
  bool getCOMPUTE_PGM_RSRC1_DebugMode() const;

  bool getCOMPUTE_PGM_RSRC1_EnableIeeeMode() const;
  void setCOMPUTE_PGM_RSRC1_EnableIeeeMode(bool value);

  // NOTE : THIS BIT MUST BE 0 AND CANT BE SET BY USER.
  bool getCOMPUTE_PGM_RSRC1_Bulky() const;

  // NOTE : THIS BIT MUST BE 0 AND CANT BE SET BY USER.
  bool getCOMPUTE_PGM_RSRC1_CdbgUser() const;

  bool getCOMPUTE_PGM_RSRC1_Fp16Ovfl() const;
  void setCOMPUTE_PGM_RSRC1_Fp16Ovfl(bool value);

  // 2 reserved bits

  bool getCOMPUTE_PGM_RSRC1_WgpMode() const;
  void setCOMPUTE_PGM_RSRC1_WgpMode(bool value);

  bool getCOMPUTE_PGM_RSRC1_MemOrdered() const;
  void setCOMPUTE_PGM_RSRC1_MemOrdered(bool value);

  bool getCOMPUTE_PGM_RSRC1_FwdProgress() const;
  void setCOMPUTE_PGM_RSRC1_FwdProgress(bool value);
  //
  // ----- COMPUTE_PGM_RSRC1 end -----

  // ----- COMPUTE_PGM_RSRC2 begin -----
  //
  // work with the entire register
  uint32_t getCOMPUTE_PGM_RSRC2() const;
  //
  // work with individual contents in the COMPUTE_PGM_RSRC2
  //
  bool getCOMPUTE_PGM_RSRC2_EnablePrivateSegment() const;
  void setCOMPUTE_PGM_RSRC2_EnablePrivateSegment(bool value);

  // NOTE : THIS IS A 5 BIT VALUE
  uint32_t getCOMPUTE_PGM_RSRC2_UserSgprCount() const;
  void setCOMPUTE_PGM_RSRC2_UserSgprCount(uint32_t value);

  bool getCOMPUTE_PGM_RSRC2_EnableTrapHandler() const;

  bool getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdX() const;
  void setCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdX(bool value);

  bool getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdY() const;
  void setCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdY(bool value);

  bool getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdZ() const;
  void setCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupIdZ(bool value);

  bool getCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupInfo() const;
  void setCOMPUTE_PGM_RSRC2_EnableSgprWorkgroupInfo(bool value);

  // NOTE : THIS IS A 2 BIT VALUE
  uint32_t getCOMPUTE_PGM_RSRC2_EnableVgprWorkitemId() const;
  void setCOMPUTE_PGM_RSRC2_EnableVgprWorkitemId(uint32_t value);

  // NOTE: USER CAN't SET THIS
  bool getCOMPUTE_PGM_RSRC2_EnableExceptionAddressWatch() const;

  bool getCOMPUTE_PGM_RSRC2_EnableExceptionMemory() const;

  // NOTE : THIS IS A 9 BIT VALUE
  uint32_t getCOMPUTE_PGM_RSRC2_GranulatedLdsSize() const;

  bool getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpInvalidOperation() const;
  void getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpInvalidOperation(bool value);

  bool getCOMPUTE_PGM_RSRC2_EnableExceptionFpDenormalSource() const;
  void setCOMPUTE_PGM_RSRC2_EnableExceptionFpDenormalSource(bool value);

  bool getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpDivisionByZero() const;
  void setCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpDivisionByZero(bool value);

  bool getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpOverflow() const;
  void setCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpOverflow(bool value);

  bool getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpUnderflow() const;
  void setCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpUnderflow(bool value);

  bool getCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpInexact() const;
  void setCOMPUTE_PGM_RSRC2_EnableExceptionIeee754FpInexact(bool value);

  bool getCOMPUTE_PGM_RSRC2_EnableExceptionIntDivideByZero() const;
  void setCOMPUTE_PGM_RSRC2_EnableExceptionIntDivideByZero(bool value);

  // one reserved bit
  //
  // ----- COMPUTE_PGM_RSRC2 end -----

  // 7 bits after COMPUTE_PGM_RSRC2
  bool getKernelCodeProperty_EnableSgprPrivateSegmentBuffer() const;
  void setKernelCodeProperty_EnableSgprPrivateSegmentBuffer(bool value);

  bool getKernelCodeProperty_EnableSgprDispatchPtr() const;
  void setKernelCodeProperty_EnableSgprDispatchPtr(bool value);

  bool getKernelCodeProperty_EnableSgprQueuePtr() const;
  void setKernelCodeProperty_EnableSgprQueuePtr(bool value);

  bool getKernelCodeProperty_EnableSgprKernargSegmentPtr() const;
  void setKernelCodeProperty_EnableSgprKernargSegmentPtr(bool value);

  bool getKernelCodeProperty_EnableSgprDispatchId() const;
  void setKernelCodeProperty_EnableSgprDispatchId(bool value);

  bool getKernelCodeProperty_EnableSgprFlatScratchInit() const;
  void setKernelCodeProperty_EnableSgprFlatScratchInit(bool value);

  bool getKernelCodeProperty_EnablePrivateSegmentSize() const;
  void setKernelCodeProperty_EnablePrivateSegmentSize(bool value);

  // done with those 7 bits

  // 3 reserved bits

  bool getKernelCodeProperty_EnableWavefrontSize32() const;
  void setKernelCodeProperty_EnableWavefrontSize32(bool value);

  // TODO : the llvm definition doesn't have this in mono repo yet
  bool getKernelCodeProperty_UsesDynamicStack() const;
  void setKernelCodeProperty_UsesDynamicStack(bool value);

  // ==== END OF ALL FIELDS ===


  unsigned getKernargPtrRegister();

  void dump(std::ostream &os) const;
  void dumpDetailed(std::ostream &os) const;

  void writeToMemory(uint8_t *memPtr) const;

  const std::string &getName() const { return name; }


  // THIS IS ONLY TO HELP PATCHING THE ORIGINAL KD AFTER UPDATING IT.
  // DON'T USE THIS FOR MODIFYING KDs.
  void *getRawPtr() { return (void *)&kdRepr; }

private:
  void dumpCOMPUTE_PGM_RSRC3(std::ostream &os) const;
  void dumpCOMPUTE_PGM_RSRC3_Gfx90aOr942(std::ostream &os) const;
  void dumpCOMPUTE_PGM_RSRC3_Gfx10Plus(std::ostream &os) const;

  void dumpCOMPUTE_PGM_RSRC1(std::ostream &os) const;

  void dumpCOMPUTE_PGM_RSRC2(std::ostream &os) const;

  void dumpKernelCodeProperties(std::ostream &os) const;

  bool isGfx6() const;
  bool isGfx7() const;
  bool isGfx8() const;
  bool isGfx9() const;
  bool isGfx90aOr942() const;
  bool isGfx9Plus() const;
  bool isGfx10() const;
  bool isGfx10Plus() const;
  bool isGfx11() const;

  bool supportsArchitectedFlatScratch() const;

  std::string name;

  // canonical kernel descriptor struct
  llvm::amdhsa::kernel_descriptor_t kdRepr;

  unsigned amdgpuMach;
};

} // namespace Dyninst
#endif
