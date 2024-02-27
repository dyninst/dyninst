/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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
#ifndef _SYMLITE_CODE_SOURCE_H_
#define _SYMLITE_CODE_SOURCE_H_

#include <map>
#include <vector>
#include <utility>
#include <string>

#include "CodeSource.h"


namespace Dyninst {
namespace ParseAPI {

class CFGModifier;

/** SymReaderCodeRegion and SymReaderCodeSource implement CodeSource for program
    binaries supported by the SymReaderAPI 
**/

class SymReaderCodeRegion : public CodeRegion {
 private:
    SymReader * _symtab;
    SymSegment * _region;
    void* rawData;
    
 public:
    PARSER_EXPORT SymReaderCodeRegion(SymReader *, SymSegment *);
    PARSER_EXPORT ~SymReaderCodeRegion();

    PARSER_EXPORT void names(Address, std::vector<std::string> &) override;
    PARSER_EXPORT bool findCatchBlock(Address addr, Address & catchStart) override;

    /** InstructionSource implementation **/
    PARSER_EXPORT bool isValidAddress(const Address) const override;
    PARSER_EXPORT void* getPtrToInstruction(const Address) const override;
    PARSER_EXPORT void* getPtrToData(const Address) const override;
    PARSER_EXPORT unsigned int getAddressWidth() const override;
    PARSER_EXPORT bool isCode(const Address) const override;
    PARSER_EXPORT bool isData(const Address) const override;
    PARSER_EXPORT bool isReadOnly(const Address) const override;
    PARSER_EXPORT Address offset() const override;
    PARSER_EXPORT Address length() const override;
    PARSER_EXPORT Architecture getArch() const override;

    /** interval **/
    PARSER_EXPORT Address low() const override { return offset(); }
    PARSER_EXPORT Address high() const override { return offset() + length(); }

    PARSER_EXPORT SymSegment * symRegion() const { return _region; }
};

class SymReaderCodeSource : public CodeSource {
 private:
    SymReader * _symtab;
    bool owns_symtab;
    mutable CodeRegion * _lookup_cache;

    // Stats information
    StatContainer * stats_parse;
    bool _have_stats;
    
 public:
    PARSER_EXPORT SymReaderCodeSource(SymReader *);
    PARSER_EXPORT SymReaderCodeSource(const char *);

    PARSER_EXPORT ~SymReaderCodeSource();

    PARSER_EXPORT bool nonReturning(Address func_entry);
    PARSER_EXPORT bool nonReturningSyscall(int num);

    PARSER_EXPORT bool resizeRegion(SymSegment *, Address newDiskSize);

    PARSER_EXPORT SymReader * getSymReaderObject() {return _symtab;} 

    /** InstructionSource implementation **/
    PARSER_EXPORT bool isValidAddress(const Address) const;
    PARSER_EXPORT void* getPtrToInstruction(const Address) const;
    PARSER_EXPORT void* getPtrToData(const Address) const;
    PARSER_EXPORT unsigned int getAddressWidth() const;
    PARSER_EXPORT bool isCode(const Address) const;
    PARSER_EXPORT bool isData(const Address) const;
    PARSER_EXPORT bool isReadOnly(const Address) const;
    PARSER_EXPORT Address offset() const;
    PARSER_EXPORT Address length() const;
    PARSER_EXPORT Architecture getArch() const;

    PARSER_EXPORT void removeHint(Hint);

    PARSER_EXPORT static void addNonReturning(std::string func_name);
    
    // statistics accessor
    PARSER_EXPORT void print_stats() const;
    PARSER_EXPORT bool have_stats() const { return _have_stats; }

    // manage statistics
    void incrementCounter(const std::string& name) const;
    void addCounter(const std::string& name, int num) const; 
    void decrementCounter(const std::string& name) const;

 private:

    CodeRegion * lookup_region(const Address addr) const;
    void removeRegion(CodeRegion *); // removes from region tree

    void overlapping_warn(const char * file, unsigned line) const;
    
    void init_regions();
    
    // statistics
    bool init_stats();
    
 
};

}
}

#endif 
