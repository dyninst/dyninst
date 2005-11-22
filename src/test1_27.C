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

// $Id: test1_27.C,v 1.2 2005/11/22 19:41:43 bpellin Exp $
/*
 * #Name: test1_27
 * #Desc: Type compatibility
 * #Dep: 
 * #Arch: !mips_sgi_irix6_4
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"

int mutateeFortran;

//
// Start Test Case #27 - type compatibility
//
int mutatorTest(BPatch_thread *, BPatch_image *appImage)
{
#if !defined(mips_sgi_irix6_4)

    if (mutateeFortran) {
	return 0;
    }

    BPatch_type *type27_1 = appImage->findType("type27_1");
    BPatch_type *type27_2 = appImage->findType("type27_2");
    BPatch_type *type27_3 = appImage->findType("type27_3");
    BPatch_type *type27_4 = appImage->findType("type27_4");

    if (!type27_1 || !type27_2 || !type27_3 || !type27_4) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr, "    Unable to locate one of type27_{1,2,3,4}\n");
	return -1;
    }

    if (!type27_1->isCompatible(type27_2)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_1 reported as incompatibile with type27_2\n");
	return -1;
    }

    if (!type27_2->isCompatible(type27_1)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_2 reported as incompatibile with type27_1\n");
	return -1;
    }

    if (!type27_3->isCompatible(type27_3)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_3 reported as incompatibile with type27_4\n");
	return -1;
    }

    if (!type27_4->isCompatible(type27_3)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_4 reported as incompatibile with type27_3\n");
	return -1;
    }

    setExpectError(112); // We're expecting type conflicts here
    if (type27_1->isCompatible(type27_3)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_1 reported as compatibile with type27_3\n");
	return -1;
    }

    if (type27_4->isCompatible(type27_2)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_4 reported as compatibile with type27_2\n");
	return -1;
    }
    setExpectError(DYNINST_NO_ERROR);

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func27_1", found_funcs)) || !found_funcs.size()) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func27_1");
      return -1;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func27_1");
    }

    BPatch_Vector<BPatch_point *> *point27_1 = found_funcs[0]->findPoint(BPatch_entry);

    assert (point27_1);

    BPatch_variableExpr *expr27_5, *expr27_6, *expr27_7, *expr27_8;

    expr27_5 = findVariable(appImage, "globalVariable27_5", point27_1);
    expr27_6 = findVariable(appImage, "globalVariable27_6", point27_1);
    expr27_7 = findVariable(appImage, "globalVariable27_7", point27_1);
    expr27_8 = findVariable(appImage, "globalVariable27_8", point27_1);

    assert(expr27_5 && expr27_6 && expr27_7 && expr27_8);

    BPatch_type *type27_5 = const_cast<BPatch_type *> (expr27_5->getType());
    BPatch_type *type27_6 = const_cast<BPatch_type *> (expr27_6->getType());
    BPatch_type *type27_7 = const_cast<BPatch_type *> (expr27_7->getType());
    BPatch_type *type27_8 = const_cast<BPatch_type *> (expr27_8->getType());

    assert(type27_5 && type27_6 && type27_7 && type27_8);

    if (!type27_5->isCompatible(type27_6)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_5 reported as incompatibile with type27_6\n");
	return -1;
    }

    // difderent number of elements
    setExpectError(112); // We're expecting type conflicts here
    if (type27_5->isCompatible(type27_7)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_5 reported as compatibile with type27_7\n");
	return -1;
    }

    // same # of elements, different type
    if (type27_5->isCompatible(type27_8)) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr,"    type27_5 reported as compatibile with type27_8\n");
	return -1;
    }

    // all ok, set the global variable, depends on test 18 working
    BPatch_variableExpr *expr27_1 = findVariable(appImage, "globalVariable27_1", point27_1);

    if (expr27_1 == NULL) {
	fprintf(stderr, "**Failed** test #27 (type compatibility)\n");
	fprintf(stderr, "    Unable to locate globalVariable27_1\n");
	return -1;
    }
    setExpectError(DYNINST_NO_ERROR);

    int n = 1;
    expr27_1->writeValue(&n, true); //ccw 31 jul 2002
#endif

    return 0;
}

// External Interface
extern "C" TEST_DLL_EXPORT int mutatorMAIN(ParameterDict &param)
{
    BPatch *bpatch;
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());


    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    mutateeFortran = isMutateeFortran(appImage);

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
