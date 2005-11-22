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

// $Id: test1_7.C,v 1.2 2005/11/22 19:42:01 bpellin Exp $
/*
 * #Name: test1_7
 * #Desc: Mutator Side - Relational Operators
 * #Arch: all
 * #Dep: 
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

int genRelTest(BPatch_image *appImage,BPatch_Vector<BPatch_snippet*> &vect7_1,
                BPatch_relOp op, int r1, int r2, const char *var1)
{

   BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func7_1", found_funcs)) || !found_funcs.size()) {
        fprintf(stderr, "    Unable to find function %s\n",
	      "func7_1");
      return -1;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func7_1");
    }

    BPatch_Vector<BPatch_point *> *point7_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point7_1 || ((*point7_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func7_1\".\n");
	return -1;
    }

    BPatch_variableExpr *expr1_1 = findVariable (appImage, var1, point7_1);

    if (!expr1_1) {
	fprintf(stderr, "**Failed** test #7 (relational operators)\n");
	fprintf(stderr, "    Unable to locate variable %s\n", var1);
	return -1;
    }
    BPatch_ifExpr *tempExpr1 = new BPatch_ifExpr(
	BPatch_boolExpr(op, BPatch_constExpr(r1), BPatch_constExpr(r2)), 
	BPatch_arithExpr(BPatch_assign, *expr1_1, BPatch_constExpr(72)));
    vect7_1.push_back(tempExpr1);

    return 0;
}

int genVRelTest(BPatch_image *appImage,
                 BPatch_Vector<BPatch_snippet*> &vect7_1, 
                 BPatch_relOp op, BPatch_variableExpr *r1, 
                 BPatch_variableExpr *r2, const char *var1)
{

   BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func7_1", found_funcs)) || !found_funcs.size()) {
      fprintf(stderr, "    Unable to find function %s\n",
	      "func7_1");
      return -1;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func7_1");
    }

    BPatch_Vector<BPatch_point *> *point7_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point7_1 || ((*point7_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func7_1\".\n");
	return -1;
    }

    BPatch_variableExpr *expr1_1 = findVariable(appImage, var1, point7_1);

    if (!expr1_1) {
	fprintf(stderr, "**Failed** test #7 (relational operators)\n");
	fprintf(stderr, "    Unable to locate variable %s\n", var1);
	return -1;
    }
    BPatch_ifExpr *tempExpr1 = new BPatch_ifExpr(
	BPatch_boolExpr(op, *r1, *r2), 
	BPatch_arithExpr(BPatch_assign, *expr1_1, BPatch_constExpr(74)));
    vect7_1.push_back(tempExpr1);

    return 0;
}

//
// Start Test Case #7 - mutator side (relational operators)
//
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func7_2"

   BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("func7_2", found_funcs)) || !found_funcs.size()) {
       fprintf(stderr, "    Unable to find function %s\n",
	      "func7_2");
      return -1;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "func7_2");
    }

    BPatch_Vector<BPatch_point *> *point7_1 = found_funcs[0]->findPoint(BPatch_entry);

    if (!point7_1 || ((*point7_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func7_2\".\n");
	return -1;
    }

    BPatch_Vector<BPatch_snippet*> vect7_1;

    if ( genRelTest(appImage, vect7_1, BPatch_lt, 0, 1, "globalVariable7_1") < 0)
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_lt, 1, 0, "globalVariable7_2") < 0 )
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_eq, 2, 2, "globalVariable7_3") < 0 )
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_eq, 2, 3, "globalVariable7_4") < 0 )
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_gt, 4, 3, "globalVariable7_5") < 0 )
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_gt, 3, 4, "globalVariable7_6") < 0 )
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_le, 3, 4, "globalVariable7_7") < 0 )
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_le, 4, 3, "globalVariable7_8") < 0 )
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_ne, 5, 6, "globalVariable7_9") < 0 )
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_ne, 5, 5, "globalVariable7_10") < 0 )
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_ge, 9, 7, "globalVariable7_11") < 0 )
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_ge, 7, 9, "globalVariable7_12") < 0 )
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_and, 1, 1, "globalVariable7_13") < 0 )
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_and, 1, 0, "globalVariable7_14") < 0 )
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_or, 1, 0, "globalVariable7_15") < 0 )
       return -1;
    if ( genRelTest(appImage, vect7_1, BPatch_or, 0, 0, "globalVariable7_16") < 0 )
       return -1;

   BPatch_Vector<BPatch_function *> found_funcs2;
    if ((NULL == appImage->findFunction("func7_1", found_funcs2)) || !found_funcs2.size()) {
       fprintf(stderr, "    Unable to find function %s\n",
	      "func7_1");
      return -1;
    }

    if (1 < found_funcs2.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs2.size(), "func7_1");
    }

    BPatch_Vector<BPatch_point *> *func7_1 = found_funcs2[0]->findPoint(BPatch_subroutine);

    if (!func7_1 || ((*func7_1).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func7_1\".\n");
	return -1;
    }

    BPatch_variableExpr *constVar0, *constVar1, *constVar2, *constVar3, *constVar4, *constVar5, *constVar6, *constVar7, *constVar9;
    constVar0 = findVariable(appImage, "constVar0", func7_1);
    constVar1 = findVariable(appImage, "constVar1", func7_1);
    constVar2 = findVariable(appImage, "constVar2", func7_1);
    constVar3 = findVariable(appImage, "constVar3", func7_1);
    constVar4 = findVariable(appImage, "constVar4", func7_1);
    constVar5 = findVariable(appImage, "constVar5", func7_1);
    constVar6 = findVariable(appImage, "constVar6", func7_1);
    constVar7 = findVariable(appImage, "constVar7", func7_1);
    constVar9 = findVariable(appImage, "constVar9", func7_1);

    if (!constVar0 || !constVar1 || !constVar2 || !constVar3 || !constVar4 ||
        !constVar5 || !constVar6 || !constVar7 || !constVar9 ) {
	fprintf(stderr, "**Failed** test #7 (relational operators)\n");
	fprintf(stderr, "    Unable to locate one of constVar?\n");
	return -1;
    }

    if ( genVRelTest(appImage, vect7_1, BPatch_lt, constVar0, constVar1, "globalVariable7_1a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_lt, constVar1, constVar0, "globalVariable7_2a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_eq, constVar2, constVar2, "globalVariable7_3a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_eq, constVar2, constVar3, "globalVariable7_4a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_gt, constVar4, constVar3, "globalVariable7_5a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_gt, constVar3, constVar4, "globalVariable7_6a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_le, constVar3, constVar4, "globalVariable7_7a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_le, constVar4, constVar3, "globalVariable7_8a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_ne, constVar5, constVar6, "globalVariable7_9a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_ne, constVar5, constVar5, "globalVariable7_10a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_ge, constVar9, constVar7, "globalVariable7_11a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_ge, constVar7, constVar9, "globalVariable7_12a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_and, constVar1, constVar1, "globalVariable7_13a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_and, constVar1, constVar0, "globalVariable7_14a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_or, constVar1, constVar0, "globalVariable7_15a") < 0 )
       return -1;
    if ( genVRelTest(appImage, vect7_1, BPatch_or, constVar0, constVar0, "globalVariable7_16a") < 0 )
       return -1;

    dprintf("relops test vector length is %d\n", vect7_1.size());

    checkCost(BPatch_sequence(vect7_1));
    appThread->insertSnippet( BPatch_sequence(vect7_1), *point7_1);

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
