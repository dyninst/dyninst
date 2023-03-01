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

#include "util.h"
#include <boost/optional.hpp>
#include <dwarf.h>
#include <elfutils/libdw.h>

namespace Dyninst {
namespace DwarfDyninst {

struct dwarf_error {};

struct dwarf_result {
  boost::optional<Dwarf_Word> value;
  bool error = false;
  dwarf_result() = default;
  dwarf_result(Dwarf_Word t) : value{t} {}
  dwarf_result(dwarf_error) : error{true} {}
  explicit operator bool() const { return !error; }
};

struct dwarf_bounds {
	dwarf_result lower, upper;
};

DYNDWARF_EXPORT dwarf_bounds dwarf_subrange_bounds(Dwarf_Die *die);
DYNDWARF_EXPORT dwarf_result dwarf_subrange_length_from_enum(Dwarf_Die *die);

} // namespace DwarfDyninst
} // namespace Dyninst
