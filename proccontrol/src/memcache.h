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

/**
 * DON'T USE THE MEMCACHE FOR ARBITRARY MEMORY OPERATIONS.  
 *
 * This class is meant to assist the SysV parser and the 
 * thread_db parser--where we're wrapping another library
 * that is not async aware.  
 * 
 * The semantics are such that this class expects that
 * reads and writes may be restarted, and will cache
 * and not redo operations it has already seen.  This
 * makes a difference when you do operations such as:
 * 
 * write 'A' -> 0x1000
 * write 'B' -> 0x1000
 * write 'A' -> 0x1000
 *
 * Under this class 0x1000 will contain 'B' after these three
 * operations.  The second 'A' write would be detected to be a
 * duplicated of the first and dropped.  We really do want these
 * semantics if we're restarting operations (we expect a write 'B'
 * will follow the second write 'A'), but this is inappropriate
 * for general purpose use.
 **/
class memEntry {
   friend struct memEntry_cmp;
   friend class memCache;
  private:
   Dyninst::Address addr;
   char *buffer;
   bool had_error;
   mem_response::ptr resp;
   result_response::ptr res_resp;
   unsigned long size;

  public:
   memEntry(Dyninst::Address a, unsigned long size);
   ~memEntry();

   Dyninst::Address getAddress() const;
   char *getBuffer() const;
   unsigned long getSize() const;
};

class memCache {
  private:
   int_process *proc;
   typedef std::map<Dyninst::Address, memEntry *> mcache_t;
   mcache_t mem_cache;
   mcache_t write_cache;

   unsigned int block_size;

   long word_cache;
   Dyninst::Address word_cache_addr;
   bool word_cache_valid;
   bool pending_async;
  public:
   memCache(int_process *p);
   ~memCache();

   typedef enum {
      ret_success,
      ret_async,
      ret_error
   } memRet_t;

   memRet_t readMemory(void *dest, Dyninst::Address src, unsigned long size, 
                        std::set<mem_response::ptr> &resps, int_thread *thrd = NULL); 
   memRet_t writeMemory(Dyninst::Address dest, void *src, unsigned long size, 
                         std::set<result_response::ptr> &resps, int_thread *thrd = NULL); 
   
   void clear();
   bool hasPendingAsync();
   void getPendingAsyncs(std::set<response::ptr> &resps);   
  private:
   memRet_t readMemoryAsync(void *dest, Dyninst::Address src, unsigned long size, 
                             std::set<mem_response::ptr> &resps,
                             int_thread *reading_thread);
   memRet_t readMemorySync(void *dest, Dyninst::Address src, unsigned long size,
                            int_thread *reading_thread);
   memRet_t writeMemoryAsync(Dyninst::Address dest, void *src, unsigned long size, 
                             std::set<result_response::ptr> &resps, 
                             int_thread *writing_thrd = NULL);
   memRet_t writeMemorySync(Dyninst::Address dest, void *src, unsigned long size, 
                            int_thread *writing_thrd = NULL);
   void updateReadCacheWithWrite(Address dest, char *src, unsigned long size);
};

#endif
