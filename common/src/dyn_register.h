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

#include <assert.h>
#include <functional>
#include <stdint.h>
#include <vector>

namespace Dyninst {

/* Dyninst has been using the following typedef for representing a register number, e.g., [0..31]
typedef unsigned int Register;
*/

// Without making intrusive changes to existing CPU architectures, we want to introduce a richer
// Register type to model different kinds of registers.

enum RegKind : uint32_t { SCALAR = 0, VECTOR = 1, MATRIX = 2, PREDICATE = 3, UNDEFINED_KIND = 4 };

enum RegUsage : uint32_t {
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

  Register(uint32_t id, RegKind kind, RegUsage usage, uint32_t count) {
    assert(id >> REG_ID_WIDTH == 0 && "id is wider than than specified");
    assert(count >> REG_COUNT_WIDTH == 0 && "count is wider than specified");
    details.id = id;
    details.kind = kind;
    details.usage = usage;
    details.count = count;
    details.reserved = 0;
  }

  // A register block represents consecutive reqisters starting from id. Not all architectures
  // need or support this.
  //
  // So count = 0 is an individual register.
  // count >= 1 means "register block" with 1 or more registers.
  constexpr bool isRegisterBlock() const { return details.count != 0; }

  constexpr operator uint32_t() const { return raw; }

  // If this is a register block, return individual registers in that block.
  void getIndividualRegisters(std::vector<Register> &individualRegisters) const {
    assert(this->isRegisterBlock() && "This must be a register block");
    for (uint32_t i = 0; i < details.count; ++i) {
      RegKind kind = static_cast<RegKind>(details.kind);
      RegUsage usage = static_cast<RegUsage>(details.usage);
      individualRegisters.push_back(Register(details.id + i, kind, usage, 0));
    }
  }

  // Required for hashmap lookup
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

// Along with the older Register type, which was just an unsigned int, use of RegValue and
// Null_Register are also scattered around the codebase.

/* register content 64-bit */
typedef long long int RegValue;

/* '255' */
constexpr Register Null_Register(static_cast<unsigned int>(-1));

} // namespace Dyninst

// Adding a hash function for the new Register type
namespace std {
template <> struct hash<Dyninst::Register> {
  size_t operator()(const Dyninst::Register &reg) const noexcept {
    return std::hash<uint32_t>{}(reg.raw);
  }
};
} // namespace std

#endif
