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

SwkSymtab::SwkSymtab(Walker *w, const std::string &exec_name) :
   SymbolLookup(w, exec_name)
{
}

#if defined(arch_x86) || defined(arch_x86_64)
std::map<Symtab*, bool> DyninstInstrStepperImpl::isRewritten;

DyninstInstrStepperImpl::DyninstInstrStepperImpl(Walker *w, FrameStepper *p) :
  FrameStepper(w),
  parent(p)
{
}

gcframe_ret_t DyninstInstrStepperImpl::getCallerFrame(const Frame &in, Frame &out)
{
   LibAddrPair lib;
   bool result;

   result = getProcessState()->getLibraryTracker()->getLibraryAtAddr(in.getRA(), lib);
   if (!result) {
      sw_printf("[%s:%u] - Stackwalking through an invalid PC at %lx\n",
                 __FILE__, __LINE__, in.getRA());
      return gcf_stackbottom;
   }

   Symtab *symtab = SymtabWrapper::getSymtab(lib.first);
   if (!symtab) {
      sw_printf("[%s:%u] - Could not open file %s with SymtabAPI %s\n",
                 __FILE__, __LINE__, lib.first.c_str());
      setLastError(err_nofile, "Could not open file for Debugging stackwalker\n");
      return gcf_error;
   }

   std::map<Symtab*, bool>::iterator i = isRewritten.find(symtab);
   bool is_rewritten_binary;
   if (i == isRewritten.end()) {
     Region *r = NULL;
     result = symtab->findRegion(r, std::string(".dyninstInst"));
     is_rewritten_binary = (r && result);
     isRewritten[symtab] = is_rewritten_binary;
   }
   else {
     is_rewritten_binary = (*i).second;
   }
   if (!is_rewritten_binary) {
     sw_printf("[%s:u] - Decided that current binary is not rewritten, "
	       "DyninstInstrStepper returning gcf_not_me at %lx\n",
	       __FILE__, __LINE__, in.getRA());
     return gcf_not_me;
   }

   std::string name;
   in.getName(name);
   const char *s = name.c_str();
   if (strstr(s, "dyninst") != s)
   {
     sw_printf("[%s:%u] - Current function %s not dyninst generated\n",
		__FILE__, __LINE__, s);
     return gcf_not_me;
   }

   if (strstr(s, "dyninstBT") != s)
   {
     sw_printf("[%s:%u] - Dyninst, but don't know how to read non-tramp %s\n",
		__FILE__, __LINE__, s);
     return gcf_not_me;
   }
    
   sw_printf("[%s:%u] - Current function %s is baseTramp\n",
	      __FILE__, __LINE__, s);
   Address base;
   unsigned size, stack_height;
   int num_read = sscanf(s, "dyninstBT_%lx_%u_%x", &base, &size, &stack_height);
   bool has_stack_frame = (num_read == 3);
   if (!has_stack_frame) {
     sw_printf("[%s:%u] - Don't know how to walk through instrumentation without a stack frame\n"
		__FILE__, __LINE__);
     return gcf_not_me;
   }
     
   return getCallerFrameArch(in, out, base, lib.second, size, stack_height);
}

unsigned DyninstInstrStepperImpl::getPriority() const
{
  return dyninstr_priority;
}

void DyninstInstrStepperImpl::registerStepperGroup(StepperGroup *group)
{
  FrameStepper::registerStepperGroup(group);
}

DyninstInstrStepperImpl::~DyninstInstrStepperImpl()
{
}
#endif

#endif
