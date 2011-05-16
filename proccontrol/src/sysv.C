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
#include "dynutil/h/SymReader.h"
#include "dynutil/h/dyntypes.h"
#include "sysv.h"
#include "irpc.h"

#if defined(os_linux)
#include "common/h/linuxKludges.h"
#elif defined(os_freebsd)
#include "common/h/freebsdKludges.h"
#endif

#include "proccontrol/src/response.h"

#include <cstring>
#include <vector>
#include <string>
#include <set>

using namespace Dyninst;
using namespace std;

int_breakpoint *sysv_process::lib_trap = NULL;

sysv_process::sysv_process(Dyninst::PID p, string e, vector<string> a, vector<string> envp, map<int,int> f) :
   int_process(p, e, a, envp, f),
   translator(NULL),
   lib_initialized(false),
   procreader(NULL),
   aout(NULL)
{
}

sysv_process::sysv_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p)
{
   sysv_process *sp = dynamic_cast<sysv_process *>(p);
   breakpoint_addr = sp->breakpoint_addr;
   lib_initialized = sp->lib_initialized;
   aout = NULL;
   if (sp->procreader)
      procreader = new PCProcReader(this);
   if (sp->translator)
      translator = AddressTranslate::createAddressTranslator(pid_,
                                                             procreader,
                                                             sp->plat_defaultSymReader());
}

sysv_process::~sysv_process()
{
   if (translator) {
      delete translator;
      translator = NULL;
   }
   if (procreader) {
      delete procreader;
      procreader = NULL;
   }
}

PCProcReader::PCProcReader(sysv_process *proc_) :
   proc(proc_),
   pending_addr(0)
{
}

PCProcReader::~PCProcReader()
{
   clearBuffers();
}

bool PCProcReader::start()
{
   word_cache_valid = false;
   return true;
}

bool PCProcReader::done()
{
   word_cache_valid = true;
   return true;
}

bool PCProcReader::isAsync()
{
   return (memresult != NULL);
}

bool PCProcReader::handleAsyncCompletion()
{
   if (!memresult->isReady()) {
      //We still have to wait for the read to finish.
      pthrd_printf("ProcReader waiting for async read at %lx\n", pending_addr);
      return false;
   }

   if (memresult->hasError()) {
      //The read failed, mark this with a NULL buffer entry at the address
      pthrd_printf("Async read at %lx failed, marking as such\n", pending_addr);
      free(memresult->getBuffer());
      async_read_buffers[pending_addr] = NULL;
   }
   else {
      pthrd_printf("ProcReader found completed async read at %lx\n", pending_addr);
      async_read_buffers[pending_addr] = memresult->getBuffer();
   }
   memresult = mem_response::ptr();
   pending_addr = 0;
   
   return true;
}

void PCProcReader::clearBuffers()
{
   map<Address, char *>::iterator i;
   for (i = async_read_buffers.begin(); i != async_read_buffers.end(); i++) {
      if (i->second)
         free(i->second);
   }
   async_read_buffers.clear();
}

bool PCProcReader::postAsyncRead(Dyninst::Address addr)
{
   //Reading from an unknown page, post an async read and
   // return 'false'.  isAsync() will latter return true,
   // letting the caller know that this was an async error
   // rather than a real one.
   assert(!memresult);
   char *new_buffer = (char *) malloc(async_read_align);
   memresult = mem_response::createMemResponse(new_buffer, async_read_align);
   pending_addr = addr;

   assert(memresult);
   bool result = proc->readMem(addr, memresult);

   if (!result) {
      pthrd_printf("Failure to async read process %d memory at %lx\n",
                   proc->getPid(), addr);
      memresult = mem_response::ptr();
      return false;
   }
   return handleAsyncCompletion();
}

