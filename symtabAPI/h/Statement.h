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

#ifndef __STATEMENT__H__
#define __STATEMENT__H__

#include "Annotatable.h"
#include "RangeLookup.h"
#include "StringTable.h"
#include "util.h"

namespace Dyninst { namespace SymtabAPI {

  class LineInformation;

  class SYMTAB_EXPORT Statement : public AddressRange {
    friend class Module;
    friend class LineInformation;

    Statement(int file_index, unsigned int line, unsigned int col = 0,
              Offset start_addr = (Offset)-1L, Offset end_addr = (Offset)-1L)
        : AddressRange(start_addr, end_addr), file_index_(file_index), line_(line), column_(col) {}

    unsigned int file_index_{}; // Maybe this should be module?
    unsigned int line_{};
    unsigned int column_{};
    StringTablePtr strings_{};

  public:
    StringTablePtr getStrings_() const;

    void setStrings_(StringTablePtr strings_);

    Statement() : AddressRange(0, 0) {}

    Statement(const Statement &) = default;

    ~Statement() = default;

    struct StatementLess {
      bool operator()(const Statement &lhs, const Statement &rhs) const;
    };

    typedef StatementLess LineNoTupleLess;
    bool operator==(const Statement &cmp) const;

    bool operator==(Offset addr) const { return AddressRange::contains(addr); }

    bool operator<(Offset addr) const { return startAddr() <= addr; }

    bool operator>(Offset addr) const { return !((*this) < addr || (*this == addr)); }

    Offset startAddr() const { return first; }

    Offset endAddr() const { return second; }

    const std::string &getFile() const;

    unsigned int getFileIndex() const { return file_index_; }

    unsigned int getLine() const { return line_; }

    unsigned int getColumn() const { return column_; }

    struct addr_range {};

    struct line_info {};

    struct upper_bound {};

    typedef Statement *Ptr;
    typedef const Statement *ConstPtr;
  };

  typedef Statement LineNoTuple;

  template <typename OS> OS &operator<<(OS &os, const Statement &s) {
    os << "<statement>: [" << std::hex << s.startAddr() << ", " << s.endAddr() << std::dec << ") @ "
       << s.getFile() << " (" << s.getFileIndex() << "): " << s.getLine();
    return os;
  }

  template <typename OS> OS &operator<<(OS &os, Statement *s) {
    os << "<statement>: [" << std::hex << s->startAddr() << ", " << s->endAddr() << std::dec
       << ") @ " << s->getFile() << " (" << s->getFileIndex() << "): " << s->getLine();
    return os;
  }

}}

#endif
