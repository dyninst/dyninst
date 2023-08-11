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
#include <boost/filesystem.hpp>
#include "boost/functional/hash.hpp"
#include "common/src/headers.h"
#include "Module.h"

#include <functional>
#include <iostream>

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using std::vector;

#include "LineInformation.h"
#include <sstream>

LineInformation::LineInformation() :strings_(new StringTable)
{
}

bool LineInformation::addLine( unsigned int lineSource,
      unsigned int lineNo, 
      unsigned int lineOffset, 
      Offset lowInclusiveAddr, 
      Offset highExclusiveAddr ) 
{
    Statement* the_stmt = new Statement(lineSource, lineNo, lineOffset,
                                        lowInclusiveAddr, highExclusiveAddr);
    Statement::Ptr insert_me(the_stmt);
    insert_me->setStrings_(strings_);
   bool result;
#pragma omp critical (addLine)
{
   result = insert( insert_me).second;
}
   return result;
}
bool LineInformation::addLine( const std::string &lineSource,
                               unsigned int lineNo,
                               unsigned int lineOffset,
                               Offset lowInclusiveAddr,
                               Offset highExclusiveAddr )
{
    // lookup or insert linesource in string table and get iterator
    auto iter = strings_->get<1>().insert(StringTableEntry(lineSource,"")).first;

    // get index of string in string table
    auto i = boost::multi_index::project<0>(*strings_, iter) - strings_->get<0>().begin();

    return addLine(i, lineNo, lineOffset, lowInclusiveAddr, highExclusiveAddr);
}

void LineInformation::addLineInfo(LineInformation *lineInfo)
{
    if(!lineInfo)
        return;
#pragma omp critical (addLine)
{
    insert(lineInfo->begin(), lineInfo->end());
}
}

bool LineInformation::addAddressRange( Offset lowInclusiveAddr, 
      Offset highExclusiveAddr, 
      const char * lineSource, 
      unsigned int lineNo, 
      unsigned int lineOffset ) 
{
   return addLine( lineSource, lineNo, lineOffset, lowInclusiveAddr, highExclusiveAddr );
}


std::string print(const Dyninst::SymtabAPI::Statement& stmt)
{
    std::stringstream stream;
    stream << std::hex << "Statement: < [" << stmt.startAddr() << ", " << stmt.endAddr() << "): "
           << std::dec << stmt.getFile() << ":" << stmt.getLine() << " >";
    return stream.str();
}


bool LineInformation::getSourceLines(Offset addressInRange,
                                     vector<Statement_t> &lines)
{
    const_iterator start_addr_valid = project<Statement::addr_range>(get<Statement::upper_bound>().lower_bound(addressInRange ));
    const_iterator end_addr_valid = impl_t::upper_bound(addressInRange );
    while(start_addr_valid != end_addr_valid && start_addr_valid != end())
    {
        if(*(*start_addr_valid) == addressInRange)
        {
            lines.push_back(*start_addr_valid);
        }
        ++start_addr_valid;
    }
    return true;
}

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
}



bool LineInformation::getAddressRanges( const char * lineSource, 
      unsigned int lineNo, vector< AddressRange > & ranges )
{
    auto found_statements = range(lineSource, lineNo);
    for(auto i = found_statements.first;
            i != found_statements.second;
            ++i)
    {
        ranges.push_back(AddressRange(**i));
    }

    return found_statements.first != found_statements.second;
}

LineInformation::const_iterator LineInformation::begin() const 
{
   return impl_t::begin();
}

LineInformation::const_iterator LineInformation::end() const 
{
   return impl_t::end();
}

LineInformation::const_iterator LineInformation::find(Offset addressInRange) const
{
    const_iterator start_addr_valid = project<Statement::addr_range>(get<Statement::upper_bound>().lower_bound(addressInRange ));
    if(start_addr_valid == end()) return end();
    const_iterator end_addr_valid = impl_t::upper_bound(addressInRange + 1);
    while(start_addr_valid != end_addr_valid && start_addr_valid != end())
    {
        if(*(*start_addr_valid) == addressInRange)
        {
            return start_addr_valid;
        }
        ++start_addr_valid;
    }
    return end();
}



unsigned LineInformation::getSize() const
{
   return impl_t::size();
}

LineInformation::const_line_info_iterator LineInformation::begin_by_source() const {
    const traits::line_info_index& i = impl_t::get<Statement::line_info>();
    return i.begin();
}

LineInformation::const_line_info_iterator LineInformation::end_by_source() const {
    const traits::line_info_index& i = impl_t::get<Statement::line_info>();
    return i.end();
}

std::pair<LineInformation::const_line_info_iterator, LineInformation::const_line_info_iterator>
LineInformation::range(std::string const& file, const unsigned int lineNo) const
{
    using namespace boost::filesystem;
    auto found_range = strings_->get<2>().equal_range(path(file).filename().string());

    std::pair<const_line_info_iterator, const_line_info_iterator > bounds;
    for(auto found = found_range.first; ((found != found_range.second) && (found != strings_->get<2>().end())); ++found)
    {
        unsigned i = strings_->project<0>(found) - strings_->begin();
        auto idx = boost::make_tuple(i, lineNo);
        bounds =  get<Statement::line_info>().equal_range(idx);
        if(bounds.first != bounds.second) {
            return bounds;
        }
    }
    bounds = make_pair(get<Statement::line_info>().end(), get<Statement::line_info>().end());
    return bounds;
}

std::pair<LineInformation::const_line_info_iterator, LineInformation::const_line_info_iterator>
LineInformation::equal_range(std::string const& file) const {
    auto found = strings_->get<1>().find(file);
    unsigned i = strings_->project<0>(found) - strings_->begin();
    return get<Statement::line_info>().equal_range(i);
}

StringTablePtr LineInformation::getStrings()  {
    return strings_;
}

void LineInformation::setStrings(StringTablePtr strings) {
    LineInformation::strings_ = strings;
}

LineInformation::const_iterator LineInformation::find(Offset addressInRange, const_iterator hint) const {
    while(hint != end())
    {
        if((**hint) == addressInRange) return hint;
        if((**hint) > addressInRange) break;
        ++hint;
    }
    return find(addressInRange);
}


void LineInformation::dump()
{
  for (auto i = begin(); i != end(); i++) {
    const Statement *stmt = *i;
    std::cerr <<
      "[" <<
      std::hex <<
      stmt->startAddr() <<
      "," <<
      stmt->endAddr() <<
      std::dec <<
      ") " <<
      stmt->getFile() <<
      ":" <<
      stmt->getLine() <<
      std::endl;
  }
}
