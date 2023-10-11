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
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/composite_key.hpp>
#include <boost/scoped_ptr.hpp>


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
        template <typename Value>
        struct RangeLookupTypes
        {
            typedef typename boost::multi_index::composite_key<Value,
                    boost::multi_index::const_mem_fun<Value, Offset, &Value::startAddr>,
                    boost::multi_index::const_mem_fun<Value, Offset, &Value::endAddr> >
                    addr_range_key;
            typedef typename boost::multi_index::composite_key<Value,
                    boost::multi_index::const_mem_fun<Value, Offset, &Value::endAddr>,
                    boost::multi_index::const_mem_fun<Value, Offset, &Value::startAddr> >
            upper_bound_key;
            typedef typename boost::multi_index::composite_key<Value,
                    boost::multi_index::const_mem_fun<Value, unsigned int, &Value::getFileIndex>,
                    boost::multi_index::const_mem_fun<Value, unsigned int, &Value::getLine> >
                    line_info_key;
            typedef typename boost::multi_index_container
                    <
                            typename Value::Ptr,
                            boost::multi_index::indexed_by<
                                    boost::multi_index::ordered_non_unique< boost::multi_index::tag<typename Value::addr_range>, addr_range_key>,
                                    boost::multi_index::ordered_non_unique< boost::multi_index::tag<typename Value::upper_bound>, upper_bound_key>,
                                    boost::multi_index::ordered_non_unique< boost::multi_index::tag<typename Value::line_info>, line_info_key >
                            >
                    > type;
            typedef typename boost::multi_index::index<type, typename Value::addr_range>::type addr_range_index;
            typedef typename boost::multi_index::index<type, typename Value::upper_bound>::type upper_bound_index;
            typedef typename boost::multi_index::index<type, typename Value::line_info>::type line_info_index;
            typedef typename type::value_type value_type;


        };


    }
}
#endif

