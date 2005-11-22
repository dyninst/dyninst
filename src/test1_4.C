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

// $Id: test1_4.C,v 1.2 2005/11/22 19:41:57 bpellin Exp $
/*
 * #Name: test1_4
 * #Desc: Mutator Side (Sequence) Use the BPatch sequence operation to glue expressions together.  The test is constructed to verify the correct execution order.
 * #Arch: all
 * #Dep: 
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

//
// Start Test Case #4 - mutator side (sequence)
//	Use the BPatch sequence operation to glue to expressions togehter.
//	The test is constructed to verify the correct execution order.
//
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func4_1"
  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findFunction("func4_1", found_funcs)) || !found_funcs.size()) {
     fprintf(stderr, "    Unable to find function %s\n",
	    "func4_1");
    return -1;
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), "func4_1");
  }
  
  BPatch_Vector<BPatch_point *> *point4_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point4_1 || ((*point4_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func4_1\".\n");
	return -1;
    }

    BPatch_variableExpr *expr4_1 = findVariable (appImage, "globalVariable4_1", point4_1);

    if (!expr4_1) {
	fprintf(stderr, "**Failed** test #4 (sequence)\n");
	fprintf(stderr, "    Unable to locate variable globalVariable4_1\n");
	return -1;
    }

    BPatch_arithExpr expr4_2(BPatch_assign, *expr4_1, BPatch_constExpr(42));
    BPatch_arithExpr expr4_3(BPatch_assign, *expr4_1, BPatch_constExpr(43));

    BPatch_Vector<BPatch_snippet*> vect4_1;
    vect4_1.push_back(&expr4_2);
    vect4_1.push_back(&expr4_3);

    BPatch_sequence expr4_4(vect4_1);
    checkCost(expr4_4);
    appThread->insertSnippet(expr4_4, *point4_1);

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
