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
#include <utility>
#include <vector>
#include <boost/make_shared.hpp>
#include <mutex>
#include <condition_variable>
#include "symutil.h"
#include "RangeLookup.h"
#include "Annotatable.h"
#include "Statement.h"
#include "StringTable.h"

#define NEW_GETSOURCELINES_INTERFACE

namespace Dyninst{
namespace SymtabAPI{

class Object;
class Module;

class DYNINST_EXPORT LineInformation final :
                        private RangeLookupTypes< Statement >::type
{
public:
    typedef RangeLookupTypes< Statement> traits;
    typedef RangeLookupTypes< Statement >::type impl_t;
    typedef impl_t::index<Statement::addr_range>::type::const_iterator const_iterator;
    typedef impl_t::index<Statement::line_info>::type::const_iterator const_line_info_iterator;
    typedef traits::value_type Statement_t;
    LineInformation(Object *o = nullptr, Module *m = nullptr): obj(o), mod(m) {}
    LineInformation(Module *m): LineInformation(nullptr, m) {}

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

    StringTablePtr getStrings() ;

    void ReaderLock(bool init = true);  // if init, initialized LineInformation
    void ReaderUnlock();
    void WriterLock(bool init = true);  // if init, initialized LineInformation
    void WriterUnlock();

    void Initialize();
    bool IsInitialized() const {return isInitialized;}

    class ReaderLockGuard
    {
        public:
            ReaderLockGuard() = default;
            ReaderLockGuard(LineInformation *li, bool init = true) : lineInfo(li)  {
                li->ReaderLock(init);
            }
            ~ReaderLockGuard()  {
                if (lineInfo) lineInfo->ReaderUnlock();
            }
            ReaderLockGuard(const ReaderLockGuard&) = delete;
            ReaderLockGuard& operator=(ReaderLockGuard&) = delete;
            ReaderLockGuard(ReaderLockGuard &&rhs) noexcept : lineInfo(rhs.lineInfo)  {
                rhs.lineInfo = nullptr;
            }
            ReaderLockGuard& operator=(ReaderLockGuard&& rhs) noexcept  {
                if (this != &rhs)  {
                    if (lineInfo) lineInfo->ReaderUnlock();
                    lineInfo = rhs.lineInfo;
                    rhs.lineInfo = nullptr;
                }
                return *this;
            }
        private:
            LineInformation *lineInfo{};
    };

    class WriterLockGuard
    {
        public:
            WriterLockGuard() = default;
            WriterLockGuard(LineInformation *li, bool init = true) : lineInfo(li)  {
                li->WriterLock(init);
            }
            ~WriterLockGuard()  {
                if (lineInfo) lineInfo->WriterUnlock();
            }
            WriterLockGuard(const WriterLockGuard&) = delete;
            WriterLockGuard& operator=(WriterLockGuard&) = delete;
            WriterLockGuard(WriterLockGuard &&rhs) noexcept : lineInfo(rhs.lineInfo)  {
                rhs.lineInfo = nullptr;
            }
            WriterLockGuard& operator=(WriterLockGuard&& rhs) noexcept  {
                if (this != &rhs)  {
                    if (lineInfo) lineInfo->WriterUnlock();
                    lineInfo = rhs.lineInfo;
                    rhs.lineInfo = nullptr;
                }
                return *this;
            }
        private:
            LineInformation *lineInfo{};
    };

    ReaderLockGuard GetReaderLockGuard(bool init = true)  {
        return ReaderLockGuard(this, init);
    }
    WriterLockGuard GetWriterLockGuard(bool init = true)  {
        return WriterLockGuard(this, init);
    }

private:
    StringTablePtr stringTable{boost::make_shared<StringTable>()};

    Object *obj{};              // Object associated with LinoInformation
    Module *mod{};              // Module associated with LinoInformation

    std::mutex                  lineInfoMutex;
    std::condition_variable     writersCV;      // waiting writers
    std::condition_variable     readersCV;      // waiting readers

    // accessed only when lineInfoMutex is locked
    bool    isInitialized{};    // line information is initialized
    bool    activeWriter{};     // a writer holds the lock
    int     activeReadersCnt{}; // number of readers holding lock
    int     allWritersCnt{};    // number of writers holding and waiting on lock
};

}//namespace SymtabAPI
}//namespace Dyninst

#endif /* ! LINE_INFORMATION_H */
