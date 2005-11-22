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

// $Id: test5_4.C,v 1.2 2005/11/22 19:42:30 bpellin Exp $
/*
 * #Name: test5_4
 * #Desc: Static Member
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
// Start Test Case #4 - (static member)
// 
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4)

  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "static_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #4 (static member)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn);
    return FAIL;
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point4_1 = f1->findPoint(BPatch_subroutine);
  BPatch_Vector<BPatch_point *> *point4_3 = f1->findPoint(BPatch_exit);
  assert(point4_3);
  assert(point4_1);
  
  int index = 0;
  BPatch_function *func;
  int bound = point4_1->size();
  BPatch_Vector<BPatch_variableExpr *> vect4_1;
  
  while ((index < bound) && (vect4_1.size() < 2)) {
    if ((func = (*point4_1)[index]->getCalledFunction()) == NULL) {
      fprintf(stderr, "**Failed** test #4 (static member)\n");
      fprintf(stderr, "    Can't find the invoked function\n");
      return FAIL;
    }

    char fn[256];
    if (!strcmp("static_test::call_cpp", func->getName(fn, 256))) {
      BPatch_Vector<BPatch_point *> *point4_2 = func->findPoint(BPatch_exit);
      assert(point4_2);
      
      // use getComponent to access this "count". However, getComponent is
      // causing core dump at this point
      BPatch_variableExpr *var4_1 = appImage->findVariable(*(*point4_2)[0],
							   "count");
      if (!var4_1) {
	fprintf(stderr, "**Failed** test #4 (static member)\n");
	fprintf(stderr, "  Can't find static variable count\n");
	return FAIL;
      }
      vect4_1.push_back(var4_1);
    }
    index ++;
  }
  
  if (2 != vect4_1.size()) {
    fprintf(stderr, "**Failed** test #4 (static member)\n");
    fprintf(stderr, "  Incorrect size of an vector\n");
    return FAIL;
  }
  if (vect4_1[0]->getBaseAddr() != vect4_1[1]->getBaseAddr()) {
    fprintf(stderr, "**Failed** test #4 (static member)\n");
    fprintf(stderr, "  Static member does not have a same address\n");
    return FAIL;
  };
  
  bpfv.clear();
  char *fn2 = "static_test_call_cpp";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #4 (static member)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn2);
    return FAIL;
  }
  BPatch_function *call4_func = bpfv[0];  

  BPatch_Vector<BPatch_snippet *> call4_args;
  BPatch_constExpr expr4_0(4);
  call4_args.push_back(&expr4_0);
  BPatch_funcCallExpr call4Expr(*call4_func, call4_args);
  
  checkCost(call4Expr);
  appThread->insertSnippet(call4Expr, *point4_3);
  
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
