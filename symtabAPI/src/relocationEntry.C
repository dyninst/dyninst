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

#include <iostream>
#include <iomanip>

#include "relocationEntry.h"
#include "Symbol.h"


namespace Dyninst  {
namespace SymtabAPI  {


SYMTAB_EXPORT relocationEntry::relocationEntry() :
   target_addr_(0),
   rel_addr_(0),
   addend_(0),
   rtype_(Region::RT_REL),
   name_(""),
   dynref_(NULL),
   relType_(0),
   rel_struct_addr_(0)
{
}

SYMTAB_EXPORT relocationEntry::relocationEntry(Offset ta, Offset ra, std::string n,
      Symbol *dynref, unsigned long relType) :
   target_addr_(ta),
   rel_addr_(ra),
   addend_(0),
   rtype_(Region::RT_REL),
   name_(n),
   dynref_(dynref),
   relType_(relType),
   rel_struct_addr_(0)
{
}

SYMTAB_EXPORT relocationEntry::relocationEntry(Offset ta, Offset ra, Offset add,
      std::string n, Symbol *dynref, unsigned long relType) :
   target_addr_(ta),
   rel_addr_(ra),
   addend_(add),
   rtype_(Region::RT_REL),
   name_(n),
   dynref_(dynref),
   relType_(relType),
   rel_struct_addr_(0)
{
}

SYMTAB_EXPORT relocationEntry::relocationEntry(Offset ra, std::string n,
      Symbol *dynref, unsigned long relType, Region::RegionType rtype) :
   target_addr_(0),
   rel_addr_(ra),
   addend_(0),
   rtype_(rtype),
   name_(n),
   dynref_(dynref),
   relType_(relType),
   rel_struct_addr_(0)
{
}

SYMTAB_EXPORT relocationEntry::relocationEntry(Offset ta, Offset ra, Offset add,
        std::string n, Symbol *dynref, unsigned long relType,
        Region::RegionType rtype) :
    target_addr_(ta),
    rel_addr_(ra),
    addend_(add),
    rtype_(rtype),
    name_(n),
    dynref_(dynref),
    relType_(relType),
    rel_struct_addr_(0)
{
}

SYMTAB_EXPORT Offset relocationEntry::target_addr() const
{
    return target_addr_;
}

SYMTAB_EXPORT void relocationEntry::setTargetAddr(const Offset off)
{
    target_addr_ = off;
}

SYMTAB_EXPORT Offset relocationEntry::rel_addr() const
{
    return rel_addr_;
}

SYMTAB_EXPORT void relocationEntry::setRelAddr(const Offset value)
{
    rel_addr_ = value;
}

SYMTAB_EXPORT const std::string &relocationEntry::name() const
{
    return name_;
}

SYMTAB_EXPORT Symbol *relocationEntry::getDynSym() const
{
    return dynref_;
}

SYMTAB_EXPORT bool relocationEntry::addDynSym(Symbol *dynref)
{
    dynref_ = dynref;
    return true;
}

SYMTAB_EXPORT Region::RegionType relocationEntry::regionType() const
{
	return rtype_;
}

SYMTAB_EXPORT unsigned long relocationEntry::getRelType() const
{
    return relType_;
}

SYMTAB_EXPORT Offset relocationEntry::addend() const
{
        return addend_;
}

SYMTAB_EXPORT void relocationEntry::setAddend(const Offset value)
{
        addend_ = value;
}

SYMTAB_EXPORT void relocationEntry::setRegionType(const Region::RegionType value)
{
        rtype_ = value;
}

SYMTAB_EXPORT void relocationEntry::setName(const std::string &newName) {
    name_ = newName;
}

bool relocationEntry::operator==(const relocationEntry &r) const
{
	if (target_addr_ != r.target_addr_) return false;
	if (rel_addr_ != r.rel_addr_) return false;
	if (addend_ != r.addend_) return false;
	if (rtype_ != r.rtype_) return false;
	if (name_ != r.name_) return false;
	if (relType_ != r.relType_) return false;
	if (dynref_ && !r.dynref_) return false;
	if (!dynref_ && r.dynref_) return false;
	if (dynref_)
	{
		if (dynref_->getMangledName() != r.dynref_->getMangledName()) return false;
		if (dynref_->getOffset() != r.dynref_->getOffset()) return false;
	}

	return true;
}

std::ostream & operator<< (std::ostream &os, const relocationEntry &r)
{
    using namespace std;

    if( r.getDynSym() != NULL ) {
        os << "Name: " << setw(20) << ( "'" + r.getDynSym()->getMangledName() + "'" );
    }else{
        os << "Name: " << setw(20) << r.name();
    }
    os << " Offset: " << std::hex << std::setfill('0') << setw(8) << r.rel_addr()
       << std::dec << std::setfill(' ')
       << " Offset: " << std::hex << std::setfill('0') << setw(8) << r.target_addr()
       << std::dec << std::setfill(' ')
       << " Addend: " << r.addend()
       << " Region: " << Region::regionType2Str(r.regionType())
       << " Type: " << setw(15) << relocationEntry::relType2Str(r.getRelType())
       << "(" << r.getRelType() << ")";
    if( r.getDynSym() != NULL ) {
        os << " Symbol Offset: " << std::hex << std::setfill('0') << setw(8) << r.getDynSym()->getOffset();
        os << std::dec << std::setfill(' ');
        if( r.getDynSym()->isCommonStorage() ) {
            os << " COM";
        }else if( r.getDynSym()->getRegion() == NULL ) {
            os << " UND";
        }
    }
    return os;
}


}
}
