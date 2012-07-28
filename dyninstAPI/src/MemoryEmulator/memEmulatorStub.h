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
#if !defined(MEM_EMULATOR_STUB)
#define MEM_EMULATOR_STUB

#if defined(cap_mem_emulation)
#error
#endif

class mapped_object;

namespace Dyninst {
class MemoryEmulator {
 public:
  MemoryEmulator(AddressSpace *) {};
  void addSpringboard(SymtabAPI::Region *, Address, int) {};
  void removeSpringboards(func_instance*) {};
  void removeSpringboards(const block_instance*) {};
  void addAllocatedRegion(Address, unsigned) {};
  void addRegion(mapped_object *) {};
  void removeRegion(Address, unsigned) {};
  void removeRegion(mapped_object *) {};
  void update() {};
  void synchShadowOrig(bool) {};
  std::pair<bool, Address> translate(Address) { return std::make_pair(false, 0);  }
  std::pair<bool, Address> translateBackwards(Address) { return std::make_pair(false, 0); }
  static const int STACK_SHIFT_VAL = 0;
  void debug() {};

};
};

#endif
