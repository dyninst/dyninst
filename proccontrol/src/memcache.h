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

#if !defined(MEMCACHE_H_)
#define MEMCACHE_H_

#include "common/h/dyntypes.h"
#include "response.h"
#include <vector>
#include <set>
#include <map>

class int_process;

typedef enum {
   aret_error = 0,
   aret_success,
   aret_async 
} async_ret_t;

typedef enum {
   token_none = 0,
   token_getmsg,
   token_seteventreporting,
   token_setevent,
   token_init
} token_t;

/**
 * DON'T USE THE MEMCACHE FOR ARBITRARY MEMORY OPERATIONS!
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
 * 
 * Update - The memcache can now store registers.  Just what
 * every memcache needs.
 **/
class memCache;
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
   int operation_num;
   bool clean_buffer;
   bool is_read;
   bool is_write;
   token_t token_type;

   void invalidate();
  public:
   memEntry();
   memEntry(token_t token);
   memEntry(Dyninst::Address remote, void *local, unsigned long size, bool is_read, memCache *cache);
   memEntry(const memEntry *me, char* b);
   memEntry& operator=(const memEntry&) = delete;
   memEntry(const memEntry&) = delete;
   ~memEntry();

   Dyninst::Address getAddress() const;
   char *getBuffer() const;
   unsigned long getSize() const;
   bool isRead() const;
   bool isWrite() const;
   bool isToken() const;
   bool operator==(const memEntry &b) const;
};

class memCache {
   friend class memEntry;
  private:
   int_process *proc;
   typedef std::vector<memEntry *> mcache_t;
   mcache_t mem_cache;
   mcache_t::iterator last_operation;

   unsigned int block_size;

   long word_cache;
   Dyninst::Address word_cache_addr;
   bool word_cache_valid;
   bool pending_async;
   bool have_writes;
   bool sync_handle;
   int operation_num;
   std::map<int_thread *, allreg_response::ptr> regs;

   async_ret_t doOperation(memEntry *me, int_thread *op_thread);
   async_ret_t getExistingOperation(mcache_t::iterator i, memEntry *orig);   
   async_ret_t lookupAsync(memEntry *me, int_thread *op_thread);
   void updateReadCacheWithWrite(Address dest, char *src, unsigned long size);
  public:
   memCache(int_process *p);
   ~memCache();

   async_ret_t readMemory(void *dest, Dyninst::Address src, unsigned long size, 
                          std::set<mem_response::ptr> &resps, int_thread *thrd = NULL); 
   async_ret_t writeMemory(Dyninst::Address dest, void *src, unsigned long size, 
                           std::set<result_response::ptr> &resps, int_thread *thrd = NULL); 
   async_ret_t getRegisters(int_thread *thr, int_registerPool &pool);

   void startMemTrace(int &record);
   void clear();
   bool hasPendingAsync();
   void getPendingAsyncs(std::set<response::ptr> &resps);   
   void setSyncHandling(bool b);
   void markToken(token_t tk);
   void condense();

  private:
   async_ret_t readMemoryAsync(void *dest, Dyninst::Address src, unsigned long size, 
                               std::set<mem_response::ptr> &resps,
                               int_thread *reading_thread);
   async_ret_t readMemorySync(void *dest, Dyninst::Address src, unsigned long size,
                              int_thread *reading_thread);
   async_ret_t writeMemoryAsync(Dyninst::Address dest, void *src, unsigned long size, 
                                std::set<result_response::ptr> &resps, 
                                int_thread *writing_thrd = NULL);
   async_ret_t writeMemorySync(Dyninst::Address dest, void *src, unsigned long size, 
                               int_thread *writing_thrd = NULL);
};

#endif
