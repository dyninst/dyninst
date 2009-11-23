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

#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/walker.h"
#include <assert.h>

using namespace Dyninst;
using namespace Dyninst::Stackwalker;

SymbolLookup::SymbolLookup(Walker *w, const std::string &exec_path) :
   walker(w),
   executable_path(exec_path)
{
  sw_printf("[%s:%u] - Creating SymbolLookup %p with walker %p\n", 
	    __FILE__, __LINE__, this, walker);
  assert(walker);
}

SymbolLookup::~SymbolLookup() {
  walker = NULL;
  sw_printf("[%s:%u] - Destroying SymbolLookup %p\n", __FILE__, __LINE__);
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

#if defined(cap_stackwalker_use_symtab)
SymbolLookup *Walker::createDefaultSymLookup(const std::string &exec_name)
{
  return new SwkSymtab(this, exec_name);
}
#else
SymbolLookup *Walker::createDefaultSymLookup(const std::string &)
{
  return NULL;
}

SwkSymtab::SwkSymtab(Walker *w, const std::string &s) :
   SymbolLookup(w, s)
{
   assert(0);
}

bool SwkSymtab::lookupAtAddr(Dyninst::Address,
                             std::string &,
                             void* &)
{
   assert(0);
   return false;
}

#endif

