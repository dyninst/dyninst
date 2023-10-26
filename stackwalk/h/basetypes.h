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

#ifndef BASETYPES_H_
#define BASETYPES_H_

#include "dyntypes.h"
#include "registers/MachRegister.h"
#include "SymReader.h"
#include <string>
#include <sstream>
#include <iostream>

namespace Dyninst {
namespace Stackwalker {

typedef enum { loc_address, loc_register, loc_unknown } storage_t;
struct location_t {
  bool operator==(const location_t &L) const
  {
    return ((val.addr == L.val.addr) &&
            (val.reg == L.val.reg) &&
            (location == L.location));
  }
  struct {
    Dyninst::Address addr;
    Dyninst::MachRegister reg;
  } val;
  storage_t location;

  std::string format() const {
      std::stringstream ret;
    switch (location) {
    case loc_unknown:
      ret << "<loc: unknown>";
      break;
    case loc_register:
      ret << "<loc: reg " << val.reg.name() << ">";
      break;
    case loc_address:
      ret << "<loc: addr " << std::hex << val.addr << ">" << std::dec;
      break;
    }
    return ret.str();
  }

};

}
}

#endif
