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

#include "proccontrol/src/memcache.h"
#include "proccontrol/src/int_process.h"

using namespace std;

memEntry::memEntry(Address a, unsigned long size_) :
   addr(a),
   had_error(false),
   size(size_)
{
   buffer = (char *) malloc(size);
}

memEntry::~memEntry()
{
   if (buffer)
      free(buffer);
   buffer = NULL;
}

Address memEntry::getAddress() const
{
   return addr;
}

char *memEntry::getBuffer() const
{
   return buffer;
}

unsigned long memEntry::getSize() const
{
   return size;
}

void onContinueMemCache(int_thread *thr)
{
   thr->llproc()->getMemCache()->clear();
}

memCache::memCache(int_process *p) :
   proc(p),
   block_size(0),
   word_cache(0),
   word_cache_addr(0),
   word_cache_valid(false),
   pending_async(false)
{
   static bool registeredMemCacheClear = false;
   if (!registeredMemCacheClear) {
      registeredMemCacheClear = true;
      int_thread::addContinueCB(onContinueMemCache);
   }
}

memCache::~memCache()
{
   clear();
}

void memCache::getPendingAsyncs(set<response::ptr> &resps)
{
   for (mcache_t::iterator i = mem_cache.begin(); i != mem_cache.end(); i++) {
      memEntry *entry = i->second;
      if (!entry->resp || !entry->res_resp)
         continue;
      assert(!entry->resp && entry->res_resp);
      response::ptr resp;
      if (entry->resp)
         resp = entry->resp;
      else 
         resp = entry->res_resp;
      if (resp->isReady() || resp->hasError())
         continue;
      resps.insert(resp);
   }
}

memCache::memRet_t memCache::readMemoryAsync(void *dest, Address src, unsigned long size, 
                                              set<mem_response::ptr> &resps,
                                              int_thread *reading_thread)
{
   pending_async = false;
   bool had_error = false;
   
   Address start_block = src - (src % block_size);
   
   for (Address cur = start_block; cur < src+size; cur += block_size) {
      mcache_t::iterator i = mem_cache.find(cur);
      if (i == mem_cache.end()) {
         pending_async = true;
         memEntry *me = new memEntry(cur, block_size);
         me->resp = mem_response::createMemResponse(me->getBuffer(), block_size);
         bool result = proc->readMem(cur, me->resp, reading_thread);
         if (!result) {
            pthrd_printf("Error caching read on %d\n", proc->getPid());
            me->had_error = true;
         }
         mem_cache[cur] = me;
         resps.insert(me->resp);
         continue;
      }
      memEntry *me = i->second;
      if (me->resp && !me->resp->isReady()) {
         //This read was already posted and isn't ready
         pending_async = true;
         continue;
      }
      if (me->resp && me->resp->hasError()) {
         //This read had an error
         had_error = true;
         continue;
      }
      if (!pending_async && !had_error) {
         char *target_mem;
         unsigned long copy_size;
         char *src_mem;

         if (cur == start_block)
            target_mem = (char *) dest;
         else
            target_mem = ((char *) dest) + (cur - src);

         Address me_start = me->addr;
         Address me_end = me->addr + block_size;
         Address src_start = src;
         Address src_end = src + size;

         //Compute intersection of i & curr
         Address intersect_start = me_start > src_start ? me_start : src_start;
         Address intersect_end = me_end < src_end ? me_end : src_end;
         assert(intersect_start <= intersect_end);
         
         copy_size = intersect_end - intersect_start;
         src_mem = me->buffer + (intersect_start - me->addr);

         memcpy(target_mem, src_mem, copy_size);
      }
   }
   
   if (had_error)
      return ret_error;
   if (pending_async)
      return ret_async;
   return ret_success;
}

