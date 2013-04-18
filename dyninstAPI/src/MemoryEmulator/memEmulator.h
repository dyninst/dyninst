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

#if !defined(cap_mem_emulation)
#include "memEmulatorStub.h"
#else 



#if !defined(_MEMORY_EMULATOR_H_)
#define _MEMORY_EMULATOR_H_

#include "common/src/IntervalTree.h"
#include "dyninstAPI/src/MemoryEmulator/memEmulatorTransformer.h"
#include "dyninstAPI/src/MemoryEmulator/memEmulatorWidget.h"

class AddressSpace;
class mapped_object;
class int_variable;

namespace Dyninst {
   namespace SymtabAPI {
      class Region;
   };

class MemoryEmulator {
  public:
   MemoryEmulator(AddressSpace *addrSpace)
      : aS_(addrSpace), mutateeBase_(0) {};
   ~MemoryEmulator() {};

   void addAllocatedRegion(Address start, unsigned size);
   void addRegion(mapped_object *obj);
   void addRegion(SymtabAPI::Region *reg, Address base, Address bufferStart, Address bufferEnd);
   void update();

   void removeRegion(mapped_object *obj);
   void removeRegion(SymtabAPI::Region *reg, Address base);
   void removeRegion(Address start, unsigned size);

   void reprocess(mapped_object *obj);

   std::pair<bool, Address> translate(Address addr);
   std::pair<bool, Address> translateBackwards(Address addr);
   
    const std::map<Address,int> & getSpringboards(SymtabAPI::Region*) const;
    void removeSpringboards(func_instance* deadfunc);
    void removeSpringboards(const block_instance* deadBBI);
    void addSpringboard(SymtabAPI::Region*, 
        Address offset,/*from start of region*/    
        int size);
    void synchShadowOrig(bool toOrig);

    void addPOPAD(Address addr);
    bool isEmulPOPAD(Address addr);

    static const int STACK_SHIFT_VAL=256;

	void debug() const;

  private:
   void addRegion(Address start, unsigned size, Address newBase);


   bool findMutateeTable();
   unsigned addrWidth();

   AddressSpace *aS_;

   // Track what the address space looks like for defensive mode. 
   typedef IntervalTree<Address, unsigned long> MemoryMapTree;
   MemoryMapTree memoryMap_;
   MemoryMapTree reverseMemoryMap_;

   Address mutateeBase_;

   std::map<SymtabAPI::Region*, std::map<Address,int> > springboards_;

   // First address: original base in memory. Second address: shadow base.
   typedef std::map<SymtabAPI::Region *, std::pair< Address, Address> > RegionMap;
   RegionMap addedRegions_;

   std::map<SymtabAPI::Region *, unsigned char *> saved;
   std::set<Address> emulatedPOPADs_;
};
};

#endif
#endif
