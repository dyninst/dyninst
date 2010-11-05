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

#if !defined(_MEMORY_EMULATOR_H_)
#define _MEMORY_EMULATOR_H_

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
   void update();

   std::pair<bool, Address> translate(SymtabAPI::Region *reg, unsigned long offset);
//   Address translate(mapped_object *obj, unsigned long offset);
   
  private:
   void addRegion(Address start, unsigned size, unsigned long shift);
   bool findMutateeTable();
   unsigned addrWidth();

   AddressSpace *aS_;

   // Track what the address space looks like for defensive mode. 
   typedef IntervalTree<Address, unsigned long> MemoryMapTree;
   MemoryMapTree memoryMap_;

   Address mutateeBase_;

   typedef std::map<SymtabAPI::Region *, Address> RegionMap;
   RegionMap addedRegions_;
};
};

#endif
