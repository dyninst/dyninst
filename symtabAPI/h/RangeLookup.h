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

#if ! defined( RANGE_LOOKUP_H )
#define RANGE_LOOKUP_H

#include <ostream>
#include <utility>
#include <map>
#include <vector>
#include <list>
#include <assert.h>
#include "dyntypes.h"
#include "util.h"
#include "IBSTree.h"

namespace Dyninst {
    namespace SymtabAPI {

        struct SYMTAB_EXPORT AddressRange : std::pair<Offset, Offset>
        {
            template <typename T>
            AddressRange(Dyninst::SimpleInterval<T> i) {
                first = i.low();
                second = i.high();
            }
            AddressRange(Offset t)
            {
                first = t;
                second = t;
            }
            AddressRange(Offset start, Offset end)
            {
                first = start;
                second = end;
            }
            AddressRange(const AddressRange& other) noexcept : std::pair<Offset, Offset>(other)
            {
            }
            AddressRange& operator=(const AddressRange& other)
            {
                first = other.first;
                second = other.second;
                return *this;
            }
            AddressRange merge(const AddressRange& other)
            {
                return AddressRange(std::min(first, other.first), std::max(second, other.second));
            }
            bool operator==(const AddressRange& rhs) const {
                return (first == rhs.first) && (second == rhs.second);
            }
            bool operator<(const AddressRange& rhs) const {
                return (first < rhs.first) || ((first == rhs.first) && (second < rhs.second));
            }
            bool contains(Offset off) const {
                return (first <= off) && (off < second);
            }

        };
        inline std::ostream& operator<<(std::ostream& os, AddressRange ar)
        {
            os << std::hex << "[" << ar.first << ", " << ar.second << ")";
            return os;
        }
    }
}
#endif
