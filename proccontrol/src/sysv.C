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
#include "common/h/SymReader.h"
#include "common/h/dyntypes.h"

#if defined(os_linux)
#include "common/src/linuxKludges.h"
#elif defined(os_freebsd)
#include "common/src/freebsdKludges.h"
#endif

#include "Handler.h"
#include "PlatFeatures.h"

#include "sysv.h"
#include "response.h"
#include "int_handler.h"

#include <algorithm>
#include <cstring>
#include <vector>
#include <string>
#include <set>
#include <iostream>

using namespace Dyninst;
using namespace std;

int_breakpoint *sysv_process::lib_trap = NULL;

sysv_process::sysv_process(Dyninst::PID p, string e, vector<string> a, vector<string> envp, map<int,int> f) :
   int_process(p, e, a, envp, f),
   int_libraryTracking(p, e, a, envp, f),
   breakpoint_addr(0),
   lib_initialized(false),
   procreader(NULL),
   aout(NULL),
   translator_(NULL),
   translator_state(NotReady)
{
   track_libraries = LibraryTracking::getDefaultTrackLibraries();
}

sysv_process::sysv_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p),
   int_libraryTracking(pid_, p)
{
   sysv_process *sp = dynamic_cast<sysv_process *>(p);
   breakpoint_addr = sp->breakpoint_addr;
   lib_initialized = sp->lib_initialized;
   track_libraries = sp->track_libraries;
   aout = new int_library(sp->aout);
   procreader = NULL;
   if (sp->procreader)
      procreader = new PCProcReader(this);
   translator_ = NULL;
   if (sp->translator_) {
     // Delay create because we can't use a
     // method in a class that inherits from us
     translator_state = Ready;
   }
   else
     translator_state = NotReady;
}

sysv_process::~sysv_process()
{
   deleteAddrTranslator();
   if (procreader) {
      delete procreader;
      procreader = NULL;
   }
}

AddressTranslate *sysv_process::constructTranslator(Dyninst::PID pid_)
{
   Address base;   
   bool result = plat_getInterpreterBase(base);
   if (result) {
      return AddressTranslate::createAddressTranslator(pid_, procreader,
                                                       getSymReader(),
                                                       INVALID_HANDLE_VALUE,
                                                       std::string(""), base);
   }
   else {
      return AddressTranslate::createAddressTranslator(pid_, procreader,
                                                       getSymReader());
   }
}

PCProcReader::PCProcReader(sysv_process *proc_) :
   proc(proc_)
{
}

PCProcReader::~PCProcReader()
{
   proc->getMemCache()->clear();
}

bool PCProcReader::start()
{
   return true;
}

bool PCProcReader::done()
{
   return true;
}

bool PCProcReader::ReadMem(Address addr, void *buffer, unsigned size)
{
   memCache *cache = proc->getMemCache();

   if (!proc->translator()) {
      //Can happen if we read during initialization.  We'll fail to read,
      // and the addrtranslate layer will handle things properly.
      return false;
   }

   proc->translator()->setReadAbort(false);
   async_ret_t ret = cache->readMemory(buffer, addr, size, memresults);
   switch (ret) {
      case aret_success:
         return true;
      case aret_async:
         proc->translator()->setReadAbort(true);
         return false;
      case aret_error:
         return false;
   }
   return true;
}

bool PCProcReader::GetReg(MachRegister /*reg*/, MachRegisterVal & /*val*/)
{
   assert(0); //Not needed
   return true;
}

bool PCProcReader::hasPendingAsync()
{
   return proc->getMemCache()->hasPendingAsync();
}

bool PCProcReader::getNewAsyncs(set<response::ptr> &resps)
{
   for (set<mem_response::ptr>::iterator i = memresults.begin(); i != memresults.end(); i++)
      resps.insert(*i);
   memresults.clear();
   return true;
}

bool sysv_process::initLibraryMechanism()
{
  if (lib_initialized) {
    if( translator() == NULL ) {
      createAddrTranslator();
      if (!translator() && procreader->isAsync()) {
         pthrd_printf("Waiting for async read to finish initializing\n");
         return false;
      }
      if (!translator()) {
         perr_printf("Error creating address translator object\n");
         return false;
      }
    }

      return true;
   }
   lib_initialized = true;

   pthrd_printf("Initializing library mechanism for process %d\n", getPid());

   if (!procreader)
      procreader = new PCProcReader(this);
   assert(procreader);


   assert(!translator_);
   createAddrTranslator();

   if (!translator() && procreader->isAsync()) {
      pthrd_printf("Waiting for async read to finish initializing\n");
      return false;
   }
   if (!translator()) {
      perr_printf("Error creating address translator object\n");
      return false;
   }

   if (!lib_trap) {
      lib_trap = new int_breakpoint(Breakpoint::ptr());
      lib_trap->setProcessStopper(true);
   }

   breakpoint_addr = translator()->getLibraryTrapAddrSysV();
   if (track_libraries) {
      pthrd_printf("Installing library breakpoint at %lx\n", breakpoint_addr);
      if (breakpoint_addr) {
         addBreakpoint(breakpoint_addr, lib_trap);
      }
   }

   return true;
}

