/*
 * Copyright (c) 1996-2009 Barton P. Miller
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

#include "parseAPI/h/CodeObject.h"
#include "dyninstAPI/src/addressSpace.h"
#include "memEmulator.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/symtab.h"
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Region.h"
#include "dyninstAPI/src/process.h"
#include "dyninstAPI/src/function.h"

using namespace Dyninst;
using namespace SymtabAPI;

bool MemoryEmulator::findMutateeTable() {
   if (mutateeBase_ != 0) return true;

   std::vector<int_variable *> memoryMapperTable;
   if (!aS_->findVarsByAll("RTmemoryMapper", memoryMapperTable)) {
      return false;
   }

   if (memoryMapperTable.size() > 1) {
      // ???
      return false;
   }
   mutateeBase_ = memoryMapperTable[0]->getAddress();
   return true;
}

void MemoryEmulator::update() {
   if (!findMutateeTable()) return;

   // 1) Create shadow copies for any MappedObject we
   // have modified.
   // 2) Update the runtime's MemoryMapper structure
   // to correspond to this.

   // First step: nonblocking synchro.
   int guardValue;
   aS_->readDataSpace((void *)mutateeBase_,
                      sizeof(int),
                      &guardValue,
                      false);
   guardValue++;
   aS_->writeDataSpace((void *)mutateeBase_,
                       sizeof(int),
                       &guardValue);
   
   sensitivity_cerr << "UpdateMemEmulator: writing guard value " << guardValue << endl;
      // 64->32 bit is annoying...
   if (addrWidth() == 4) {
      struct MemoryMapper32 newMapper;
      
      aS_->readDataSpace((void *)mutateeBase_,
                         sizeof(newMapper),
                         &newMapper,
                         false);
      
      // First step: 
      newMapper.guard1 = guardValue;
      newMapper.guard2 = guardValue;
      newMapper.size = memoryMap_.size();
      sensitivity_cerr << "\t new values: " << newMapper.guard1 << "/" << newMapper.guard2 << "/" << newMapper.size << endl;
      std::vector<MemoryMapTree::Entry> elements;
      memoryMap_.elements(elements);
      for (unsigned i = 0; i < elements.size(); ++i) {
         newMapper.elements[i].lo = elements[i].first.first;
         newMapper.elements[i].hi = elements[i].first.second;
         assert(newMapper.elements[i].hi > newMapper.elements[i].lo);
         newMapper.elements[i].shift = elements[i].second;
         sensitivity_cerr << "\t\t Element: " << hex << newMapper.elements[i].lo << "->" << newMapper.elements[i].hi << ": " << newMapper.elements[i].shift << dec << endl;
      }
      aS_->writeDataSpace((void *)mutateeBase_,
                          sizeof(newMapper),
                          &newMapper);
   }
   else {
      // TODO copy
      //assert(0);
   }
}

void MemoryEmulator::addAllocatedRegion(Address start, unsigned size) {
   addRegion(start, size, -1);
}

void MemoryEmulator::addRegion(mapped_object *obj) {
   //cerr << "addRegion for " << obj->fileName() << endl;

   // Add each code region
   std::vector<Region *> codeRegions;
   obj->parse_img()->getObject()->getCodeRegions(codeRegions);

   for (unsigned i = 0; i < codeRegions.size(); ++i) {
      Region *reg = codeRegions[i];

      addRegion(reg, obj->codeBase());
   }         
}

void MemoryEmulator::addRegion(Region *reg, Address base) {
   
   //cerr << "\t\t Region " << i << ": " << hex
   //<< codeRegions[i]->getMemOffset() + obj->codeBase() << " -> " 
   //<< codeRegions[i]->getMemOffset() + codeRegions[i]->getMemSize() + obj->codeBase() << endl;
   
   if (addedRegions_.find(reg) != addedRegions_.end()) return;
      
   process *proc = dynamic_cast<process *>(aS_);
   char *buffer = (char *)malloc(reg->getMemSize());
   if (proc) {
       if (!proc->readDataSpace((void*)(base + reg->getMemOffset()), 
                               reg->getMemSize(), buffer, false)) {
           assert(0);
       }
   } else {
       memset(buffer, 0, reg->getMemSize());
       memcpy(buffer, reg->getPtrToRawData(), reg->getDiskSize());
   }
   
   unsigned long allocSize = reg->getMemSize();
   if (proc) {
      allocSize += proc->getMemoryPageSize();
   }
   
   Address mutateeBase = aS_->inferiorMalloc(allocSize);
   assert(mutateeBase);
   
   // "Upcast" it to align with a page boundary - Kevin's request
   if (proc) {         
      mutateeBase += proc->getMemoryPageSize();
      mutateeBase -= mutateeBase % proc->getMemoryPageSize();
   }
   
   
   aS_->writeDataSpace((void *)mutateeBase,
                       reg->getMemSize(),
                       (void *)buffer);
   if (aS_->proc() && BPatch_defensiveMode == aS_->proc()->getHybridMode()) {
#if defined (os_windows)
       using namespace SymtabAPI;
       unsigned winrights = 0;
       Region::perm_t reg_rights = reg->getRegionPermissions();
       switch (reg_rights) {
       case Region::RP_R:
           winrights = PAGE_READONLY;
           break;
       case Region::RP_RW: 
           winrights = PAGE_READWRITE;
           break;
       case Region::RP_RX:
           winrights = PAGE_EXECUTE_READ;
           break;
       case Region::RP_RWX:
           winrights = PAGE_EXECUTE_READWRITE;
           break;
       default:
           assert(0);
       }
       dyn_lwp *stoppedlwp = aS_->proc()->query_for_stopped_lwp();
       assert(stoppedlwp);
       stoppedlwp->changeMemoryProtections(mutateeBase, reg->getMemSize(), winrights, false);
#endif
   }
   
    Address regionBase = base + reg->getMemOffset();

    sensitivity_cerr << hex << " Adding region with base " << base << " and mem offset " << reg->getMemOffset()
        << ", allocated buffer base " << mutateeBase << " and so shift " << mutateeBase - regionBase << dec << endl;

   addRegion(regionBase,
             reg->getMemSize(),
             mutateeBase - regionBase);
   
   addedRegions_[reg] = mutateeBase;
   free(buffer);
}

void MemoryEmulator::addRegion(Address start, unsigned size, Address shift) {
   if (size == 0) return;

   Address end = start + size;
   assert(end > start);

   // Okay. For efficiency, we want to merge this if possible with an existing
   // range. We do this because our allocation tends to be contiguous.
   // Two options: we're immediately above an existing range or we're immediately
   // below. Check both. 


   Address lb, ub;
   unsigned long val;
   if (memoryMap_.find(start, lb, ub, val)) {
      // This is possibly very bad. 
      if (start != ub) {
         if ((start == lb) &&
             (end == ub) &&
             (val == shift)) {
            return;
         }
         // Yeah, data inconsistency == bad
         assert(0);
      }
      if (val == shift) {
         // Accumulate
         memoryMap_.remove(lb);
         memoryMap_.insert(lb, end, shift);
         return;
      }
      else {
         memoryMap_.insert(start, end, shift);
      }
   }
   else if (memoryMap_.find(end, lb, ub, val)) {
      // See the above
      if (end != ub) {
         fprintf(stderr, "ERROR: adding range 0x%lx -> 0x%lx (0x%lx), found range 0x%lx -> 0x%lx (0x%lx)\n",
                 start, end, shift, lb, ub, val);

         assert(0);
      }
      if (val == shift) {
         memoryMap_.remove(lb);
         memoryMap_.insert(start, ub, shift);
      }
      else {
         memoryMap_.insert(start, end, shift);
      }
   }
   else {
      memoryMap_.insert(start, end, shift);
   }

   if (shift != (unsigned long) -1) {
      reverseMemoryMap_.insert(start + shift, end + shift, shift);
   }

   return;   
}

unsigned MemoryEmulator::addrWidth() {
   return aS_->getAddressWidth();
}

std::pair<bool, Address> MemoryEmulator::translate(Address orig) {
   // Mimic the translation performed in the RT library
   Address lb, ub;
   unsigned long val;
   if (!memoryMap_.find(orig, lb, ub, val)) {
      return std::make_pair(false, 0);
   }
   if (val == (unsigned long) -1) {
      return std::make_pair(true, orig);
   }
   return std::make_pair(true, orig + val);
}

std::pair<bool, Address> MemoryEmulator::translateBackwards(Address addr) {
   // Mimic the translation performed in the RT library
   Address lb, ub;
   unsigned long val;
   if (!reverseMemoryMap_.find(addr, lb, ub, val)) {
      return std::make_pair(false, 0);
   }
   if (val == (unsigned long) -1) {
      return std::make_pair(true, addr);
   }
   return std::make_pair(true, addr - val);
}

void MemoryEmulator::synchShadowOrig(mapped_object * obj, bool toOrig) 
{//KEVINTODO: this should only change rights to ranges that contain analyzed code, and shouldn't things to writeable unless they were so originally
    if (toOrig) malware_cerr << "Syncing shadow to orig for obj " << obj->fileName() << endl;
    else        malware_cerr << "Syncing orig to shadow for obj " << obj->fileName() << endl;
    using namespace SymtabAPI;
    vector<Region*> regs;
    obj->parse_img()->getObject()->getCodeRegions(regs);
    for (unsigned idx=0; idx < regs.size(); idx++) {
        Region * reg = regs[idx];
        unsigned char* regbuf = (unsigned char*) malloc(reg->getMemSize());
        Address from = 0;
        if (toOrig) {
            from = addedRegions_[reg];
        } else {
            from = obj->codeBase() + reg->getMemOffset();
        }
        cerr << "SYNC READ FROM " << hex << from << " -> " << from + reg->getMemSize() << dec << endl;
        if (!aS_->readDataSpace((void *)from,
                                reg->getMemSize(),
                                regbuf,
                                false)) 
        {
            assert(0);
        }
        std::map<Address,int>::const_iterator sit = springboards_[reg].begin();
        Address cp_start = 0;
        Address toBase;
        if (toOrig) {
            toBase = obj->codeBase() + reg->getMemOffset();
        } else {
            toBase = addedRegions_[reg];
        }
        //cerr << "SYNC WRITE TO " << hex << toBase << dec << endl;
        for (; sit != springboards_[reg].end(); sit++) {
            assert(cp_start <= sit->first);
            int cp_size = sit->first - cp_start;
            //cerr << "\t Write " << hex << toBase + cp_start << "..." << toBase + cp_start + cp_size << dec << endl;
            if (cp_size &&
                !aS_->writeDataSpace((void *)(toBase + cp_start),
                                     cp_size,
                                     regbuf + cp_start))
            {
                assert(0);
            }
            cp_start = sit->first + sit->second;
        }
        //cerr << "\t Finishing write " << hex << toBase + cp_start << " -> " << toBase + cp_start + reg->getMemSize() - cp_start << dec << endl;

        if (cp_start < reg->getMemSize() &&
            !aS_->writeDataSpace((void *)(toBase + cp_start),
                                 reg->getMemSize() - cp_start,
                                 regbuf + cp_start))
        {
            assert(0);
        }
        free(regbuf);
    }
}


void MemoryEmulator::addSpringboard(Region *reg, Address offset, int size) 
{
    // DEBUGGING
    //map<Address,int>::iterator sit = springboards_[reg].begin();
    //for (; sit != springboards_[reg].end(); ++sit) {
    //    if (sit->first == offset) continue;
    //    if (sit->first + sit->second <= offset) continue;
    //    if (sit->first >= offset + size) break;
    //    assert(0);
    //}
    if (offset == 0xd3d2 ||
        offset == 0xd3d3) {
            cerr << "DEBUG BREAKPOINT!" << endl;
    }
    for (Address tmp = offset; tmp < offset + size; ++tmp) {
        springboards_[reg].erase(tmp);
    }

    springboards_[reg][offset] = size;
}

void MemoryEmulator::removeSpringboards(int_function * func) 
{
    malware_cerr << "untracking springboards from deadfunc " << hex << func->getAddress() << dec << endl;

    const set<int_block*,int_block::compare> & blocks = func->blocks();
    set<int_block*,int_block::compare>::const_iterator bit = blocks.begin();
    for (; bit != blocks.end(); bit++) {
        removeSpringboards((*bit));
    }
}

void MemoryEmulator::removeSpringboards(const int_block *bbi) 
{
    if (bbi->llb()->start() == 0xd3d2 ||
        bbi->llb()->start() == 0xd323)
        cerr << "DEBUG BREAKPOINT" << endl;
    malware_cerr << "  untracking springboards from deadblock [" << hex 
         << bbi->start() << " " << bbi->end() << ")" << dec <<endl;
    SymtabAPI::Region * reg = 
        ((ParseAPI::SymtabCodeRegion*)bbi->func()->ifunc()->region())->symRegion();
    if (springboards_[reg].find((bbi->llb()->start() - reg->getMemOffset())) == springboards_[reg].end()) {
        cerr << "ERROR IN DELETING SPRINGBOARD!" << endl;
    }
    springboards_[reg].erase(bbi->llb()->start() - reg->getMemOffset());
    if (springboards_[reg].empty()) springboards_.erase(reg);
}
