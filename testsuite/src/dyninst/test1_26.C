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

// $Id: test1_26.C,v 1.1 2008/10/30 19:18:36 legendre Exp $
/*
 * #Name: test1_26
 * #Desc: Struct Elements
 * #Dep: 
 * #Arch: !mips_sgi_irix6_4_test
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_point.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"
#include "dyninst_comp.h"

class test1_26_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_26_factory() 
{
	return new test1_26_Mutator();
}

//
// Start Test Case #26 - struct elements
//

test_results_t test1_26_Mutator::executeTest() 
{
	if (appImage == NULL)
		return SKIPPED;

	if (isMutateeFortran(appImage)) 
	{
		return SKIPPED;
	}

	//     First verify that we can find a local variable in call26_1
	const char *funcName = "test1_26_call1";
	BPatch_Vector<BPatch_function *> found_funcs;

	if ((NULL == appImage->findFunction(funcName, found_funcs, 1)) || !found_funcs.size()) 
	{
		logerror("    Unable to find function %s\n", funcName);
		return FAILED;
	}

	if (1 < found_funcs.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), funcName);
	}

	BPatch_Vector<BPatch_point *> *point26_1 = found_funcs[0]->findPoint(BPatch_subroutine);
	BPatch_Vector<BPatch_point *> *point26_2 = found_funcs[0]->findPoint(BPatch_subroutine);

	if ( !point26_1 )
	{
		logerror("**Failed** test #26 (struct elements)\n");
		logerror("  point26_1 incorrect\n");
		return FAILED;
	}

	if ( ! point26_2 )
	{
		logerror("**Failed** test #26 (struct elements)\n");
		logerror("  point26_2 NULL\n");
		return FAILED;
	}


	BPatch_variableExpr *lvar;
	BPatch_variableExpr *gvar[14];

	int i;
	for (i=1; i <= 13; i++) 
	{
		char name[80];

		sprintf (name, "test1_26_globalVariable%d", i);
		gvar [i] = findVariable(appImage, name, point26_2);

		if (!gvar[i]) 
		{
			logerror("**Failed** test #26 (struct elements)\n");
			logerror("  can't find variable %s\n", name);
			return FAILED;
		}
	}

	// start of code for globalVariable26_1
	BPatch_Vector<BPatch_variableExpr *> *fields = gvar[1]->getComponents();

	if (!fields) 
	{
		logerror("**Failed** test #26 (struct elements)\n");
		logerror("  struct lacked correct number of elements\n");
		return FAILED;
	}

        if (fields->size() < 4) {
           logerror("**Failed** test #26; expected 4 fields, got %d\n", fields->size());
           return FAILED;
        }

	for (i=0; i < 4; i++) {
		char fieldName[80];
		sprintf(fieldName, "field%d", i+1);
		if (!(*fields)[i]->getName())
			logerror("NULL NAME!\n");
		if (strcmp(fieldName, (*fields)[i]->getName())) {
			logerror("field %d of the struct is %s, not %s\n",
					i+1, fieldName, (*fields)[i]->getName());
			return FAILED;
		}
	}

	// 	   globalVariable26_2 = globalVariable26_1.field1
	BPatch_arithExpr assignment1(BPatch_assign, *gvar[2], *((*fields)[0]));
	appAddrSpace->insertSnippet(assignment1, *point26_2);

	// 	   globalVariable26_3 = globalVariable26_1.field2
	BPatch_arithExpr assignment2(BPatch_assign, *gvar[3], *((*fields)[1]));
	appAddrSpace->insertSnippet(assignment2, *point26_2);

	// 	   globalVariable26_4 = globalVariable26_1.field3[0]
	BPatch_arithExpr assignment3(BPatch_assign, *gvar[4],
			BPatch_arithExpr(BPatch_ref, *((*fields)[2]), BPatch_constExpr(0)));
	appAddrSpace->insertSnippet(assignment3, *point26_2);

	// 	   globalVariable26_5 = globalVariable26_1.field3[5]
	BPatch_arithExpr assignment4(BPatch_assign, *gvar[5],
			BPatch_arithExpr(BPatch_ref, *((*fields)[2]), BPatch_constExpr(5)));
	appAddrSpace->insertSnippet(assignment4, *point26_2);

	BPatch_Vector<BPatch_variableExpr *> *subfields =
		(*fields)[3]->getComponents();

	if (subfields == NULL) 
	{
		logerror("**Failed** test #26 (struct elements)\n");
		logerror("  struct lacked correct number of elements\n");
		return FAILED;
	}


	// 	   globalVariable26_6 = globalVariable26_1.field4.field1
	BPatch_arithExpr assignment5(BPatch_assign, *gvar[6], *((*subfields)[0]));
	appAddrSpace->insertSnippet(assignment5, *point26_2);

	// 	   globalVariable26_7 = globalVariable26_1.field4.field2
	BPatch_arithExpr assignment6(BPatch_assign, *gvar[7], *((*subfields)[1]));
	appAddrSpace->insertSnippet(assignment6, *point26_2);

	// start of code for localVariable26_1
	setExpectError(100);

	lvar = appImage->findVariable(*(*point26_1) [0], "localVariable26_1");
	if (!lvar)
		lvar = appImage->findVariable(*(*point26_1) [0], "localvariable26_1");

	if (!lvar) 
	{
		logerror("**Failed** test #26 (struct elements)\n");
		logerror("  could not find localVariable26_1\n");
		return FAILED;
	}

	setExpectError(DYNINST_NO_ERROR);

	fields = lvar->getComponents();

	if (!fields || (fields->size() < 4)) 
	{
		logerror("**Failed** test #26 (struct elements)\n");
		logerror("  struct lacked correct number of elements\n");
		return FAILED;
	}

	for (i=0; i < 4; i++) 
	{
		char fieldName[80];
		sprintf(fieldName, "field%d", i+1);
		if (strcmp(fieldName, (*fields)[i]->getName())) 
		{
			logerror("field %d of the local struct is %s, not %s\n",
					i+1, fieldName, (*fields)[i]->getName());
			return FAILED;
		}
	}

	// 	   globalVariable26_8 = localVariable26_1.field1

	BPatch_arithExpr assignment7(BPatch_assign, *gvar[8], *((*fields)[0]));
	appAddrSpace->insertSnippet(assignment7, *point26_1);

	// 	   globalVariable26_9 = localVariable26_1.field2

	BPatch_arithExpr assignment8(BPatch_assign, *gvar[9], *((*fields)[1]));
	appAddrSpace->insertSnippet(assignment8, *point26_1);

	// 	   globalVariable26_10 = localVariable26_1.field3[0]

	BPatch_arithExpr assignment9(BPatch_assign, *gvar[10],
			BPatch_arithExpr(BPatch_ref, *((*fields)[2]), BPatch_constExpr(0)));
	appAddrSpace->insertSnippet(assignment9, *point26_1);

	// 	   globalVariable26_11 = localVariable26_1.field3[5]

	BPatch_arithExpr assignment10(BPatch_assign, *gvar[11],
			BPatch_arithExpr(BPatch_ref, *((*fields)[2]), BPatch_constExpr(5)));
	appAddrSpace->insertSnippet(assignment10, *point26_1);

	subfields = (*fields)[3]->getComponents();

	if (subfields == NULL) 
	{
		logerror("**Failed** test #26 (struct elements)\n");
		logerror("  subfields NULL \n");
		return FAILED;
	}

	// 	   globalVariable26_12 = localVariable26_1.field4.field1

	BPatch_arithExpr assignment11(BPatch_assign, *gvar[12], *((*subfields)[0]));
	appAddrSpace->insertSnippet(assignment11, *point26_1);

	// 	   globalVariable26_13 = localVariable26_1.field4.field2

	BPatch_arithExpr assignment12(BPatch_assign, *gvar[13], *((*subfields)[1]));
	appAddrSpace->insertSnippet(assignment12, *point26_1);

	return PASSED;
}
