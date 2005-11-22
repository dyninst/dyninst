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

// $Id: test1_29.C,v 1.2 2005/11/22 19:41:45 bpellin Exp $
/*
 * #Name: test1_29
 * #Desc: getParent/Child
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"


bool printSrcObj(BPatch_sourceObj *p, int level)
{
    unsigned int i;
    bool ret = true;

    BPatch_Vector<BPatch_sourceObj *> curr;

    if (!p) return(true);

    switch (p->getSrcType()) {
	case BPatch_sourceProgram:
	    if (level != 0) ret = false;
	    break;

	case BPatch_sourceModule: 
	    if (level != 1) ret = false;
	    break;

	case BPatch_sourceFunction: 
	    if (level != 2) ret = false;
	    break;

	default:
	    printf("<unknown type>");
    }

    if (!p->getSourceObj(curr)) {
	// eveything down to functions should have something
	return((level == 2) ? true : false);
    }

    for (i=0; i < curr.size(); i++) {
	p = curr[i];
	ret = printSrcObj(p, level+1) && ret;
    }

    return ret;
}

//
// Start Test Case #29 - getParent/Child
//
int mutatorTest(BPatch_thread *, BPatch_image *appImage)
{
    BPatch_sourceObj *p;
    bool passedTest;


    p = (BPatch_sourceObj *) appImage;
    passedTest = printSrcObj(p, 0);

    if (!passedTest) {
	fprintf(stderr, "**Failed** test #29 (class BPatch_srcObj)\n");
	return -1;
    }

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func29_1", found_funcs)) || !found_funcs.size()) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func29_1");
      return -1;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func29_1");
    }

    BPatch_Vector<BPatch_point *> *point29_1 = found_funcs[0]->findPoint(BPatch_entry);

    assert (point29_1);

    BPatch_variableExpr *expr29_1 = findVariable(appImage, "globalVariable29_1", point29_1);

    if (expr29_1 == NULL) {
	fprintf(stderr, "**Failed** test #29 (class BPatch_srcObj)\n");
	fprintf(stderr, "    Unable to locate globalVariable29_1\n");
	return -1;
    }
    setExpectError(DYNINST_NO_ERROR);

    int n = 1;
    expr29_1->writeValue(&n,true); //ccw 31 jul 2002

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
