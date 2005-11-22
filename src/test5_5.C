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

// $Id: test5_5.C,v 1.2 2005/11/22 19:42:31 bpellin Exp $
/*
 * #Name: test5_5
 * #Desc: Namespace
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4,i386_unknown_linux2_0,x86_64_unknown_linux2_4,ia64_unknown_linux2_4
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"


//  
// Start Test Case #5 - (namespace)
// 
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4)

  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "namespace_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #5 (namespace)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn);
    return FAIL;
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point5_1 = f1->findPoint(BPatch_exit);

   assert(point5_1);
   BPatch_variableExpr *var1 = appImage->findVariable(*(*point5_1)[0],
                                                      "local_fn_var");
   BPatch_variableExpr *var2 = appImage->findVariable(*(*point5_1)[0],
                                                      "local_file_var");
   BPatch_variableExpr *var3 = appImage->findVariable(*(*point5_1)[0],
                                                      "CPP_DEFLT_ARG");
   
   if (!var1 || !var2 || !var3) {
      fprintf(stderr, "**Failed** test #5 (namespace)\n");
      if (!var1)
         fprintf(stderr, "  can't find local variable local_fn_var\n");
      if (!var2)
         fprintf(stderr, "  can't find local variable file local_file_var\n");
      if (!var3)
         fprintf(stderr, "  can't find global variable CPP_DEFLT_ARG\n");
      return FAIL;
    }

   bpfv.clear();
   char *fn2 = "main";
   if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	 || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #5 (namespace)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn2);
    return FAIL;
  }
  BPatch_function *f2 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point5_2 = f2->findPoint(BPatch_allLocations);

   if (!point5_2 || (point5_2->size() < 1)) {
      fprintf(stderr, "Unable to find point in main.\n");
      return FAIL;
   }
   BPatch_variableExpr *expr5_1=appImage->findVariable(*(*point5_2)[0], "test5");
   if (!expr5_1) {
      fprintf(stderr, "**Failed** test #5 (namespace)\n");
      fprintf(stderr, "    Unable to locate test5 in main\n");
   }
   
   BPatch_Vector<BPatch_variableExpr *> *fields = expr5_1->getComponents();
   if (!fields || fields->size() == 0 ) {
      fprintf(stderr, "**Failed** test #5 (namespace)\n");
      fprintf(stderr, "  struct lacked correct number of elements\n");
      return FAIL;
   }
   
   unsigned int index = 0;
   while ( index < fields->size() ) {
      if (!strcmp("class_variable", (*fields)[index]->getName()) ) {

	BPatch_Vector<BPatch_function *> bpfv2;
	char *fn3 = "cpp_test_util::call_cpp";
	if (NULL == appImage->findFunction(fn3, bpfv2) || !bpfv2.size()
	    || NULL == bpfv2[0]){
	  fprintf(stderr, "**Failed** test #5 (namespace)\n");
	  fprintf(stderr, "    Unable to find function %s\n", fn3);
	  return FAIL;
	}
	BPatch_function *call5_func = bpfv2[0];  
            
         BPatch_variableExpr *this5 = appImage->findVariable("test5");
         if (this5 == NULL) {
            fprintf(stderr, "**Failed** test #5 (namespace)\n");
            fprintf(stderr, "Unable to find variable \"test5\"\n");
            return FAIL;
         }
         
         BPatch_Vector<BPatch_snippet *> call5_args;
         
         BPatch_constExpr expr5_0((void *)this5->getBaseAddr());
         call5_args.push_back(&expr5_0);
         BPatch_constExpr expr5_1(5);
         call5_args.push_back(&expr5_1);
         BPatch_funcCallExpr call5Expr(*call5_func, call5_args);
         checkCost(call5Expr);
         appThread->insertSnippet(call5Expr, *point5_1);
         return PASS;
      }
      index ++;
   }
   fprintf(stderr, "**Failed** test #5 (namespace)\n");
   fprintf(stderr, "    Can't find class member variables\n");
   return FAIL;
#endif

   return PASS;
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
