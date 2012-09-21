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

// $Id: test1_16.C,v 1.1 2008/10/30 19:17:53 legendre Exp $
/*
 * #Name: test1_16
 * #Desc: Mutator Side - If else
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

class test1_16_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
   bool inst2();
   bool inst3();
   bool inst4();
};

extern "C" DLLEXPORT  TestMutator *test1_16_factory() 
{
	return new test1_16_Mutator();
}

//
// Start Test Case #16 - mutator side (if-else)
//

test_results_t test1_16_Mutator::executeTest() 
{
   if (!inst2()) return FAILED;
   if (!inst3()) return FAILED;
   if (!inst4()) return FAILED;
   return PASSED;
}

bool test1_16_Mutator::inst4() {
	const char *funcName = "test1_16_func4";
	BPatch_Vector<BPatch_function *> found_funcs;

	if ((NULL == appImage->findFunction(funcName, found_funcs)) || !found_funcs.size()) 
	{
		logerror("    Unable to find function %s\n", funcName);
		return false;
	}

	if (1 < found_funcs.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), funcName);
	}

	BPatch_Vector<BPatch_point *> *func16_1 = found_funcs[0]->findPoint(BPatch_entry);

	if (!func16_1 || ((*func16_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return false;
	}

	BPatch_variableExpr *expr16_1, *expr16_2, *expr16_3, *expr16_4, *expr16_5,
						*expr16_6, *expr16_7, *expr16_8, *expr16_9, *expr16_10;

	expr16_1 = findVariable(appImage, "test1_16_globalVariable16_1", func16_1);
	expr16_2 = findVariable(appImage, "test1_16_globalVariable16_2", func16_1);
	expr16_3 = findVariable(appImage, "test1_16_globalVariable16_3", func16_1);
	expr16_4 = findVariable(appImage, "test1_16_globalVariable16_4", func16_1);
	expr16_5 = findVariable(appImage, "test1_16_globalVariable16_5", func16_1);
	expr16_6 = findVariable(appImage, "test1_16_globalVariable16_6", func16_1);
	expr16_7 = findVariable(appImage, "test1_16_globalVariable16_7", func16_1);
	expr16_8 = findVariable(appImage, "test1_16_globalVariable16_8", func16_1);
	expr16_9 = findVariable(appImage, "test1_16_globalVariable16_9", func16_1);
	expr16_10 = findVariable(appImage, "test1_16_globalVariable16_10", func16_1);

	if (!expr16_1 || !expr16_2 || !expr16_3 || !expr16_4 || !expr16_5 ||
			!expr16_6 || !expr16_7 || !expr16_8 || !expr16_9 || !expr16_10) 
	{
		logerror("**Failed** test #16 (if-else)\n");
		logerror("    Unable to locate one of test1_16_globalVariable16_?\n");
		return false;
	}

	BPatch_arithExpr assign16_5(BPatch_assign, *expr16_5, BPatch_constExpr(1));
	BPatch_arithExpr assign16_6(BPatch_assign, *expr16_6, BPatch_constExpr(1));
	BPatch_sequence longExpr16_1(genLongExpr(&assign16_5));


	BPatch_ifExpr if16_4(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0),
				BPatch_constExpr(1)), longExpr16_1, assign16_6);

	BPatch_arithExpr assign16_7(BPatch_assign, *expr16_7, BPatch_constExpr(1));
	BPatch_arithExpr assign16_8(BPatch_assign, *expr16_8, BPatch_constExpr(1));
	BPatch_sequence longExpr16_2(genLongExpr(&assign16_8));

	BPatch_ifExpr if16_5(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0),
				BPatch_constExpr(1)), assign16_7, longExpr16_2);

	BPatch_arithExpr assign16_9(BPatch_assign, *expr16_9, BPatch_constExpr(1));
	BPatch_arithExpr assign16_10(BPatch_assign, *expr16_10,BPatch_constExpr(1));
	BPatch_sequence longExpr16_3(genLongExpr(&assign16_9));
	BPatch_sequence longExpr16_4(genLongExpr(&assign16_10));

	BPatch_ifExpr if16_6(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0),
				BPatch_constExpr(1)), longExpr16_3, longExpr16_4);

	if (!insertSnippetAt(appAddrSpace, appImage, "test1_16_func4",
                             BPatch_entry, if16_4, 16, "if-else")) return false;
	if (!insertSnippetAt(appAddrSpace, appImage, "test1_16_func4",
                             BPatch_entry, if16_5, 16, "if-else")) return false;
	if (!insertSnippetAt(appAddrSpace, appImage, "test1_16_func4",
                             BPatch_entry, if16_6, 16, "if-else")) return false;

	return true;
}

bool test1_16_Mutator::inst2() {
	const char *funcName = "test1_16_func2";
	BPatch_Vector<BPatch_function *> found_funcs;

	if ((NULL == appImage->findFunction(funcName, found_funcs)) || !found_funcs.size()) 
	{
		logerror("    Unable to find function %s\n", funcName);
		return false;
	}

	if (1 < found_funcs.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), funcName);
	}

	BPatch_Vector<BPatch_point *> *func16_1 = found_funcs[0]->findPoint(BPatch_entry);

	if (!func16_1 || ((*func16_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return false;
	}

	BPatch_variableExpr *expr16_1, *expr16_2, *expr16_3, *expr16_4, *expr16_5,
						*expr16_6, *expr16_7, *expr16_8, *expr16_9, *expr16_10;

	expr16_1 = findVariable(appImage, "test1_16_globalVariable16_1", func16_1);
	expr16_2 = findVariable(appImage, "test1_16_globalVariable16_2", func16_1);
	expr16_3 = findVariable(appImage, "test1_16_globalVariable16_3", func16_1);
	expr16_4 = findVariable(appImage, "test1_16_globalVariable16_4", func16_1);
	expr16_5 = findVariable(appImage, "test1_16_globalVariable16_5", func16_1);
	expr16_6 = findVariable(appImage, "test1_16_globalVariable16_6", func16_1);
	expr16_7 = findVariable(appImage, "test1_16_globalVariable16_7", func16_1);
	expr16_8 = findVariable(appImage, "test1_16_globalVariable16_8", func16_1);
	expr16_9 = findVariable(appImage, "test1_16_globalVariable16_9", func16_1);
	expr16_10 = findVariable(appImage, "test1_16_globalVariable16_10", func16_1);

	if (!expr16_1 || !expr16_2 || !expr16_3 || !expr16_4 || !expr16_5 ||
			!expr16_6 || !expr16_7 || !expr16_8 || !expr16_9 || !expr16_10) 
	{
		logerror("**Failed** test #16 (if-else)\n");
		logerror("    Unable to locate one of test1_16_globalVariable16_?\n");
		return false;
	}

	BPatch_arithExpr assign16_1(BPatch_assign, *expr16_1, BPatch_constExpr(1));
	BPatch_arithExpr assign16_2(BPatch_assign, *expr16_2, BPatch_constExpr(1));

	BPatch_ifExpr if16_2(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(1),
				BPatch_constExpr(1)), assign16_1, assign16_2);

	if (!insertSnippetAt(appAddrSpace, appImage, "test1_16_func2",
                             BPatch_entry, if16_2, 16, "if-else")) return false;

	return true;
}

bool test1_16_Mutator::inst3() {
	const char *funcName = "test1_16_func3";
	BPatch_Vector<BPatch_function *> found_funcs;

	if ((NULL == appImage->findFunction(funcName, found_funcs)) || !found_funcs.size()) 
	{
		logerror("    Unable to find function %s\n", funcName);
		return false;
	}

	if (1 < found_funcs.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), funcName);
	}

	BPatch_Vector<BPatch_point *> *func16_1 = found_funcs[0]->findPoint(BPatch_entry);

	if (!func16_1 || ((*func16_1).size() == 0)) 
	{
		logerror("Unable to find entry point to \"%s\".\n", funcName);
		return false;
	}

	BPatch_variableExpr *expr16_1, *expr16_2, *expr16_3, *expr16_4, *expr16_5,
						*expr16_6, *expr16_7, *expr16_8, *expr16_9, *expr16_10;

	expr16_1 = findVariable(appImage, "test1_16_globalVariable16_1", func16_1);
	expr16_2 = findVariable(appImage, "test1_16_globalVariable16_2", func16_1);
	expr16_3 = findVariable(appImage, "test1_16_globalVariable16_3", func16_1);
	expr16_4 = findVariable(appImage, "test1_16_globalVariable16_4", func16_1);
	expr16_5 = findVariable(appImage, "test1_16_globalVariable16_5", func16_1);
	expr16_6 = findVariable(appImage, "test1_16_globalVariable16_6", func16_1);
	expr16_7 = findVariable(appImage, "test1_16_globalVariable16_7", func16_1);
	expr16_8 = findVariable(appImage, "test1_16_globalVariable16_8", func16_1);
	expr16_9 = findVariable(appImage, "test1_16_globalVariable16_9", func16_1);
	expr16_10 = findVariable(appImage, "test1_16_globalVariable16_10", func16_1);

	if (!expr16_1 || !expr16_2 || !expr16_3 || !expr16_4 || !expr16_5 ||
			!expr16_6 || !expr16_7 || !expr16_8 || !expr16_9 || !expr16_10) 
	{
		logerror("**Failed** test #16 (if-else)\n");
		logerror("    Unable to locate one of test1_16_globalVariable16_?\n");
		return false;
	}

	BPatch_arithExpr assign16_3(BPatch_assign, *expr16_3, BPatch_constExpr(1));
	BPatch_arithExpr assign16_4(BPatch_assign, *expr16_4, BPatch_constExpr(1));

	BPatch_ifExpr if16_3(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0),
				BPatch_constExpr(1)), assign16_3, assign16_4);

	if (!insertSnippetAt(appAddrSpace, appImage, "test1_16_func3",
                             BPatch_entry, if16_3, 16, "if-else")) return false;

	return true;
}
