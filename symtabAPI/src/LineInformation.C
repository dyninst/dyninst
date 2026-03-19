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
#include <filesystem>
#include <list>
#include <cstring>
#include "common/src/headers.h"
#include "Module.h"

#include <iostream>
#include <limits>

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using std::vector;

#include "LineInformation.h"
#include <sstream>

LineInformation::LineInformation() :strings_(new StringTable)
{
}

bool LineInformation::StatementAddrLess::operator()(Statement::Ptr lhs, Statement::Ptr rhs) const
{
    if (lhs->startAddr() != rhs->startAddr()) return lhs->startAddr() < rhs->startAddr();
    if (lhs->endAddr() != rhs->endAddr()) return lhs->endAddr() < rhs->endAddr();
    if (lhs->getFileIndex() != rhs->getFileIndex()) return lhs->getFileIndex() < rhs->getFileIndex();
    if (lhs->getLine() != rhs->getLine()) return lhs->getLine() < rhs->getLine();
    if (lhs->getColumn() != rhs->getColumn()) return lhs->getColumn() < rhs->getColumn();
    return false;
}

bool LineInformation::LineInfoKeyLess::operator()(const LineInfoKey &lhs, const LineInfoKey &rhs) const
{
    if (lhs.file_index != rhs.file_index) return lhs.file_index < rhs.file_index;
    return lhs.line < rhs.line;
}

LineInformation::LineInfoKey LineInformation::make_key(unsigned int file_index, unsigned int line_no)
{
    return LineInfoKey{file_index, line_no};
}

std::pair<LineInformation::const_iterator, bool> LineInformation::insert(Statement::Ptr statement)
{
    auto inserted = by_addr_.insert(statement);
    if (inserted.second) {
        by_line_.emplace(make_key(statement->getFileIndex(), statement->getLine()), statement);
        by_end_.emplace(statement->endAddr(), statement);
    }
    return inserted;
}

bool LineInformation::addLine( unsigned int lineSource,
      unsigned int lineNo, 
      unsigned int lineOffset, 
      Offset lowInclusiveAddr, 
      Offset highExclusiveAddr ) 
{
    if (lowInclusiveAddr == highExclusiveAddr)  {
        // if the range is [low, low), adjust it to be [low, low + 1)
        // as the DWARF spec says the address is include along with any
        // subsequent address up to but not including the next records address
        ++highExclusiveAddr;
    }
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
    auto i = strings_->ensure(lineSource);
    return addLine(static_cast<unsigned int>(i), lineNo, lineOffset, lowInclusiveAddr, highExclusiveAddr);
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
    for (auto iter = by_end_.upper_bound(addressInRange); iter != by_end_.end(); ++iter) {
        if (*(iter->second) == addressInRange) {
            lines.push_back(iter->second);
        }
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
        ranges.push_back(AddressRange(*(i->second)));
    }

    return found_statements.first != found_statements.second;
}

LineInformation::const_iterator LineInformation::begin() const 
{
   return by_addr_.begin();
}

LineInformation::const_iterator LineInformation::end() const 
{
   return by_addr_.end();
}

LineInformation::const_iterator LineInformation::find(Offset addressInRange) const
{
    for (auto iter = by_addr_.begin(); iter != by_addr_.end(); ++iter) {
        if (**iter == addressInRange) {
            return iter;
        }
    }
    return end();
}



unsigned LineInformation::getSize() const
{
   return by_addr_.size();
}

LineInformation::const_line_info_iterator LineInformation::begin_by_source() const {
    return by_line_.begin();
}

LineInformation::const_line_info_iterator LineInformation::end_by_source() const {
    return by_line_.end();
}

std::pair<LineInformation::const_line_info_iterator, LineInformation::const_line_info_iterator>
LineInformation::range(std::string const& file, const unsigned int lineNo) const
{
    namespace fs = std::filesystem;
    auto found_range = strings_->find_by_filename(fs::path(file).filename().string());

    std::pair<const_line_info_iterator, const_line_info_iterator > bounds;
    for(auto found : found_range)
    {
        bounds = by_line_.equal_range(make_key(static_cast<unsigned int>(found), lineNo));
        if(bounds.first != bounds.second) {
            return bounds;
        }
    }
    bounds = std::make_pair(by_line_.end(), by_line_.end());
    return bounds;
}

std::pair<LineInformation::const_line_info_iterator, LineInformation::const_line_info_iterator>
LineInformation::equal_range(std::string const& file) const {
    auto found = strings_->find(file);
    if (!found) {
        return std::make_pair(by_line_.end(), by_line_.end());
    }
    return std::make_pair(by_line_.lower_bound(make_key(static_cast<unsigned int>(*found), 0)),
                          by_line_.upper_bound(make_key(static_cast<unsigned int>(*found),
                                                        std::numeric_limits<unsigned int>::max())));
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
