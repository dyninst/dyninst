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

#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/procstate.h"
#include "common/h/SymReader.h"

#include "stackwalk/src/libstate.h"

#include <assert.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

SymbolLookup::SymbolLookup(std::string exec_path) :
   walker(NULL),
   executable_path(exec_path)
{
  sw_printf("[%s:%d] - Creating SymbolLookup %p\n", 
	    FILE__, __LINE__, (void*)this);
}

SymbolLookup::~SymbolLookup() {
  walker = NULL;
  sw_printf("[%s:%d] - Destroying SymbolLookup %p\n", FILE__, __LINE__, (void*)this);
}

Walker *SymbolLookup::getWalker()
{
  assert(walker);
  return walker;
}
 
ProcessState *SymbolLookup::getProcessState()
{
  return walker->getProcessState();
}

SymbolLookup *Walker::createDefaultSymLookup(std::string exec_name)
{
   return new SymDefaultLookup(exec_name);
}

SwkSymtab::SwkSymtab(std::string s) :
   SymbolLookup(s)
{
   assert(0);
}

bool SwkSymtab::lookupAtAddr(Dyninst::Address,
                             std::string &,
                             void* &)
{
   sw_printf("[%s:%d] - Error: Called root symbol lookup\n", FILE__, __LINE__);
   assert(0);
   return false;
}

SwkSymtab::~SwkSymtab()
{
}

SymDefaultLookup::SymDefaultLookup(std::string s) :
   SymbolLookup(s)
{
}

bool SymDefaultLookup::lookupAtAddr(Dyninst::Address addr, 
                                    std::string &out_name, 
                                    void* &out_value)
{
   LibraryState *ls = walker->getProcessState()->getLibraryTracker();
   LibAddrPair lib;
   bool result = ls->getLibraryAtAddr(addr, lib);
   if (!result) {
      sw_printf("[%s:%d] - Failed to find a library at %lx for lookup\n",
                FILE__, __LINE__, addr);
      return false;
   }

   SymReader *reader = LibraryWrapper::getLibrary(lib.first);
   if (!reader) {
      sw_printf("[%s:%d] - Failed to open a symbol reader for %s\n", 
                FILE__, __LINE__, lib.first.c_str());
      return false;
   }

   Offset off = addr - lib.second;
   Symbol_t sym = reader->getContainingSymbol(off);
   if (!reader->isValidSymbol(sym)) {
      sw_printf("[%s:%d] - Could not find symbol in binary\n", FILE__, __LINE__);
      return false;
   }

   out_name = reader->getDemangledName(sym);
   out_value = NULL;
   sw_printf("[%s:%d] - Found symbol %s at address %lx\n", FILE__, __LINE__, out_name.c_str(), addr);
   return true;
}

SymDefaultLookup::~SymDefaultLookup()
{
}
