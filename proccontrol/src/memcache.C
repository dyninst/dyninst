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

memEntry::memEntry(Address a, unsigned long size) :
   addr(a),
   ready(false),
   had_error(false)
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

memCache::memCache(int_process *p) :
   proc(p),
   block_size(p->plat_getRecommendedReadSize())
{
}

memCache::~memCache()
{
   clear();
}

memCache::readRet_t memCache::readMemoryAsync(int_thread *thr, void *dest, Address src, unsigned long size, 
                                              set<mem_response::ptr> &resps)
{
   resps.clear();

   for (map<mem_response::ptr, memEntry *>::iterator i = response_map.begin(); 
        i != response_map.end(); i++) 
   {
      Address a = i->second->addr;
      if (a >= src && a < src + size)
         return ret_async;
   }

   bool have_pending = false;
   bool had_error = false;
   
   Address start_block = src - (src % block_size);
   
   for (Address cur = start_block; cur < src+size; cur += block_size) {
      mcache_t::iterator i = mem_cache.find(cur);
      if (i == mem_cache.end()) {
         have_pending = true;
         memEntry *me = new memEntry(cur, block_size);
         mem_response::ptr resp = mem_response::createMemResponse(me->getBuffer(), block_size);
         bool result = proc->plat_readMemAsync(thr, cur, resp);
         if (!result) {
            pthrd_printf("Error caching read on %d/%d\n", proc->getPid(), thr->getLWP());
            me->had_error = true;
         }
         mem_cache[cur] = me;
         resps.insert(resp);
         response_map[resp] = me;
         continue;
      }
      memEntry *me = i->second;
      if (!me->ready) {
         //This read was already ready and isn't complete
         have_pending = true;
         continue;
      }
      if (me->had_error) {
         //This read had an error
         had_error = true;
         continue;
      }
      if (!have_pending && !had_error) {
         char *target_mem;
         unsigned long copy_size;
         char *src_mem;

         if (cur == start_block)
            target_mem = (char *) dest;
         else
            target_mem = ((char *) dest) + (cur - src);

         Address me_start = me->addr;
         Address me_end = me->addr + block_size;
         Address src_start = cur;
         Address src_end = cur + size;

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
   if (have_pending)
      return ret_async;
   return ret_success;
}

memCache::readRet_t memCache::readMemorySync(int_thread *thr, void *dest, Address src, unsigned long size)
{
   bool result = proc->plat_readMem(thr, dest, src, size);
   if (!result)
      return ret_error;
   return ret_success;
}

memCache::readRet_t memCache::readMemory(int_thread *thr, void *dest, Address src, unsigned long size, 
                                         set<mem_response::ptr> &resps)
{
   if (proc->plat_needsAsyncIO())
      return readMemoryAsync(thr, dest, src, size, resps);
   else
      return readMemorySync(thr, dest, src, size);
}

bool memCache::postReadResult(mem_response::ptr resp)
{
   map<mem_response::ptr, memEntry *>::iterator i = response_map.find(resp);
   assert(i != response_map.end());

   memEntry *me = i->second;
   if (resp->hasError())
      me->had_error = true;
   else if (resp->isReady())
      me->ready = true;
   
   response_map.erase(i);
   return true;
}

void memCache::clear()
{
   assert(response_map.empty());

   for (mcache_t::iterator i = mem_cache.begin(); i != mem_cache.end(); i++)
      delete i->second;
   mem_cache.clear();
}

