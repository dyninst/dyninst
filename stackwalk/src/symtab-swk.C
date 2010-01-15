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

SymtabWrapper* SymtabWrapper::wrapper;
std::map<Symtab*, bool> DyninstInstrStepperImpl::isRewritten;

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

SymtabLibState::SymtabLibState(ProcessState *parent, std::string executable) : 
   LibraryState(parent),
   needs_update(true),
   lookup(NULL),
   procreader(parent, executable)
#if defined(os_linux)
   , vsyscall_mem(NULL)
   , vsyscall_symtab(NULL)
   , vsyscall_page_set(vsys_unset)
#endif
{
   PID pid = procstate->getProcessId();
   sw_printf("[%s:%u] - Creating a SymtabLibState on pid %d\n",
             __FILE__, __LINE__, pid);
   if (procstate->isFirstParty()) {
      lookup = AddressLookup::createAddressLookup(&procreader);
   }
   else {
      lookup = AddressLookup::createAddressLookup(pid, &procreader);
   }
   if (!lookup) {
      sw_printf("[%s:%u] - Creation of AddressLookup failed for ProcSelf "
                "on pid %d!\n", __FILE__, __LINE__, pid);
   }
   assert(lookup);
}

bool SymtabLibState::updateLibs()
{
   if (!needs_update)
      return true;
   needs_update = false;

   PID pid = procstate->getProcessId();
   bool result = lookup->refresh();
   if (!result) {
      sw_printf("[%s:%u] - Could not get load addresses out of SymtabAPI for %d."
                "This may happen during process create before libs have be set up\n",
                 __FILE__, __LINE__, pid);
      needs_update = true;
   }

   if (!updateLibsArch())
   {
      sw_printf("[%s:%u] - updateLibsArch failed\n",  __FILE__, __LINE__);
   }

   return true;
}

bool SymtabLibState::getLibraryAtAddr(Address addr, LibAddrPair &olib)
{
   bool result = refresh();
   if (!result) {
      sw_printf("[%s:%u] - Failed to refresh library.\n", __FILE__, __LINE__);
      setLastError(err_symtab, "Failed to refresh library list");
      return false;
   }
   
   Address load_addr;
   
   std::vector<std::pair<LibAddrPair, unsigned> >::iterator i;
   for (i = arch_libs.begin(); i != arch_libs.end(); i++) {
      load_addr = (*i).first.second;
      unsigned size = (*i).second;
      if ((addr >= load_addr) && (addr < load_addr + size)) {
         olib = (*i).first;
         return true;
      }
   }

   Symtab *symtab;
   Offset offset;
   result = lookup->getOffset(addr, symtab, offset);
   if (!result) {
      sw_printf("[%s:%u] - no file loaded at %x\n", __FILE__, __LINE__, offset);
      setLastError(err_nofile, "No file loaded at specified address");
      return false;
   }
   
   result = lookup->getLoadAddress(symtab, load_addr);
   if (!result) {
      setLastError(err_symtab, "Couldn't get load address for Symtab object");
      return false;
   }
   
   std::string name = symtab->file();
   olib.first = name;
   olib.second = load_addr;
   SymtabWrapper::notifyOfSymtab(symtab, name);
   
   return true;
}

bool SymtabLibState::getLibraries(std::vector<LibAddrPair> &olibs)
{
   bool result = refresh();
   if (!result) {
      setLastError(err_symtab, "Failed to refresh library list");
      return false;
   }
   
   std::vector<Symtab *> tabs;
   result = lookup->getAllSymtabs(tabs);
   if (!result) {
      setLastError(err_symtab, "No objects in process");
      return false;
   }
   
   for (unsigned i=0; i<tabs.size(); i++) {
      Address load_addr;
      result = lookup->getLoadAddress(tabs[i], load_addr);
      if (!result) {
         setLastError(err_symtab, "File has no load address");
         return false;
      }
      std::string name = tabs[i]->file();

      LibAddrPair olib;
      olib.first = name;
      olib.second = load_addr;
      SymtabWrapper::notifyOfSymtab(tabs[i], name);      
      olibs.push_back(olib);
   }

   std::vector<std::pair<LibAddrPair, unsigned> >::iterator i;
   for (i = arch_libs.begin(); i != arch_libs.end(); i++) {
      olibs.push_back((*i).first);
   }

   return true;
}

bool SymtabLibState::refresh()
{
   bool result = updateLibs();
   if (!result)
      return false;
   //TODO: Determine difference, notify steppergroup of diff
   return true;
}

Address SymtabLibState::getLibTrapAddress() {
   return lookup->getLibraryTrapAddrSysV();
}

SymtabLibState::~SymtabLibState() {
#if defined(os_linux)
   if (vsyscall_mem)
      free(vsyscall_mem);
#endif
}

void SymtabLibState::notifyOfUpdate() {
   //This may be called under a signal handler, keep simple
   needs_update = true;
}

LibAddrPair SymtabLibState::getAOut() {
   LoadedLibrary exec;
   updateLibs();
   bool result = lookup->getExecutable(exec);
   if (!result) {
      sw_printf("[%s:%u] - Error.  SymtabAPI getExecutable failed\n",
                __FILE__, __LINE__);
      return LibAddrPair(std::string(""), 0x0);
   }

   std::vector<LibAddrPair> libs;
   result = getLibraries(libs);
   if (!result) {
      sw_printf("[%s:%u] - Error.  getLibraries failed\n",
                __FILE__, __LINE__);
      return LibAddrPair(std::string(""), 0x0);
   }

   std::vector<LibAddrPair>::iterator i;
   for (i = libs.begin(); i != libs.end(); i++) {
      if ((*i).first == exec.name) {
         return *i;
      }
   }

   sw_printf("[%s:%u] - Could not find a.out in library list\n",
             __FILE__, __LINE__);
   return LibAddrPair(std::string(""), 0x0);
}

swkProcessReader::swkProcessReader(ProcessState *pstate, const std::string& executable) :
   ProcessReader(0, executable),
   procstate(pstate)
{
}

bool swkProcessReader::readAddressSpace(Address inTraced, unsigned amount,
                                        void *inSelf)
{
  pid = procstate->getProcessId();
  return procstate->readMem(inSelf, inTraced, amount);
}

swkProcessReader::~swkProcessReader()
{
}

bool swkProcessReader::start()
{
   return true;
}

bool swkProcessReader::done()
{
   return true;
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