memCache::memRet_t memCache::readMemorySync(void *buffer, Address addr, unsigned long size,
                                             int_thread *reading_thread)
{
   if (size != 1) {
      mem_response::ptr memresult = mem_response::createMemResponse((char *) buffer, size);
      bool result = proc->readMem(addr, memresult, reading_thread);
      if (!result) {
         pthrd_printf("Failed to read memory for proc reader\n");
         return ret_error;
      }
      result = memresult->isReady();
      assert(result);
      return ret_success;
   }

   //Try to optimially handle a case where the calling code
   // reads a string one char at a time.  This is mostly for
   // ptrace platforms, but won't harm any others.
   assert(size == 1);
   Address aligned_addr = addr - (addr % sizeof(word_cache));
   if (!word_cache_valid || aligned_addr != word_cache_addr) {
      mem_response::ptr memresult = mem_response::createMemResponse((char *) &word_cache, sizeof(word_cache));
      bool result = proc->readMem(aligned_addr, memresult, reading_thread);
      if (!result) {
         pthrd_printf("Failed to read memory for proc reader\n");
         return ret_error;
      }
      result = memresult->isReady();
      assert(result);
      word_cache_addr = aligned_addr;
      word_cache_valid = true;
   }
   *((char *) buffer) = ((char *) &word_cache)[addr - aligned_addr];
   return ret_success;
}

memCache::memRet_t memCache::writeMemoryAsync(Dyninst::Address dest, void *src, unsigned long size, 
                                              std::set<result_response::ptr> &resps,
                                              int_thread *writing_thread)
{
   pending_async = false;
   pthrd_printf("Performing memCache::async_write to %lx/%lu on %d\n", dest, size, proc->getPid());
   mcache_t::iterator i = write_cache.find(dest);
   if (i != write_cache.end()) {
      memEntry *me = i->second;
      if (me->getSize() == size &&
          !memcmp(me->getBuffer(), src, size))
      {
         pthrd_printf("Already performed this write... returning existing state\n");
         assert(me->res_resp);
         if (me->res_resp->hasError()) {
            return ret_error;
         }
         if (!me->res_resp->isReady()) {
            pending_async = true;
            resps.insert(me->res_resp);
            return ret_async;
         }
         return ret_success;
      }
   }

   memEntry *me = new memEntry(dest, size);
   memcpy(me->getBuffer(), src, size);
   me->res_resp = result_response::createResultResponse();
   write_cache[dest] = me;

   bool result = proc->writeMem(src, dest, size, me->res_resp, writing_thread);
   if (!result || me->res_resp->hasError()) {
      pthrd_printf("Error writing memory\n");
      return ret_error;
   }
   if (!me->res_resp->isReady()) {
      pthrd_printf("Async result while writing memory\n");
      pending_async = true;
      resps.insert(me->res_resp);
      return ret_async;
   }
   return ret_success;
}

memCache::memRet_t memCache::writeMemorySync(Dyninst::Address dest, void *src, unsigned long size,
                                             int_thread *write_thread)
{
   result_response::ptr resp = result_response::createResultResponse();
   bool result = proc->writeMem(src, dest, size, resp, write_thread);
   if (!result) {
      pthrd_printf("Error writing memory at %lx/%lu on %d\n",
                   dest, size, proc->getPid());
      return ret_error;
   }
   result = resp->isReady();
   assert(result);
   return ret_success;
}

memCache::memRet_t memCache::readMemory(void *dest, Address src, unsigned long size, 
                                         set<mem_response::ptr> &resps, int_thread *thrd)
{
   if (!block_size) 
      block_size = proc->plat_getRecommendedReadSize();
   if (proc->plat_needsAsyncIO())
      return readMemoryAsync(dest, src, size, resps, thrd);
   else
      return readMemorySync(dest, src, size, thrd);
}

memCache::memRet_t memCache::writeMemory(Dyninst::Address dest, void *src, unsigned long size,
                                         std::set<result_response::ptr> &resps, int_thread *thrd)
{
   if (!block_size)
      block_size = proc->plat_getRecommendedReadSize();
   if (proc->plat_needsAsyncIO())
      return writeMemoryAsync(dest, src, size, resps, thrd);
   else
      return writeMemorySync(dest, src, size, thrd);
}

void memCache::clear()
{
   pthrd_printf("Clearing memCache\n");
   for (mcache_t::iterator i = mem_cache.begin(); i != mem_cache.end(); i++)
      delete i->second;
   mem_cache.clear();

   for (mcache_t::iterator i = write_cache.begin(); i != write_cache.end(); i++)
      delete i->second;
   write_cache.clear();

   word_cache_valid = false;
   pending_async = false;
}

bool memCache::hasPendingAsync() {
   return pending_async;
}

