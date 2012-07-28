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
#include "Module.h"
#include "Symbol.h"

using namespace Dyninst;
using namespace SymtabAPI;

class test_module_Mutator : public SymtabMutator {
public:
   test_module_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_module_factory()
{
   return new test_module_Mutator();
}

bool malformed_module(SymtabAPI::Module *mod)
{
	if (!mod)
	{
      logerror("%s[%d]: malformed module: nonexistant module\n", FILE__, __LINE__);
	  return true;
	}

	std::string modname = mod->fileName();
	if (!modname.length())
	{
      logerror("%s[%d]: malformed module: bad file name\n", FILE__, __LINE__);
	  return true;
	}

	std::string modfullname = mod->fullName();
	if (!modfullname.length())
	{
      logerror("%s[%d]: malformed module: bad full name for %s\n", FILE__, __LINE__, modname.c_str());
	  return true;
	}

	Symtab *parent = mod->exec();
	if (!parent)
	{
      logerror("%s[%d]: malformed module: NULL parent for %s\n", FILE__, __LINE__, modname.c_str());
	  return true;
	}

	return false;
}

test_results_t test_module_Mutator::executeTest()
{
	if (createmode == DESERIALIZE)
		return SKIPPED;
   std::vector<SymtabAPI::Module *> mods;
   bool result = symtab->getAllModules(mods);

   if (!result || !mods.size() )
   {
      logerror("%s[%d]: Unable to getAllModules\n", FILE__, __LINE__);
      return FAILED;
   }

   for (unsigned int i = 0; i < mods.size(); ++i)
   {
	   SymtabAPI::Module *mod = mods[i];
	   if (!mod)
	   {
		   logerror("%s[%d]: Error: NULL module returned\n", FILE__, __LINE__);
		   return FAILED;
	   }

	   //  Check that we have built our lookup hashes correctly
	   //  (All modules are properly indexed by name and offset

	   std::string modname = mod->fileName();
	   std::string modfullname = mod->fullName();
	   Offset offset = mod->addr();

	   //fprintf(stderr, "%s[%d]:  considering module %s\n", FILE__, __LINE__, modname.c_str());

	   if (malformed_module(mod))
	   {
		   logerror("%s[%d]: bad module: %s\n", FILE__, __LINE__,
				   modname.c_str());
		   return FAILED;
	   }

	   SymtabAPI::Module *test_mod = NULL;
	   result = symtab->findModuleByName(test_mod, modname);

	   if (!result || !test_mod)
	   {
		   logerror("%s[%d]: Error: no module found with name %s\n", FILE__, __LINE__,
				   modname.c_str());
		   return FAILED;
	   }

	   test_mod = NULL;
	   result = symtab->findModuleByName(test_mod, modfullname);

	   if (!result || !test_mod)
	   {
		   logerror("%s[%d]: Error: no module found with name %s\n", FILE__, __LINE__,
				   modfullname.c_str());
		   return FAILED;
	   }

	   test_mod = NULL;
	   result = symtab->findModuleByOffset(test_mod, offset);

	   if (!result || !test_mod)
	   {
		   logerror("%s[%d]: Error: no module found with offset %lu\n", FILE__, __LINE__,
				   offset);
		   return FAILED;
	   }
   }

   return PASSED;
}

