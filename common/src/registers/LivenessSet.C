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

#include "registers/LivenessSet.h"
#include "registers/abi.h"

#include <cassert>
#include <ostream>

namespace Dyninst {

  LivenessSet::LivenessSet(ABI const& abi) : abi_(&abi), bits_(abi.size()) {}

  LivenessSet::LivenessSet(ABI const& abi, register_set const& regs) : LivenessSet(abi) {
    for(auto r : regs) {
      insert(r);
    }
  }

  bool LivenessSet::contains(MachRegister reg) const {
    assert(abi_);
    auto const i = abi_->getIndex(reg);
    return i >= 0 && bits_.test(static_cast<std::size_t>(i));
  }

  // Registers outside the ABI's layout are ignored.
  void LivenessSet::insert(MachRegister reg) {
    assert(abi_);
    auto const i = abi_->getIndex(reg);
    if(i >= 0) {
      bits_.set(static_cast<std::size_t>(i));
    }
  }

  void LivenessSet::erase(MachRegister reg) {
    assert(abi_);
    auto const i = abi_->getIndex(reg);
    if(i >= 0) {
      bits_.reset(static_cast<std::size_t>(i));
    }
  }

  register_set LivenessSet::registers() const {
    register_set regs;
    for(auto i = bits_.find_first(); i != bits_type::npos; i = bits_.find_next(i)) {
      regs.insert(abi_->getRegister(i));
    }
    return regs;
  }

  LivenessSet& LivenessSet::insertAll() {
    bits_.set();
    return *this;
  }

  LivenessSet& LivenessSet::clear() {
    bits_.reset();
    return *this;
  }

  void LivenessSet::checkCompatible(LivenessSet const& rhs) const {
    assert(abi_ && abi_ == rhs.abi_ && "LivenessSets from different ABIs");
    (void)rhs;
  }

  LivenessSet& LivenessSet::operator|=(LivenessSet const& rhs) {
    checkCompatible(rhs);
    bits_ |= rhs.bits_;
    return *this;
  }

  LivenessSet& LivenessSet::operator&=(LivenessSet const& rhs) {
    checkCompatible(rhs);
    bits_ &= rhs.bits_;
    return *this;
  }

  LivenessSet& LivenessSet::operator-=(LivenessSet const& rhs) {
    checkCompatible(rhs);
    bits_ -= rhs.bits_;
    return *this;
  }

  LivenessSet& LivenessSet::operator^=(LivenessSet const& rhs) {
    checkCompatible(rhs);
    bits_ ^= rhs.bits_;
    return *this;
  }

  LivenessSet LivenessSet::operator~() const {
    auto tmp = *this;
    tmp.bits_.flip();
    return tmp;
  }

  bool LivenessSet::operator==(LivenessSet const& rhs) const {
    return abi_ == rhs.abi_ && bits_ == rhs.bits_;
  }

  bool LivenessSet::isSubsetOf(LivenessSet const& rhs) const {
    checkCompatible(rhs);
    return bits_.is_subset_of(rhs.bits_);
  }

  bool LivenessSet::intersects(LivenessSet const& rhs) const {
    checkCompatible(rhs);
    return bits_.intersects(rhs.bits_);
  }

  std::ostream& operator<<(std::ostream& os, LivenessSet const& s) {
    os << '{';
    char const* sep = "";
    for(auto i = s.bits().find_first(); i != LivenessSet::bits_type::npos; i = s.bits().find_next(i)) {
      os << sep << s.getABI()->getRegister(i).name();
      sep = ", ";
    }
    return os << '}';
  }

}
