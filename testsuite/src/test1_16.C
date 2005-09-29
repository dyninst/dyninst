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

// $Id: test1_16.C,v 1.1 2005/09/29 20:38:16 bpellin Exp $
/*
 * #Name: test1_16
 * #Desc: Mutator Side - If else
 * #Dep: 
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

//
// Start Test Case #16 - mutator side (if-else)
//
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func16_1", found_funcs)) || !found_funcs.size()) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func16_1");
      return -1;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func16_1");
    }

    BPatch_Vector<BPatch_point *> *func16_1 = found_funcs[0]->findPoint(BPatch_subroutine);

    if (!func16_1 || ((*func16_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func16_1\".\n");
        return -1;
    }

    BPatch_variableExpr *expr16_1, *expr16_2, *expr16_3, *expr16_4, *expr16_5,
        *expr16_6, *expr16_7, *expr16_8, *expr16_9, *expr16_10;

    expr16_1 = findVariable(appImage, "globalVariable16_1", func16_1);
    expr16_2 = findVariable(appImage, "globalVariable16_2", func16_1);
    expr16_3 = findVariable(appImage, "globalVariable16_3", func16_1);
    expr16_4 = findVariable(appImage, "globalVariable16_4", func16_1);
    expr16_5 = findVariable(appImage, "globalVariable16_5", func16_1);
    expr16_6 = findVariable(appImage, "globalVariable16_6", func16_1);
    expr16_7 = findVariable(appImage, "globalVariable16_7", func16_1);
    expr16_8 = findVariable(appImage, "globalVariable16_8", func16_1);
    expr16_9 = findVariable(appImage, "globalVariable16_9", func16_1);
    expr16_10 = findVariable(appImage, "globalVariable16_10", func16_1);

    if (!expr16_1 || !expr16_2 || !expr16_3 || !expr16_4 || !expr16_5 ||
        !expr16_6 || !expr16_7 || !expr16_8 || !expr16_9 || !expr16_10) {
	fprintf(stderr, "**Failed** test #16 (if-else)\n");
	fprintf(stderr, "    Unable to locate one of globalVariable16_?\n");
	return -1;
    }

    BPatch_arithExpr assign16_1(BPatch_assign, *expr16_1, BPatch_constExpr(1));
    BPatch_arithExpr assign16_2(BPatch_assign, *expr16_2, BPatch_constExpr(1));

    BPatch_ifExpr if16_2(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(1),
        BPatch_constExpr(1)), assign16_1, assign16_2);

    BPatch_arithExpr assign16_3(BPatch_assign, *expr16_3, BPatch_constExpr(1));
    BPatch_arithExpr assign16_4(BPatch_assign, *expr16_4, BPatch_constExpr(1));

    BPatch_ifExpr if16_3(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0),
        BPatch_constExpr(1)), assign16_3, assign16_4);

    BPatch_arithExpr assign16_5(BPatch_assign, *expr16_5, BPatch_constExpr(1));
    BPatch_arithExpr assign16_6(BPatch_assign, *expr16_6, BPatch_constExpr(1));
    BPatch_sequence longExpr16_1(genLongExpr(&assign16_5));


    BPatch_ifExpr if16_4(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0),
        BPatch_constExpr(1)), longExpr16_1, assign16_6);

    BPatch_arithExpr assign16_7(BPatch_assign, *expr16_7, BPatch_constExpr(1));
    BPatch_arithExpr assign16_8(BPatch_assign, *expr16_8, BPatch_constExpr(1));
    BPatch_sequence longExpr16_2(genLongExpr(&assign16_8));

    BPatch_ifExpr if16_5(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0),
        BPatch_constExpr(1)), assign16_7, longExpr16_2);

    BPatch_arithExpr assign16_9(BPatch_assign, *expr16_9, BPatch_constExpr(1));
    BPatch_arithExpr assign16_10(BPatch_assign, *expr16_10,BPatch_constExpr(1));
    BPatch_sequence longExpr16_3(genLongExpr(&assign16_9));
    BPatch_sequence longExpr16_4(genLongExpr(&assign16_10));

    BPatch_ifExpr if16_6(BPatch_boolExpr(BPatch_eq, BPatch_constExpr(0),
        BPatch_constExpr(1)), longExpr16_3, longExpr16_4);

    RETURNONNULL(insertSnippetAt(appThread, appImage, "func16_2", BPatch_entry, if16_2,
	16, "if-else"));
    RETURNONNULL(insertSnippetAt(appThread, appImage, "func16_3", BPatch_entry, if16_3,
	16, "if-else"));
    RETURNONNULL(insertSnippetAt(appThread, appImage, "func16_4", BPatch_entry, if16_4,
	16, "if-else"));
    RETURNONNULL(insertSnippetAt(appThread, appImage, "func16_4", BPatch_entry, if16_5,
	16, "if-else"));
    RETURNONNULL(insertSnippetAt(appThread, appImage, "func16_4", BPatch_entry, if16_6,
	16, "if-else"));

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

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
