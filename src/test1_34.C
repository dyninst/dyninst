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

// $Id: test1_34.C,v 1.2 2005/11/22 19:41:51 bpellin Exp $
/*
 * #Name: test1_34
 * #Desc: Loop Information
 * #Dep: 
 * #Arch: !os_windows && !os_aix
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

int numContainedLoops(BPatch_basicBlockLoop *loop)
{
    BPatch_Vector<BPatch_basicBlockLoop*> containedLoops;
    loop->getContainedLoops(containedLoops);

    return containedLoops.size();
}

//
// Start Test Case #34 - (loop information)
//
int mutatorTest( BPatch_thread * /*appThread*/, BPatch_image * appImage )
{
#if !defined(os_windows) && !defined(os_aix)
    unsigned int i;

    BPatch_Vector<BPatch_function *> bpfv;
    char *fn = "func34_2";
    if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      fprintf(stderr, "**Failed** test #34 (loop information)\n");
      fprintf(stderr, "    Unable to find function %s\n", fn);
      return -1;
    }
    
    BPatch_function *func2 = bpfv[0];

    BPatch_flowGraph *cfg = func2->getCFG();
    if (cfg == NULL) {
	fprintf(stderr, "**Failed** test #34 (loop information)\n");
	fprintf(stderr, "  Unable to get control flow graph of func34_2\n");
	return -1;
    }

    BPatch_Vector<BPatch_basicBlockLoop*> loops;
    cfg->getLoops(loops);
    if (loops.size() != 4) {
	fprintf(stderr, "**Failed** test #34 (loop information)\n");
	fprintf(stderr, "  Detected %d loops, should have been four.\n",
		loops.size());
	return -1;
    }

    /*
     * Find the loop that contains two loops (that should be the outermost
     * one).
     */
    BPatch_basicBlockLoop *outerLoop = NULL;
    for (i = 0; i < loops.size(); i++) {
	if (numContainedLoops(loops[i]) == 3) {
	    outerLoop = loops[i];
	    break;
	}
    }

    if (outerLoop == NULL) {
	fprintf(stderr, "**Failed** test #34 (loop information)\n");
	fprintf(stderr, "  Unable to find a loop containing two other loops.\n");
	return -1;
    }

//     if (numBackEdges(outerLoop) != 1) {
// 	fprintf(stderr, "**Failed** test #34 (loop information)\n");
// 	fprintf(stderr, "  There should be exactly one backedge in the outer loops, but there are %d\n", numBackEdges(outerLoop));
// 	return -1;
//     }

    BPatch_Vector<BPatch_basicBlockLoop*> insideOuterLoop;
    outerLoop->getContainedLoops(insideOuterLoop);
    assert(insideOuterLoop.size() == 3);

    bool foundFirstLoop = false;
    int deepestLoops = 0;
    for (i = 0; i < insideOuterLoop.size(); i++) {
	BPatch_Vector<BPatch_basicBlockLoop*> tmpLoops;
	insideOuterLoop[i]->getContainedLoops(tmpLoops);

	if (tmpLoops.size() == 1) { /* The first loop has one nested inside. */
	    if (foundFirstLoop) {
		fprintf(stderr, "**Failed** test #34 (loop information)\n");
		fprintf(stderr, "  Found more than one second-level loop with one nested inside.\n");
		return -1;
	    }
	    foundFirstLoop = true;

// 	    if (numBackEdges(insideOuterLoop[i]) != 1) {
// 		fprintf(stderr, "**Failed** test #34 (loop information)\n");
// 		fprintf(stderr, "  There should be exactly one backedge in the first inner loop, but there are %d\n", numBackEdges(tmpLoops[0]));
// 		return -1;
// 	    }

// 	    if (numBackEdges(tmpLoops[0]) != 1) {
// 		fprintf(stderr, "**Failed** test #34 (loop information)\n");
// 		fprintf(stderr, "  There should be exactly one backedge in the third level loop, but there are %d\n", numBackEdges(tmpLoops[0]));
// 		return -1;
// 	    }

	    if (numContainedLoops(tmpLoops[0]) != 0) {
		fprintf(stderr, "**Failed** test #34 (loop information)\n");
		fprintf(stderr, "  The first loop at the third level should not have any loops nested inside,\n");
		fprintf(stderr, "  but %d were detected.\n",
			numContainedLoops(tmpLoops[0]));
		return -1;
	    }

	} else if(tmpLoops.size() == 0) { /* The second loop has none nested. */
	    if (deepestLoops >= 2) {
		fprintf(stderr, "**Failed** test #34 (loop information)\n");
		fprintf(stderr, "  Found more than two loops without any nested inside.\n");
		return -1;
	    }
	    deepestLoops++;

// 	    if (numBackEdges(insideOuterLoop[i]) != 1) {
// 		fprintf(stderr, "**Failed** test #34 (loop information)\n");
// 		fprintf(stderr, "  Unexpected number of backedges in loop (%d)\n", numBackEdges(insideOuterLoop[i]));
// 		return -1;
// 	    }
	} else { /* All loops should be recognized above. */
	    fprintf(stderr, "**Failed** test #34 (loop information)\n");
	    fprintf(stderr, "  Found a loop containing %d loops, should be one or  none.\n", tmpLoops.size());
	    return -1;
	}
    }

    if (!foundFirstLoop || deepestLoops < 2) {
	/* We shouldn't be able to get here. */
	fprintf(stderr, "**Failed** test #34 (loop information)\n");
	if (!foundFirstLoop)
	    fprintf(stderr, "  Could not find the first nested loop.\n");
	if (deepestLoops < 2)
	    fprintf(stderr, "  Could not find all the deepest level loops.\n");
	return -1;
    }

    // test getOuterLoops
    // i'd like to be able to swap the order of BPatch_flowGraph::loops
    // around so that the hasAncestor code is tested

    BPatch_Vector<BPatch_basicBlockLoop*> outerLoops;
    cfg->getOuterLoops(outerLoops);

    if (outerLoops.size() != 1) {
	fprintf(stderr, "**Failed** test #34 (loop information)\n");
	fprintf(stderr, "  Detected %d outer loops, should have been one.\n",
		outerLoops.size());
	return -1;
    }

    BPatch_Vector<BPatch_basicBlockLoop*> outerLoopChildren;
    outerLoops[0]->getOuterLoops(outerLoopChildren);

    if (outerLoopChildren.size() != 2) {
	fprintf(stderr, "**Failed** test #34 (loop information)\n");
	fprintf(stderr, "  Detected %d outer loops, should have been two.\n",
		outerLoopChildren.size());
	return -1;
    }

    BPatch_Vector<BPatch_basicBlockLoop*> outerLoopGrandChildren0;
    outerLoopChildren[0]->getOuterLoops(outerLoopGrandChildren0);

    BPatch_Vector<BPatch_basicBlockLoop*> outerLoopGrandChildren1;
    outerLoopChildren[1]->getOuterLoops(outerLoopGrandChildren1);

    // one has no children, the other has 1 child
    if (!((outerLoopGrandChildren0.size() == 0 || 
	   outerLoopGrandChildren1.size() == 0) &&
	  (outerLoopGrandChildren0.size() == 1 || 
	   outerLoopGrandChildren1.size() == 1))) {
	fprintf(stderr, "**Failed** test #34 (loop information)\n");
	fprintf(stderr, "  Detected %d and %d outer loops, should have been zero and one.\n",
		outerLoopGrandChildren0.size(), 
		outerLoopGrandChildren1.size());
	return -1;
    }
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

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
