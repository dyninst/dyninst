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

#ifndef __REGION__H__ 
#define __REGION__H__
#include <iosfwd>
#include <string>
#include <vector>
#include "symutil.h"
#include "Annotatable.h"

namespace Dyninst{
namespace SymtabAPI{

class Symbol;
class relocationEntry;
class Symtab;


class SYMTAB_EXPORT Region : public AnnotatableSparse {
   friend class Object;
   friend class Symtab;
   friend class SymtabTranslatorBase;
   friend class SymtabTranslatorBin;

   public:  

   enum perm_t
   {
      RP_R, 
      RP_RW, 
      RP_RX,
      RP_RWX
   };

   static const char *permissions2Str(perm_t);

   enum RegionType 
   {
      RT_TEXT,
      RT_DATA,
      RT_TEXTDATA,
      RT_SYMTAB,
      RT_STRTAB,
      RT_BSS,
      RT_SYMVERSIONS,
      RT_SYMVERDEF,
      RT_SYMVERNEEDED,
      RT_REL,
      RT_RELA,
      RT_PLTREL,
      RT_PLTRELA,
      RT_DYNAMIC,
      RT_HASH,
      RT_GNU_HASH,
      RT_DYNSYM,
      RT_OTHER,
      RT_INVALID = -1
   };

   static const char *regionType2Str(RegionType);

   Region();
   static Region *createRegion(Offset diskOff, perm_t perms, RegionType regType,
                unsigned long diskSize = 0, Offset memOff = 0, unsigned long memSize = 0,
                std::string name = "", char *rawDataPtr = NULL, bool isLoadable = false,
                bool isTLS = false, unsigned long memAlign = sizeof(unsigned));
   Region(const Region &reg);
   Region& operator=(const Region &reg);
   std::ostream& operator<< (std::ostream &os);
   bool operator== (const Region &reg);

   ~Region();

   unsigned getRegionNumber() const;
   bool setRegionNumber(unsigned regnumber);
   std::string getRegionName() const;

   Offset getDiskOffset() const;
   unsigned long getDiskSize() const;
   unsigned long getFileOffset();

   Offset getMemOffset() const;
   unsigned long getMemSize() const;
   unsigned long getMemAlignment() const;
   void setMemOffset(Offset);
   void setMemSize(unsigned long);
   void setDiskSize(unsigned long);
   void setFileOffset(Offset);

   void *getPtrToRawData() const;
   bool setPtrToRawData(void *, unsigned long);//also sets diskSize

   bool isBSS() const;
   bool isText() const;
   bool isData() const;
   bool isTLS() const;
   bool isOffsetInRegion(const Offset &offset) const;
   bool isLoadable() const;
   bool setLoadable(bool isLoadable);
   bool isDirty() const;
   std::vector<relocationEntry> &getRelocations();
   bool patchData(Offset off, void *buf, unsigned size);
   bool isStandardCode();

   perm_t getRegionPermissions() const;
   bool setRegionPermissions(perm_t newPerms);
   RegionType getRegionType() const;

   bool addRelocationEntry(Offset relocationAddr, Symbol *dynref, unsigned long relType, Region::RegionType rtype = Region::RT_REL);
   bool addRelocationEntry(const relocationEntry& rel);

   bool updateRelocations(Address start, Address end, Symbol *oldsym, Symbol *newsym);

   Symtab *symtab() const { return symtab_; }
   protected:                     
   Region(unsigned regnum, std::string name, Offset diskOff,
			unsigned long diskSize, Offset memOff, unsigned long memSize,
			char *rawDataPtr, perm_t perms, RegionType regType, bool isLoadable = false,
			bool isTLS = false, unsigned long memAlign = sizeof(unsigned));
   void setSymtab(Symtab *sym) { symtab_ = sym; }
   private:
   unsigned regNum_;
   std::string name_;
   Offset diskOff_;
   unsigned long diskSize_;
   Offset memOff_;
   unsigned long memSize_;
   Offset fileOff_;
   void *rawDataPtr_;
   perm_t permissions_;
   RegionType rType_;
   bool isDirty_;
   std::vector<relocationEntry> rels_;
   char *buffer_;  //To hold dirty data
   bool isLoadable_;
   bool isTLS_;
   unsigned long memAlign_;
   Symtab *symtab_;
};

}//namespace SymtabAPI

}//namespace Dyninst
#endif
