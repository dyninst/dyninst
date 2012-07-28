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
#include "Aggregate.h"

using namespace Dyninst;
using namespace SymtabAPI;

class test_lookup_func_Mutator : public SymtabMutator {
public:
   test_lookup_func_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_lookup_var_factory()
{
   return new test_lookup_func_Mutator();
}

test_results_t test_lookup_func_Mutator::executeTest()
{
	if (createmode == DESERIALIZE)
		return SKIPPED;
   std::vector<Variable *> vars;
   bool result = symtab->findVariablesByName(vars, std::string("lookup_var"));

   if (!result || !vars.size())
   {
      logerror("[%s:%u] - Unable to find lookup_var\n", 
               __FILE__, __LINE__);
      return FAILED;
   }

   if (vars.size() != 1)
   {
      logerror("[%s:%u] - found too many (%d) lookup_var\n", 
               __FILE__, __LINE__, vars.size());
      return FAILED;
   }

   return PASSED;
}
