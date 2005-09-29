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

// $Id: test1_9.C,v 1.1 2005/09/29 20:38:58 bpellin Exp $
/*
 * #Name: test1_9
 * #Desc: Mutator Side - Preserve Registers - FuncCall
 * #Arch: all
 * #Dep: 
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"


int mutateeFortran;

//
// Start Test Case #9 - mutator side (preserve registers - funcCall)
//
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func9_1"

  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func9_1", found_funcs)) || !found_funcs.size()) {
       fprintf(stderr, "    Unable to find function %s\n",
	      "func9_1");
      return -1;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func9_1");
    }

    BPatch_Vector<BPatch_point *> *point9_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point9_1 || ((*point9_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func9_1\".\n");
	return -1;
    }

    BPatch_Vector<BPatch_function *> bpfv;
    char *fn = "call9_1";
    if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      fprintf(stderr, "    Unable to find function %s\n", fn);
      return -1;
    }

    BPatch_function *call9_func = bpfv[0];
 
    BPatch_Vector<BPatch_snippet *> call9_args;

    BPatch_variableExpr *expr9_1 = appThread->malloc (*appImage->findType ("int"));
    BPatch_constExpr constExpr9_1 (0);
    BPatch_arithExpr arithexpr9_1 (BPatch_assign, *expr9_1, BPatch_constExpr (91));
    appThread->insertSnippet (arithexpr9_1, *point9_1);

    if (mutateeFortran) {
        constExpr9_1 = expr9_1->getBaseAddr ();
    } else {
        constExpr9_1 = 91;
    }

    call9_args.push_back(&constExpr9_1);

    BPatch_variableExpr *expr9_2 = appThread->malloc (*appImage->findType ("int"));
    BPatch_constExpr constExpr9_2 (0);
    BPatch_arithExpr arithexpr9_2 (BPatch_assign, *expr9_2, BPatch_constExpr (92));
    appThread->insertSnippet (arithexpr9_2, *point9_1);

    if (mutateeFortran) {
        constExpr9_2 = expr9_2->getBaseAddr ();
    } else {
        constExpr9_2 = 92;
    }

    call9_args.push_back(&constExpr9_2);

    BPatch_variableExpr *expr9_3 = appThread->malloc (*appImage->findType ("int"));
    BPatch_constExpr constExpr9_3 (0);
    BPatch_arithExpr arithexpr9_3 (BPatch_assign, *expr9_3, BPatch_constExpr (93));
    appThread->insertSnippet (arithexpr9_3, *point9_1);

    if (mutateeFortran) {
        constExpr9_3 = expr9_3->getBaseAddr ();
    } else {
        constExpr9_3 = 93;
    }

    call9_args.push_back(&constExpr9_3);

    BPatch_variableExpr *expr9_4 = appThread->malloc (*appImage->findType ("int"));
    BPatch_constExpr constExpr9_4 (0);
    BPatch_arithExpr arithexpr9_4 (BPatch_assign, *expr9_4, BPatch_constExpr (94));
    appThread->insertSnippet (arithexpr9_4, *point9_1);

    if (mutateeFortran) {
        constExpr9_4 = expr9_4->getBaseAddr ();
    } else {
        constExpr9_4 = 94;
    }

    call9_args.push_back(&constExpr9_4);

    BPatch_variableExpr *expr9_5 = appThread->malloc (*appImage->findType ("int"));
    BPatch_constExpr constExpr9_5 (0);
    BPatch_arithExpr arithexpr9_5 (BPatch_assign, *expr9_5, BPatch_constExpr (95));
    appThread->insertSnippet (arithexpr9_5, *point9_1);

    if (mutateeFortran) {
        constExpr9_5 = expr9_5->getBaseAddr ();
    } else {
        constExpr9_5 = 95;
    }

    call9_args.push_back(&constExpr9_5);

    BPatch_funcCallExpr call9Expr(*call9_func, call9_args);

    checkCost(call9Expr);
    appThread->insertSnippet(call9Expr, *point9_1, BPatch_callBefore, BPatch_lastSnippet);

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
