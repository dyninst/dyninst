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

class test_line_info_Mutator : public SymtabMutator {
public:
   test_line_info_Mutator() { };
   virtual test_results_t executeTest();
   test_results_t basic_verification();

   bool statement_ok(Statement *s)
   {
	   if (!s)
	   {
		   logerror( "%s[%d]:  NULL statement returned\n", FILE__, __LINE__);
		   return false;
	   }

	   if (!s->getFile().length())
	   {
		   logerror( "%s[%d]:  statement without file returned\n", FILE__, __LINE__);
		   return false;
	   }

	   if ( 0 == s->getLine())
	   {
		   //  a zero lineno may in fact be fine in the real world, but we don't really
		   //  expect to see it here.  (This check can probably be removed if necessary)
		   logerror( "%s[%d]:  statement with line = 0\n", FILE__, __LINE__);
		   return false;
	   }

	   //  check for -1L because that's used in the default statement ctor.
	   if ( (0 == s->startAddr()) || (-1L == s->startAddr()))
	   {
		   logerror( "%s[%d]:  statement with NULL startAddr: %s[%d]: %lu\n", 
				   FILE__, __LINE__, s->getFile().c_str(), s->getLine(), s->startAddr());
		   return false;
	   }
	   //  check for -1L because that's used in the default statement ctor.
	   if ( (0 == s->endAddr()) || (-1L == s->endAddr()))
	   {
		   logerror( "%s[%d]:  statement with NULL endAddr: %s[%d]: %lu\n", 
				   FILE__, __LINE__, s->getFile().c_str(), s->getLine(), s->endAddr());
		   return false;
	   }

	   if (s->endAddr() < s->startAddr())
	   {
		   logerror( "%s[%d]:  statement with endAddr (%lu) < startAddr (%lu)\n", 
				   FILE__, __LINE__, s->endAddr(), s->startAddr());
		   return false;
	   }

	   return true;
   }
};

extern "C" DLLEXPORT TestMutator* test_line_info_factory()
{
   return new test_line_info_Mutator();
}

test_results_t test_line_info_Mutator::basic_verification()
{
	std::vector<SymtabAPI::Module *> mods;
	bool result = symtab->getAllModules(mods);

	if (!result || !mods.size() )
	{
		logerror("%s[%d]: Unable to getAllModules\n", FILE__, __LINE__);
		return FAILED;
	}

	//  First run over all statements and verify that we return sane-ish results.
	
	for (unsigned int i = 0; i < mods.size(); ++i)
	{
		SymtabAPI::Module *mod = mods[i];
		if (!mod)
		{
			logerror("%s[%d]: Error: NULL module returned\n", FILE__, __LINE__);
			return FAILED;
		}

		std::string modname = mod->fileName();

		logerror("%s[%d]:  considering module %s\n", FILE__, __LINE__, modname.c_str());

		std::vector<Statement *> statements;

		if (!mod->getStatements(statements) || (0 == statements.size()))
		{
			//  We try to test all statements in all modules that have them,
			//  but not all do.  Thus we only fail here if the module that has
			//  no statements is one of ours.
			if ( (modname == "mutatee_util.c") 
					|| (modname == "solo_mutatee_boilerplate.c")
					|| (modname == "mutatee_driver.c"))
			{
				logerror( "%s[%d]:  getStatements failed for module %s\n", 
						FILE__, __LINE__, modname.c_str());
				return FAILED;
			}

			logerror("%s[%d]:  skipping module %s\n", FILE__, __LINE__, modname.c_str());
			continue;
		}
		for (unsigned int j = 0; j < statements.size(); ++j)
		{
			//  statement_ok does NULL check, so don't bother doing it here 

			if (!statement_ok(statements[j]))
			{
				logerror( "%s[%d]:  bad statement returned for module %s\n", 
						FILE__, __LINE__, modname.c_str());
				return FAILED;
			}
		}
	}

	return PASSED;
}

test_results_t test_line_info_Mutator::executeTest()
{

	if (createmode == DESERIALIZE)
		return SKIPPED;
#if defined (os_aix_test)
	//if (useAttach == DESERIALIZE)
		return SKIPPED;
#endif
	if (FAILED == basic_verification())
	{
		logerror( "%s[%d]:  failed basic verifications, skipping rest...\n", 
				FILE__, __LINE__);
		return FAILED;
	}

	std::vector<Function *> funcs;
	bool result = symtab->findFunctionsByName(funcs, std::string("test_line_info_func"));

	if (!result || !funcs.size() )
	{
		logerror("[%s:%u] - Unable to find test_line_info_func\n", 
				__FILE__, __LINE__);
		return FAILED;
	}

	if (funcs.size() != 1)
	{
		logerror("[%s:%u] - Too many functions found??: %d\n", 
				__FILE__, __LINE__, funcs.size());
		return FAILED;
	}

	Function *f = funcs[0];
	if (!f)
	{
		logerror("[%s:%u] - NULL function returned\n", 
				__FILE__, __LINE__);
		return FAILED;
	}

	std::vector<localVar *> params;
	std::vector<localVar *> local_vars;

    if (!f->getParams(params))
	{
		logerror( "%s[%d]:  failed to getParams()\n", FILE__, __LINE__);
		return FAILED;
	}

	if (params.size() != 1)
	{
		logerror( "%s[%d]:  bad number of params: %d, not 1\n", 
				FILE__, __LINE__, params.size());
		return FAILED;
	}

	int param_line_no = params[0]->getLineNum();
#if !defined(os_linux_test)
	//The rest of this test will only work on Linux (DWARF platforms).
	return PASSED;
#endif
	if ((compiler == std::string("pgcc")) 
			|| (compiler == std::string("pgCC")))
	{
		return PASSED;
	}

	//  we use the #line preprocessor directive in the mutatee to set the expected value
	if (param_line_no != 1000)
	{
		logerror( "%s[%d]:  param_line_no = %d not 1000\n", 
				FILE__, __LINE__, param_line_no);
		return FAILED;
	}

    if (!f->getLocalVariables(local_vars))
	{
		logerror( "%s[%d]:  failed to getLocalVariables()\n", FILE__, __LINE__);
		return FAILED;
	}

	if (local_vars.size() != 3)
	{
		logerror( "%s[%d]:  bad number of local_vars: %d, not 3\n", 
				FILE__, __LINE__, params.size());
		return FAILED;
	}

	//  we use the #line preprocessor directive in the mutatee to set the expected value
	int expected_lv_line_no = 2000;
	for (unsigned int i = 0; i < local_vars.size(); ++i)
	{
		int lv_line_no = local_vars[i]->getLineNum();
		if (lv_line_no != expected_lv_line_no)
		{
			logerror( "%s[%d]:  local var %d:  line# = %d, expected %d\n", 
					FILE__, __LINE__, i, lv_line_no, expected_lv_line_no);
			return FAILED;
		}
		expected_lv_line_no++;
	}


	return PASSED;
}

