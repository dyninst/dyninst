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
 * License as published by the Free Software Foundation; either * version 2.1 of the License, or (at your option) any later version.
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

#include "Region.h"
#include "Symtab.h"
#include "common/h/serialize.h"
#include <iostream>

using namespace Dyninst;
using namespace Dyninst::SymtabAPI;
using namespace std;


DLLEXPORT Region::Region(): rawDataPtr_(NULL), buffer_(NULL)
{
}

DLLEXPORT Region::Region(unsigned regnum, std::string name, Offset diskOff,
                    unsigned long diskSize, Offset memOff, unsigned long memSize,
                    char *rawDataPtr, perm_t perms, RegionType regType, bool isLoadable):
    regNum_(regnum), name_(name), diskOff_(diskOff), diskSize_(diskSize), memOff_(memOff),
    memSize_(memSize), rawDataPtr_(rawDataPtr), permissions_(perms), rType_(regType),
    isDirty_(false), buffer_(NULL), isLoadable_(isLoadable)
{
     if(memOff)
        isLoadable_ = true;
}

DLLEXPORT Region::Region(const Region &reg) :
   Serializable(),
   regNum_(reg.regNum_), name_(reg.name_),
   diskOff_(reg.diskOff_), diskSize_(reg.diskSize_), memOff_(reg.memOff_),
   memSize_(reg.memSize_), rawDataPtr_(reg.rawDataPtr_), permissions_(reg.permissions_),
   rType_(reg.rType_), isDirty_(reg.isDirty_), rels_(reg.rels_), buffer_(reg.buffer_)
{
}

DLLEXPORT Region& Region::operator=(const Region &reg)
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
    buffer_ = reg.buffer_;

    return *this;
}


DLLEXPORT bool Region::operator==(const Region &reg){
    return ((regNum_== reg.regNum_) &&
            (name_ == reg.name_) &&
            (diskOff_ == reg.diskOff_) &&
            (diskSize_ == reg.diskSize_) &&
            (memOff_ == reg.memOff_) &&
            (memSize_ == reg.memSize_) &&
            (rawDataPtr_ == reg.rawDataPtr_) &&
            (permissions_ == reg.permissions_) &&
            (rType_ == reg.rType_) &&
            (isDirty_ == reg.isDirty_) &&
            (buffer_ == reg.buffer_));
}

DLLEXPORT ostream& Region::operator<< (ostream &os)
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

DLLEXPORT Region::~Region() 
{
    if(buffer_)
        free(buffer_);
}

const char *Region::permissions2Str(perm_t p)
{
   switch(p) {
   CASE_RETURN_STR(RP_R);
   CASE_RETURN_STR(RP_RW);
   CASE_RETURN_STR(RP_RX);
   CASE_RETURN_STR(RP_RWX);
   };
   return "bad_permissions";
}

const char *Region::regionType2Str(RegionType rt)
{
   switch(rt) {
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
   CASE_RETURN_STR(RT_DYNAMIC);
   CASE_RETURN_STR(RT_OTHER);
   };
   return "bad_RegionTypeype";
};

void Region::serialize(SerializerBase *sb, const char *tag)
{
   ifxml(SerializerXML::start_xml_element, sb, tag);
   gtranslate(sb, regNum_, "RegionNumber");
   gtranslate(sb, name_, "RegionName");
   gtranslate(sb, diskOff_, "DiskOffset");
   gtranslate(sb, diskSize_, "RegionDiskSize");
   gtranslate(sb, memOff_, "MemoryOffset");
   gtranslate(sb, memSize_, "RegionMemorySize");
   gtranslate(sb, permissions_, permissions2Str, "Permissions");
   gtranslate(sb, rType_, regionType2Str, "RegionType");
   gtranslate(sb, isDirty_, "Dirty");
   gtranslate(sb, rels_, "Relocations", "Relocation");
   gtranslate(sb, buffer_, "Buffer");
   gtranslate(sb, isLoadable_, "isLoadable");
   ifxml(SerializerXML::end_xml_element, sb, tag);
}

DLLEXPORT unsigned Region::getRegionNumber() const
{
    return regNum_;
}

DLLEXPORT bool Region::setRegionNumber(unsigned regnumber){
    regNum_ = regnumber;
    return true;
}

DLLEXPORT std::string Region::getRegionName() const{
    return name_;
}

DLLEXPORT Offset Region::getRegionAddr() const
{
#if defined(_MSC_VER)
        return memOff_;
#else
        return diskOff_;
#endif
}

DLLEXPORT Offset Region::getRegionSize() const
{
#if defined(_MSC_VER)
        return memSize_;
#else
        return diskSize_;
#endif
}

DLLEXPORT Offset Region::getDiskOffset() const{
    return diskOff_;
}

DLLEXPORT unsigned long Region::getDiskSize() const{
    return diskSize_;
}

DLLEXPORT Offset Region::getMemOffset() const{
    return memOff_;
}

DLLEXPORT unsigned long Region::getMemSize() const{
    return memSize_;
}

DLLEXPORT void *Region::getPtrToRawData() const{
    return rawDataPtr_;
}

DLLEXPORT bool Region::setPtrToRawData(void *buf, unsigned long newsize){
    rawDataPtr_ = buf;
    diskSize_ = newsize;
    isDirty_ = true;
    return true;
}

DLLEXPORT bool Region::isBSS() const {
    return rType_==RT_BSS;
}

DLLEXPORT bool Region::isText() const{
    return rType_==RT_TEXT;
}

DLLEXPORT bool Region::isData() const{
    return rType_ == RT_DATA;
}

DLLEXPORT bool Region::isOffsetInRegion(const Offset &offset) const {
    return (offset >= diskOff_ && offset<=(diskOff_+diskSize_));
}

DLLEXPORT bool Region::isLoadable() const{
    if(isLoadable_)
        return true;
    return (memOff_ != 0);
}

DLLEXPORT bool Region::isDirty() const{
    return isDirty_;
}

DLLEXPORT std::vector<relocationEntry> &Region::getRelocations(){
    return rels_;
}

DLLEXPORT bool Region::patchData(Offset off, void *buf, unsigned size){
    if(off+size > diskSize_)
        return false;
    if(!buffer_)
        memcpy(buffer_, rawDataPtr_, diskSize_);
    memcpy((char *)buffer_+off, buf, size);
    return setPtrToRawData(buffer_, diskSize_);
}

DLLEXPORT bool Region::addRelocationEntry(Offset ra, Symbol *dynref, unsigned long relType, Region::RegionType rtype){
    rels_.push_back(relocationEntry(ra, dynref->getPrettyName(), dynref, relType, rtype));
    return true;
}

DLLEXPORT Region::perm_t Region::getRegionPermissions() const {
    return permissions_;
}

DLLEXPORT bool Region::setRegionPermissions(Region::perm_t newPerms){
    permissions_ = newPerms;
    return true;
}

DLLEXPORT Region::RegionType Region::getRegionType() const {
    return rType_;
}


