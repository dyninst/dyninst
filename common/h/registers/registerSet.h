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

#ifndef DYNINST_DATAFLOW_REGISTERSET_H
#define DYNINST_DATAFLOW_REGISTERSET_H

#include "registers/MachRegister.h"
#include <unordered_set>
#include <initializer_list>

namespace Dyninst { namespace abi {

  namespace detail {
    struct reg_hasher final {
      std::size_t operator()(Dyninst::MachRegister m) const {
        return m.val();
      }
    };
  }

  class registerSet final {
    std::unordered_set<Dyninst::MachRegister, detail::reg_hasher> regs;
  public:
    registerSet(std::initializer_list<Dyninst::MachRegister> l) : regs(std::move(l)) {}
    registerSet() = default;

    bool contains(Dyninst::MachRegister m) const {
      return regs.find(m) != regs.end();
    }
    void insert(MachRegister m) {
      regs.insert(m);
    }
    void remove(MachRegister m) {
      regs.erase(m);
    }
    auto begin() const -> decltype(regs.begin()) {
      return regs.begin();
    }
    auto end() const -> decltype(regs.end()) {
      return regs.end();
    }
    void operator|=(registerSet const& rhs) {
      for(auto r : rhs) {
        regs.insert(r);
      }
    }
    registerSet operator|(registerSet const& rhs) const {
      auto tmp = *this;
      tmp |= rhs;
      return tmp;
    }
    void operator&=(registerSet const& rhs) {
      for(auto r : regs) {
        if(rhs.contains(r)) {
          regs.erase(r);
        }
      }
    }
    registerSet operator&(registerSet const& rhs) const {
      auto tmp = *this;
      tmp &= rhs;
      return tmp;
    }
  };

}}

#endif
