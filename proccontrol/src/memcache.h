/*
 * Copyright (c) 1996-2011 Barton P. Miller
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

#if !defined(MEMCACHE_H_)
#define MEMCACHE_H_

#include "dynutil/h/dyntypes.h"
#include "proccontrol/src/response.h"
#include <set>
#include <map>

class int_process;

class memEntry {
   friend struct memEntry_cmp;
   friend class memCache;
  private:
   Dyninst::Address addr;
   char *buffer;
   bool ready;
   bool had_error;

  public:
   memEntry(Dyninst::Address a, unsigned long size);
   ~memEntry();

   Dyninst::Address getAddress() const;
   char *getBuffer() const;
};

class memCache {
  private:
   int_process *proc;
   typedef std::map<Dyninst::Address, memEntry *> mcache_t;
   mcache_t mem_cache;
   std::map<mem_response::ptr, memEntry *> response_map;
   unsigned int block_size;
  public:
   memCache(int_process *p);
   ~memCache();

   typedef enum {
      ret_success,
      ret_async,
      ret_error
   } readRet_t;

   readRet_t readMemory(int_thread *thr, void *dest, Dyninst::Address src, unsigned long size, 
                        std::set<mem_response::ptr> &resps); 
   bool postReadResult(mem_response::ptr);
   void clear();
  private:
   readRet_t readMemoryAsync(int_thread *thr, void *dest, Dyninst::Address src, unsigned long size, 
                             std::set<mem_response::ptr> &resps); 
   readRet_t readMemorySync(int_thread *thr, void *dest, Dyninst::Address src, unsigned long size); 
};

#endif
