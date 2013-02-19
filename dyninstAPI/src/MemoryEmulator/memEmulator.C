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

#include "parseAPI/h/CodeObject.h"
#include "dyninstAPI/src/addressSpace.h"
#include "memEmulator.h"
#include "dyninstAPI/src/mapped_object.h"
#include "dyninstAPI/src/image.h"
#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Region.h"
#include "dyninstAPI/src/dynProcess.h"
#include "dyninstAPI/src/function.h"
#include "dyninstAPI/src/debug.h"

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
          Address base = elements[i].first.first;

          newMapper.elements[i].lo = base;
          newMapper.elements[i].hi = elements[i].first.second;
          assert(newMapper.elements[i].hi > newMapper.elements[i].lo);
          newMapper.elements[i].shift = elements[i].second;
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
    if (aS_->runtime_lib.find(obj) != aS_->runtime_lib.end())
    {
        // Runtime library, skip
        return;
    }
   // Add each code region
    std::vector<Region *> codeRegions;
    obj->parse_img()->getObject()->getCodeRegions(codeRegions);

    for (unsigned i = 0; i < codeRegions.size(); ++i) {
        Region *reg = codeRegions[i];

      addRegion(reg, obj->codeBase(), 0, 0);
    }
}

void MemoryEmulator::removeRegion(mapped_object *obj) {
	sensitivity_cerr << "Removing region " << obj->fileName() << endl;
	sensitivity_cerr << "\t Before: " << endl;
	debug();
	// Remove each code region
	std::vector<Region *> codeRegions;
	obj->parse_img()->getObject()->getCodeRegions(codeRegions);

	for (unsigned i = 0; i < codeRegions.size(); ++i) {
		Region *reg = codeRegions[i];

		removeRegion(reg, obj->codeBase());
	}
	sensitivity_cerr << "\t After: " << endl;
	debug();
}

void MemoryEmulator::addRegion(Region *reg, Address base, Address, Address) {

   if (addedRegions_.find(reg) != addedRegions_.end()) return;
      
   PCProcess *proc = dynamic_cast<PCProcess *>(aS_);
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
   allocSize += 0x1000;

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
       using namespace SymtabAPI;
	   PCProcess::PCMemPerm memPerm_;
       Region::perm_t reg_rights = reg->getRegionPermissions();
       switch (reg_rights) {
       case Region::RP_R:
           memPerm_.setR();  // PAGE_READONLY;
           break;
       case Region::RP_RW: 
           memPerm_.setR().setW();  // PAGE_READWRITE;
           break;
       case Region::RP_RX:
           memPerm_.setR().setX();  // PAGE_EXECUTE_READ;
           break;
       case Region::RP_RWX:
           memPerm_.setR().setW().setX();  // PAGE_EXECUTE_READWRITE;
           break;
       default:
           assert(0);
       }

	   PCProcess *proc = aS_->proc();
       assert(proc);
	   proc->stopProcess();
       proc->changeMemoryProtections(mutateeBase, reg->getMemSize(), memPerm_, false);
   }
   
    Address regionBase = base + reg->getMemOffset();

   addRegion(regionBase,
             reg->getMemSize(),
             mutateeBase - regionBase);
   

   addedRegions_[reg] = std::make_pair(base + reg->getMemOffset(), mutateeBase);
   free(buffer);
}

void MemoryEmulator::removeRegion(Region *reg, Address base) {
   
   //cerr << "\t\t Region " << i << ": " << hex
   //<< codeRegions[i]->getMemOffset() + obj->codeBase() << " -> " 
   //<< codeRegions[i]->getMemOffset() + codeRegions[i]->getMemSize() + obj->codeBase() << endl;
   
	RegionMap::iterator iter = addedRegions_.find(reg);
	if (iter == addedRegions_.end()) return;

	// First, nuke our track of the springboards
    springboards_.erase(reg);

    // Second, nuke it from the list of regions to copy on a sync
    addedRegions_.erase(reg);

   // Deallocate the shadow pages in the mutatee
   //  -- this is TODO; we mangle the allocation base and therefore can't
   //     really call inferiorfree on it. 

   // Remove the region from the translation map
   removeRegion(base + reg->getMemOffset(), reg->getMemSize());
}

void MemoryEmulator::addRegion(Address start, unsigned size, Address shift) {
    if (size == 0) return;
   //debug();
   //cerr << endl;
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
   //debug();
   return;   
}

