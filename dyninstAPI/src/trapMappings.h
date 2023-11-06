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

#if !defined(trapMappings_h_)
#define trapMappings_h_

#define MIN_TRAP_TABLE_SIZE 256
#define INDEX_INVALID UINT_MAX

#include <vector>
#include <set>
#include "dyntypes.h"

class AddressSpace;
class int_variable;

class trampTrapMappings {
 public:
   typedef struct {
      Dyninst::Address from_addr;
      Dyninst::Address to_addr;
      bool written;
      bool mutatee_side;
      unsigned cur_index;
   } tramp_mapping_t;
 private:

   dyn_hash_map<Dyninst::Address, tramp_mapping_t> mapping;
   std::set<tramp_mapping_t *> updated_mappings;

   static void arrange_mapping(tramp_mapping_t &m, bool should_sort,
                               std::vector<tramp_mapping_t*> &mappings_to_add,
                               std::vector<tramp_mapping_t*> &mappings_to_update);

   bool needs_updating;
   AddressSpace *as;

   const int_variable *trapTableUsed;
   const int_variable *trapTableVersion;
   const int_variable *trapTable;
   const int_variable *trapTableSorted;

   void writeToBuffer(unsigned char *buffer, unsigned long val, 
                      unsigned addr_width);
   void writeTrampVariable(const int_variable *var, unsigned long val);

   unsigned long table_version;
   unsigned long table_used;
   unsigned long table_allocated;
   unsigned long table_mutatee_size;
   Dyninst::Address current_table;
   Dyninst::Address table_header;
   bool blockFlushes;
   
 public:
   trampTrapMappings(AddressSpace *a);
   void copyTrapMappings(trampTrapMappings *parent);
   void clearTrapMappings();

   void addTrapMapping(Dyninst::Address from, Dyninst::Address to, bool write_to_mutatee = false);
   Dyninst::Address getTrapMapping(Dyninst::Address from);
   bool definesTrapMapping(Dyninst::Address from);
   bool needsUpdating();
   void flush();
   void allocateTable();
   void shouldBlockFlushes(bool b) { blockFlushes = b; }

   bool empty();

   AddressSpace *proc() const;
};

#endif
