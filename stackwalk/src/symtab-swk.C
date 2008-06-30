/*
 * Copyright (c) 1996-2007 Barton P. Miller
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "stackwalk/h/symlookup.h"
#include "stackwalk/h/swk_errors.h"
#include "stackwalk/h/walker.h"
#include <assert.h>

#include "symtabAPI/h/Symtab.h"
#include "symtabAPI/h/Symbol.h"

using namespace Dyninst;
using namespace Dyninst::Stackwalker;
using namespace Dyninst::SymtabAPI;

bool SwkDynSymtab::lookupAtAddr(Address addr, std::string &out_name,
				void* &out_value)
{
  Address maps_load_addr;
  std::string libname;
  bool result;

  result = lookupLibrary(addr, maps_load_addr, libname);

  Symtab *symtab;
  result = Symtab::openFile(symtab, libname);
  if (!result) {
    //TODO: Error handling
    sw_printf("[%s:%u] - Couldn't open %s\n", libname.c_str());
    return false;
  }

  std::vector<Symbol *> syms;
  result = symtab->getAllSymbolsByType(syms, Symbol::ST_FUNCTION);
  if (!result) {
    sw_printf("[%s:%u] - Couldn't get symbols for %s\n", __FILE__, __LINE__, libname.c_str());
    return false;
  }

  Offset sym_load_addr = symtab->getLoadOffset();

  Symbol *candidate = NULL;
  unsigned long distance = 0;
  for (unsigned i = 0; i < syms.size(); i++)
  {
    Offset sym_addr = syms[i]->getAddr();
    if (!candidate || (addr - sym_addr < distance)) {
      distance = addr - sym_addr ;
      candidate = syms[i];
    }
    if (!candidate || (addr - (sym_addr + sym_load_addr) < distance)) {
      distance = addr - (sym_addr + sym_load_addr);
      candidate = syms[i];
    }
    if (!candidate || (addr - (sym_addr + maps_load_addr) < distance)) {
      distance = addr - (sym_addr + maps_load_addr);
      candidate = syms[i];
    }
  }

  out_name = candidate->getTypedName();
  if (!out_name.length())
    out_name = candidate->getPrettyName();
  if (!out_name.length())
    out_name = candidate->getName();
  out_value = (void *) candidate;

  sw_printf("[%s:%u] - Found name for %x : %s\n", __FILE__, __LINE__,
	    addr, out_name.c_str());  
  
  return true;
}

SwkDynSymtab::SwkDynSymtab(Walker *w, const std::string &exec_name)
  : SymbolLookup(w, exec_name)
{
}
