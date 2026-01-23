//===----------------------------------------------------------------------===//
//
// This file contains a copy of the e_flags struct definition from
//
// repo: https://github.com/llvm/llvm-project.git
// tag:  llvmorg-21.1.4
// hash: 222fc11f2b8f
// path: llvm/include/llvm/BinaryFormat/ELF.h
// date: 2025-10-21 08:14:55Z
// line: 766-922
//
// This file is also under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef AMDGPU_E_FLAGS_H
#define AMDGPU_E_FLAGS_H

// AMDGPU specific e_flags.
enum : unsigned {
  // Processor selection mask for EF_AMDGPU_MACH_* values.
  EF_AMDGPU_MACH = 0x0ff,

  // Not specified processor.
  EF_AMDGPU_MACH_NONE = 0x000,

  // R600-based processors.

  // Radeon HD 2000/3000 Series (R600).
  EF_AMDGPU_MACH_R600_R600 = 0x001,
  EF_AMDGPU_MACH_R600_R630 = 0x002,
  EF_AMDGPU_MACH_R600_RS880 = 0x003,
  EF_AMDGPU_MACH_R600_RV670 = 0x004,
  // Radeon HD 4000 Series (R700).
  EF_AMDGPU_MACH_R600_RV710 = 0x005,
  EF_AMDGPU_MACH_R600_RV730 = 0x006,
  EF_AMDGPU_MACH_R600_RV770 = 0x007,
  // Radeon HD 5000 Series (Evergreen).
  EF_AMDGPU_MACH_R600_CEDAR = 0x008,
  EF_AMDGPU_MACH_R600_CYPRESS = 0x009,
  EF_AMDGPU_MACH_R600_JUNIPER = 0x00a,
  EF_AMDGPU_MACH_R600_REDWOOD = 0x00b,
  EF_AMDGPU_MACH_R600_SUMO = 0x00c,
  // Radeon HD 6000 Series (Northern Islands).
  EF_AMDGPU_MACH_R600_BARTS = 0x00d,
  EF_AMDGPU_MACH_R600_CAICOS = 0x00e,
  EF_AMDGPU_MACH_R600_CAYMAN = 0x00f,
  EF_AMDGPU_MACH_R600_TURKS = 0x010,

  // Reserved for R600-based processors.
  EF_AMDGPU_MACH_R600_RESERVED_FIRST = 0x011,
  EF_AMDGPU_MACH_R600_RESERVED_LAST = 0x01f,

  // First/last R600-based processors.
  EF_AMDGPU_MACH_R600_FIRST = EF_AMDGPU_MACH_R600_R600,
  EF_AMDGPU_MACH_R600_LAST = EF_AMDGPU_MACH_R600_TURKS,