void MemoryEmulator::removeRegion(Address addr, unsigned size) {
	Address lb = 0, ub = 0;
	unsigned long shiftVal;

	cerr << "MemoryEmulator: removing region " << hex << addr << " : " << size << dec << endl;

   //debug();
   //cerr << endl;

	Address lowLB = 0, lowUB = 0, hiLB = 0, hiUB = 0;

	// We are guaranteed to be either our own allocated range or
	// coalesced with another range. 
	if (!memoryMap_.find(addr, lb, ub, shiftVal)) {
		return;
	}

	if ((lb != 0) && (lb < addr)) {
		lowLB = lb;
		lowUB = addr;
	}
	if ((ub != 0) && (ub > (addr + size))) {
		hiLB = (addr + size);
		hiUB = ub;
	}
	memoryMap_.remove(lb);
	if (lowLB || lowUB) {
		memoryMap_.insert(lowLB, lowUB, shiftVal);
	}
	if (hiLB || hiUB) {
		memoryMap_.insert(hiLB, hiUB, shiftVal);
	}
	
	reverseMemoryMap_.remove(addr + shiftVal);
	//debug();
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

void MemoryEmulator::synchShadowOrig(bool toOrig) 
{

    if (toOrig) {
        malware_cerr << "Syncing shadow to orig" << endl;
    }
    else {
        malware_cerr << "Syncing orig to shadow" << endl;
    }

    using namespace SymtabAPI;

    for (RegionMap::iterator iter = addedRegions_.begin();
        iter != addedRegions_.end(); ++iter) {
        Region * reg = iter->first;

        // We copy "source" (where we're copying from) and "target" (where we're
        // copying to). We then select snippets of target (where springboards reside)
        // and copy them into source, and then write source into target.

        unsigned char *source = (unsigned char*) malloc(reg->getMemSize());
        unsigned char *target = (unsigned char *) malloc(reg->getMemSize());

        Address from = 0;
        Address to = 0;
        if (toOrig) {
            from = iter->second.second;
            to = iter->second.first;
        } else {
            from = iter->second.first;
            to = iter->second.second;
        }

        if (!aS_->readDataSpace((void *)from,
                                reg->getMemSize(),
                                source,
                                false)) 
        {
            assert(0);
        }
        if (!aS_->readDataSpace((void *)to,
            reg->getMemSize(),
            target,
            false)) 
        {
            assert(0);
        }

        std::map<Address,int>::const_iterator sit = springboards_[reg].begin();
        for (; sit != springboards_[reg].end(); sit++) {

            Address fromLB = (Address) target + sit->first;
            Address toLB = (Address) source + sit->first;
            memcpy((void *)toLB, (void *)fromLB, sit->second);
        }

        if (!aS_->writeDataSpace((void *)to,
            reg->getMemSize(),
            source))
        {
            assert(0);
        }
        free(source);
        free(target);

    }

}


void MemoryEmulator::addSpringboard(Region *reg, Address offset, int size) 
{
    // Look up whether there is a previous springboard that overlaps with us; 
    // clearly, it's getting removed. 

    std::map<SymtabAPI::Region *, std::map<Address, int> >::iterator s_iter = springboards_.find(reg);
    if (s_iter == springboards_.end()) {
        springboards_[reg][offset] = size;
        return;
    }
    std::map<Address, int> &smap = s_iter->second;

    springboard_cerr << "Inserting SB [" << hex << offset << "," << offset + size << "]" << dec << endl;

    std::map<Address, int>::iterator iter = smap.find(offset);
    if (iter == smap.end()) {
        smap[offset] = size;
    }
    else if (size > iter->second) {
        smap[offset] = size;
    }
    // Otherwise keep the current value
    springboard_cerr << "\t New value: " << hex << offset << " -> " << smap[offset] + offset << dec << endl;
}

void MemoryEmulator::removeSpringboards(func_instance * func) 
{
   malware_cerr << "untracking springboards from deadfunc " << hex << func->addr() << dec << endl;

   const PatchFunction::Blockset & blocks = func->blocks();
   PatchFunction::Blockset::const_iterator bit = blocks.begin();
   for (; bit != blocks.end(); bit++) {
      removeSpringboards(SCAST_BI(*bit));
   }
}

void MemoryEmulator::removeSpringboards(const block_instance *bbi) 
{
    malware_cerr << "  untracking springboards from deadblock [" << hex 
         << bbi->start() << " " << bbi->end() << ")" << dec <<endl;
    SymtabAPI::Region * reg = 
       ((ParseAPI::SymtabCodeRegion*)bbi->llb()->region())->symRegion();
    springboards_[reg].erase(bbi->llb()->start() - reg->getMemOffset());
    if (springboards_[reg].empty()) springboards_.erase(reg);
}

void  MemoryEmulator::debug() const {
   if (!dyn_debug_sensitivity) {
      return;
   }
	std::vector<MemoryMapTree::Entry> elements;
	memoryMap_.elements(elements);
	cerr << "\t Forward map: " << endl;
	for (std::vector<MemoryMapTree::Entry>::iterator iter = elements.begin(); iter != elements.end(); ++iter)
	{
		cerr << "\t\t " << hex << "[" << iter->first.first << "," << iter->first.second << "]: " << iter->second << dec << endl;
#if 0 // debug output
      if (iter->first.first == 0x40d000) {
         Address val;
         Address addr = (iter->second + 0x40d84b);
         assert(sizeof(Address) == aS_->getAddressWidth());
         Address width = aS_->getAddressWidth();
         for (Address idx=0; idx < 0x40; idx+=width) {
            aS_->readDataSpace((void*)(addr+idx), width, &val, true);
            cerr << hex << " " << 0x40d84b + idx << "[" << addr+idx << "]:  ";
            fprintf(stderr,"%2x",((unsigned char*)&val)[3]);
            fprintf(stderr,"%2x",((unsigned char*)&val)[2]);
            fprintf(stderr,"%2x",((unsigned char*)&val)[1]);
            fprintf(stderr,"%2x\n",((unsigned char*)&val)[0]);
         }
      }
#endif
	}
	elements.clear();
	cerr << "\t Backwards map: " << endl;
	reverseMemoryMap_.elements(elements);
	for (std::vector<MemoryMapTree::Entry>::iterator iter = elements.begin(); iter != elements.end(); ++iter)
	{
		cerr << "\t\t " << hex << "[" << iter->first.first << "," << iter->first.second << "]: " << iter->second << dec << endl;
	}
	elements.clear();

}

void MemoryEmulator::addPOPAD(Address addr)
{
    emulatedPOPADs_.insert(addr);
}

bool MemoryEmulator::isEmulPOPAD(Address addr)
{
    Address orig = -1;
    std::vector<func_instance*> dontcare1;
    baseTramp *dontcare2;
    if (!aS_->getAddrInfo(addr, orig, dontcare1, dontcare2)) {
        assert(0);
    }
    return emulatedPOPADs_.end() != emulatedPOPADs_.find(orig);
}
