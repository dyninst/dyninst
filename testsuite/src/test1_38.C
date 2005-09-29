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

// $Id: test1_38.C,v 1.1 2005/09/29 20:38:44 bpellin Exp $
/*
 * #Name: test1_38
 * #Desc: CFG Loop Callee Tree
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

int mutateeFortran;

//
// Start Test Case #38 - (CFG loop/callee tree)
//


int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
    if (mutateeFortran) {
	return 0;
    } 

    BPatch_image *dummy = appThread->getImage();
    assert (appThread->getImage());

    BPatch_Vector<BPatch_function *> funcs0;
    
    appImage->findFunction("call38_1", funcs0);

    if (!funcs0.size()) {
        fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
        fprintf(stderr,"    cannot find function call38_1.\n");
        return -1;
    }

    BPatch_function *func = funcs0[0];

    BPatch_flowGraph *cfg = func->getCFG();

    // check that funcs are inserted in the proper places in the loop hierarchy
    BPatch_loopTreeNode *root = cfg->getLoopTree();

    if (!root->children.size()) {
        fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
        fprintf(stderr,"    no kids.\n");
        return -1;
    }

    BPatch_loopTreeNode *firstForLoop  = root->children[0];

    // determine which node is the while loop and which is the second
    // for loop, this is platform dependent

    if (firstForLoop->children.size() < 2) {
        fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
        fprintf(stderr,"    not enough kids.\n");
        return -1;
    }

    BPatch_loopTreeNode *secondForLoop = firstForLoop->children[0];
    BPatch_loopTreeNode *whileLoop     = firstForLoop->children[1];

    // swap if got wrong
    if (firstForLoop->children[0]->children.size() == 0) {
	secondForLoop = firstForLoop->children[1];
	whileLoop     = firstForLoop->children[0];
    }

    BPatch_loopTreeNode *thirdForLoop  = secondForLoop->children[0];

    // root loop has 1 child, the outer for loop
    if (1 != root->children.size()) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    root loop should have 1 child, found %d.\n",
		root->children.size());
	return -1;
    }

    // call38_1 and call38_7 should be off the root
    const char * f38_1 = root->getCalleeName(0);
    const char * f38_7 = root->getCalleeName(1);

    if (2 != root->numCallees()) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    root loop should have 2 functions, found %d.\n",
		root->numCallees());
	return -1;
    }
    if (0 != strcmp("funCall38_1",f38_1)) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    expected funCall38_1 not %s.\n",f38_1);
	return -1;
    }
    if (0 != strcmp("funCall38_7",f38_7)) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    expected funCall38_7 not %s.\n",f38_7);
	return -1;
    }

    // the first for loop should have 3 children and 2 functions
    if (3 != firstForLoop->numCallees()) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    first for loop found %d funcs not 3.\n", 
		firstForLoop->numCallees());
	return -1;
    }
    if (2 != firstForLoop->children.size()) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    first for loop had %d children, not 2.\n",
		firstForLoop->children.size());
	return -1;
    }

    // call38_2, call38_4 and call38_6 should be under the outer loop
    const char * f38_2 = firstForLoop->getCalleeName(0);
    const char * f38_4 = firstForLoop->getCalleeName(1);
    const char * f38_6 = firstForLoop->getCalleeName(2);

    if (0 != strcmp("funCall38_2",f38_2)) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    expected funCall38_2 not %s.\n",f38_2);
	return -1;
    }
    if (0 != strcmp("funCall38_4",f38_4)) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    expected funCall38_4 not %s.\n",f38_4);
	return -1;
    }
    if (0 != strcmp("funCall38_6",f38_6)) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    expected funCall38_6 not %s.\n",f38_6);
	return -1;
    }

    // the second for loop should have one child and no nested functions
    if (1 != secondForLoop->children.size()) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    second for loop had %d children, not 1.\n",
		secondForLoop->children.size());
	return -1;
    }
    if (0 != secondForLoop->numCallees()) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    second for loop had %d funcs (%s), should be 0.\n",
		secondForLoop->numCallees(),
		secondForLoop->getCalleeName(0));
	return -1;
    }

    // third for loop has no children and one function funCall38_3
    if (0 != thirdForLoop->children.size()) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    third for loop had %d children, not 0.\n",
		thirdForLoop->children.size());
	return -1;
    }
    if (1 != thirdForLoop->numCallees()) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    third for loop had %d funcs, not 1.\n",
		thirdForLoop->numCallees());
	return -1;
    }

    const char * f38_3 = thirdForLoop->getCalleeName(0);
    if (0 != strcmp("funCall38_3",f38_3)) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    expected funCall38_3 not %s.\n",f38_3);
	return -1;
    }

    // the while loop has no children and one function (funCall38_5)
    if (0 != whileLoop->children.size()) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    while loop had %d children, not 0.\n",
		whileLoop->children.size());
	return -1;
    }
    if (1 != whileLoop->numCallees()) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    while loop had %d functions, not 1.\n",
		whileLoop->numCallees());
	return -1;
    }

    const char * f38_5 = whileLoop->getCalleeName(0);
    if (0 != strcmp("funCall38_5",f38_5)) {
	fprintf(stderr,"**Failed** test #38 (CFG loop/callee tree)\n");
	fprintf(stderr,"    expected funCall38_5 not %s.\n",f38_5);
	return -1;
    }

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
