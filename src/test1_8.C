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

// $Id: test1_8.C,v 1.2 2005/11/22 19:42:02 bpellin Exp $
/*
 * #Name: test1_8
 * #Desc: Mutator Side - Preserve Registers - expr
 * #Arch: all
 * #Dep: 
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
//#include "test1.h"

//
// Start Test Case #8 - mutator side (preserve registers - expr)
//
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func8_1"

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func8_1", found_funcs)) || !found_funcs.size()) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func8_1");
      return -1;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func8_1");
    }

    BPatch_Vector<BPatch_point *> *point8_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point8_1 || ((*point8_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func8_1\".\n");
	return -1;
    }

    BPatch_Vector<BPatch_snippet*> vect8_1;

    BPatch_variableExpr *expr8_1 = findVariable(appImage, "globalVariable8_1", point8_1);

    if (!expr8_1) {
	fprintf(stderr, "**Failed** test #3 (passing variables)\n");
	fprintf(stderr, "    Unable to locate variable globalVariable8_1\n");
	return -1;
    }

    BPatch_arithExpr arith8_1 (BPatch_assign, *expr8_1, 
      BPatch_arithExpr(BPatch_plus, 
	    BPatch_arithExpr(BPatch_plus, 
		BPatch_arithExpr(BPatch_plus, BPatch_constExpr(81), 
					      BPatch_constExpr(82)),
		BPatch_arithExpr(BPatch_plus, BPatch_constExpr(83), 
					      BPatch_constExpr(84))),
	    BPatch_arithExpr(BPatch_plus, 
		BPatch_arithExpr(BPatch_plus, BPatch_constExpr(85), 
					      BPatch_constExpr(86)),
		BPatch_arithExpr(BPatch_plus, BPatch_constExpr(87), 
					      BPatch_constExpr(88)))));
    vect8_1.push_back(&arith8_1);

    checkCost(BPatch_sequence(vect8_1));
    appThread->insertSnippet( BPatch_sequence(vect8_1), *point8_1);

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
