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

#include "Region.h"
#include "Symtab.h"
#include <iostream>

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;


Region *Region::createRegion( Offset diskOff, perm_t perms, RegionType regType,
                              unsigned long diskSize, Offset memOff, 
                              unsigned long memSize, std::string name, 
                              char *rawDataPtr, bool isLoadable, bool isTLS,
                              unsigned long memAlign)
{
   Region *newreg = new Region(0, name, diskOff, 
                               diskSize, memOff, memSize, 
                               rawDataPtr, perms, regType, isLoadable, isTLS,
                               memAlign);
   return newreg;
}

Region::Region(): regNum_(0), diskOff_(0), diskSize_(0), memOff_(0),
    memSize_(0), fileOff_(0), rawDataPtr_(NULL), permissions_(RP_R),
    rType_(RT_INVALID), isDirty_(false), buffer_(NULL), isLoadable_(false),
    isTLS_(false), memAlign_(0), symtab_(NULL)
{
}

Region::Region(unsigned regnum, std::string name, Offset diskOff,
                    unsigned long diskSize, Offset memOff, unsigned long memSize,
                    char *rawDataPtr, perm_t perms, RegionType regType, bool isLoadable,
                    bool isThreadLocal, unsigned long memAlignment) :
    regNum_(regnum), name_(name), diskOff_(diskOff), diskSize_(diskSize), memOff_(memOff),
    memSize_(memSize), fileOff_(0), rawDataPtr_(rawDataPtr), permissions_(perms), rType_(regType),
    isDirty_(false), buffer_(NULL), isLoadable_(isLoadable), isTLS_(isThreadLocal),
    memAlign_(memAlignment), symtab_(NULL)
{
    //cerr << "Region " << name << ": [" << diskOff << ", " << diskOff + diskSize << ") (disk), ";
    //cerr << "[" << memOff << ", " << memOff+memSize << ") (memory)" << endl;
   if (memOff)
      isLoadable_ = true;
}

Region::Region(const Region &reg) :
   AnnotatableSparse(),
   regNum_(reg.regNum_), name_(reg.name_),
   diskOff_(reg.diskOff_), diskSize_(reg.diskSize_), memOff_(reg.memOff_),
   memSize_(reg.memSize_), fileOff_(reg.fileOff_), rawDataPtr_(reg.rawDataPtr_),
   permissions_(reg.permissions_), rType_(reg.rType_), isDirty_(reg.isDirty_),
   rels_(reg.rels_), buffer_(NULL), isLoadable_(reg.isLoadable_),
   isTLS_(reg.isTLS_), memAlign_(reg.memAlign_), symtab_(reg.symtab_)
{
}

Region& Region::operator=(const Region &reg)
{
    regNum_ = reg.regNum_;
    name_ = reg.name_;
    diskOff_ = reg.diskOff_;
    diskSize_ = reg.diskSize_;
    memOff_ = reg.memOff_;
    memSize_ = reg.memSize_;
    rawDataPtr_ = reg.rawDataPtr_;
    permissions_ = reg.permissions_;
    rType_ = reg.rType_;
    isDirty_ = reg.isDirty_;
    rels_ = reg.rels_;
    buffer_ = NULL;
    isLoadable_ = reg.isLoadable_;
    isTLS_ = reg.isTLS_;
    memAlign_ = reg.memAlign_;

    return *this;
}

bool Region::operator==(const Region &reg)
{

	if (rels_.size() != reg.rels_.size()) return false;

	for (unsigned int i = 0; i < rels_.size(); ++i)
	{
		if (!(rels_[i]== reg.rels_[i])) return false;
	}

    return ((regNum_== reg.regNum_) &&
            (name_ == reg.name_) &&
            (diskOff_ == reg.diskOff_) &&
            (diskSize_ == reg.diskSize_) &&
            (memOff_ == reg.memOff_) &&
            (memSize_ == reg.memSize_) &&
            (permissions_ == reg.permissions_) &&
            (rType_ == reg.rType_) &&
            (isDirty_ == reg.isDirty_) &&
            (isLoadable_ == reg.isLoadable_) &&
            (isTLS_ == reg.isTLS_) &&
            (memAlign_ == reg.memAlign_));
}

ostream& Region::operator<< (ostream &os)
{
    return os   << "{"
                << " Region Number="      << regNum_
                << " name="    << name_
                << " disk offset="    << diskOff_
                << " disk size="    << diskSize_
                << " memory offset="    << memOff_
                << " memory size="    << memSize_
                << " Permissions=" << permissions_
                        << " region type " << rType_
                << " }" << endl;
}

Region::~Region() 
{
    if (buffer_)
        free(buffer_);
}

const char *Region::permissions2Str(perm_t p)
{
   switch(p) 
   {
      CASE_RETURN_STR(RP_R);
      CASE_RETURN_STR(RP_RW);
      CASE_RETURN_STR(RP_RX);
      CASE_RETURN_STR(RP_RWX);
   };
   return "bad_permissions";
}

