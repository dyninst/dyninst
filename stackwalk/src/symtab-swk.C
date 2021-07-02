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


#if defined(WITH_SYMTAB_API)

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
  
  sw_printf("[%s:%d] - Trying to open symtab object %s\n", 
            FILE__, __LINE__, filename.c_str());
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

#endif
