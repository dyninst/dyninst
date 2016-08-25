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

#include <assert.h>
#include <list>
#include <cstring>
#include "boost/functional/hash.hpp"
#include "common/src/headers.h"
#include "Module.h"
#include "Serialization.h"

#include <functional>

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using std::vector;

#include "LineInformation.h"

LineInformation::LineInformation()
{
} /* end LineInformation constructor */

bool LineInformation::addItem_impl(Statement s)
{
    insert(Statement::ConstPtr(new Statement(s)));
//    insert(s);
	return true;
}
bool LineInformation::addLine( const char * lineSource, 
      unsigned int lineNo, 
      unsigned int lineOffset, 
      Offset lowInclusiveAddr, 
      Offset highExclusiveAddr ) 
{


   bool ret = addItem_impl( Statement(lineSource, lineNo, lineOffset, 
                                      lowInclusiveAddr, highExclusiveAddr)); 

   return ret;
} /* end setLineToAddressRangeMapping() */

void LineInformation::addLineInfo(LineInformation *lineInfo)
{
    if(!lineInfo)
        return;
    insert(lineInfo->begin(), lineInfo->end());
}

bool LineInformation::addAddressRange( Offset lowInclusiveAddr, 
      Offset highExclusiveAddr, 
      const char * lineSource, 
      unsigned int lineNo, 
      unsigned int lineOffset ) 
{
   return addLine( lineSource, lineNo, lineOffset, lowInclusiveAddr, highExclusiveAddr );
} /* end setAddressRangeToLineMapping() */

bool LineInformation::getSourceLines(Offset addressInRange,
                                     vector<Statement_t> &lines)
{
    using namespace std::placeholders;
    auto start_addr_valid = lower_bound(addressInRange + 1);
    std::copy_if(begin(),
                 start_addr_valid,
                 std::back_inserter(lines),
                 std::bind(&Statement::contains, std::placeholders::_1, addressInRange));
    return start_addr_valid != begin();
} /* end getLinesFromAddress() */

bool LineInformation::getSourceLines( Offset addressInRange,
                                      vector<LineNoTuple> &lines)
{
    vector<Statement_t> tmp;
    if(!getSourceLines(addressInRange, tmp)) return false;
    for(auto i = tmp.begin(); i != tmp.end(); ++i)
    {
        lines.push_back(**i);
    }
    return true;
} /* end getLinesFromAddress() */



bool LineInformation::getAddressRanges( const char * lineSource, 
      unsigned int lineNo, vector< AddressRange > & ranges ) 
{
    auto found_statements = equal_range(lineSource, lineNo);
    std::transform(found_statements.first, found_statements.second,
                   std::back_inserter(ranges),
    std::mem_fn(&Statement::addressRange));
    return found_statements.first != found_statements.second;
} /* end getAddressRangesFromLine() */

LineInformation::const_iterator LineInformation::begin() const 
{
   return impl_t::begin();
} /* end begin() */

LineInformation::const_iterator LineInformation::end() const 
{
   return impl_t::end();
} /* end end() */

LineInformation::const_iterator LineInformation::find(Offset addressInRange) const
{
    return impl_t::find(addressInRange);
} /* end find() */



unsigned LineInformation::getSize() const
{
   return impl_t::size();
}



LineInformation::~LineInformation() 
{
}

LineInformation::const_line_info_iterator LineInformation::begin_by_source() const {
    const traits::line_info_index& i = get<traits::line_info>();
    return i.begin();
}

LineInformation::const_line_info_iterator LineInformation::end_by_source() const {
    const traits::line_info_index& i = get<traits::line_info>();
    return i.end();
}

std::pair<LineInformation::const_line_info_iterator, LineInformation::const_line_info_iterator>
LineInformation::equal_range(std::string file, const unsigned int lineNo) const {
    return get<traits::line_info>().equal_range(std::make_tuple(file, lineNo));

}

std::pair<LineInformation::const_line_info_iterator, LineInformation::const_line_info_iterator>
LineInformation::equal_range(std::string file) const {
    return get<traits::line_info>().equal_range(file);
//    const traits::line_info_index& by_line_info = impl_t::get<traits::line_info>();
//    return by_line_info.equal_range(file);

}

/* end LineInformation destructor */

