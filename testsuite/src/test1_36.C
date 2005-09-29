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

// $Id: test1_36.C,v 1.1 2005/09/29 20:38:42 bpellin Exp $
/*
 * #Name: test1_36
 * #Desc: Callsite Parameter Referencing
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

BPatch_arithExpr *makeTest36paramExpr(BPatch_snippet *expr, int paramId)
{
   if (mutateeFortran) {
       // Fortran is call by reference
       BPatch_arithExpr *derefExpr = new BPatch_arithExpr(BPatch_deref, *(new BPatch_paramExpr(paramId)));
       assert(derefExpr);
       return new BPatch_arithExpr(BPatch_assign, *expr, *derefExpr);
   } else {
       return new BPatch_arithExpr(BPatch_assign, *expr, *(new BPatch_paramExpr(paramId)));
   }
}

//
// Start Test Case #36 - (callsite parameter referencing)
//
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
   // Find the entry point to the procedure "func13_1"
   BPatch_Vector<BPatch_function *> found_funcs;
   if ((NULL == appImage->findFunction("func36_1", found_funcs)) || !found_funcs.size()) {
      fprintf(stderr, "    Unable to find function %s\n",
              "func36_1");
      return -1;
   }
   
   if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
              __FILE__, __LINE__, found_funcs.size(), "func36_1");
   }
   
   BPatch_Vector<BPatch_point *> *all_points36_1 =
      found_funcs[0]->findPoint(BPatch_subroutine);
   
   if (!all_points36_1 || (all_points36_1->size() < 1)) {
      fprintf(stderr, "Unable to find point func36_1 - entry.\n");
      return -1;
   }

   BPatch_point *point36_1 = NULL;
   for(unsigned i=0; i<(*all_points36_1).size(); i++) {
      BPatch_point *cur_point = (*all_points36_1)[i];
      if(cur_point == NULL) continue;
      BPatch_function *func = cur_point->getCalledFunction();
      char funcname[100];
      if (!func) continue;
      
      if(func->getName(funcname, 99)) {
         if(strstr(funcname, "call36_1"))
            point36_1 = cur_point;
      }
   }
   if(point36_1 == NULL) {
      fprintf(stderr, "Unable to find callsite %s\n",
              "call36_1");
      return -1;
   }

   BPatch_variableExpr *expr36_1 =findVariable(appImage, "globalVariable36_1", all_points36_1);
   BPatch_variableExpr *expr36_2 =findVariable(appImage, "globalVariable36_2", all_points36_1);
   BPatch_variableExpr *expr36_3 =findVariable(appImage, "globalVariable36_3", all_points36_1);
   BPatch_variableExpr *expr36_4 =findVariable(appImage, "globalVariable36_4", all_points36_1);
   BPatch_variableExpr *expr36_5 =findVariable(appImage, "globalVariable36_5", all_points36_1);
   BPatch_variableExpr *expr36_6 =findVariable(appImage, "globalVariable36_6", all_points36_1);
   BPatch_variableExpr *expr36_7 =findVariable(appImage, "globalVariable36_7", all_points36_1);
   BPatch_variableExpr *expr36_8 =findVariable(appImage, "globalVariable36_8", all_points36_1);
   BPatch_variableExpr *expr36_9 =findVariable(appImage, "globalVariable36_9", all_points36_1);
   BPatch_variableExpr *expr36_10 = findVariable(appImage, "globalVariable36_10", all_points36_1);
   
   if (expr36_1 == NULL || expr36_2 == NULL || expr36_3 == NULL ||
       expr36_4 == NULL || expr36_5 == NULL || expr36_6 == NULL ||
       expr36_7 == NULL || expr36_8 == NULL || expr36_9 == NULL ||
       expr36_10 == NULL)
   {
      fprintf(stderr,"**Failed** test #36 (callsite parameter referencing)\n");
      fprintf(stderr, "    Unable to locate at least one of "
              "globalVariable36_{1...10}\n");
      return -1;
	}

   BPatch_Vector<BPatch_snippet *> snippet_seq;
   snippet_seq.push_back(makeTest36paramExpr(expr36_1, 0));
   snippet_seq.push_back(makeTest36paramExpr(expr36_2, 1));
   snippet_seq.push_back(makeTest36paramExpr(expr36_3, 2));
   snippet_seq.push_back(makeTest36paramExpr(expr36_4, 3));
   snippet_seq.push_back(makeTest36paramExpr(expr36_5, 4));
   snippet_seq.push_back(makeTest36paramExpr(expr36_6, 5));
#if !defined(alpha_dec_osf4_0) && !defined(arch_x86_64)  /* alpha and AMD64 don't handle more than 6 */
   snippet_seq.push_back(makeTest36paramExpr(expr36_7, 6));
   snippet_seq.push_back(makeTest36paramExpr(expr36_8, 7));

   // Solaris Fortran skips 9th paramter
#if defined(sparc_sun_solaris2_4) 
   if (!mutateeFortran)
#endif
       snippet_seq.push_back(makeTest36paramExpr(expr36_9, 8));

#if !defined(sparc_sun_solaris2_4)
   snippet_seq.push_back(makeTest36paramExpr(expr36_10, 9));
#endif
#endif
   BPatch_sequence seqExpr(snippet_seq);

   checkCost(seqExpr);
   appThread->insertSnippet(seqExpr, *point36_1);

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