  // AMDGCN-based processors.
  // clang-format off
  EF_AMDGPU_MACH_AMDGCN_GFX600          = 0x020,
  EF_AMDGPU_MACH_AMDGCN_GFX601          = 0x021,
  EF_AMDGPU_MACH_AMDGCN_GFX700          = 0x022,
  EF_AMDGPU_MACH_AMDGCN_GFX701          = 0x023,
  EF_AMDGPU_MACH_AMDGCN_GFX702          = 0x024,
  EF_AMDGPU_MACH_AMDGCN_GFX703          = 0x025,
  EF_AMDGPU_MACH_AMDGCN_GFX704          = 0x026,
  EF_AMDGPU_MACH_AMDGCN_RESERVED_0X27   = 0x027,
  EF_AMDGPU_MACH_AMDGCN_GFX801          = 0x028,
  EF_AMDGPU_MACH_AMDGCN_GFX802          = 0x029,
  EF_AMDGPU_MACH_AMDGCN_GFX803          = 0x02a,
  EF_AMDGPU_MACH_AMDGCN_GFX810          = 0x02b,
  EF_AMDGPU_MACH_AMDGCN_GFX900          = 0x02c,
  EF_AMDGPU_MACH_AMDGCN_GFX902          = 0x02d,
  EF_AMDGPU_MACH_AMDGCN_GFX904          = 0x02e,
  EF_AMDGPU_MACH_AMDGCN_GFX906          = 0x02f,
  EF_AMDGPU_MACH_AMDGCN_GFX908          = 0x030,
  EF_AMDGPU_MACH_AMDGCN_GFX909          = 0x031,
  EF_AMDGPU_MACH_AMDGCN_GFX90C          = 0x032,
  EF_AMDGPU_MACH_AMDGCN_GFX1010         = 0x033,
  EF_AMDGPU_MACH_AMDGCN_GFX1011         = 0x034,
  EF_AMDGPU_MACH_AMDGCN_GFX1012         = 0x035,
  EF_AMDGPU_MACH_AMDGCN_GFX1030         = 0x036,
  EF_AMDGPU_MACH_AMDGCN_GFX1031         = 0x037,
  EF_AMDGPU_MACH_AMDGCN_GFX1032         = 0x038,
  EF_AMDGPU_MACH_AMDGCN_GFX1033         = 0x039,
  EF_AMDGPU_MACH_AMDGCN_GFX602          = 0x03a,
  EF_AMDGPU_MACH_AMDGCN_GFX705          = 0x03b,
  EF_AMDGPU_MACH_AMDGCN_GFX805          = 0x03c,
  EF_AMDGPU_MACH_AMDGCN_GFX1035         = 0x03d,
  EF_AMDGPU_MACH_AMDGCN_GFX1034         = 0x03e,
  EF_AMDGPU_MACH_AMDGCN_GFX90A          = 0x03f,
  EF_AMDGPU_MACH_AMDGCN_RESERVED_0X40   = 0x040,
  EF_AMDGPU_MACH_AMDGCN_GFX1100         = 0x041,
  EF_AMDGPU_MACH_AMDGCN_GFX1013         = 0x042,
  EF_AMDGPU_MACH_AMDGCN_GFX1150         = 0x043,
  EF_AMDGPU_MACH_AMDGCN_GFX1103         = 0x044,
  EF_AMDGPU_MACH_AMDGCN_GFX1036         = 0x045,
  EF_AMDGPU_MACH_AMDGCN_GFX1101         = 0x046,
  EF_AMDGPU_MACH_AMDGCN_GFX1102         = 0x047,
  EF_AMDGPU_MACH_AMDGCN_GFX1200         = 0x048,
  EF_AMDGPU_MACH_AMDGCN_GFX1250         = 0x049,
  EF_AMDGPU_MACH_AMDGCN_GFX1151         = 0x04a,
  EF_AMDGPU_MACH_AMDGCN_RESERVED_0X4B   = 0x04b,
  EF_AMDGPU_MACH_AMDGCN_GFX942          = 0x04c,
  EF_AMDGPU_MACH_AMDGCN_RESERVED_0X4D   = 0x04d,
  EF_AMDGPU_MACH_AMDGCN_GFX1201         = 0x04e,
  EF_AMDGPU_MACH_AMDGCN_GFX950          = 0x04f,
  EF_AMDGPU_MACH_AMDGCN_RESERVED_0X50   = 0x050,
  EF_AMDGPU_MACH_AMDGCN_GFX9_GENERIC    = 0x051,
  EF_AMDGPU_MACH_AMDGCN_GFX10_1_GENERIC = 0x052,
  EF_AMDGPU_MACH_AMDGCN_GFX10_3_GENERIC = 0x053,
  EF_AMDGPU_MACH_AMDGCN_GFX11_GENERIC   = 0x054,
  EF_AMDGPU_MACH_AMDGCN_GFX1152         = 0x055,
  EF_AMDGPU_MACH_AMDGCN_RESERVED_0X56   = 0x056,
  EF_AMDGPU_MACH_AMDGCN_RESERVED_0X57   = 0x057,
  EF_AMDGPU_MACH_AMDGCN_GFX1153         = 0x058,
  EF_AMDGPU_MACH_AMDGCN_GFX12_GENERIC   = 0x059,
  EF_AMDGPU_MACH_AMDGCN_GFX9_4_GENERIC  = 0x05f,
  // clang-format on

