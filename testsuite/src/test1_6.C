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

// $Id: test1_6.C,v 1.1 2005/09/29 20:38:55 bpellin Exp $
/*
 * #Name: test1_6
 * #Desc: Mutator Side - Arithmetic Operators
 * #Arch: all
 * #Dep: 
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

//
// Start Test Case #6 - mutator side (arithmetic operators)
//
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
    // Find the entry point to the procedure "func6_2"

    
  BPatch_Vector<BPatch_function *> found_funcs;
  if ((NULL == appImage->findFunction("func6_2", found_funcs)) || !found_funcs.size()) {
    fprintf(stderr, "    Unable to find function %s\n",
	    "func6_2");
    return -1;
  }
  
  if (1 < found_funcs.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs.size(), "func6_2");
  }
  
  BPatch_Vector<BPatch_point *> *point6_1 = found_funcs[0]->findPoint(BPatch_entry);  

  if (!point6_1 || ((*point6_1).size() == 0)) {
    fprintf(stderr, "Unable to find entry point to \"func6_2\".\n");
    return -1;
  }

  BPatch_Vector<BPatch_function *> found_funcs2;
  if ((NULL == appImage->findFunction("func6_1", found_funcs2)) || !found_funcs2.size()) {
     fprintf(stderr, "    Unable to find function %s\n",
	    "func6_1");
    return -1;
  }
  
  if (1 < found_funcs2.size()) {
    fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	    __FILE__, __LINE__, found_funcs2.size(), "func6_1");
  }
  
  BPatch_Vector<BPatch_point *> *point6_2 = found_funcs2[0]->findPoint(BPatch_subroutine);  

    if (!point6_2 || ((*point6_2).size() == 0)) {
	fprintf(stderr, "Unable to find entry point to \"func6_1\".\n");
	return -1;
    }

    BPatch_variableExpr *expr6_1, *expr6_2, *expr6_3, *expr6_4, *expr6_5, *expr6_6,
        *expr6_1a, *expr6_2a, *expr6_3a, *expr6_4a, *expr6_5a, *expr6_6a,
        *constVar1, *constVar2, *constVar3, *constVar5, *constVar6,
        *constVar10, *constVar60, *constVar64, *constVar66, *constVar67;

    expr6_1 = findVariable(appImage, "globalVariable6_1", point6_2);
    expr6_2 = findVariable(appImage, "globalVariable6_2", point6_2);
    expr6_3 = findVariable(appImage, "globalVariable6_3", point6_2);
    expr6_4 = findVariable(appImage, "globalVariable6_4", point6_2);
    expr6_5 = findVariable(appImage, "globalVariable6_5", point6_2);
    expr6_6 = findVariable(appImage, "globalVariable6_6", point6_2);
    expr6_1a = findVariable(appImage, "globalVariable6_1a", point6_2);
    expr6_2a = findVariable(appImage, "globalVariable6_2a", point6_2);
    expr6_3a = findVariable(appImage, "globalVariable6_3a", point6_2);
    expr6_4a = findVariable(appImage, "globalVariable6_4a", point6_2);
    expr6_5a = findVariable(appImage, "globalVariable6_5a", point6_2);
    expr6_6a = findVariable(appImage, "globalVariable6_6a", point6_2);

    constVar1 = findVariable(appImage, "constVar1", point6_2);
    constVar2 = findVariable(appImage, "constVar2", point6_2);
    constVar3 = findVariable(appImage, "constVar3", point6_2);
    constVar5 = findVariable(appImage, "constVar5", point6_2);
    constVar6 = findVariable(appImage, "constVar6", point6_2);
    constVar10 = findVariable(appImage, "constVar10", point6_2);
    constVar60 = findVariable(appImage, "constVar60", point6_2);
    constVar64 = findVariable(appImage, "constVar64", point6_2);
    constVar66 = findVariable(appImage, "constVar66", point6_2);
    constVar67 = findVariable(appImage, "constVar67", point6_2);

    if (!expr6_1 || !expr6_2 || !expr6_3 || !expr6_4 ||
	!expr6_5 || !expr6_6 || !expr6_1a || !expr6_2a || !expr6_3a ||
	!expr6_4a || !expr6_5a || !expr6_6a) {
	fprintf(stderr, "**Failed** test #6 (arithmetic operators)\n");
	fprintf(stderr, "    Unable to locate one of globalVariable6_?\n");
	return -1;
    }

    if (!constVar1 || !constVar2 || !constVar3 || !constVar5 ||
	!constVar6 || !constVar10 || !constVar60 || !constVar64 || 
	!constVar66 || !constVar67) {
	fprintf(stderr, "**Failed** test #6 (arithmetic operators)\n");
	fprintf(stderr, "    Unable to locate one of constVar?\n");
	return -1;
    }

    BPatch_Vector<BPatch_snippet*> vect6_1;

    // globalVariable6_1 = 60 + 2
    BPatch_arithExpr arith6_1 (BPatch_assign, *expr6_1,
      BPatch_arithExpr(BPatch_plus,BPatch_constExpr(60), BPatch_constExpr(2)));
    vect6_1.push_back(&arith6_1);

    // globalVariable6_2 = 64 - 1
    BPatch_arithExpr arith6_2 (BPatch_assign, *expr6_2, 
      BPatch_arithExpr(BPatch_minus,BPatch_constExpr(64),BPatch_constExpr(1)));
    vect6_1.push_back(&arith6_2);

    // globalVariable6_3 = 553648128 / 25165824 = 22
    //    - make these big constants to test loading constants larger than
    //      small immediate - jkh 6/22/98
    BPatch_arithExpr arith6_3 (BPatch_assign, *expr6_3, BPatch_arithExpr(
      BPatch_divide,BPatch_constExpr(553648128),BPatch_constExpr(25165824)));
    vect6_1.push_back(&arith6_3);

    // globalVariable6_4 = 67 / 3
    BPatch_arithExpr arith6_4 (BPatch_assign, *expr6_4, BPatch_arithExpr(
      BPatch_divide,BPatch_constExpr(67),BPatch_constExpr(3)));
    vect6_1.push_back(&arith6_4);
    // globalVariable6_5 = 6 * 5
    BPatch_arithExpr arith6_5 (BPatch_assign, *expr6_5, BPatch_arithExpr(
      BPatch_times,BPatch_constExpr(6),BPatch_constExpr(5)));
    vect6_1.push_back(&arith6_5);

    // globalVariable6_6 = 10,3
    BPatch_arithExpr arith6_6 (BPatch_assign, *expr6_6, 
	BPatch_arithExpr(BPatch_seq,BPatch_constExpr(10),BPatch_constExpr(3)));
    vect6_1.push_back(&arith6_6);

    // globalVariable6_1a = 60 + 2
    BPatch_arithExpr arith6_1a (BPatch_assign, *expr6_1a, 
      BPatch_arithExpr(BPatch_plus, *constVar60, *constVar2));
    vect6_1.push_back(&arith6_1a);

    // globalVariable6_2a = 64 - 1
    BPatch_arithExpr arith6_2a (BPatch_assign, *expr6_2a, 
      BPatch_arithExpr(BPatch_minus, *constVar64, *constVar1));
    vect6_1.push_back(&arith6_2a);

    // globalVariable6_3a = 66 / 3
    BPatch_arithExpr arith6_3a (BPatch_assign, *expr6_3a, BPatch_arithExpr(
      BPatch_divide, *constVar66, *constVar3));
    vect6_1.push_back(&arith6_3a);

    // globalVariable6_4a = 67 / 3
    BPatch_arithExpr arith6_4a (BPatch_assign, *expr6_4a, BPatch_arithExpr(
      BPatch_divide, *constVar67, *constVar3));
    vect6_1.push_back(&arith6_4a);

    // globalVariable6_5a = 6 * 5
    BPatch_arithExpr arith6_5a (BPatch_assign, *expr6_5a, BPatch_arithExpr(
      BPatch_times, *constVar6, *constVar5));
    vect6_1.push_back(&arith6_5a);

    // globalVariable6_6a = 10,3
    // BPatch_arithExpr arith6_6a (BPatch_assign, *expr6_6a, *constVar3);
    //	BPatch_arithExpr(BPatch_seq, *constVar10, BPatch_constExpr(3)));
    BPatch_arithExpr arith6_6a (BPatch_assign, *expr6_6a,
	BPatch_arithExpr(BPatch_seq, *constVar10, *constVar3));
    vect6_1.push_back(&arith6_6a);

    checkCost(BPatch_sequence(vect6_1));
    appThread->insertSnippet( BPatch_sequence(vect6_1), *point6_1);

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