bool sysv_process::refresh_libraries(set<int_library *> &added_libs,
                                     set<int_library *> &rmd_libs,
                                     bool &waiting_for_async,
                                     set<response::ptr> &async_responses)
{
   waiting_for_async = false;
   pthrd_printf("Refreshing list of loaded libraries\n");
   
   bool result = initLibraryMechanism();
   if (!result && procreader && procreader->hasPendingAsync()) {
      procreader->getNewAsyncs(async_responses);
      waiting_for_async = true;
      pthrd_printf("Waiting for async result to init libraries\n");
      return false;
   }
   if (!result) {
      pthrd_printf("refresh_libraries failed to init the libraries\n");
      return false;
   }

   assert(translator());
   result = translator()->refresh();
   if (!result && procreader->hasPendingAsync()) {
      procreader->getNewAsyncs(async_responses);
      waiting_for_async = true;
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
   translator()->getLibs(ll_libs);
   for (vector<LoadedLib *>::iterator i = ll_libs.begin(); i != ll_libs.end(); i++)
   {
      LoadedLib *ll = *i;
      int_library *lib = (int_library *) ll->getUpPtr();
      pthrd_printf("Found library %s at %lx\n", ll->getName().c_str(), 
                   ll->getCodeLoadAddr());
      if (!lib) {
         pthrd_printf("Creating new library object for %s\n", ll->getName().c_str());
         // Note: we set them all to "I'm a shared library"; the a.out is overridden below.

         lib = new int_library(ll->getName(), true, ll->getCodeLoadAddr(), ll->getDynamicAddr());
         lib->setMapAddress(ll->getMapAddr());
         assert(lib);
         added_libs.insert(lib);
         ll->setUpPtr((void *) lib);
         mem->addLibrary(lib);
      }
      if (!lib->mapAddress()) {
         lib->setMapAddress(ll->getMapAddr());
         lib->setLoadAddress(ll->getCodeLoadAddr());
         lib->setDynamicAddress(ll->getDynamicAddr());
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
      i++;
      rmd_libs.insert(lib);
      mem->rmLibrary(lib);
   }

   if (!aout) {
      LoadedLib *ll_aout = translator()->getExecutable();
      aout = (int_library *) (ll_aout ? ll_aout->getUpPtr() : NULL);
      if (aout)  {
         aout->markAOut();
      }
   }

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
   deleteAddrTranslator();
   if (procreader) {
      delete procreader;
      procreader = NULL;
   }
   breakpoint_addr = 0x0;
   lib_initialized = false;

   for (;;) {
      set<response::ptr> aresps;
      async_ret_t result = initializeAddressSpace(aresps);
      if (result == aret_async) {
         //Not doing performant async handling, as BG does not have exec.
         waitForAsyncEvent(aresps);
      }
      return (result == aret_success);
   }
}

bool sysv_process::plat_isStaticBinary()
{
  return (breakpoint_addr == 0);
}

int_library *sysv_process::plat_getExecutable()
{
   return aout;
}

bool sysv_process::addSysVHandlers(HandlerPool *) {
   //Matt: I deleted the SysV handler that was here, but
   // am leaving the hook in place if new ones ever come
   // along.
   return true;
}

bool sysv_process::plat_getInterpreterBase(Address &)
{
   return false;
}

bool sysv_process::setTrackLibraries(bool b, int_breakpoint* &bp, Address &addr, bool &add_bp)
{
   if (b == track_libraries) {
      bp = NULL;
      return true;
   }
   track_libraries = b;
   add_bp = track_libraries;
   bp = lib_trap;
   addr = breakpoint_addr;
   return true;
}

bool sysv_process::isTrackingLibraries()
{
   return track_libraries;
}

AddressTranslate *sysv_process::translator() {
  switch(translator_state) {
  case NotReady:
    return NULL;
  case Ready:
    translator_state = Creating;
    translator_ = constructTranslator(getPid());
    translator_state = Created;
    return translator_;
  case Creating:
    return NULL;
  case Created:
    return translator_;
  default:
    return NULL;
  }
}

void sysv_process::createAddrTranslator() {
  translator_state = Ready;
}

void sysv_process::deleteAddrTranslator() {
  translator_state = NotReady;
  if (translator_) delete translator_;
  translator_ = NULL;
}
