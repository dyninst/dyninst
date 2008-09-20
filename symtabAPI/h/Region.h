/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __REGION__H__ 
#define __REGION__H__
 
#include "Serialization.h"

namespace Dyninst{
namespace SymtabAPI{

class Symbol;
class relocationEntry;



class Region : public Serializable {
   friend class Object;
   friend class Symtab;
   friend class SymtabTranslatorBase;
   friend class SymtabTranslatorBin;

   public:  

   typedef enum perm_t
   {
      RP_R, 
      RP_RW, 
      RP_RX, 
      RP_RWX
   };

   static const char *permissions2Str(perm_t);

   typedef enum RegionType 
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
      RT_DYNAMIC,
      RT_OTHER
   };

   static const char *regionType2Str(RegionType);

   DLLEXPORT Region();
   DLLEXPORT static bool createRegion( Offset diskOff, perm_t perms, RegionType regType,
         unsigned long diskSize = 0, Offset memOff = 0, unsigned long memSize = 0,
         std::string name = "", char *rawDataPtr = NULL);
   DLLEXPORT Region(const Region &reg);
   DLLEXPORT Region& operator=(const Region &reg);
   DLLEXPORT void serialize(SerializerBase *sb, const char *tag = "Region");
   DLLEXPORT std::ostream& operator<< (std::ostream &os);
   DLLEXPORT bool operator== (const Region &reg);

   DLLEXPORT ~Region();

   DLLEXPORT unsigned getRegionNumber() const;
   DLLEXPORT bool setRegionNumber(unsigned regnumber);
   DLLEXPORT std::string getRegionName() const;

   //  getRegionAddr returns diskOffset on unixes, memory offset on windows
   DLLEXPORT Offset getRegionAddr() const;
   DLLEXPORT unsigned long getRegionSize() const;

   DLLEXPORT Offset getDiskOffset() const;
   DLLEXPORT unsigned long getDiskSize() const;
   DLLEXPORT Offset getMemOffset() const;
   DLLEXPORT unsigned long getMemSize() const;
   DLLEXPORT void *getPtrToRawData() const;
   DLLEXPORT bool setPtrToRawData(void *, unsigned long); 

   DLLEXPORT bool isBSS() const;
   DLLEXPORT bool isText() const;
   DLLEXPORT bool isData() const;
   DLLEXPORT bool isOffsetInRegion(const Offset &offset) const;
   DLLEXPORT bool isLoadable() const;
   DLLEXPORT bool setLoadable(bool isLoadable);
   DLLEXPORT bool isDirty() const;
   DLLEXPORT std::vector<relocationEntry> &getRelocations();
   DLLEXPORT bool patchData(Offset off, void *buf, unsigned size);

   DLLEXPORT perm_t getRegionPermissions() const;
   DLLEXPORT bool setRegionPermissions(perm_t newPerms);
   DLLEXPORT RegionType getRegionType() const;

   DLLEXPORT bool addRelocationEntry(Offset relocationAddr, Symbol *dynref, unsigned long relType, Region::RegionType rtype = Region::RT_REL);

   protected:                     
   DLLEXPORT Region(unsigned regnum, std::string name, Offset diskOff,
         unsigned long diskSize, Offset memOff, unsigned long memSize,
         char *rawDataPtr, perm_t perms, RegionType regType, bool isLoadable = false);
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
};

}//namespace SymtabAPI

}//namespace Dyninst
#endif
