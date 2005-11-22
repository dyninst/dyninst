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

// $Id: test1_20.C,v 1.3 2005/11/22 19:41:36 bpellin Exp $
/*
 * #Name: test1_20
 * #Desc: Mutator Side - Instrumentation at arbitrary points
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
int mergeTramp;

//
// Start Test Case #20 - mutator side (instrumentation at arbitrary points)
//
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
  if (mergeTramp == 1)
    bpatch->setMergeTramp(true);
  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "call20_1";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "    Unable to find function %s\n", fn);
    return -1;
  }


    BPatch_function *call20_1_func = bpfv[0];

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_funcCallExpr call20_1Expr(*call20_1_func, nullArgs);
    checkCost(call20_1Expr);

    bpfv.clear();
    char *fn2 = "func20_2";
    if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      fprintf(stderr, "    Unable to find function %s\n", fn2);
      return -1;
    }

    BPatch_function *f = bpfv[0];

    // We really don't want to use function size... grab a flowgraph and
    // do this right.
    BPatch_flowGraph *cfg = f->getCFG();
    if (!cfg) {
	fprintf(stderr, "**Failed** test #20 (instrumentation at arbitrary points)\n");
	fprintf(stderr, "    no flowgraph for function 20_2\n");
	return -1;
    }

    BPatch_point *p = NULL;
    bool found_one = false;

    /* We expect certain errors from createInstPointAtAddr. */
    BPatchErrorCallback oldError =
	bpatch->registerErrorCallback(createInstPointError);


    BPatch_Set<BPatch_basicBlock *> blocks;
    if (!cfg->getAllBasicBlocks(blocks))
        assert(0); // This can't return false :)
    if (blocks.size() == 0) {
	fprintf(stderr, "**Failed** test #20 (instrumentation at arbitrary points)\n");
	fprintf(stderr, "    no blocks for function 20_2\n");
        return -1;
    }
    
    appThread->getProcess()->beginInsertionSet();

    BPatch_Set<BPatch_basicBlock *>::iterator blockIter = blocks.begin();
    for (; blockIter != blocks.end(); blockIter++) {
        BPatch_basicBlock *block = *blockIter;
        assert(block);

        for (unsigned long i = block->getStartAddress();
             i < block->getEndAddress();
             i++) {
            // This is the second stupidest way to do this. The stupidest 
            // is the old way: from start to (start+size) by 1. This at least
            // uses basic blocks. We _should_ be using an instruction iterator
            // of some sort...
            p = appImage->createInstPointAtAddr((char *)block->getStartAddress());
            
            if (p) {
                if (p->getPointType() == BPatch_arbitrary) {
                    found_one = true;
                    if (appThread->insertSnippet(call20_1Expr, *p) == NULL) {
                        fprintf(stderr,
                                "Unable to insert snippet into function \"func20_2.\"\n");
                        return -1;
                    }
                }
            }
        }
    }

    appThread->getProcess()->finalizeInsertionSet(false);

    bpatch->registerErrorCallback(oldError);

    if (!found_one) {
	fprintf(stderr, "Unable to find a point to instrument in function \"func20_2.\"\n");
	return -1;
    }

    bpatch->setMergeTramp(false);

    return 0;
}

// External Interface
extern "C" TEST_DLL_EXPORT int mutatorMAIN(ParameterDict &param)
{
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());
    mergeTramp = param["mergeTramp"]->getInt();


    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
