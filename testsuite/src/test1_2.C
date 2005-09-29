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

// $Id: test1_2.C,v 1.1 2005/09/29 20:38:34 bpellin Exp $
/*
 * #Name: test1_2
 * #Desc: Mutator Side (call a four argument function)
 * #Arch: all
 * #Dep: 
 * #Notes: Uses mutateeFortran variable
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "test1.h"



int mutateeFortran;

//
// Start Test Case #2 - mutator side (call a four argument function)
//
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
  const char* testName = "four parameter function";
  int testNo = 2;
    // Find the entry point to the procedure "func2_1"

  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findFunction("func2_1", found_funcs)) || !found_funcs.size()) {
    fprintf(stderr, "    Unable to find function %s\n",
	    "func2_1");
    return -1;
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), "func2_1");
  }
  
  BPatch_Vector<BPatch_point *> *point2_1 = found_funcs[0]->findPoint(BPatch_entry);

  if (!point2_1 || ((*point2_1).size() == 0)) {
    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
    fprintf(stderr, "    Unable to find entry point to \"func2_1.\"\n");
    return -1;
  }

  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "call2_1";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #%d (%s)\n", testNo, testName);
    fprintf(stderr, "    Unable to find function %s\n", fn);
    return -1;
  }
  BPatch_function *call2_func = bpfv[0];

    void *ptr;

#if defined(mips_sgi_irix6_4)
    BPatch_variableExpr *pointerSizeVar = appImage->findVariable("pointerSize");
    if (!pointerSizeVar) {
	fprintf(stderr, "**Failed** test #2 (four parameter function)\n");
	fprintf(stderr, "    Unable to locate variable pointerSize\n");
	return -1;
    }

    int pointerSize;
    if (!pointerSizeVar->readValue(&pointerSize)) {
	fprintf(stderr, "**Failed** test #2 (four parameter function)\n");
	fprintf(stderr, "    Unable to read value of variable pointerSize\n");
	return -1;
    }

    assert(sizeof(void *) == sizeof(unsigned long) &&
	   sizeof(void *) == TEST_PTR_SIZE);

    /* Determine the size of pointer we should use dynamically. */
    if (pointerSize == 4) {
	ptr = TEST_PTR_32BIT;
    } else if (pointerSize == 8) {
	ptr = TEST_PTR_64BIT;
    } else {
	fprintf(stderr, "**Failed** test #2 (four parameter function)\n");
	fprintf(stderr, "    Unexpected value for pointerSize\n");
	return -1;
    }
#else
    /* For platforms where there is only one possible size for a pointer. */
    ptr = TEST_PTR;
#endif

    BPatch_Vector<BPatch_snippet *> call2_args;

    BPatch_constExpr expr2_1 (0), expr2_2 (0), expr2_3 (0), expr2_4 (0);

    if (mutateeFortran) {
        BPatch_variableExpr *expr2_5 = appThread->malloc (*appImage->findType ("int"));
        BPatch_variableExpr *expr2_6 = appThread->malloc (*appImage->findType ("int"));

        expr2_1 = expr2_5->getBaseAddr ();
        expr2_2 = expr2_6->getBaseAddr ();

        BPatch_arithExpr expr2_7 (BPatch_assign, *expr2_5, BPatch_constExpr(1));
        appThread->insertSnippet (expr2_7, *point2_1);

        BPatch_arithExpr expr2_8 (BPatch_assign, *expr2_6, BPatch_constExpr(2));
        appThread->insertSnippet (expr2_8, *point2_1);

        expr2_3 = "testString2_1";
        expr2_4 = 13;
    } else {
        expr2_1 = 1;
        expr2_2 = 2;
        expr2_3 = "testString2_1";
        expr2_4 = ptr;
    }

    call2_args.push_back(&expr2_1);
    call2_args.push_back(&expr2_2);
    call2_args.push_back(&expr2_3);
    call2_args.push_back(&expr2_4);

    BPatch_funcCallExpr call2Expr(*call2_func, call2_args);

    dprintf("Inserted snippet2\n");
    checkCost(call2Expr);
    appThread->insertSnippet(call2Expr, *point2_1, BPatch_callBefore, BPatch_lastSnippet);

    return 0;
}

// External Interface
extern "C" int mutatorMAIN(ParameterDict &param)
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
