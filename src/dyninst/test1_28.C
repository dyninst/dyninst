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

// $Id: test1_28.C,v 1.1 2008/10/30 19:18:43 legendre Exp $
/*
 * #Name: test1_28
 * #Desc: User Defined Fields
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_point.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "dyninst_comp.h"

class test1_28_Mutator : public DyninstMutator {
	virtual test_results_t executeTest();
};

extern "C" DLLEXPORT  TestMutator *test1_28_factory() 
{
	return new test1_28_Mutator();
}

//
// Start Test Case #28 - user defined fields
//

test_results_t test1_28_Mutator::executeTest() 
{
	int i;

	if (isMutateeFortran(appImage)) 
	{
		return SKIPPED;
	}

	//	   Create the types
	BPatch_type *intType = appImage->findType("int");
	assert(intType);

	BPatch_Vector<char *> names;
	BPatch_Vector<BPatch_type *> types;

	names.push_back(const_cast<char*>("field1"));
	names.push_back(const_cast<char*>("field2"));
	types.push_back(intType);
	types.push_back(intType);

	//	struct28_1 { int field1, int field 2; }
	BPatch_type *struct28_1 = BPatch::bpatch->createStruct("test1_28_struct1", names, types);
	BPatch_type *union28_1 = BPatch::bpatch->createUnion("testUnion27_1", names, types);
	assert(union28_1);

	names.push_back(const_cast<char*>("field3"));
	names.push_back(const_cast<char*>("field4"));

	BPatch_type *intArray = BPatch::bpatch->createArray("intArray", intType, 0, 9);

	types.push_back(intArray);
	types.push_back(struct28_1);

	// struct28_2 { int field1, int field 2, int field3[10],struct26_1 field4 } 

	BPatch_type *struct28_2 = BPatch::bpatch->createStruct("test1_28_struct2", names, types);
	BPatch_type *type28_2 = BPatch::bpatch->createTypedef("type28_2", struct28_2);

	// now create variables of these types.

	BPatch_variableExpr *globalVariable28_1 = 
		appImage->findVariable("test1_28_globalVariable1");

	if (!globalVariable28_1)
	{
		logerror("[%s:%u] - Unable to find variable test1_28_globalVariable1\n", 
				__FILE__, __LINE__);
		return FAILED;
	}

	globalVariable28_1->setType(type28_2);

	BPatch_variableExpr *globalVariable28_8 = 
		appImage->findVariable("test1_28_globalVariable8");

	if (!globalVariable28_8)
	{
		logerror("[%s:%u] - Unable to find variable test1_28_globalVariable8\n", 
				__FILE__, __LINE__);
		return FAILED;
	}

	globalVariable28_8->setType(union28_1);

	//     Next verify that we can find a local variable in call28
	const char *fname = "test1_28_call1";
	BPatch_Vector<BPatch_function *> found_funcs;

	if ((NULL == appImage->findFunction(fname, found_funcs)) || !found_funcs.size()) 
	{
		logerror("    Unable to find function %s\n", fname);
		return FAILED;
	}

	if (1 < found_funcs.size()) 
	{
		logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
				__FILE__, __LINE__, found_funcs.size(), fname);
	}

	BPatch_Vector<BPatch_point *> *point28 = found_funcs[0]->findPoint(BPatch_entry);

	assert(point28 && (point28->size() == 1));

	// FIXME We didn't look up a local variable!?

	BPatch_variableExpr *gvar[8];

	for (i=1; i <= 7; i++) 
	{
		char name[80];

		sprintf(name, "test1_28_globalVariable%d", i);
		gvar[i] = appImage->findVariable(name);
		if (!gvar[i]) 
		{
			logerror("**Failed** test #28 (user defined fields)\n");
			logerror("  can't find variable %s\n", name);
			return FAILED;
		}
	}

	// start of code for globalVariable28
	BPatch_Vector<BPatch_variableExpr *> *fields = gvar[1]->getComponents();
	assert(fields && (fields->size() == 4));

	for (i=0; i < 4; i++) 
	{
		char fieldName[80];
		sprintf(fieldName, "field%d", i+1);
		if (strcmp(fieldName, (*fields)[i]->getName())) 
		{
			logerror("field %d of the struct is %s, not %s\n",
					i+1, fieldName, (*fields)[i]->getName());
			return FAILED;
		}
	}

	// 	   globalVariable28 = globalVariable28.field1
	BPatch_arithExpr assignment1(BPatch_assign, *gvar[2], *((*fields)[0]));
	appAddrSpace->insertSnippet(assignment1, *point28);

	// 	   globalVariable28 = globalVariable28.field2
	BPatch_arithExpr assignment2(BPatch_assign, *gvar[3], *((*fields)[1]));
	appAddrSpace->insertSnippet(assignment2, *point28);

	// 	   globalVariable28 = globalVariable28.field3[0]
	BPatch_arithExpr assignment3(BPatch_assign, *gvar[4], 
			BPatch_arithExpr(BPatch_ref, *((*fields)[2]), BPatch_constExpr(0)));
	appAddrSpace->insertSnippet(assignment3, *point28);

	// 	   globalVariable28 = globalVariable28.field3[5]
	BPatch_arithExpr assignment4(BPatch_assign, *gvar[5], 
			BPatch_arithExpr(BPatch_ref, *((*fields)[2]), BPatch_constExpr(5)));
	appAddrSpace->insertSnippet(assignment4, *point28);

	BPatch_Vector<BPatch_variableExpr *> *subfields = 
		(*fields)[3]->getComponents();
	assert(subfields != NULL);

	// 	   globalVariable28 = globalVariable28.field4.field1
	BPatch_arithExpr assignment5(BPatch_assign, *gvar[6], *((*subfields)[0]));
	appAddrSpace->insertSnippet(assignment5, *point28);

	// 	   globalVariable28 = globalVariable28.field4.field2
	BPatch_arithExpr assignment6(BPatch_assign, *gvar[7], *((*subfields)[1]));
	appAddrSpace->insertSnippet(assignment6, *point28);

	// 
	BPatch_Vector<BPatch_variableExpr *> *unionfields = globalVariable28_8->getComponents();

	int n=1;
	int val1, val2, val3; 

	((*unionfields)[0])->writeValue(&n,true);
	((*unionfields)[0])->readValue(&val1);

	if (val1 != 1) 
	{
		logerror("**Failed** test #28 (user defined fields)\n");
		logerror("  union field1 has wrong value after first set\n");
		return FAILED;
	}

	n=2;
	((*unionfields)[1])->writeValue(&n,true);
	((*unionfields)[1])->readValue(&val2);
	if (val2 != 2) 
	{
		logerror("**Failed** test #28 (user defined fields)\n");
		logerror("  union field2 has wrong value after second set\n");
		return FAILED;
	}

	((*unionfields)[1])->readValue(&val3);
	if (val3 != 2) 
	{
		logerror("**Failed** test #28 (user defined fields)\n");
		logerror("  union field1 has wrong value after second set\n");
		return FAILED;
	}

	// create a scalar
	BPatch_type *newScalar1 = BPatch::bpatch->createScalar("scalar1", 8);
	assert(newScalar1);

	int scalarSize = newScalar1->getSize();

	if (scalarSize != 8) 
	{
		logerror("**Failed** test #28 (user defined fields)\n");
		logerror("  created scalar is %d bytes, expected %d\n", scalarSize, 8);
		return FAILED;
	}

	// create an enum
	BPatch_Vector<char *> enumItems;
	BPatch_Vector<int> enumVals;

	enumItems.push_back(const_cast<char*>("item1"));
	enumItems.push_back(const_cast<char*>("item2"));
	enumItems.push_back(const_cast<char*>("item3"));

	enumVals.push_back(42);
	enumVals.push_back(43);
	enumVals.push_back(44);

	BPatch_type *newEnum1 = BPatch::bpatch->createEnum("enum1", enumItems);
	BPatch_type *newEnum2 = BPatch::bpatch->createEnum("enum2", enumItems, enumVals);

	if (!newEnum1 || !newEnum2) 
	{
		logerror("**Failed** test #28 (user defined fields)\n");
		logerror("  failed to create enums as expected\n");
		return FAILED;
	}

	if (!newEnum1->isCompatible(newEnum1)) 
	{
		logerror("**Failed** test #28 (user defined fields)\n");
		logerror("  identical enums reported incompatible\n");
		return FAILED;
	}

	if (newEnum1->isCompatible(newEnum2)) 
	{
		logerror("**Failed** test #28 (user defined fields)\n");
		logerror("  different enums declared compatible\n");
		return FAILED;

	}

	return PASSED;
}

