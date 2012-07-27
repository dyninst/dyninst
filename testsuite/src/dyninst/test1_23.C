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

// $Id: test1_23.C,v 1.1 2008/10/30 19:18:27 legendre Exp $
/*
 * #Name: test1_23
 * #Desc: Local Variables
 * #Dep: !mips_sgi_irix6_4_test
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_point.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test1_23_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_23_factory() 
{
	return new test1_23_Mutator();
}

//
// Start Test Case #23 - local variables
//

test_results_t test1_23_Mutator::executeTest() 
{
	const char *funcName = "test1_23_call1";
	BPatch_Vector<BPatch_function *> found_funcs;

	if ((NULL == appImage->findFunction(funcName, found_funcs, 1)) 
			|| !found_funcs.size()) 
	{
		logerror("    Unable to find function %s\n", funcName);
		return FAILED;
	}

	if (1 < found_funcs.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), funcName);
	}

	BPatch_Vector<BPatch_point *> *point23_calls = found_funcs[0]->findPoint(BPatch_subroutine);    

	if (!point23_calls || (point23_calls->size() < 1)) 
	{
		logerror("**Failed** test #23 (local variables)\n");
		logerror("  Unable to find point %s - subroutine calls\n", funcName);
		return FAILED;
	}

	/* We only want the first one... */
	BPatch_Vector<BPatch_point *> point23_1;
	point23_1.push_back((*point23_calls)[0]);

	BPatch_variableExpr *var1 = appImage->findVariable(*(point23_1[0]),
			"localVariable23_1");
	BPatch_variableExpr *var2 = appImage->findVariable(*(point23_1[0]),
			"test1_23_shadowVariable1");
	BPatch_variableExpr *var3 = appImage->findVariable("test1_23_shadowVariable2");
	BPatch_variableExpr *var4 = appImage->findVariable("test1_23_globalVariable1");

	if (!var1 || !var2 || !var3 || !var4) 
	{
		logerror("**Failed** test #23 (local variables)\n");

		if (!var1)
		{
			logerror("  can't find local variable localVariable23_1\n");
			BPatch_function *f = point23_1[0]->getCalledFunction();
			assert(f);
			BPatch_Vector<BPatch_localVar *> *lvars = f->getVars();
			assert(lvars);
			logerror("%s[%d]:  have vars\n", FILE__, __LINE__);
			for (unsigned int i = 0; i < lvars->size(); ++i)
			{
				logerror("\t%s\n", (*lvars)[i]->getName());
			}
		}
		if (!var2)
			logerror("  can't find local variable test1_23_shadowVariable1\n");
		if (!var3)
			logerror("  can't find global variable test1_23_shadowVariable2\n");
		return FAILED;
	}

	BPatch_arithExpr expr23_1(BPatch_assign, *var1, BPatch_constExpr(2300001));
	BPatch_arithExpr expr23_2(BPatch_assign, *var2, BPatch_constExpr(2300012));
	BPatch_arithExpr expr23_3(BPatch_assign, *var3, BPatch_constExpr(2300023));
	BPatch_arithExpr expr23_4(BPatch_assign, *var4, *var1);

	BPatch_Vector<BPatch_snippet *> exprs;

	exprs.push_back(&expr23_1);
	exprs.push_back(&expr23_2);
	exprs.push_back(&expr23_3);
	exprs.push_back(&expr23_4);

	BPatch_sequence allParts(exprs);


	appAddrSpace->insertSnippet(allParts, point23_1);

	return PASSED;
}
