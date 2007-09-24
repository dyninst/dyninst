/*
 * Copyright (c) 1996-2004 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
 * 
 * This license is for research uses.  For such uses, there is no
 * charge. We define "research use" to mean you may freely use it
 * inside your organization for whatever purposes you see fit. But you
 * may not re-distribute Paradyn or parts of Paradyn, in any form
 * source or binary (including derivatives), electronic or otherwise,
 * to any other organization or entity without our permission.
 * 
 * (for other uses, please contact us at paradyn@cs.wisc.edu)
 * 
 * All warranties, including without limitation, any warranty of
 * merchantability or fitness for a particular purpose, are hereby
 * excluded.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * Even if advised of the possibility of such damages, under no
 * circumstances shall we (or any other person or entity with
 * proprietary rights in the software licensed hereunder) be liable
 * to you or any third party for direct, indirect, or consequential
 * damages of any character regardless of type of action, including,
 * without limitation, loss of profits, loss of use, loss of good
 * will, or computer failure or malfunction.  You agree to indemnify
 * us (and any other person or entity with proprietary rights in the
 * software licensed hereunder) for any and all liability it may
 * incur to third parties resulting from your use of Paradyn.
 */

// $Id: test1_28.C,v 1.1 2007/09/24 16:37:42 cooksey Exp $
/*
 * #Name: test1_28
 * #Desc: User Defined Fields
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

// static int mutateeFortran;
// static BPatch *bpatch;

#include "TestMutator.h"
class test1_28_Mutator : public TestMutator {
  BPatch *bpatch;

  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t preExecution();
};
extern "C" TEST_DLL_EXPORT TestMutator *test1_28_factory() {
  return new test1_28_Mutator();
}

//
// Start Test Case #28 - user defined fields
//
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
// {
test_results_t test1_28_Mutator::preExecution() {
    int i;

    if (isMutateeFortran(appImage)) {
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
    BPatch_type *struct28_1 = bpatch->createStruct("test1_28_struct1", names, types);
    BPatch_type *union28_1 = bpatch->createUnion("testUnion27_1", names, types);
    assert(union28_1);

    names.push_back(const_cast<char*>("field3"));
    names.push_back(const_cast<char*>("field4"));

    BPatch_type *intArray = bpatch->createArray("intArray", intType, 0, 9);

    types.push_back(intArray);
    types.push_back(struct28_1);

    // struct28_2 { int field1, int field 2, int field3[10],struct26_1 field4 } 
    BPatch_type *struct28_2 = bpatch->createStruct("test1_28_struct2", names, types);
    BPatch_type *type28_2 = bpatch->createTypedef("type28_2", struct28_2);

    // now create variables of these types.
    BPatch_variableExpr *globalVariable28_1 = 
	appImage->findVariable("test1_28_globalVariable1");
    assert(globalVariable28_1);
    globalVariable28_1->setType(type28_2);

    BPatch_variableExpr *globalVariable28_8 = 
	appImage->findVariable("test1_28_globalVariable8");
    assert(globalVariable28_8);
    globalVariable28_8->setType(union28_1);

    //     Next verify that we can find a local variable in call28
    char *fname = "test1_28_call1";
    BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction(fname, found_funcs)) || !found_funcs.size()) {
       logerror("    Unable to find function %s\n", fname);
      return FAILED;
    }

    if (1 < found_funcs.size()) {
      logerror("%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), fname);
    }

    BPatch_Vector<BPatch_point *> *point28 = found_funcs[0]->findPoint(BPatch_entry);

    assert(point28 && (point28->size() == 1));

    // FIXME We didn't look up a local variable!?

    BPatch_variableExpr *gvar[8];

    for (i=1; i <= 7; i++) {
	char name[80];

	sprintf(name, "test1_28_globalVariable%d", i);
	gvar[i] = appImage->findVariable(name);
	if (!gvar[i]) {
	    logerror("**Failed** test #28 (user defined fields)\n");
	    logerror("  can't find variable %s\n", i, name);
	    return FAILED;
	}
    }

    // start of code for globalVariable28
    BPatch_Vector<BPatch_variableExpr *> *fields = gvar[1]->getComponents();
    assert(fields && (fields->size() == 4));

    for (i=0; i < 4; i++) {
	 char fieldName[80];
	 sprintf(fieldName, "field%d", i+1);
	 if (strcmp(fieldName, (*fields)[i]->getName())) {
	      logerror("field %d of the struct is %s, not %s\n",
		  i+1, fieldName, (*fields)[i]->getName());
	      return FAILED;
	 }
    }

    // 	   globalVariable28 = globalVariable28.field1
    BPatch_arithExpr assignment1(BPatch_assign, *gvar[2], *((*fields)[0]));
    appThread->insertSnippet(assignment1, *point28);

    // 	   globalVariable28 = globalVariable28.field2
    BPatch_arithExpr assignment2(BPatch_assign, *gvar[3], *((*fields)[1]));
    appThread->insertSnippet(assignment2, *point28);

    // 	   globalVariable28 = globalVariable28.field3[0]
    BPatch_arithExpr assignment3(BPatch_assign, *gvar[4], 
	BPatch_arithExpr(BPatch_ref, *((*fields)[2]), BPatch_constExpr(0)));
    appThread->insertSnippet(assignment3, *point28);

    // 	   globalVariable28 = globalVariable28.field3[5]
    BPatch_arithExpr assignment4(BPatch_assign, *gvar[5], 
	BPatch_arithExpr(BPatch_ref, *((*fields)[2]), BPatch_constExpr(5)));
    appThread->insertSnippet(assignment4, *point28);

    BPatch_Vector<BPatch_variableExpr *> *subfields = 
	(*fields)[3]->getComponents();
    assert(subfields != NULL);

    // 	   globalVariable28 = globalVariable28.field4.field1
    BPatch_arithExpr assignment5(BPatch_assign, *gvar[6], *((*subfields)[0]));
    appThread->insertSnippet(assignment5, *point28);

    // 	   globalVariable28 = globalVariable28.field4.field2
    BPatch_arithExpr assignment6(BPatch_assign, *gvar[7], *((*subfields)[1]));
    appThread->insertSnippet(assignment6, *point28);

    // 
    BPatch_Vector<BPatch_variableExpr *> *unionfields = globalVariable28_8->getComponents();

    int n=1;
    int val1, val2, val3; 

    ((*unionfields)[0])->writeValue(&n,true);
    ((*unionfields)[0])->readValue(&val1);
    if (val1 != 1) {
	logerror("**Failed** test #28 (user defined fields)\n");
	logerror("  union field1 has wrong value after first set\n");
	return FAILED;
    }

    n=2;
    ((*unionfields)[1])->writeValue(&n,true);
    ((*unionfields)[1])->readValue(&val2);
    if (val2 != 2) {
	logerror("**Failed** test #28 (user defined fields)\n");
	logerror("  union field2 has wrong value after second set\n");
	return FAILED;
    }

    ((*unionfields)[1])->readValue(&val3);
    if (val3 != 2) {
	logerror("**Failed** test #28 (user defined fields)\n");
	logerror("  union field1 has wrong value after second set\n");
	return FAILED;
    }

    // create a scalar
    BPatch_type *newScalar1 = bpatch->createScalar("scalar1", 8);
    assert(newScalar1);
    int scalarSize = newScalar1->getSize();
    if (scalarSize != 8) {
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

    BPatch_type *newEnum1 = bpatch->createEnum("enum1", enumItems);
    BPatch_type *newEnum2 = bpatch->createEnum("enum2", enumItems, enumVals);

    if (!newEnum1 || !newEnum2) {
	logerror("**Failed** test #28 (user defined fields)\n");
	logerror("  failed to create enums as expected\n");
	return FAILED;
    }

    if (!newEnum1->isCompatible(newEnum1)) {
	logerror("**Failed** test #28 (user defined fields)\n");
	logerror("  identical enums reported incompatible\n");
	return FAILED;
    }

    if (newEnum1->isCompatible(newEnum2)) {
	logerror("**Failed** test #28 (user defined fields)\n");
	logerror("  different enums declared compatible\n");
	return FAILED;
	
    }

    return PASSED;
}

// External Interface
// extern "C" TEST_DLL_EXPORT int test1_28_mutatorMAIN(ParameterDict &param)
// {
test_results_t test1_28_Mutator::setup(ParameterDict &param) {
    bool useAttach = param["useAttach"]->getInt();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    appThread = (BPatch_thread *)(param["appThread"]->getPtr());

    // Get log file pointers
//     FILE *outlog = (FILE *)(param["outlog"]->getPtr());
//     FILE *errlog = (FILE *)(param["errlog"]->getPtr());
//     setOutputLog(outlog);
//     setErrorLog(errlog);

    // Read the program's image and get an associated image object
    appImage = appThread->getImage();

    if ( useAttach )
    {
      if ( ! signalAttached(appThread, appImage) )
         return FAILED;
    }

    // Run mutator code
    return PASSED;
}
