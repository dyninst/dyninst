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
#include <boost/tuple/tuple.hpp>

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

// only called for the runtime library
void MemoryEmulator::addAllocatedRegion(Address start, unsigned size) {
   addRange(start, size, -1);
}

static void mergeRanges(const mapped_object *obj,
                        unsigned int rangeSize, 
                        std::set<Address> &in, 
                        std::vector<pair<Address,unsigned int> > &out)
{
   Address prevEnd = 0xbadadd;
   SymtabAPI::Region *prevReg = NULL;
   int count = 0;
   for (set<Address>::iterator pit =in.begin(); pit != in.end(); pit++) {
       SymtabAPI::Region *reg = obj->parse_img()->getObject()->findEnclosingRegion(*pit - obj->codeBase());
       if (prevEnd != 0xbadadd && (prevEnd != (*pit) || reg != prevReg)) {
           out.push_back(pair<Address,unsigned>(prevEnd - (rangeSize * count),
                                                rangeSize * count));
           count = 0;
       }
       prevEnd = (*pit) + rangeSize;
       prevReg = reg;
       count++;
   }
   if (count) {
       out.push_back(pair<Address,unsigned>(prevEnd - (rangeSize * count), 
                                            rangeSize * count));
   }
}

void MemoryEmulator::addObject(const mapped_object *obj) {

   if (emulatedObjs.end() != emulatedObjs.find(obj)) {
       return;
   }
   emulatedObjs.insert(obj);

   sensitivity_cerr << "memEmulator::addObject for " << obj->fileName() << endl;

   // Get all pages containing analyzed code, 
   // coalesce them into larger ranges (that don't span region boundaries), 
   // and add them to our datastructures
   set<Address> pages;
   unsigned int pageSize = obj->getAnalyzedCodePages(pages);
   vector<pair<Address,unsigned int> > ranges;
   mergeRanges(obj,pageSize,pages,ranges);
   for (vector<pair<Address,unsigned int> >::iterator rit=ranges.begin(); 
        rit != ranges.end(); 
        rit++) 
   {
       addRange(obj, rit->first, rit->second);
   }
}

void MemoryEmulator::addNewCode(const mapped_object *obj, 
                                const vector<image_basicBlock*> &blks) {
    // we'll only add new code if we're monitoring the program at runtime
    assert(aS_->proc());

    unsigned int pageSize = aS_->proc()->getMemoryPageSize();
    set<Address> pageAddrs;
    unsigned long dontcare=0;
    for(vector<image_basicBlock*>::const_iterator bit = blks.begin(); 
        bit != blks.end(); 
        bit++) 
    {
        Address pageAddr = (*bit)->start() 
            - ((*bit)->start() % pageSize) 
            + obj->codeBase();
        Address endPageAddr = (*bit)->end() - 1 
           - ((*bit)->start() % pageSize) 
           + obj->codeBase();
        while (pageAddr <= endPageAddr) {
            if (! memoryMap_.find(pageAddr, dontcare) ) {
                pageAddrs.insert(pageAddr);
            }
            pageAddr += pageSize;
        }
    }
   vector<pair<Address,unsigned int> > ranges;
   mergeRanges(obj,pageSize,pageAddrs,ranges);
   for (vector<pair<Address,unsigned int> >::iterator rit=ranges.begin(); 
        rit != ranges.end(); 
        rit++) 
   {
       addRange(obj, rit->first, rit->second);
   }
}

void MemoryEmulator::addRange(const mapped_object *obj, 
                              Address start, 
                              unsigned int size) 
{
   unsigned long dontcare;
   if (memoryMap_.find(start,dontcare)) 
       assert(0);

   process *proc = dynamic_cast<process *>(aS_);
   unsigned char *buffer = (unsigned char *) malloc(size);
   if (proc) {
       if (!proc->readDataSpace((void*)start, size, buffer, false)) {
           assert(0);
       }
   } else {
       memset(buffer, 0, size);
       SymtabAPI::Region *reg = obj->parse_img()->getObject()->
           findEnclosingRegion(start - obj->codeBase());
       Address offset = start - obj->codeBase() - reg->getMemOffset();
       if (offset < reg->getDiskSize()) {
           memcpy(buffer, 
                  ((char*)reg->getPtrToRawData()) + offset, 
                  reg->getDiskSize() - offset);
       }
   }
   
   unsigned long allocSize = size;
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
                       size,
                       (void *)buffer);
   if (proc && BPatch_defensiveMode == obj->hybridMode()) {
#if defined (os_windows)
       using namespace SymtabAPI;
       unsigned winrights = 0;
       Region::perm_t reg_rights = obj->parse_img()->getObject()->
           findEnclosingRegion(start - obj->codeBase())->getRegionPermissions();
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
       dyn_lwp *stoppedlwp = proc->query_for_stopped_lwp();
       assert(stoppedlwp);
       stoppedlwp->changeMemoryProtections(mutateeBase, size, winrights, false);
#endif
   }
   
   sensitivity_cerr << hex << " Adding range [0x" << start << " 0x" << start + size
       << "), allocated buffer base " << mutateeBase << " and so shift " 
       << mutateeBase - start << dec << endl;

   addRange(start,
            size,
            mutateeBase - start);
   
   //addedRanges_[start] = end;
   free(buffer);
}

