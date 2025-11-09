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

#ifndef INSTRUCTIONAPI_TYPE_CONVERSION_H
#define INSTRUCTIONAPI_TYPE_CONVERSION_H

#include "Result.h"

namespace Dyninst {
namespace InstructionAPI {

// clang-format off
  inline Result_Type size_to_type_unsigned(uint8_t cap_size) {
    switch (cap_size) {
      case 1:  return u8;
      case 2:  return u16;
      case 3:  return u24;
      case 4:  return u32;
      case 6:  return u48;
      case 8:  return u64;
      default: return invalid_type;
    }
  }
  inline Result_Type size_to_type_signed(uint8_t cap_size) {
    switch (cap_size) {
      case 1:  return s8;
      case 2:  return s16;
      case 4:  return s32;
      case 6:  return s48;
      case 8:  return s64;
      default: return invalid_type;
    }
  }
  inline Result_Type size_to_type_float(uint8_t cap_size) {
    switch (cap_size) {
      case 4:  return sp_float;
      case 8:  return dp_float;
      case 16: return dbl128;
      default: return invalid_type;
    }
  }
  inline Result_Type size_to_type_memory(uint8_t cap_size) {
    switch (cap_size) {
      case 10: return m80;
      case 12: return m96;
      case 14: return m14;
      case 16: return m128;
      case 20: return m160;
      case 24: return m192;
      case 28: return m224;
      case 32: return m256;
      case 36: return m288;
      case 40: return m320;
      case 44: return m352;
      case 48: return m384;
      case 52: return m416;
      case 56: return m448;
      case 60: return m480;
      case 64: return m512;
      default: return invalid_type;
    }
  }

// clang-format on

} // namespace InstructionAPI
} // namespace Dyninst

#endif
