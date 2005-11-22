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

// $Id: test1_3.C,v 1.2 2005/11/22 19:41:46 bpellin Exp $
/*
 * #Name: test1_3
 * #Desc: Mutator Side (passing variables to a function)
 * #Arch: all
 * #Dep: 
 * #Note: Mutatee fortran
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"


int mutateeFortran;

//
// Start Test Case #3 - mutator side (passing variables to function)
//
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
  // Find the entry point to the procedure "func3_1"

  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findFunction("func3_1", found_funcs)) || !found_funcs.size()) {
    fprintf(stderr, "    Unable to find function %s\n",
	    "func3_1");
    return -1;
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), "func3_1");
  }
  
  BPatch_Vector<BPatch_point *> *point3_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point3_1 || ((*point3_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func3_1.\"\n");
	return -1;
    }

  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "call3_1";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "    Unable to find function %s\n", fn);
    return -1;
  }
  BPatch_function *call3_func = bpfv[0];

    BPatch_Vector<BPatch_snippet *> call3_args;

  BPatch_Vector<BPatch_function *> found_funcs2;
  if ((NULL == appImage->findFunction("call3_1", found_funcs2)) || !found_funcs2.size()) {
    fprintf(stderr, "    Unable to find function %s\n",
	    "call3_1");
    return -1;
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs2.size(), "call3_1");
  }
  
  BPatch_Vector<BPatch_point *> *call3_1 = found_funcs2[0]->findPoint(BPatch_subroutine);

    if (!call3_1 || ((*call3_1).size() == 0)) {
        fprintf(stderr, "    Unable to find entry point to \"call3_1.\"\n");
        return -1;
    }

    BPatch_variableExpr *expr3_1 = findVariable (appImage, "globalVariable3_1", call3_1);

    if (!expr3_1) {
        fprintf(stderr, "**Failed** test #3 (passing variables)\n");
        fprintf(stderr, "    Unable to locate variable globalVariable3_1\n");
        return -1;
    }

    // see if we can find the address
    if (expr3_1->getBaseAddr() <= 0) {
        printf("*Error*: address %p for globalVariable3_1 is not valid\n",
            expr3_1->getBaseAddr());
    }

    BPatch_variableExpr *expr3_2 = appThread->malloc(*appImage->findType("int"));
    if (!expr3_2) {
	fprintf(stderr, "**Failed** test #3 (passing variables)\n");
	fprintf(stderr, "    Unable to create new int variable\n");
	return -1;
    }

	BPatch_constExpr expr3_3 (expr3_1->getBaseAddr ());
	BPatch_constExpr expr3_4 (expr3_2->getBaseAddr ());

	if (mutateeFortran) {
	    call3_args.push_back (&expr3_3);
	    call3_args.push_back (&expr3_4);
	} else {
	    call3_args.push_back(expr3_1);
	    call3_args.push_back(expr3_2);
	}

    BPatch_funcCallExpr call3Expr(*call3_func, call3_args);
    checkCost(call3Expr);
    appThread->insertSnippet(call3Expr, *point3_1);

    BPatch_arithExpr expr3_5(BPatch_assign, *expr3_2, BPatch_constExpr(32));
    checkCost(expr3_5);
    appThread->insertSnippet(expr3_5, *point3_1);

    dprintf("Inserted snippet3\n");

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