// helper for addRegion(reg,size), do not call directly
void MemoryEmulator::addRange(Address start, unsigned size, Address shift) {
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

/* Synchronizes shadow and original memory pages that contain analyzed code
 * if toOrig == true: copies shadow memory into original
 * if toOrig == false: copies original memory into shadow memory
 */ 
void MemoryEmulator::synchShadowOrig(mapped_object * obj, bool toOrig) 
{
    if (toOrig) malware_cerr << "Syncing shadow to orig for obj " << obj->fileName() << endl;
    else        malware_cerr << "Syncing orig to shadow for obj " << obj->fileName() << endl;
    using namespace SymtabAPI;
    vector<Region*> regs;
    obj->parse_img()->getObject()->getCodeRegions(regs);
    std::vector< std::pair<std::pair<Address,Address>,unsigned long> > elements;
    memoryMap_.elements(elements);
    for (unsigned idx=0; idx < elements.size(); idx++) {
        std::pair<Address,Address> range(0,0);
        unsigned int shift = 0;
        boost::tie(range,shift) = elements[idx];
        unsigned char *rangebuf = (unsigned char*) malloc(range.second-range.first);
        Address from = 0;
        Address toShift = 0;
        if (toOrig) {
            from = range.first;
            toShift = shift;
        } else {
            from = obj->codeBase() + shift;
            toShift = 0;
        }
        if (!aS_->readDataSpace((void *)from,
                                range.second-range.first,
                                rangebuf,
                                false)) 
        {
            assert(0);
        }
        std::map<Address,int>::const_iterator sit = springboards_.begin();
        Address cp_start = range.first;
        cerr << "SYNC WRITE TO [" << hex << range.first << " " << range.second << ")" << dec << endl;
        for (; sit != springboards_.end() && sit->first < range.second; sit++) {
            if ((*sit).first < range.first) {
                continue;
            }
            int cp_size = sit->first - cp_start;
            cerr << "\t Write " << hex << cp_start +toShift << "..." << cp_start + cp_size + toShift<< dec << endl;
            if (cp_size &&
                !aS_->writeDataSpace((void *)(cp_start + toShift),
                                     cp_size,
                                     rangebuf + cp_start))
            {
                assert(0);
            }
            cp_start = sit->first + sit->second;
        }

        cerr << "\t Write " << hex << cp_start +toShift << "..." << range.second - cp_start<< dec << endl;
        if (cp_start < range.second &&
            !aS_->writeDataSpace((void *)(cp_start + toShift),
                                 range.second - cp_start,
                                 rangebuf + cp_start))
        {
            assert(0);
        }
        free(rangebuf);
    }
}


void MemoryEmulator::addSpringboard(Address addr, int size) 
{
    // DEBUGGING
    //map<Address,int>::iterator sit = springboards_[reg].begin();
    //for (; sit != springboards_[reg].end(); ++sit) {
    //    if (sit->first == offset) continue;
    //    if (sit->first + sit->second <= offset) continue;
    //    if (sit->first >= offset + size) break;
    //    assert(0);
    //}
    for (Address tmp = addr; tmp < addr + size; ++tmp) {
        springboards_.erase(tmp);
    }

    springboards_[addr] = size;
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
    malware_cerr << "  untracking springboards from deadblock [" << hex 
         << bbi->start() << " " << bbi->end() << ")" << dec <<endl;
    if (springboards_.find(bbi->start()) == springboards_.end()) {
        cerr << "ERROR IN DELETING SPRINGBOARD!" << endl;
    }
    springboards_.erase(bbi->start());
}
