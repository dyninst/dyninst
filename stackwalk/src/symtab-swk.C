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


#if defined(cap_stackwalker_use_symtab)

#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/walker.h"
#include "stackwalk/h/frame.h"
#include <assert.h>

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Symbol.h"
#include "symtabAPI/h/AddrLookup.h"
#include "stackwalk/src/symtab-swk.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;
using namespace Dyninst::SymtabAPI;
using namespace std;

SymtabWrapper* SymtabWrapper::wrapper;

SymtabWrapper::SymtabWrapper()
{
}

Symtab *SymtabWrapper::getSymtab(std::string filename)
{
  if (!wrapper) {
     //TODO: Thread safety
     wrapper = new SymtabWrapper();
  }
  
  if (wrapper->map.count(filename)) {
     return wrapper->map[filename];
  }
  
  sw_printf("[%s:%u] - Trying to open symtab object %s\n", 
            __FILE__, __LINE__, filename.c_str());
  Symtab *symtab;
  bool result = Symtab::openFile(symtab, filename);
  if (!result) {
     setLastError(err_nofile, "Couldn't open file through SymtabAPI\n");
     return NULL;
  }

  wrapper->map[filename] = symtab;
  return symtab;
}

SymtabWrapper::~SymtabWrapper()
{
   dyn_hash_map<std::string, Symtab *>::iterator i = map.begin();
   for (; i != map.end(); i++)
   {
      Symtab *symtab = (*i).second;
      delete symtab;
   }
   
   wrapper = NULL;
}

void SymtabWrapper::notifyOfSymtab(Symtab *symtab, std::string name)
{
  if (!wrapper) {
     //TODO: Thread safety
     wrapper = new SymtabWrapper();
  }
  
  if (wrapper->map.count(name)) {
     return;
  }

  wrapper->map[name] = symtab;
}

bool SwkSymtab::lookupAtAddr(Address addr, std::string &out_name,
				void* &out_value)
{
  Address load_addr;
  std::string libname;
  bool result;

  LibraryState *libtracker = walker->getProcessState()->getLibraryTracker();
  if (!libtracker) {
     sw_printf("[%s:%u] - getLibraryTracker() failed\n", __FILE__, __LINE__);
     setLastError(err_nolibtracker, "No library tracker object registered");
     return false;
  }

  LibAddrPair lib;
  result = libtracker->getLibraryAtAddr(addr, lib);
  if (!result) {
     sw_printf("[%s:%u] - getLibraryAtAddr() failed: %s\n", __FILE__, __LINE__, getLastErrorMsg());
    return false;
  }

  Symtab *symtab = SymtabWrapper::getSymtab(lib.first);
  assert(symtab);
  load_addr = lib.second;

  //TODO: Cache symbol vector and use binary search
  std::vector<Symbol *> syms;
  result = symtab->getAllSymbolsByType(syms, Symbol::ST_FUNCTION);
  if (!result) {
    sw_printf("[%s:%u] - Couldn't get symbols for %s\n", 
              __FILE__, __LINE__, libname.c_str());
    return false;
  }
  Symbol *candidate = NULL;
  unsigned long distance = 0;
  for (unsigned i = 0; i < syms.size(); i++)
  {
    unsigned long cur_distance = (addr - load_addr) - syms[i]->getAddr();
    if (!candidate || cur_distance < distance) {
      distance = cur_distance;
      candidate = syms[i];
    }
  }

  out_name = candidate->getTypedName();
  if (!out_name.length())
    out_name = candidate->getPrettyName();
  if (!out_name.length())
    out_name = candidate->getName();
  out_value = (void *) candidate;

  sw_printf("[%s:%u] - Found name for %lx : %s\n", __FILE__, __LINE__,
	    addr, out_name.c_str());  
  
  return true;
}

SwkSymtab::SwkSymtab(Walker *w, std::string exec_name) :
   SymbolLookup(w, exec_name)
{
}

#endif
