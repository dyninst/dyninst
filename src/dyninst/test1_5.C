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

// $Id: test1_5.C,v 1.1 2008/10/30 19:19:43 legendre Exp $
/*
 * #Name: test1_5
 * #Desc: Mutator Side - If without else
 * #Arch: all
 * #Dep: 
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test1_5_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_5_factory() 
{
	return new test1_5_Mutator();
}

//
// Start Test Case #5 - mutator side (if w.o. else)
//

test_results_t test1_5_Mutator::executeTest() 
{
	// Find the entry point to the procedure "func5_2"
	const char *funcName = "test1_5_func2";

	BPatch_Vector<BPatch_function *> found_funcs;
	if ((NULL == appImage->findFunction(funcName, found_funcs))
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

	BPatch_Vector<BPatch_point *> *point5_1 = found_funcs[0]->findPoint(BPatch_entry);  

	if (!point5_1 || ((*point5_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return FAILED;
	}

	const char *funcName2 = "test1_5_func1";
	BPatch_Vector<BPatch_function *> found_funcs2;

	if ((NULL == appImage->findFunction(funcName2, found_funcs2))
			|| !found_funcs2.size()) 
	{
		logerror("    Unable to find function %s\n", funcName2);
		return FAILED;
	}

	if (1 < found_funcs2.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs2.size(), funcName2);
	}

	BPatch_Vector<BPatch_point *> *point5_2 = found_funcs2[0]->findPoint(BPatch_subroutine);  

	if (!point5_2 || ((*point5_2).size() == 0)) 
	{
		logerror("Unable to find subroutine call points in \"%s\".\n", funcName2);
		return FAILED;
	}

	const char *globalVar1 = "test1_5_globalVariable5_1";
	const char *globalVar2 = "test1_5_globalVariable5_2";
	BPatch_variableExpr *expr5_1 = findVariable (appImage, globalVar1, point5_1);
	BPatch_variableExpr *expr5_2 = findVariable (appImage, globalVar2, point5_1);

	if (!expr5_1 || !expr5_2) 
	{
		logerror("**Failed** test #5 (1f w.o. else)\n");
		logerror("    Unable to locate variable %s or ", globalVar1);
		logerror("    variable %s\n", globalVar2);
		return FAILED;
	}

	BPatch_Vector<BPatch_snippet*> vect5_1;

	// if (0 == 1) globalVariable5_1 = 52;
	BPatch_ifExpr expr5_3(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0),
				BPatch_constExpr(1)), 
			BPatch_arithExpr(BPatch_assign, *expr5_1,
				BPatch_constExpr(52)));

	// if (1 == 1) globalVariable5_2 = 53;
	BPatch_ifExpr expr5_4(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(1),
				BPatch_constExpr(1)), 
			BPatch_arithExpr(BPatch_assign, *expr5_2,
				BPatch_constExpr(53)));

	vect5_1.push_back(&expr5_3);
	vect5_1.push_back(&expr5_4);

	BPatch_sequence expr5_5(vect5_1);
	checkCost(expr5_5);
	appAddrSpace->insertSnippet(expr5_5, *point5_1);

	return PASSED;
}
