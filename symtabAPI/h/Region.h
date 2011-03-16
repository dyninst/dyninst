/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#ifndef __REGION__H__ 
#define __REGION__H__
#include "symutil.h"
#include "Serialization.h"
#include "Annotatable.h"

namespace Dyninst{
namespace SymtabAPI{

class Symbol;
class relocationEntry;



class Region : public Serializable, public AnnotatableSparse {
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
      RT_OTHER,
      RT_INVALID = -1
   };

   static const char *regionType2Str(RegionType);

   SYMTAB_EXPORT Region();
   SYMTAB_EXPORT static Region *createRegion(Offset diskOff, perm_t perms, RegionType regType,
                unsigned long diskSize = 0, Offset memOff = 0, unsigned long memSize = 0,
                std::string name = "", char *rawDataPtr = NULL, bool isLoadable = false,
                bool isTLS = false, unsigned long memAlign = sizeof(unsigned));
   SYMTAB_EXPORT Region(const Region &reg);
   SYMTAB_EXPORT Region& operator=(const Region &reg);
   SYMTAB_EXPORT std::ostream& operator<< (std::ostream &os);
   SYMTAB_EXPORT bool operator== (const Region &reg);

   SYMTAB_EXPORT ~Region();

   SYMTAB_EXPORT unsigned getRegionNumber() const;
   SYMTAB_EXPORT bool setRegionNumber(unsigned regnumber);
   SYMTAB_EXPORT std::string getRegionName() const;

   //  getRegionAddr returns diskOffset on unixes, memory offset on windows
   SYMTAB_EXPORT Offset getRegionAddr() const;
   SYMTAB_EXPORT unsigned long getRegionSize() const;

   SYMTAB_EXPORT Offset getDiskOffset() const;
   SYMTAB_EXPORT unsigned long getDiskSize() const;

   SYMTAB_EXPORT Offset getMemOffset() const;
   SYMTAB_EXPORT unsigned long getMemSize() const;
   SYMTAB_EXPORT unsigned long getMemAlignment() const;
   SYMTAB_EXPORT void setMemOffset(Offset);
   SYMTAB_EXPORT void setMemSize(long);

   SYMTAB_EXPORT void *getPtrToRawData() const;
   SYMTAB_EXPORT bool setPtrToRawData(void *, unsigned long); 

   SYMTAB_EXPORT bool isBSS() const;
   SYMTAB_EXPORT bool isText() const;
   SYMTAB_EXPORT bool isData() const;
   SYMTAB_EXPORT bool isTLS() const;
   SYMTAB_EXPORT bool isOffsetInRegion(const Offset &offset) const;
   SYMTAB_EXPORT bool isLoadable() const;
   SYMTAB_EXPORT bool setLoadable(bool isLoadable);
   SYMTAB_EXPORT bool isDirty() const;
   SYMTAB_EXPORT std::vector<relocationEntry> &getRelocations();
   SYMTAB_EXPORT bool patchData(Offset off, void *buf, unsigned size);
   SYMTAB_EXPORT bool isStandardCode();

   SYMTAB_EXPORT perm_t getRegionPermissions() const;
   SYMTAB_EXPORT bool setRegionPermissions(perm_t newPerms);
   SYMTAB_EXPORT RegionType getRegionType() const;

   SYMTAB_EXPORT bool addRelocationEntry(Offset relocationAddr, Symbol *dynref, unsigned long relType, Region::RegionType rtype = Region::RT_REL);
   SYMTAB_EXPORT bool addRelocationEntry(const relocationEntry& rel);

   SYMTAB_EXPORT Serializable * serialize_impl(SerializerBase *sb, 
		   const char *tag = "Region") THROW_SPEC (SerializerError);

   protected:                     
   SYMTAB_EXPORT Region(unsigned regnum, std::string name, Offset diskOff,
         unsigned long diskSize, Offset memOff, unsigned long memSize,
         char *rawDataPtr, perm_t perms, RegionType regType, bool isLoadable = false,
         bool isTLS = false, unsigned long memAlign = sizeof(unsigned));
   private:
   unsigned regNum_;
   std::string name_;
   Offset diskOff_;
   unsigned long diskSize_;
   Offset memOff_;
   unsigned long memSize_;
   void *rawDataPtr_;
   perm_t permissions_;
   RegionType rType_;
   bool isDirty_;
   std::vector<relocationEntry> rels_;
   char *buffer_;  //To hold dirty data
   bool isLoadable_;
   bool isTLS_;
   unsigned long memAlign_;
};

}//namespace SymtabAPI

}//namespace Dyninst
#endif