bool PCProcReader::ReadMemAsync(Address addr, void *buffer, unsigned size)
{
   //If we're on an Async IO platform (BlueGene) then we may do
   // this read as multiple async reads, caching memory as we go.
   // Once we've completed all async reads, then we'll finally 
   // return success from this function.
   //
   //The page aligned caching may seem weird (and could produce
   // unnecessary extra reads), but I' think it'll provide a better
   // access pattern in most cases.
   Dyninst::Address aligned_addr = addr & ~(async_read_align-1);
   int buffer_offset = 0;

   if (!proc->translator) {
      //Can happen if we read during initialization.  We'll fail to read,
      // and the addrtranslate layer will handle things properly.
      return false;
   }

   Dyninst::Address cur_addr = aligned_addr;
   for (; cur_addr < addr+size; cur_addr += async_read_align) {
      map<Address, char *>::iterator i = async_read_buffers.find(cur_addr);
      if (i == async_read_buffers.end()) {
         bool result = postAsyncRead(cur_addr);
         if (!result) {
            //Need to wait for the async read to complete
            pthrd_printf("Returning async from read\n");
            proc->translator->setReadAbort(true);
            return false;
         }
         i = async_read_buffers.find(cur_addr);
         assert(i != async_read_buffers.end());
      }
      //We have a buffer cached, copy the correct parts of
      // its contents to the target buffer.
      char *cached_buffer = i->second;
      if (!cached_buffer) {
         //This read had resulted in an error.  Return as such.
         pthrd_printf("Returning error from read\n");
         proc->translator->setReadAbort(false);
         return false;
      }
      int start_offset = (addr > cur_addr) ? (addr - cur_addr) : 0;
      int end_offset = (addr+size < cur_addr+async_read_align) ? 
         addr+size - cur_addr : async_read_align;
      memcpy(((char *) buffer) + buffer_offset, 
             cached_buffer + start_offset, 
             end_offset - start_offset);
   }
   //All reads completed succfully.  Phew.
   pthrd_printf("Returning success from read\n");
   static_cast<sysv_process *>(proc)->translator->setReadAbort(false);
   return true;
}

bool PCProcReader::ReadMem(Address addr, void *buffer, unsigned size)
{
   if (proc->plat_needsAsyncIO()) {
      return ReadMemAsync(addr, buffer, size);
   }

   if (size != 1) {
      mem_response::ptr memresult = mem_response::createMemResponse((char *) buffer, size);
      bool result = proc->readMem(addr, memresult);
      if (!result) {
         pthrd_printf("Failed to read memory for proc reader\n");
         return false;
      }
      result = memresult->isReady();
      assert(result);
      return true;
   }

   //Try to optimially handle a case where the calling code
   // reads a string one char at a time.  This is mostly for
   // ptrace platforms, but won't harm any others.
   assert(size == 1);
   Address aligned_addr = addr - (addr % sizeof(word_cache));
   if (!word_cache_valid || aligned_addr != word_cache_addr) {
      mem_response::ptr memresult = mem_response::createMemResponse((char *) &word_cache, sizeof(word_cache));
      bool result = proc->readMem(aligned_addr, memresult);
      if (!result) {
         pthrd_printf("Failed to read memory for proc reader\n");
         return false;
      }
      result = memresult->isReady();
      assert(result);
      word_cache_addr = aligned_addr;
      word_cache_valid = true;
   }
   *((char *) buffer) = ((char *) &word_cache)[addr - aligned_addr];
   return true;
}

bool PCProcReader::GetReg(MachRegister /*reg*/, MachRegisterVal & /*val*/)
{
   assert(0); //Not needed
   return true;
}

bool sysv_process::initLibraryMechanism()
{
   if (lib_initialized) {
      return true;
   }
   lib_initialized = true;

   pthrd_printf("Initializing library mechanism for process %d\n", getPid());

   if (!procreader)
      procreader = new PCProcReader(this);
   assert(procreader);

   assert(!translator);
   translator = AddressTranslate::createAddressTranslator(getPid(), 
                                                          procreader,
                                                          plat_defaultSymReader());
   if (!translator && procreader->isAsync()) {
      pthrd_printf("Waiting for async read to finish initializing\n");
      return false;
   }
   if (!translator) {
      perr_printf("Error creating address translator object\n");
      return false;
   }

   if (!lib_trap) {
      lib_trap = new int_breakpoint(Breakpoint::ptr());
      lib_trap->setProcessStopper(true);
   }

   breakpoint_addr = translator->getLibraryTrapAddrSysV();
   pthrd_printf("Installing library breakpoint at %lx\n", breakpoint_addr);
   bool result = false;
   if (breakpoint_addr) {
      result = addBreakpoint(breakpoint_addr, lib_trap);
   }

   return true;
}

