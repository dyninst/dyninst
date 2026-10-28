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

#ifndef DYNINST_COMMON_REGISTERS_LIVENESSSET_H
#define DYNINST_COMMON_REGISTERS_LIVENESSSET_H

#include "dyninst_visibility.h"
#include "registers/MachRegister.h"
#include "registers/register_set.h"

#include <boost/dynamic_bitset.hpp>
#include <cstddef>
#include <iosfwd>

namespace Dyninst {

  class ABI;

  /*
   * A set of registers, stored as one bit per register of an ABI's layout.
   *
   * This is the value type of register liveness analysis: it supports the set
   * algebra the dataflow equations need at bit-array speed, while letting
   * callers ask about MachRegisters instead of raw bit indices. Any alias of a
   * tracked register can be used to query or update it (see ABI).
   *
   * Sets combined with each other must come from the same ABI. A
   * default-constructed set belongs to no ABI and only becomes usable once it
   * is assigned one that does.
   */
  class DYNINST_EXPORT LivenessSet final {
  public:
    using bits_type = boost::dynamic_bitset<unsigned long>;

    LivenessSet() = default;

    // An empty set laid out by `abi`.
    explicit LivenessSet(ABI const& abi);

    // The registers of `regs` that `abi` tracks.
    LivenessSet(ABI const& abi, register_set const& regs);

    ABI const* getABI() const { return abi_; }

    // Register view
    bool contains(MachRegister reg) const;
    void insert(MachRegister reg);
    void erase(MachRegister reg);
    register_set registers() const;

    // Bit view
    std::size_t size() const { return bits_.size(); }
    std::size_t count() const { return bits_.count(); }
    bool any() const { return bits_.any(); }
    bool none() const { return bits_.none(); }
    bits_type const& bits() const { return bits_; }

    LivenessSet& insertAll();
    LivenessSet& clear();

    // Set algebra. Complement is relative to every register the ABI tracks.
    LivenessSet& operator|=(LivenessSet const& rhs);
    LivenessSet& operator&=(LivenessSet const& rhs);
    LivenessSet& operator-=(LivenessSet const& rhs);
    LivenessSet& operator^=(LivenessSet const& rhs);
    LivenessSet operator~() const;

    friend LivenessSet operator|(LivenessSet lhs, LivenessSet const& rhs) { return lhs |= rhs; }
    friend LivenessSet operator&(LivenessSet lhs, LivenessSet const& rhs) { return lhs &= rhs; }
    friend LivenessSet operator-(LivenessSet lhs, LivenessSet const& rhs) { return lhs -= rhs; }
    friend LivenessSet operator^(LivenessSet lhs, LivenessSet const& rhs) { return lhs ^= rhs; }

    bool operator==(LivenessSet const& rhs) const;
    bool operator!=(LivenessSet const& rhs) const { return !(*this == rhs); }

    // True if every register in this set is also in `rhs`.
    bool isSubsetOf(LivenessSet const& rhs) const;
    bool intersects(LivenessSet const& rhs) const;

  private:
    ABI const* abi_{nullptr};
    bits_type bits_;

    void checkCompatible(LivenessSet const& rhs) const;
  };

  // Prints the names of the registers in the set, e.g. "{rax, rsp, zmm0}".
  DYNINST_EXPORT std::ostream& operator<<(std::ostream& os, LivenessSet const& s);

}

#endif