const char *Region::regionType2Str(RegionType rt)
{
   switch(rt) 
   {
      CASE_RETURN_STR(RT_TEXT);
      CASE_RETURN_STR(RT_DATA);
      CASE_RETURN_STR(RT_TEXTDATA);
      CASE_RETURN_STR(RT_SYMTAB);
      CASE_RETURN_STR(RT_STRTAB);
      CASE_RETURN_STR(RT_BSS);
      CASE_RETURN_STR(RT_SYMVERSIONS);
      CASE_RETURN_STR(RT_SYMVERDEF);
      CASE_RETURN_STR(RT_SYMVERNEEDED);
      CASE_RETURN_STR(RT_REL);
      CASE_RETURN_STR(RT_RELA);
      CASE_RETURN_STR(RT_PLTREL);
      CASE_RETURN_STR(RT_PLTRELA);
      CASE_RETURN_STR(RT_DYNAMIC);
      CASE_RETURN_STR(RT_HASH);
      CASE_RETURN_STR(RT_GNU_HASH);
      CASE_RETURN_STR(RT_OTHER);
      CASE_RETURN_STR(RT_INVALID);
      CASE_RETURN_STR(RT_DYNSYM);
   };
   return "bad_RegionTypeype";
}

unsigned Region::getRegionNumber() const
{
    return regNum_;
}

bool Region::setRegionNumber(unsigned regnumber)
{
    regNum_ = regnumber;
    return true;
}

std::string Region::getRegionName() const
{
    return name_;
}

Offset Region::getDiskOffset() const
{
    return diskOff_;
}

unsigned long Region::getDiskSize() const
{
    return diskSize_;
}

unsigned long Region::getFileOffset()
{
    return fileOff_;
}

Offset Region::getMemOffset() const
{
    return memOff_;
}

unsigned long Region::getMemSize() const
{
    return memSize_;
}

unsigned long Region::getMemAlignment() const
{
    return memAlign_;
}

void Region::setMemOffset(Offset newoff)
{
    memOff_ = newoff;
}

void Region::setFileOffset(Offset newoff)
{
    fileOff_ = newoff;
}

void Region::setMemSize(unsigned long newsize)
{
    memSize_ = newsize;
}

void Region::setDiskSize(unsigned long newsize)
{
    diskSize_ = newsize;
}

void *Region::getPtrToRawData() const
{
    return rawDataPtr_;
}

bool Region::setPtrToRawData(void *buf, unsigned long newsize)
{
   rawDataPtr_ = buf;
    diskSize_ = newsize;
    isDirty_ = true;
    return true;
}

bool Region::isBSS() const 
{
    return rType_==RT_BSS;
}

bool Region::isText() const
{
    return rType_==RT_TEXT;
}

bool Region::isData() const
{
    return rType_ == RT_DATA;
}

bool Region::isTLS() const
{
    return isTLS_;
}

bool Region::isOffsetInRegion(const Offset &offset) const 
{
    return (offset >= diskOff_ && offset<(diskOff_+diskSize_));
}

bool Region::isLoadable() const
{
    if (isLoadable_)
        return true;
    return (memOff_ != 0);
}

bool Region::isDirty() const
{
    return isDirty_;
}

std::vector<relocationEntry> &Region::getRelocations()
{
    return rels_;
}

bool Region::patchData(Offset off, void *buf, unsigned size)
{
    if (off+size > diskSize_)
        return false;

    if (!buffer_) {
        buffer_ = (char *)malloc(diskSize_*sizeof(char));
        memcpy(buffer_, rawDataPtr_, diskSize_);
    }

    memcpy(&buffer_[off], buf, size);

    return setPtrToRawData(buffer_, diskSize_);
}

bool Region::addRelocationEntry(Offset ra, Symbol *dynref, unsigned long relType, 
      Region::RegionType rtype)
{
    rels_.push_back(relocationEntry(ra, dynref->getMangledName(), dynref, relType, rtype));
    return true;
}

bool Region::addRelocationEntry(const relocationEntry& rel) {
    rels_.push_back(rel);
    return true;
}

Region::perm_t Region::getRegionPermissions() const 
{
    return permissions_;
}


bool Region::setRegionPermissions(Region::perm_t newPerms)
{
    permissions_ = newPerms;
    return true;
}

Region::RegionType Region::getRegionType() const 
{
    return rType_;
}

bool Region::updateRelocations(Address start,
                               Address end,
                               Symbol *oldsym,
                               Symbol *newsym) {
   
   for (unsigned i = 0; i < rels_.size(); ++i) {
      // If the relocation entry matches, update the symbol. We
      // have an address range and an old symbol...
      relocationEntry &e = rels_[i];
      if (!e.getDynSym()) continue;

      if (e.getDynSym()->getMangledName() != oldsym->getMangledName()) {
         continue;
      }
      if (e.rel_addr() < start) {
         continue;
      }
      if (e.rel_addr() > end) {
         continue;
      }
      e.addDynSym(newsym);
   }
   return true;
}

