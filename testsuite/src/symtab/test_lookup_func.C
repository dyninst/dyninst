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

class test_lookup_func_Mutator : public SymtabMutator {
public:
   test_lookup_func_Mutator() { };
   virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator* test_lookup_func_factory()
{
   return new test_lookup_func_Mutator();
}

test_results_t test_lookup_func_Mutator::executeTest()
{
	if (createmode == DESERIALIZE)
		return SKIPPED;
    std::vector<Symbol*> funcSyms;
    bool result = symtab->getAllSymbolsByType(funcSyms, Symbol::ST_FUNCTION);
    if(result == false)
    {
        logerror("getAllSymbolsByType returned false\n");
        return FAILED;
    }
    if(funcSyms.empty())
    {
        logerror("getAllSymbolsByType returned empty vector of function symbols\n");
        return FAILED;
    }
    logerror("getAllSymbolsByType found %d functions\n", funcSyms.size());
   std::vector<Function *> funcs;
   result = symtab->findFunctionsByName(funcs, std::string("lookup_func"));

   if (!result || !funcs.size() )
   {
      logerror("[%s:%u] - Unable to find test_lookup_func\n", 
               __FILE__, __LINE__);
      return FAILED;
   }

   if (funcs.size() != 1)
   {
      logerror("[%s:%u] - Too many functions found??: %d\n", 
               __FILE__, __LINE__, funcs.size());
      return FAILED;
   }

   Function *f  = funcs[0];

   if (!f)
   {
      logerror("[%s:%u] - NULL function returned\n", 
               __FILE__, __LINE__);
      return FAILED;
   }

   if (0 == f->getOffset())
   {
      logerror("[%s:%u] - function with zero offsetn", 
               __FILE__, __LINE__);
      return FAILED;
   }

   std::vector<localVar *> lvars;

   if (!f->getLocalVariables(lvars))
   {
      logerror("[%s:%u] - failed to find local vars\n", 
               __FILE__, __LINE__);
      return FAILED;
   }

   if (!lvars.size())
   {
      logerror("[%s:%u] - failed to find local vars\n", 
               __FILE__, __LINE__);
      return FAILED;
   }

   if (lvars.size() != 1)
   {
      logerror("[%s:%u] - wrong number oflocal vars: %d, not 1\n", 
               __FILE__, __LINE__, lvars.size());
      return FAILED;
   }

   localVar *lv = lvars[0];

   if (!lv)
   {
      logerror("[%s:%u] - NULL local var\n", 
               __FILE__, __LINE__);
      return FAILED;
   }

   if (lv->getName() != std::string("my_local_var"))
   {
      logerror("[%s:%u] - local vars has bad name: %s, not my_local_var\n", 
               __FILE__, __LINE__, lv->getName().c_str());
      return FAILED;
   }

   std::vector<localVar *> params;

   if (!f->getParams(params))
   {
      logerror("[%s:%u] - failed to find params\n", 
               __FILE__, __LINE__);
      return FAILED;
   }

   if (!params.size())
   {
      logerror("[%s:%u] - failed to find params\n", 
               __FILE__, __LINE__);
      return FAILED;
   }

   if (params.size() != 1)
   {
      logerror("[%s:%u] - wrong number of params: %d, not 1\n", 
               __FILE__, __LINE__, params.size());
      return FAILED;
   }

   localVar *param = params[0];

   if (!param)
   {
      logerror("[%s:%u] - NULL param\n", 
               __FILE__, __LINE__);
      return FAILED;
   }

   if (param->getName() != std::string("my_param"))
   {
      logerror("[%s:%u] - local vars has bad name: %s, not my_local_var\n", 
               __FILE__, __LINE__, param->getName().c_str());
      return FAILED;
   }


   return PASSED;
}

