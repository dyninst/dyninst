/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

#if !defined(trapMappings_h_)
#define trapMappings_h_

#define MIN_TRAP_TABLE_SIZE 256
#define INDEX_INVALID UINT_MAX

#include <vector>
#include <set>

class AddressSpace;
class int_variable;

class trampTrapMappings {
 public:
   typedef struct {
      Address from_addr;
      Address to_addr;
      bool written;
      bool mutatee_side;
      unsigned cur_index;
   } tramp_mapping_t;
 private:

   hash_map<Address, tramp_mapping_t> mapping;
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
   Address current_table;
   
 public:
   trampTrapMappings(AddressSpace *a);
   void copyTrapMappings(trampTrapMappings *parent);
   void clearTrapMappings();

   void addTrapMapping(Address from, Address to, bool write_to_mutatee = false);
   Address getTrapMapping(Address from);
   bool definesTrapMapping(Address from);
   bool needsUpdating();
   void flush();
   AddressSpace *proc() const;
};

#endif
