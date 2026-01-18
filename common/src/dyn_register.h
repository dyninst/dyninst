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

#ifndef DYNINST_REGISTER
#define DYNINST_REGISTER

#include <functional>

namespace Dyninst {
/* This needs to be an int since it is sometimes used to pass offsets
   to the code generator (i.e. if-statement) - jkh 5/24/99 */

#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908) || defined(DYNINST_CODEGEN_ARCH_I386)
enum RegKind : uint32_t { SCALAR = 0, VECTOR = 1, MATRIX = 2, PREDICATE = 3, UNDEFINED_KIND = 4 };

enum RegUsage : uint16_t {
  GENERAL_PURPOSE = 0,
  SPECIAL_PURPOSE = 1,
  TEMPORARY = 2,
  UNDEFINED_USAGE = 3
};

constexpr uint32_t REG_ID_WIDTH = 12;
constexpr uint32_t REG_KIND_WIDTH = 4;
constexpr uint32_t REG_USAGE_WIDTH = 4;
constexpr uint32_t REG_COUNT_WIDTH = 4;
constexpr uint32_t REG_RESERVED_WIDTH = 8;
// total = 32

struct __attribute__((packed)) RegisterFields {
  uint32_t id : REG_ID_WIDTH;

  uint32_t kind : REG_KIND_WIDTH;

  uint32_t usage : REG_USAGE_WIDTH;

  uint32_t count : REG_COUNT_WIDTH;

  uint32_t reserved : REG_RESERVED_WIDTH;
};

union Register {
  RegisterFields details;
  uint32_t raw;

  constexpr Register(uint32_t val) : raw(val) {}

  constexpr Register() : raw(static_cast<uint32_t>(-1)) {}

  // A register block represents consecutive reqisters starting from id. Not all architectures
  // support this.
  //
  // For backward compatibility in codegen across architectures, we need to keep this 0 when we
  // want to have a single register.
  // So count = 0 and count = 1 will have the same effect.
  constexpr bool isRegisterBlock() const { return details.count != 0; }

  constexpr operator uint32_t() const { return raw; }

  // Required for hash map lookup
  constexpr bool operator==(const Register &other) const { return raw == other.raw; }

  constexpr bool operator!=(const Register &other) const { return !(*this == other); }

  // Required for compatibility with integers
  constexpr bool operator==(const int &value) const { return raw == static_cast<uint32_t>(value); }

  constexpr bool operator!=(const int &value) const { return !(*this == value); }

  constexpr bool operator==(const unsigned &value) const {
    return raw == static_cast<uint32_t>(value);
  }

  constexpr bool operator!=(const unsigned &value) const { return !(*this == value); }
};
#else
/* a register number, e.g., [0..31]  */
typedef unsigned int Register;
#endif

/* register content 64-bit */
typedef long long int RegValue;

/* '255' */
constexpr Register Null_Register{static_cast<unsigned int>(-1)};

}

#if defined(DYNINST_CODEGEN_ARCH_AMDGPU_GFX908) || defined(DYNINST_CODEGEN_ARCH_I386)
namespace std {
template <> struct hash<Dyninst::Register> {
  size_t operator()(const Dyninst::Register &reg) const noexcept {
    return std::hash<uint32_t>{}(reg.raw);
  }
};
} // namespace std
#endif
#endif