bool sysv_process::refresh_libraries(set<int_library *> &added_libs,
                                     set<int_library *> &rmd_libs,
                                     set<response::ptr> &async_responses)
{
   pthrd_printf("Refreshing list of loaded libraries\n");

   if (procreader && procreader->isAsync()) {
      //We (may) have a posted async completion.  Handle it.
      bool result = procreader->handleAsyncCompletion();
      if (!result) {
         return false;
      }
   }
   assert(!procreader || !procreader->isAsync());

   bool result = initLibraryMechanism();
   if (!result && procreader && procreader->isAsync()) {
      async_responses.insert(procreader->memresult);
      pthrd_printf("Waiting for async result to init libraries\n");
      return false;
   }
   if (!result) {
      pthrd_printf("refresh_libraries failed to init the libraries\n");
      return false;
   }

   result = translator->refresh();
   if (!result && procreader->isAsync()) {
      async_responses.insert(procreader->memresult);
      pthrd_printf("Waiting for async result to read libraries\n");
      return false;
   }
   if (!result) {
      pthrd_printf("Failed to refresh library list for %d\n", getPid());
   }

   for (set<int_library *>::iterator i = mem->libs.begin(); 
        i != mem->libs.end(); i++) 
   {
      (*i)->setMark(false);
   }

   vector<LoadedLib *> ll_libs;
   translator->getLibs(ll_libs);
   for (vector<LoadedLib *>::iterator i = ll_libs.begin(); i != ll_libs.end(); i++)
   {
      LoadedLib *ll = *i;
      int_library *lib = (int_library *) ll->getUpPtr();
      pthrd_printf("Found library %s at %lx\n", ll->getName().c_str(), 
                   ll->getCodeLoadAddr());
      if (!lib) {
         pthrd_printf("Creating new library object for %s\n", ll->getName().c_str());
         lib = new int_library(ll->getName(), ll->getCodeLoadAddr(), ll->getDynamicAddr());
         assert(lib);
         added_libs.insert(lib);
         ll->setUpPtr((void *) lib);
         mem->libs.insert(lib);
      }
      lib->setMark(true);
   }

   set<int_library *>::iterator i = mem->libs.begin();
   while (i != mem->libs.end()) {
      int_library *lib = *i;
      if (lib->isMarked()) {
         i++;
         continue;
      }
      pthrd_printf("Didn't find old library %s at %lx, unloading\n",
                   lib->getName().c_str(), lib->getAddr());
      rmd_libs.insert(lib);
      mem->libs.erase(i++);
   }

   procreader->clearBuffers();
   return true;
}

Dyninst::Address sysv_process::getLibBreakpointAddr() const
{
   return breakpoint_addr;
}

bool sysv_process::plat_execed()
{
   pthrd_printf("Rebuilding library trap mechanism after exec on %d\n", getPid());
   if (aout) {
      // aout has already been deleted in the forking process
      aout = NULL;
   }
   if (translator) {
      delete translator;
      translator = NULL;
   }
   if (procreader) {
      delete procreader;
      procreader = NULL;
   }
   breakpoint_addr = 0x0;
   lib_initialized = false;

   bool result = initLibraryMechanism();
   if (!result) {
      pthrd_printf("Error initializing library mechanism\n");
      return false;
   }

   std::set<int_library*> added, rmd;
   for (;;) {
      std::set<response::ptr> async_responses;
      bool result = refresh_libraries(added, rmd, async_responses);
      if (!result && !async_responses.empty()) {
         result = waitForAsyncEvent(async_responses);
         if (!result) {
            pthrd_printf("Failure waiting for async completion\n");
            return false;
         }
         continue;
      }
      if (!result) {
         pthrd_printf("Failure refreshing libraries for %d\n", getPid());
         return false;
      }
      return true;
   }
}

bool sysv_process::plat_isStaticBinary()
{
  return (breakpoint_addr == 0);
}

int_library *sysv_process::getExecutableLib()
{
   if (aout)
      return aout;

   LoadedLib *ll = translator->getExecutable();
   aout = (int_library *) ll->getUpPtr();
   if (aout)
      return aout;

   aout = new int_library(ll->getName(), ll->getCodeLoadAddr(), ll->getDynamicAddr());
   ll->setUpPtr((void *) aout);

   return aout;
}
