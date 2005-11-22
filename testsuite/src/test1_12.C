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

// $Id: test1_12.C,v 1.2 2005/11/22 19:41:27 bpellin Exp $
/*
 * #Name: test1_12
 * #Desc: Mutator Side - Insert/Remove and Malloc/Free
 * #Dep: 
 * #Notes: 
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"

BPatch *bpatch;
BPatchSnippetHandle *snippetHandle12_1;
BPatch_variableExpr *varExpr12_1;

const int HEAP_TEST_UNIT_SIZE = 5000;

//
// Start Test Case #12 - mutator side (insert/remove and malloc/free)
//
int mutatorTesta(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func12_2"
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func12_2", found_funcs)) || !found_funcs.size()) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func12_2");
      return -1;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func12_2");
    }

    BPatch_Vector<BPatch_point *> *point12_2 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point12_2 || (point12_2->size() < 1)) {
	fprintf(stderr, "Unable to find point func12_2 - entry.\n");
        return -1;
    }

    varExpr12_1 = appThread->malloc(100);
    if (!varExpr12_1) {
	fprintf(stderr, "Unable to allocate 100 bytes in mutatee\n");
        return -1;
    }

    // Heap stress test - allocate memory until we run out, free it all
    //   and then allocate a small amount of memory.
    setExpectError(66); // We're expecting a heap overflow error
    BPatch_variableExpr* memStuff[30000];
    BPatch_variableExpr *temp;
    temp = appThread->malloc(HEAP_TEST_UNIT_SIZE); 
    int count = 0;
    while (temp) {
#if defined(USES_DYNAMIC_INF_HEAP)
        if (! temp) {
	     printf("*** Inferior malloc stress test failed\n"); 
	     exit(-1);
	}
#endif /* USES_DYNAMIC_INF_HEAP */
	memStuff[count++] = temp;
	temp = appThread->malloc(HEAP_TEST_UNIT_SIZE);
#if defined(USES_DYNAMIC_INF_HEAP)
	// heap will grow indefinitely on dynamic heap platforms
	//if (count == 10000) break;
	// I get tired of waiting
	if (count == 500) break;
#endif /* USES_DYNAMIC_INF_HEAP */
	assert(count < 30000);
    }
    setExpectError(DYNINST_NO_ERROR);

    int freeCount = 0;
    for (int i =0; i < count; i++) {
	appThread->free(*memStuff[i]);
	freeCount++;
    }

    temp = appThread->malloc(500); 
    if (!temp) {
	printf("*** Unable to allocate memory after using then freeing heap\n");
    }

    BPatch_Vector<BPatch_function *> bpfv;
    char *fn = "call12_1";
    if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      fprintf(stderr, "    Unable to find function %s\n", fn);
      return -1;
    }

    BPatch_function *call12_1_func = bpfv[0];

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_funcCallExpr call12_1Expr(*call12_1_func, nullArgs);

    checkCost(call12_1Expr);
    snippetHandle12_1 = appThread->insertSnippet(call12_1Expr, *point12_2);
    if (!snippetHandle12_1) {
	fprintf(stderr,
		"Unable to insert snippet to call function \"call12_1.\"\n");
        return -1;
    }

    return 0;
}

int mutatorTestb(BPatch_thread *appThread, BPatch_image * /*appImage*/)
{
    waitUntilStopped(bpatch, appThread, 12, "insert/remove and malloc/free");

    // remove instrumentation and free memory
    if (!appThread->deleteSnippet(snippetHandle12_1)) {
	printf("**Failed test #12 (insert/remove and malloc/free)\n");
	printf("    deleteSnippet returned an error\n");
        return -1;
    }
    appThread->free(*varExpr12_1);

    // continue process
    appThread->continueExecution();

    return 0;
}

// External Interface
extern "C" TEST_DLL_EXPORT int mutatorMAIN(ParameterDict &param)
{
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());


    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    // Run 12a
    int result = mutatorTesta(appThread, appImage);
    if ( result != 0 )
    {
       return result;
    }

    appThread->continueExecution();

    // Run 12b
    return mutatorTestb(appThread, appImage);

    

}