  // First/last AMDGCN-based processors.
  EF_AMDGPU_MACH_AMDGCN_FIRST = EF_AMDGPU_MACH_AMDGCN_GFX600,
  EF_AMDGPU_MACH_AMDGCN_LAST = EF_AMDGPU_MACH_AMDGCN_GFX9_4_GENERIC,

  // Indicates if the "xnack" target feature is enabled for all code contained
  // in the object.
  //
  // Only valid for ELFOSABI_AMDGPU_HSA and ELFABIVERSION_AMDGPU_HSA_V2.
  EF_AMDGPU_FEATURE_XNACK_V2 = 0x01,
  // Indicates if the trap handler is enabled for all code contained
  // in the object.
  //
  // Only valid for ELFOSABI_AMDGPU_HSA and ELFABIVERSION_AMDGPU_HSA_V2.
  EF_AMDGPU_FEATURE_TRAP_HANDLER_V2 = 0x02,

  // Indicates if the "xnack" target feature is enabled for all code contained
  // in the object.
  //
  // Only valid for ELFOSABI_AMDGPU_HSA and ELFABIVERSION_AMDGPU_HSA_V3.
  EF_AMDGPU_FEATURE_XNACK_V3 = 0x100,
  // Indicates if the "sramecc" target feature is enabled for all code
  // contained in the object.
  //
  // Only valid for ELFOSABI_AMDGPU_HSA and ELFABIVERSION_AMDGPU_HSA_V3.
  EF_AMDGPU_FEATURE_SRAMECC_V3 = 0x200,

  // XNACK selection mask for EF_AMDGPU_FEATURE_XNACK_* values.
  //
  // Only valid for ELFOSABI_AMDGPU_HSA and ELFABIVERSION_AMDGPU_HSA_V4.
  EF_AMDGPU_FEATURE_XNACK_V4 = 0x300,
  // XNACK is not supported.
  EF_AMDGPU_FEATURE_XNACK_UNSUPPORTED_V4 = 0x000,
  // XNACK is any/default/unspecified.
  EF_AMDGPU_FEATURE_XNACK_ANY_V4 = 0x100,
  // XNACK is off.
  EF_AMDGPU_FEATURE_XNACK_OFF_V4 = 0x200,
  // XNACK is on.
  EF_AMDGPU_FEATURE_XNACK_ON_V4 = 0x300,

  // SRAMECC selection mask for EF_AMDGPU_FEATURE_SRAMECC_* values.
  //
  // Only valid for ELFOSABI_AMDGPU_HSA and ELFABIVERSION_AMDGPU_HSA_V4.
  EF_AMDGPU_FEATURE_SRAMECC_V4 = 0xc00,
  // SRAMECC is not supported.
  EF_AMDGPU_FEATURE_SRAMECC_UNSUPPORTED_V4 = 0x000,
  // SRAMECC is any/default/unspecified.
  EF_AMDGPU_FEATURE_SRAMECC_ANY_V4 = 0x400,
  // SRAMECC is off.
  EF_AMDGPU_FEATURE_SRAMECC_OFF_V4 = 0x800,
  // SRAMECC is on.
  EF_AMDGPU_FEATURE_SRAMECC_ON_V4 = 0xc00,

  // Generic target versioning. This is contained in the list byte of EFLAGS.
  EF_AMDGPU_GENERIC_VERSION = 0xff000000,
  EF_AMDGPU_GENERIC_VERSION_OFFSET = 24,
  EF_AMDGPU_GENERIC_VERSION_MIN = 1,
  EF_AMDGPU_GENERIC_VERSION_MAX = 0xff,
};

#endif
