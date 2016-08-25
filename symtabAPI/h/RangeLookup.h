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

#include <map>
#include <vector>
#include <list>
#include <assert.h>
#include "dyntypes.h"
#include "util.h"
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
            AddressRange(T t) {
                first = t.startAddr();
                second = t.endAddr();
            }
            AddressRange(Offset t)
            {
                first = t;
                second = t;
            }
            bool operator==(const AddressRange& rhs) const {
                return first == rhs.first && second == rhs.second;
            }
            bool operator<(const AddressRange& rhs) const {
                return first < rhs.first || (first == rhs.first && second < rhs.second);
            }
            bool contains(Offset off) const {
                return first <= off && off < second;
            }

        };
        template <typename Value>
        struct RangeLookupTypes
        {
            struct addr_range {};
            struct line_info {};
            typedef typename boost::multi_index::const_mem_fun<Value, AddressRange, &Value::addressRange> addr_range_key;
            typedef typename boost::multi_index::composite_key<Value,
                    boost::multi_index::const_mem_fun<Value, std::string, &Value::getFile>,
                    boost::multi_index::const_mem_fun<Value, unsigned int, &Value::getLine> > line_info_key;
            typedef typename boost::multi_index_container
                    <
                            typename Value::ConstPtr,
                            boost::multi_index::indexed_by<
                                    boost::multi_index::ordered_unique< boost::multi_index::tag<addr_range>, addr_range_key>,
                                    boost::multi_index::ordered_non_unique< boost::multi_index::tag<line_info>, line_info_key >
                            >
                    > type;
            typedef typename boost::multi_index::index<type, addr_range>::type addr_range_index;
            typedef typename boost::multi_index::index<type, line_info>::type line_info_index;
            typedef typename type::value_type value_type;


        };

//        template<typename Value, typename Compare>
//        class SYMTAB_EXPORT RangeLookup : public RangeLookupIndexTypes<Value>::Values {
//
//        public:
//            typedef std::pair<Offset, Offset> AddressRange;
//            typedef typename RangeLookupIndexTypes<Value>::Values parent;
//            RangeLookup();
//
//            /* Values are copied: a RangeLookup considers itself the primary repository. */
//            bool addValue(Value v) {
//                parent::insert(v);
//                return true;
//            }
//
//            /* Likewise, copies of the values are returned. */
//            bool getValues(Offset addressInRange, std::vector<Value> &values) {
//                auto by_addr_range = parent::get<0>();
//                auto found = by_addr_range.equal_range(addressInRange);
//                std::copy(found.first, found.second, std::back_inserter(values));
//                return found.first != found.second;
//            }
//            template <typename _OI>
//            bool getAddressRanges(const char* lineSource, int lineNo, _OI out_iter) const {
//                auto &by_line_info = parent::get<1>();
//                if(lineNo <= 0) // wildcard it
//                {
//                    auto lines = by_line_info.equal_range(lineSource);
//                    std::copy(lines.first, lines.second, out_iter);
//                    return lines.first != lines.second;
//                }
//                auto lines = by_line_info.equal_range(lineSource, lineNo);
//                std::copy(lines.first, lines.second, out_iter);
//                return lines.first != lines.second;
//
//            }
//            virtual ~RangeLookup();
//
//
//        }; /* end class RangeLookup */

    }
}
#endif