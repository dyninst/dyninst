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
 * Lesser General Public License for more info.
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

// Without making intrusive changes to existing CPU architectures, we want to introduce a richer
// Register type to model different kinds of registers.
enum RegKind : uint32_t { SCALAR, VECTOR, SCALAR_PREDICATE, UNDEFINED_KIND };

constexpr uint32_t REG_ID_WIDTH = 16;
constexpr uint32_t REG_KIND_WIDTH = 2;
constexpr uint32_t REG_COUNT_WIDTH = 14;
// total = 32

struct __attribute__((packed)) RegisterInfo {
  uint32_t id : REG_ID_WIDTH;
  uint32_t kind : REG_KIND_WIDTH;
  uint32_t count : REG_COUNT_WIDTH;

  constexpr RegisterInfo(uint32_t regId, uint32_t regKind, uint32_t numRegs)
      : id(regId), kind(regKind), count(numRegs) {}
};

class Register {
  union {
    RegisterInfo info;
    uint32_t value;
  };

public:
  constexpr Register(uint32_t val = -1) : value(val) {}

  Register(uint32_t id, RegKind kind, uint32_t count) {
    assert(id >> REG_ID_WIDTH == 0 && "id is wider than than specified");
    assert(count >> REG_COUNT_WIDTH == 0 && "count is wider than specified");
    assert(count != 0 && "count must be non-zero");
    info = RegisterInfo(id, static_cast<uint32_t>(kind), count);
  }

  constexpr uint32_t getId() const { return info.id; }

  constexpr RegKind getKind() const { return static_cast<RegKind>(info.kind); }

  constexpr uint32_t getCount() const { return info.count; }

  constexpr operator uint32_t() const { return value; }

  // If this is a contiguous register block, return individual registers in that block.
  void getIndividualRegisters(std::vector<Register> &individualRegisters) const {
    uint32_t baseId = this->getId();
    uint32_t numRegs = this->getCount();
    uint32_t lastId = baseId + numRegs - 1;
    RegKind kind = this->getKind();

    assert(numRegs > 1 && "This must be a register block");
    for (uint32_t id = baseId; id <= lastId; ++id) {
      individualRegisters.push_back(Register(id, kind, 1));
    }
  }

  // Required for hashmap lookup
  constexpr bool operator==(const Register &other) const { return value == uint32_t(other); }

  constexpr bool operator!=(const Register &other) const { return !(*this == other); }

  // Required for compatibility with integers
  constexpr bool operator==(const int &other) const {
    return value == static_cast<uint32_t>(other);
  }

  constexpr bool operator!=(const int &other) const { return !(*this == other); }

  constexpr bool operator==(const unsigned &other) const {
    return value == static_cast<uint32_t>(other);
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
    return std::hash<uint32_t>{}(uint32_t(reg));
  }
};
} // namespace std

#endif
