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

#ifndef __RELOCATIONENTRY_H__
#define __RELOCATIONENTRY_H__

#include <iosfwd>
#include <string>

#include "util.h"
#include "dyntypes.h"
#include "Annotatable.h"
#include "Region.h"


namespace Dyninst {

   struct SymSegment;

namespace SymtabAPI {

class Symbol;

class SYMTAB_EXPORT relocationEntry : public AnnotatableSparse {
   public:

      relocationEntry();
      relocationEntry(Offset ta, Offset ra, Offset add,
			  std::string n, Symbol *dynref = NULL, unsigned long relType = 0);
      relocationEntry(Offset ta, Offset ra, std::string n,
			  Symbol *dynref = NULL, unsigned long relType = 0);
      relocationEntry(Offset ra, std::string n, Symbol *dynref = NULL,
			  unsigned long relType = 0, Region::RegionType rtype = Region::RT_REL);
      relocationEntry(Offset ta, Offset ra, Offset add,
                          std::string n, Symbol *dynref = NULL, unsigned long relType = 0,
                          Region::RegionType rtype = Region::RT_REL);

      Offset target_addr() const;
      Offset rel_addr() const;
      Offset addend() const;
      Region::RegionType regionType() const;
      const std::string &name() const;
      Symbol *getDynSym() const;
      bool addDynSym(Symbol *dynref);
      unsigned long getRelType() const;

      void setTargetAddr(const Offset);
      void setRelAddr(const Offset);
      void setAddend(const Offset);
      void setRegionType(const Region::RegionType);
      void setName(const std::string &newName);
      void setRelType(unsigned long relType) { relType_ = relType; }

      // dump output.  Currently setup as a debugging aid, not really
      //  for object persistance....
      //std::ostream & operator<<(std::ostream &s) const;
      friend SYMTAB_EXPORT std::ostream & operator<<(std::ostream &os, const relocationEntry &q);

      enum {pltrel = 1, dynrel = 2};
      bool operator==(const relocationEntry &) const;

      enum category { relative, jump_slot, absolute };

      // Architecture-specific functions
      static unsigned long getGlobalRelType(unsigned addressWidth, Symbol *sym = NULL);
      static const char *relType2Str(unsigned long r, unsigned addressWidth = sizeof(Address));
      category getCategory( unsigned addressWidth );

   private:
      Offset target_addr_;	// target address of call instruction
      Offset rel_addr_;		// address of corresponding relocation entry
      Offset addend_;       // addend (from RELA entries)
      Region::RegionType rtype_;        // RT_REL vs. RT_RELA
      std::string  name_;
      Symbol *dynref_;
      unsigned long relType_;
      Offset rel_struct_addr_;
};


// relocation information for calls to functions not in this image
SYMTAB_EXPORT std::ostream &operator<<(std::ostream &os, const relocationEntry &q);

}//namespace SymtabAPI

}//namespace Dyninst
#endif
