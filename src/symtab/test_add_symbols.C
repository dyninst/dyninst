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

#include "symtab_comp.h"
#include "test_lib.h"

#include "Symtab.h"
#include "Symbol.h"
#include "Function.h"
#include "Variable.h"

using namespace Dyninst;
using namespace SymtabAPI;

class test_add_symbols_Mutator : public SymtabMutator {
public:
   test_add_symbols_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_add_symbols_factory()
{
   return new test_add_symbols_Mutator();
}

test_results_t test_add_symbols_Mutator::executeTest()
{
  // Test 1 of 2: add a name to a function
  // Test 2 of 2: directly add a symbol into the image
  
  // Test 1 of 2
  // Find the test function

  std::vector<Function *> funcs;
  bool result = symtab->findFunctionsByName(funcs, std::string("add_sym_func"));
  
  if (!result || !funcs.size() ) {
    logerror("[%s:%u] - Unable to find test_add_symbols\n", 
	     __FILE__, __LINE__);
    return FAILED;
  }
  
  if (funcs.size() != 1) {
    logerror("[%s:%u] - Too many functions found??: %d\n", 
	     __FILE__, __LINE__, funcs.size());
    return FAILED;
  }
  
  Function *f  = funcs[0];
  
  if (!f) {
    logerror("[%s:%u] - NULL function returned\n", 
	     __FILE__, __LINE__);
    return FAILED;
  }
  
  if (0 == f->getOffset()) {
    logerror("[%s:%u] - function with zero offset\n", 
	     __FILE__, __LINE__);
    return FAILED;
  }

  // Get the pre-added symbols
  std::vector<Symbol *> oldSyms;
  f->getSymbols(oldSyms);

  // That was long and involved. Now, add a name
  if (!f->addMangledName("add_sym_func_newname", false)) {
    logerror("[%s:%u]: failed to add name to function\n", __FILE__, __LINE__);
    return FAILED;
  }

  // Now wasn't that nice. 

  std::vector<Symbol *> syms;
  f->getSymbols(syms);
  if ((syms.size() != (oldSyms.size() + 1)) &&
      (syms.size() != (oldSyms.size() + 2))) {
    logerror("[%s:%u] - function has %d symbols, expected %d or %d\n",
	     __FILE__, __LINE__, syms.size(), oldSyms.size() + 1, oldSyms.size() + 2);
    for (unsigned i = 0; i < syms.size(); ++i) {
      logerror("\t %s (%p) (0x%lx) (%s)\n",
	       syms[i]->getMangledName().c_str(), 
	       syms[i], 
	       syms[i]->getOffset(),
	       syms[i]->isInDynSymtab() ? "<dyn>" : "<stat>");
    }
    return FAILED;
  }

  // Test 2 of 2: directly adding a symbol. We will copy it
  // off of add_sym_func to make life easy. 
  
  Symbol *newsym = new Symbol("add_sym_newsymbol",
			      Symbol::ST_FUNCTION,
			      Symbol::SL_GLOBAL,
			      Symbol::SV_DEFAULT,
			      syms[0]->getOffset(),
			      syms[0]->getModule(),
			      syms[0]->getRegion());
  if (!symtab->addSymbol(newsym)) {
    logerror("[%s:%u]: failed to add symbol\n", __FILE__, __LINE__);
    return FAILED;
  }

  // Test mechanism
  // We emit this as a new file, re-read it, and go
  // symbol hunting. 
  
  // 1) Create the directory and file
  if (mkdir("./binaries", 0755)) {
    if (errno != EEXIST) {
      logerror("[%s:%u]: failed to create ./binaries directory\n", __FILE__, __LINE__);
      return FAILED;
    }
  }

  // 2) Emit this into a tmpfile
  char *filename = strdup("./binaries/test_add_syms_XXXXXX");
  mkstemp(filename);
  if (!symtab->emit(filename)) {
    logerror("[%s:%u]: failed to emit test to %s\n", __FILE__, __LINE__, filename);
    return FAILED;
  }

  // 3) Reread it
  Symtab *newSymtab = NULL;
  if (!Symtab::openFile(newSymtab, filename)) {
    logerror("[%s:%u]: failed to open %s\n", __FILE__, __LINE__, filename);
    return FAILED;
  }

  // 4) And look for our added symbol...
  std::vector<Symbol *> lookupSyms;
  if (!newSymtab->findSymbol(lookupSyms, "add_sym_newsymbol")) {
    logerror("[%s:%u]: failed to find symbol add_sym_newsymbol\n", __FILE__, __LINE__);
    return FAILED;
  }
  if (lookupSyms.size() != 1) {
    logerror("[%s:%u]: got unexpected number of symbols matching add_sym_newsymbol: %d != 1\n", __FILE__, __LINE__, lookupSyms.size());
    return FAILED;
  }
  if (lookupSyms[0]->getOffset() != syms[0]->getOffset()) {
    logerror("[%s:%u]: added symbol offset 0x%lx not equal to expected 0x%lx\n", __FILE__, __LINE__,
	     lookupSyms[0]->getOffset(), syms[0]->getOffset());
    return FAILED;
  }

  // 5) And function name
  std::vector<Function *> lookupFunc;
  if (!newSymtab->findFunctionsByName(lookupFunc, "add_sym_func_newname")) {
    logerror("[%s:%u]: failed to find function named add_sym_func_newname\n", __FILE__, __LINE__);
    return FAILED;
  }

  if (lookupFunc.size() != 1) {
    logerror("[%s:%u]: unexpected number of functions named add_sym_func_newname: %d != 1\n", __FILE__, __LINE__, lookupFunc.size());
    return FAILED;
  }

  // Should check whether this is the right function...

  // 6) We're good. 
  unlink(filename);
  free(filename);

   return PASSED;
}

