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

#if ! defined( LINE_INFORMATION_H )
#define LINE_INFORMATION_H

#include <string>
#include <map>
#include <set>
#include <utility>
#include <vector>
#include "symutil.h"
#include "Annotatable.h"
#include "RangeLookup.h"
#include "Statement.h"

#define NEW_GETSOURCELINES_INTERFACE

namespace Dyninst{
namespace SymtabAPI{

class SYMTAB_EXPORT LineInformation final {
public:
    typedef Statement::Ptr Statement_t;

private:
    struct StatementAddrLess {
        bool operator()(Statement::Ptr lhs, Statement::Ptr rhs) const;
    };

    struct LineInfoKey {
        unsigned int file_index{};
        unsigned int line{};
    };

    struct LineInfoKeyLess {
        bool operator()(const LineInfoKey &lhs, const LineInfoKey &rhs) const;
    };

    using addr_index_t = std::set<Statement::Ptr, StatementAddrLess>;
    using line_index_t = std::multimap<LineInfoKey, Statement::Ptr, LineInfoKeyLess>;
    using upper_bound_index_t = std::multimap<Offset, Statement::Ptr>;

public:
    typedef addr_index_t::const_iterator const_iterator;
    typedef line_index_t::const_iterator const_line_info_iterator;

      LineInformation();

      bool addLine( const std::string &lineSource,
            unsigned int lineNo, 
            unsigned int lineOffset, 
            Offset lowInclusiveAddr, 
            Offset highExclusiveAddr );
    bool addLine( unsigned int fileIndex,
                  unsigned int lineNo,
                  unsigned int lineOffset,
                  Offset lowInclusiveAddr,
                  Offset highExclusiveAddr );

      void addLineInfo(LineInformation *lineInfo);	      

      bool addAddressRange( Offset lowInclusiveAddr, 
            Offset highExclusiveAddr, 
            const char * lineSource, 
            unsigned int lineNo, 
            unsigned int lineOffset = 0 );

      bool getSourceLines(Offset addressInRange, std::vector<Statement_t> &lines);
    bool getSourceLines(Offset addressInRange, std::vector<Statement> &lines);

      bool getAddressRanges( const char * lineSource, unsigned int LineNo, std::vector< AddressRange > & ranges );
      const_line_info_iterator begin_by_source() const;
      const_line_info_iterator end_by_source() const;
      std::pair<const_line_info_iterator, const_line_info_iterator> range(std::string const& file,
                                                                                const unsigned int lineNo) const;
      std::pair<const_line_info_iterator, const_line_info_iterator> equal_range(std::string const& file) const;
      const_iterator begin() const;
      const_iterator end() const;
      const_iterator find(Offset addressInRange) const;
      const_iterator find(Offset addressInRange, const_iterator hint) const;

      unsigned getSize() const;

      void dump();

      ~LineInformation() = default;
        StringTablePtr strings_;

    StringTablePtr getStrings() ;

    void setStrings(StringTablePtr strings_);

private:
    std::pair<const_iterator, bool> insert(Statement::Ptr statement);

    template <typename Iterator>
    void insert(Iterator first, Iterator last) {
        for (; first != last; ++first) {
            insert(*first);
        }
    }

    static LineInfoKey make_key(unsigned int file_index, unsigned int line_no);

    addr_index_t by_addr_;
    line_index_t by_line_;
    upper_bound_index_t by_end_;
};

}//namespace SymtabAPI
}//namespace Dyninst

#endif /* ! LINE_INFORMATION_H */
