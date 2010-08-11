/*
 * Copyright (c) 1996-2009 Barton P. Miller
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
#include "common/h/SymLite-elf.h"
#include "sysv.h"

#include <vector>
#include <string>
#include <set>

using namespace Dyninst;
using namespace std;

SymbolReaderFactory *sysv_process::symreader_factory = NULL;
int_breakpoint *sysv_process::lib_trap = NULL;

sysv_process::sysv_process(Dyninst::PID p, string e, vector<string> a, map<int,int> f) :
   int_process(p, e, a, f),
   translator(NULL),
   lib_initialized(false),
   procreader(NULL)
{
}

sysv_process::sysv_process(Dyninst::PID pid_, int_process *p) :
   int_process(pid_, p)
{
   sysv_process *sp = static_cast<sysv_process *>(p);
   loaded_libs = sp->loaded_libs;
   breakpoint_addr = sp->breakpoint_addr;
   lib_initialized = sp->lib_initialized;
   if (sp->procreader)
      procreader = new PCProcReader(this);
   if (sp->translator)
      translator = AddressTranslate::createAddressTranslator(pid_,
                                                             procreader,
                                                             symreader_factory);
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

PCProcReader::PCProcReader(int_process *proc_) :
   proc(proc_)
{
}

PCProcReader::~PCProcReader()
{
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

bool PCProcReader::ReadMem(Address addr, void *buffer, unsigned size)
{
   if (size != 1) {
      bool result = proc->readMem(buffer, addr, size);
      if (!result) {
         pthrd_printf("Failed to read memory for proc reader\n");
      }
      return result;
   }

   //Try to optimially handle a case where the calling code
   // reads a string one char at a time.
   assert(size == 1);
   Address aligned_addr = addr - (addr % sizeof(word_cache));
   if (!word_cache_valid || aligned_addr != word_cache_addr) {
      bool result = proc->readMem(&word_cache, aligned_addr, sizeof(word_cache));
      if (!result) {
         pthrd_printf("Failed to read memory for proc reader\n");
         return false;
      }
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
   assert(!procreader);
   procreader = new PCProcReader(this);
   symreader_factory = getDefaultSymbolReader();

   assert(!translator);
   translator = AddressTranslate::createAddressTranslator(getPid(), 
                                                          procreader,
                                                          symreader_factory);
   if (!translator) {
      perr_printf("Error creating address translator object\n");
      return false;
   }

   if (!lib_trap) {
      lib_trap = new int_breakpoint(Breakpoint::ptr());
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
                                     set<int_library *> &rmd_libs)
{
   pthrd_printf("Refreshing list of loaded libraries\n");
   bool result = initLibraryMechanism();
   if (!result) {
      pthrd_printf("refresh_libraries failed to init the libraries\n");
      return false;
   }

   result = translator->refresh();
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
         lib = new int_library(ll->getName(), ll->getCodeLoadAddr());
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

   return true;
}

Dyninst::Address sysv_process::getLibBreakpointAddr() const
{
   return breakpoint_addr;
}

bool sysv_process::plat_execed()
{
   pthrd_printf("Rebuilding library trap mechanism after exec on %d\n", getPid());
   if (translator) {
      delete translator;
      translator = NULL;
   }
   if (procreader) {
      delete procreader;
      procreader = NULL;
   }
   breakpoint_addr = 0x0;
   loaded_libs.clear();
   lib_initialized = false;
   return initLibraryMechanism();
}

SymbolReaderFactory *Dyninst::ProcControlAPI::getDefaultSymbolReader()
{
   static SymbolReaderFactory *symreader_factory = NULL;
   if (!symreader_factory) {
      symreader_factory = (SymbolReaderFactory *) new SymElfFactory();
      assert(symreader_factory);
   }
   return symreader_factory;
}
