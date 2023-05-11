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

#include "memcache.h"
#include "int_process.h"
#include <string.h>

using namespace std;

memEntry::memEntry() :
   addr(0),
   buffer(NULL),
   had_error(false),
   size(0),
   operation_num(0),
   clean_buffer(false),
   is_read(false),
   is_write(false),
   token_type(token_none)
{
}

memEntry::memEntry(Dyninst::Address remote, void *local, unsigned long size_, bool is_read_, memCache *cache) :
   addr(remote),
   buffer((char *) local),
   had_error(false),
   size(size_),
   operation_num(cache->operation_num),
   clean_buffer(false),
   is_read(is_read_),
   is_write(!is_read_),
   token_type(token_none)
{
}

memEntry::memEntry(token_t t) :
   addr(0),
   buffer(NULL),
   had_error(false),
   size(0),
   operation_num(0),
   clean_buffer(false),
   is_read(false),
   is_write(false),
   token_type(t)
{
}

memEntry::memEntry(const memEntry *me, char *b) :
   addr(me->addr),
   buffer(b),
   had_error(me->had_error),
   size(me->size),
   operation_num(me->operation_num),
   clean_buffer(true),
   is_read(me->is_read),
   is_write(me->is_write),
   token_type(me->token_type)
{
}

memEntry::~memEntry()
{
   if (buffer && clean_buffer)
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

bool memEntry::isRead() const
{
   return is_read;
}

bool memEntry::isWrite() const
{
   return is_write;
}

bool memEntry::isToken() const
{
   return token_type != token_none;
}

bool memEntry::operator==(const memEntry &b) const
{
   if (is_read != b.is_read || is_write != b.is_write || 
       token_type != b.token_type || addr != b.addr || size != b.size)
   {
      return false;
   }
   if (is_write && (memcmp(buffer, b.buffer, size) != 0))
      return false;
   return true;
}

void memEntry::invalidate() {
   is_read = false;
   is_write = false;
   token_type = token_none;
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
   pending_async(false),
   have_writes(false),
   sync_handle(false),
   operation_num(0)
{
   static bool registeredMemCacheClear = false;
   if (!registeredMemCacheClear) {
      registeredMemCacheClear = true;
      int_thread::addContinueCB(onContinueMemCache);
   }
   last_operation = mem_cache.end();
}

memCache::~memCache()
{
   clear();
}

void memCache::getPendingAsyncs(set<response::ptr> &resps)
{
   for (mcache_t::iterator i = mem_cache.begin(); i != mem_cache.end(); i++) {
      memEntry *entry = *i;
      if (!entry->resp && !entry->res_resp)
         continue;
      assert(entry->resp || entry->res_resp);
      response::ptr resp;
      if (entry->resp)
         resp = entry->resp;
      else 
         resp = entry->res_resp;
      if (resp->isReady() || resp->hasError())
         continue;
      resps.insert(resp);
   }
   for (map<int_thread *, allreg_response::ptr>::iterator j = regs.begin(); j != regs.end(); j++) {
      response::ptr resp = j->second;
      if (resp->isReady() || resp->hasError())
         continue;
      resps.insert(resp);
   }
      
}

void memCache::setSyncHandling(bool b)
{
   sync_handle = b;
}

void memCache::markToken(token_t tk)
{
   if (!proc->plat_needsAsyncIO())
      return;
      
   mcache_t::iterator i;
   for (i = mem_cache.begin(); i != mem_cache.end(); i++) {
      if ((*i)->token_type != tk)
         continue;
      last_operation = i;
      return;
   }
   memEntry *newEntry = new memEntry(tk);
   mem_cache.push_back(newEntry);
   last_operation = mem_cache.end();
   last_operation--;
}

void memCache::updateReadCacheWithWrite(Address dest, char *src, unsigned long size)
{
   Address start_block = dest - (dest % block_size);
   for (Address cur = start_block; cur < dest+size; cur += block_size) {
      Address write_start = dest;
      Address write_end = dest + size;
      for (mcache_t::iterator i = mem_cache.begin(); i != mem_cache.end(); i++) {
         if (!(*i)->isRead())
            continue;

         Address read_start = (*i)->getAddress();
         Address read_end = read_start + (*i)->getSize();
         if (write_start >= read_end || read_start >= write_end)
            continue;

         //Compute intersection of write and read
         Address intersect_start = write_start > read_start ? write_start : read_start;
         Address intersect_end = write_end < read_end ? write_end : read_end;
         
         char *target_mem = (*i)->getBuffer() + (intersect_start - cur);
         char *src_mem = src + (intersect_start - dest);
         unsigned long copy_size = intersect_end - intersect_start;
         
         memcpy(target_mem, src_mem, copy_size);      
      }
   }
}

void memCache::condense()
{
   if (!proc->plat_needsAsyncIO())
      return;
      
   mcache_t::iterator i;
   for (i = mem_cache.begin(); i != mem_cache.end(); i++) {
      memEntry *ent = *i;
      if (ent->isToken()) {
         ent->invalidate();
      }
      else if (ent->isWrite()) {
         updateReadCacheWithWrite(ent->addr, ent->buffer, ent->size);
         ent->invalidate();
      }
   }
   have_writes = false;
}

async_ret_t memCache::doOperation(memEntry *me, int_thread *op_thread)
{
   bool result = false;
   response::ptr resp;
   unsigned size = me->getSize();
   char *buffer = (char *) malloc(size);
   if (me->isRead()) {
      pthrd_printf("Performing async read memory in memCache\n");
      assert(!me->getBuffer());
      me->buffer = buffer;
      me->resp = mem_response::createMemResponse(buffer, size);
      if (sync_handle) me->resp->markSyncHandled();
      result = proc->readMem(me->getAddress(), me->resp, op_thread);
      resp = me->resp;
   }
   else if (me->isWrite()) {
      assert(me->isWrite());
      pthrd_printf("Performing async write memory in memCache\n");
      memcpy(buffer, me->getBuffer(), size);
      me->buffer = buffer;
      me->operation_num = ++operation_num;
      me->res_resp = result_response::createResultResponse();
      if (sync_handle) me->res_resp->markSyncHandled();
      result = proc->writeMem(buffer, me->getAddress(), size,
                              me->res_resp, op_thread);
      resp = me->res_resp;
   }
   else {
      assert(0);
   }
   
   if (!result || resp->hasError()) {
      pthrd_printf("Error accessing memory in memCache\n");
      return aret_error;
   }

   mem_cache.push_back(new memEntry(me, me->buffer));
   last_operation = mem_cache.end();
   last_operation--;

   if (!resp->isReady()) {
      return aret_async;
   }

   return aret_success;
}

async_ret_t memCache::getExistingOperation(mcache_t::iterator i, memEntry *orig)
{
   memEntry *me = *i;
   response::ptr resp;

   if (me->isRead()) {
      resp = me->resp;
      orig->resp = me->resp;
      orig->buffer = me->buffer;
   }
   else if (me->isWrite()) {
      resp = me->res_resp;
      orig->res_resp = me->res_resp;
   }
   else {
      assert(0);
   }

   last_operation = i;
   if (resp->hasError()) {
      pthrd_printf("Previous entry had error accessing memory in memCache\n");
      return aret_error;
   }
   if (!resp->isReady()) {
      return aret_async;
   }
   return aret_success;
}

async_ret_t memCache::lookupAsync(memEntry *me, int_thread *op_thread)
{
   if (last_operation == mem_cache.end()) {
      pthrd_printf("Async request is first in empty cache\n");
      return doOperation(me, op_thread);
   }

   mcache_t::iterator matching_entry = last_operation;
   bool first = true;
   for (;;) {
      if (matching_entry == last_operation && !first)
         break;
      first = false;

      if (matching_entry == mem_cache.end()) {
         //If there aren't any writes in the memory cache then
         // go ahead and search the whole thing, there aren't
         // going to be any consistency issues.
         if (have_writes) {
            break;
         }
         matching_entry = mem_cache.begin();
         continue;
      }

      if (**matching_entry == *me) {
         /*pthrd_printf("Memcache: Found existing operation: %s of %lx, size %lu\n",
           me->is_read ? "read" : "write", me->getAddress(), me->getSize());*/
         return getExistingOperation(matching_entry, me);
      }
      matching_entry++;
   }

   pthrd_printf("Async request not found in cache.  Triggering.\n");
   return doOperation(me, op_thread);
}

async_ret_t memCache::readMemoryAsync(void *dest, Address src, unsigned long size, 
                                      set<mem_response::ptr> &resps,
                                      int_thread *reading_thread)
{
   pending_async = false;
   Address start_block = src - (src % block_size);
   mcache_t::size_type cache_start_size = mem_cache.size();
   
   for (Address cur = start_block; cur < src+size; cur += block_size) {
      memEntry me(cur, NULL, block_size, true, this);
      async_ret_t result = lookupAsync(&me, reading_thread);
      if (result == aret_success) {
         char *target_mem;
         unsigned long copy_size;
         char *src_mem;

         if (cur == start_block)
            target_mem = (char *) dest;
         else
            target_mem = ((char *) dest) + (cur - src);

         Address me_start = me.addr;
         Address me_end = me.addr + block_size;
         Address src_start = src;
         Address src_end = src + size;

         //Compute intersection of i & curr
         Address intersect_start = me_start > src_start ? me_start : src_start;
         Address intersect_end = me_end < src_end ? me_end : src_end;
         assert(intersect_start <= intersect_end);
         
         copy_size = intersect_end - intersect_start;
         src_mem = me.buffer + (intersect_start - me_start);

         memcpy(target_mem, src_mem, copy_size);
      }
      else if (result == aret_error) {
         pthrd_printf("Error return from memCache::readMemAsync\n");
         return aret_error;
      }
      else if (result == aret_async) {
         pending_async = true;
      }
   }

   if (pending_async) {
      mcache_t::size_type cache_end_size = mem_cache.size();
      //assert(cache_start_size != cache_end_size);
      for (mcache_t::size_type i = cache_start_size; i != cache_end_size; i++) {
         assert(mem_cache[i]->resp);
         resps.insert(mem_cache[i]->resp);
      }
      return aret_async;
   }

   return aret_success;
}

async_ret_t memCache::readMemorySync(void *buffer, Address addr, unsigned long size,
                                     int_thread *reading_thread)
{
   if (size != 1) {
      mem_response::ptr memresult = mem_response::createMemResponse((char *) buffer, size);
      bool result = proc->readMem(addr, memresult, reading_thread);
      if (!result) {
         pthrd_printf("Failed to read memory for proc reader\n");
         return aret_error;
      }
      result = memresult->isReady();
      assert(result);
      return aret_success;
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
         return aret_error;
      }
      result = memresult->isReady();
      assert(result);
      word_cache_addr = aligned_addr;
      word_cache_valid = true;
   }
   *((char *) buffer) = ((char *) &word_cache)[addr - aligned_addr];
   return aret_success;
}

async_ret_t memCache::writeMemoryAsync(Dyninst::Address dest, void *src, unsigned long size, 
                                       std::set<result_response::ptr> &resps,
                                       int_thread *writing_thread)
{
   memEntry me(dest, (char *) src, size, false, this);
   pending_async = false;
   async_ret_t result = lookupAsync(&me, writing_thread);
   have_writes = true;
   if (result == aret_async) {
      mcache_t::iterator i = mem_cache.end();
      i--;
      assert((*i)->res_resp);
      resps.insert((*i)->res_resp);
      pending_async = true;
      
      bool found_token = false;
      for (i = mem_cache.begin(); i != mem_cache.end(); i++) {
         if ((*i)->token_type != token_none) {
            found_token = true;
            break;
         }
      }
      assert(found_token);
   }
   return result;
}

async_ret_t memCache::writeMemorySync(Dyninst::Address dest, void *src, unsigned long size,
                                      int_thread *write_thread)
{
   result_response::ptr resp = result_response::createResultResponse();
   bool result = proc->writeMem(src, dest, size, resp, write_thread);
   if (!result) {
      pthrd_printf("Error writing memory at %lx/%lu on %d\n",
                   dest, size, proc->getPid());
      return aret_error;
   }
   result = resp->isReady();
   assert(result);
   return aret_success;
}

async_ret_t memCache::readMemory(void *dest, Address src, unsigned long size, 
                                 set<mem_response::ptr> &resps, int_thread *thrd)
{
   // Normalize addresses
   if (proc->getAddressWidth() == 4) {
      src &= 0xffffffff;
   }

   if (proc->plat_needsAsyncIO()) {
      if (!block_size) 
         block_size = proc->plat_getRecommendedReadSize();
      return readMemoryAsync(dest, src, size, resps, thrd);
   }
   else
      return readMemorySync(dest, src, size, thrd);
}

async_ret_t memCache::writeMemory(Dyninst::Address dest, void *src, unsigned long size,
                                  std::set<result_response::ptr> &resps, int_thread *thrd)
{
   // Normalize addresses
   if (proc->getAddressWidth() == 4) {
      dest &= 0xffffffff;
   }

   if (proc->plat_needsAsyncIO()) {
      if (!block_size)
         block_size = proc->plat_getRecommendedReadSize();
      return writeMemoryAsync(dest, src, size, resps, thrd);
   }
   else
      return writeMemorySync(dest, src, size, thrd);
}

async_ret_t memCache::getRegisters(int_thread *thr, int_registerPool &pool)
{
   allreg_response::ptr resp;
   bool result = true;
   map<int_thread *, allreg_response::ptr>::iterator i = regs.find(thr);
   if (i == regs.end()) {
      int_registerPool *new_pool = new int_registerPool();
      resp = allreg_response::createAllRegResponse(new_pool);
      regs[thr] = resp;
      if (sync_handle) resp->markSyncHandled();
      result = thr->getAllRegisters(resp);
   }
   else {
      resp = i->second;
   }

   if (!result || resp->hasError()) {
      pending_async = false;
      return aret_error;
   }
   else if (!resp->isReady()) {
      pending_async = true;
      return aret_async;
   }
   pool = *resp->getRegPool();
   pending_async = false;
   return aret_success;
}

void memCache::clear()
{
   pthrd_printf("Clearing memCache\n");
   
   for (map<int_thread *, allreg_response::ptr>::iterator i = regs.begin(); i != regs.end(); i++) {
      delete i->second->getRegPool();
   }
   for (mcache_t::iterator j = mem_cache.begin(); j != mem_cache.end(); j++) {
      delete *j;
   }
   mem_cache.clear();
   regs.clear();

   last_operation = mem_cache.end();
   word_cache_valid = false;
   pending_async = false;
   have_writes = false;
}

bool memCache::hasPendingAsync() {
   return pending_async;
}
