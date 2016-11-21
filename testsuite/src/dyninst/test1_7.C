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

// $Id: test1_7.C,v 1.1 2008/10/30 19:19:53 legendre Exp $
/*
 * #Name: test1_7
 * #Desc: Mutator Side - Relational Operators
 * #Arch: all
 * #Dep: 
 */

#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test1_7_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_7_factory() 
{
	return new test1_7_Mutator();
}

static int genRelTest(BPatch_image *appImage,
		BPatch_Vector<BPatch_snippet*> &vect7_1,
		BPatch_relOp op, int r1, int r2, const char *var1) 
{
	const char *funcName = "test1_7_func2";
	BPatch_Vector<BPatch_function *> found_funcs;
	if ((NULL == appImage->findFunction(funcName, found_funcs))
			|| !found_funcs.size()) 
	{
		logerror("    Unable to find function %s\n", funcName);
		return -1;
	}

	if (1 < found_funcs.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), funcName);
	}

	BPatch_Vector<BPatch_point *> *point7_1 = found_funcs[0]->findPoint(BPatch_entry);

	if (!point7_1 || ((*point7_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return -1;
	}

	BPatch_variableExpr *expr1_1 = findVariable (appImage, var1, point7_1);

	if (!expr1_1) 
	{
		logerror("**Failed** test #7 (relational operators)\n");
		logerror("    Unable to locate variable %s\n", var1);
		return -1;
	}

	BPatch_ifExpr *tempExpr1 = new BPatch_ifExpr(
			BPatch_boolExpr(op, BPatch_constExpr(r1), BPatch_constExpr(r2)), 
			BPatch_arithExpr(BPatch_assign, *expr1_1, BPatch_constExpr(72)));
	vect7_1.push_back(tempExpr1);

	return 0;
}

static int genVRelTest(BPatch_image *appImage,
		BPatch_Vector<BPatch_snippet*> &vect7_1, 
		BPatch_relOp op, BPatch_variableExpr *r1, 
		BPatch_variableExpr *r2, const char *var1)
{
	const char *funcName = "test1_7_func2";
	BPatch_Vector<BPatch_function *> found_funcs;

	if ((NULL == appImage->findFunction(funcName, found_funcs)) || !found_funcs.size()) 
	{
		logerror("    Unable to find function %s\n", funcName);
		return -1;
	}

	if (1 < found_funcs.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), funcName);
	}

	BPatch_Vector<BPatch_point *> *point7_1 = found_funcs[0]->findPoint(BPatch_entry);

	if (!point7_1 || ((*point7_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return -1;
	}

	BPatch_variableExpr *expr1_1 = findVariable(appImage, var1, point7_1);

	if (!expr1_1) 
	{
		logerror("**Failed** test #7 (relational operators)\n");
		logerror("    Unable to locate variable %s\n", var1);
		return -1;
	}

	BPatch_ifExpr *tempExpr1 = new BPatch_ifExpr(
			BPatch_boolExpr(op, *r1, *r2), 
			BPatch_arithExpr(BPatch_assign, *expr1_1, BPatch_constExpr(74)));
	vect7_1.push_back(tempExpr1);

	return 0;
}

//
// Start Test Case #7 - mutator side (relational operators)
//

test_results_t test1_7_Mutator::executeTest() 
{
	// Find the entry point to the procedure "func7_2"
	const char *funcName = "test1_7_func2";
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

	BPatch_Vector<BPatch_point *> *point7_1 = found_funcs[0]->findPoint(BPatch_entry);

	if (!point7_1 || ((*point7_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return FAILED;
	}

	BPatch_Vector<BPatch_snippet*> vect7_1;

	if ( genRelTest(appImage, vect7_1, BPatch_lt, 0, 1,
				"test1_7_globalVariable1") < 0)
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_lt, 1, 0,
				"test1_7_globalVariable2") < 0 )
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_eq, 2, 2,
				"test1_7_globalVariable3") < 0 )
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_eq, 2, 3,
				"test1_7_globalVariable4") < 0 )
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_gt, 4, 3,
				"test1_7_globalVariable5") < 0 )
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_gt, 3, 4,
				"test1_7_globalVariable6") < 0 )
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_le, 3, 4,
				"test1_7_globalVariable7") < 0 )
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_le, 4, 3,
				"test1_7_globalVariable8") < 0 )
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_ne, 5, 6,
				"test1_7_globalVariable9") < 0 )
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_ne, 5, 5,
				"test1_7_globalVariable10") < 0 )
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_ge, 9, 7,
				"test1_7_globalVariable11") < 0 )
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_ge, 7, 9,
				"test1_7_globalVariable12") < 0 )
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_and, 1, 1,
				"test1_7_globalVariable13") < 0 )
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_and, 1, 0,
				"test1_7_globalVariable14") < 0 )
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_or, 1, 0,
				"test1_7_globalVariable15") < 0 )
		return FAILED;
	if ( genRelTest(appImage, vect7_1, BPatch_or, 0, 0,
				"test1_7_globalVariable16") < 0 )
		return FAILED;

	const char *funcName2 = "test1_7_func2";
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

	BPatch_Vector<BPatch_point *> *func7_1 = found_funcs2[0]->findPoint(BPatch_entry);

	if (!func7_1 || ((*func7_1).size() == 0)) 
	{
		logerror("Unable to find entry points in \"%s\".\n", funcName2);
		return FAILED;
	}

	BPatch_variableExpr *constVar0, *constVar1, *constVar2, *constVar3, *constVar4, 
						*constVar5, *constVar6, *constVar7, *constVar9;

	constVar0 = findVariable(appImage, "test1_7_constVar0", func7_1);
	constVar1 = findVariable(appImage, "test1_7_constVar1", func7_1);
	constVar2 = findVariable(appImage, "test1_7_constVar2", func7_1);
	constVar3 = findVariable(appImage, "test1_7_constVar3", func7_1);
	constVar4 = findVariable(appImage, "test1_7_constVar4", func7_1);
	constVar5 = findVariable(appImage, "test1_7_constVar5", func7_1);
	constVar6 = findVariable(appImage, "test1_7_constVar6", func7_1);
	constVar7 = findVariable(appImage, "test1_7_constVar7", func7_1);
	constVar9 = findVariable(appImage, "test1_7_constVar9", func7_1);

	if (!constVar0 || !constVar1 || !constVar2 || !constVar3 || !constVar4 ||
			!constVar5 || !constVar6 || !constVar7 || !constVar9 ) 
	{
		logerror("**Failed** test #7 (relational operators)\n");
		logerror("    Unable to locate one of test1_7_constVar?\n");
		return FAILED;
	}

	if ( genVRelTest(appImage, vect7_1, BPatch_lt, constVar0, constVar1,
				"test1_7_globalVariable1a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_lt, constVar1, constVar0,
				"test1_7_globalVariable2a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_eq, constVar2, constVar2,
				"test1_7_globalVariable3a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_eq, constVar2, constVar3,
				"test1_7_globalVariable4a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_gt, constVar4, constVar3,
				"test1_7_globalVariable5a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_gt, constVar3, constVar4,
				"test1_7_globalVariable6a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_le, constVar3, constVar4,
				"test1_7_globalVariable7a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_le, constVar4, constVar3,
				"test1_7_globalVariable8a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_ne, constVar5, constVar6,
				"test1_7_globalVariable9a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_ne, constVar5, constVar5,
				"test1_7_globalVariable10a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_ge, constVar9, constVar7,
				"test1_7_globalVariable11a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_ge, constVar7, constVar9,
				"test1_7_globalVariable12a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_and, constVar1, constVar1,
				"test1_7_globalVariable13a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_and, constVar1, constVar0,
				"test1_7_globalVariable14a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_or, constVar1, constVar0,
				"test1_7_globalVariable15a") < 0 )
		return FAILED;
	if ( genVRelTest(appImage, vect7_1, BPatch_or, constVar0, constVar0,
				"test1_7_globalVariable16a") < 0 )
		return FAILED;

	dprintf("relops test vector length is %d\n", vect7_1.size());

	checkCost(BPatch_sequence(vect7_1));
	appAddrSpace->insertSnippet( BPatch_sequence(vect7_1), *point7_1);

	return PASSED;
}
