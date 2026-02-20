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
#include <stdint.h>
#include <vector>

#include <boost/container_hash/hash.hpp>

namespace Dyninst {

enum class RegKind : uint8_t { SCALAR, VECTOR, SCALAR_PREDICATE, UNKOWN_KIND };

class OperandRegId final {
  uint32_t id_;

public:
  explicit constexpr OperandRegId(uint32_t id) : id_(id) {}
  constexpr uint32_t getId() const { return id_; }
};

class BlockSize final {
  uint32_t count_;

public:
  explicit constexpr BlockSize(uint32_t count) : count_(count) {}
  constexpr uint32_t getCount() const { return count_; }
};

class Register {
  OperandRegId id;
  RegKind kind;
  BlockSize count;

public:
  static Register makeScalarRegister(OperandRegId regId, BlockSize blockSize) {
    return Register(regId, RegKind::SCALAR, blockSize);
  }

  static Register makeVectorRegister(OperandRegId regId, BlockSize blockSize) {
    return Register(regId, RegKind::VECTOR, blockSize);
  }

  // Default is an invalid register.
  constexpr Register() noexcept
      : id(OperandRegId(static_cast<uint32_t>(-1))), kind(RegKind::UNKOWN_KIND), count(BlockSize(1)) {}

  // Support existing Register usage that initializes with uint32_t.
  constexpr Register(uint32_t rawId) noexcept
      : id(OperandRegId(rawId)), kind(RegKind::SCALAR), count(BlockSize(1)) {}

  constexpr Register(OperandRegId regId, RegKind regKind, BlockSize blockSize) noexcept
      : id(regId), kind(regKind), count(blockSize) {}

  constexpr uint32_t getId() const { return id.getId(); }

  constexpr RegKind getKind() const { return kind; }

  constexpr uint32_t getCount() const { return count.getCount(); }

  constexpr bool isScalar() const { return kind == RegKind::SCALAR; }

  constexpr bool isVector() const { return kind == RegKind::VECTOR; }

  constexpr bool isScalarPredicate() const { return kind == RegKind::SCALAR_PREDICATE; }

  constexpr bool isUnkownKind() const { return kind == RegKind::UNKOWN_KIND; }

  constexpr operator uint32_t() const { return this->getId(); }

  // If this is a contiguous register block, return individual registers in that block.
  std::vector<Register> getIndividualRegisters() const {
    std::vector<Register> individualRegisters;

    uint32_t baseId = this->getId();
    uint32_t numRegs = this->getCount();
    uint32_t lastId = baseId + numRegs - 1;
    RegKind regKind = this->kind;

    for (uint32_t idNum = baseId; idNum <= lastId; ++idNum) {
      individualRegisters.emplace_back(OperandRegId(idNum), regKind, BlockSize(1));
    }
    return individualRegisters;
  }

  // Required for hashmap lookup
  constexpr bool operator==(const Register &other) const {
    return id.getId() == other.getId() &&
           count.getCount() == other.getCount() &&
           this->kind == other.kind;
  }

  constexpr bool operator!=(const Register &other) const { return !(*this == other); }

  // Required for compatibility with integers.
  // TODO: This must go away in the future.
  constexpr bool operator==(int other) const {
    return *this == Register(other);
  }

  constexpr bool operator!=(int other) const { return !(*this == other); }

  constexpr bool operator==(unsigned other) const {
    return *this == Register(other);
  }

  constexpr bool operator!=(unsigned other) const { return !(*this == other); }
};

/* register content 64-bit */
typedef long long int RegValue;

/* '255' */
constexpr Register Null_Register{};

} // namespace Dyninst

// Adding a hash function for the new Register type
namespace std {
template <> struct hash<Dyninst::Register> {
  size_t operator()(const Dyninst::Register &reg) const noexcept {
    size_t seed = 0;

    boost::hash_combine(seed, reg.getId());
    boost::hash_combine(seed, reg.getKind());
    boost::hash_combine(seed, reg.getCount());

    return seed;
  }
};
} // namespace std

#endif
