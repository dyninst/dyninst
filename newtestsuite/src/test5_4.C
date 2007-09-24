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

// $Id: test5_4.C,v 1.1 2007/09/24 16:40:05 cooksey Exp $
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

#include "TestMutator.h"
class test5_4_Mutator : public TestMutator {
public:
  virtual test_results_t preExecution();
};
extern "C" TEST_DLL_EXPORT TestMutator *test5_4_factory() {
  return new test5_4_Mutator();
}

//  
// Start Test Case #4 - (static member)
// 
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test5_4_Mutator::preExecution() {
#if defined(os_solaris) || defined(os_linux) || defined(os_windows)

  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "static_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror("**Failed** test #4 (static member)\n");
    logerror("    Unable to find function %s\n", fn);
    return FAILED;
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
  
  //fprintf(stderr, "[%s:%u] - index = %d, bound = %d, vect4_1.size() = %u\n", __FILE__, __LINE__, index, bound, vect4_1.size()); /*DEBUG*/
  while ((index < bound) && (vect4_1.size() < 2)) {
    // Iterating over function calls in static_test::func_cpp()
    //fprintf(stderr, "[%s:%u] - iterating over function calls in static_test::func_cpp()\n", __FILE__, __LINE__); /*DEBUG*/
    if ((func = (*point4_1)[index]->getCalledFunction()) == NULL) {
      logerror("**Failed** test #4 (static member)\n");
      logerror("    Can't find the invoked function\n");
      return FAILED;
    }

    char fn[256];
    if (!strcmp("static_test::call_cpp", func->getName(fn, 256))) {
      //fprintf(stderr, "[%s:%u] - found static_test::call_cpp()\n", __FILE__, __LINE__); /*DEBUG*/
      BPatch_Vector<BPatch_point *> *point4_2 = func->findPoint(BPatch_exit);
      assert(point4_2);
      
      // use getComponent to access this "count". However, getComponent is
      // causing core dump at this point
      BPatch_variableExpr *var4_1 = appImage->findVariable(*(*point4_2)[0],
							   "count");
      if (!var4_1) {
	logerror("**Failed** test #4 (static member)\n");
	logerror("  Can't find static variable count\n");
	return FAILED;
      }
      //fprintf(stderr, "[%s:%u] - found count variable\n", __FILE__, __LINE__); /*DEBUG*/
      vect4_1.push_back(var4_1);
    } else {
      //fprintf(stderr, "[%s:%u] - found function '%s'\n", __FILE__, __LINE__, fn); /*DEBUG*/
    }

    index ++;
  }

  if (2 != vect4_1.size()) {
    logerror("**Failed** test #4 (static member)\n");
    logerror("  Incorrect size of a vector\n");
    return FAILED;
  }
  if (vect4_1[0]->getBaseAddr() != vect4_1[1]->getBaseAddr()) {
    logerror("**Failed** test #4 (static member)\n");
    logerror("  Static member does not have a same address\n");
    return FAILED;
  };
  
  bpfv.clear();
  char *fn2 = "static_test::pass";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror("**Failed** test #4 (static member)\n");
    logerror("    Unable to find function %s\n", fn2);
    return FAILED;
  }
  BPatch_function *call4_func = bpfv[0];  

  BPatch_variableExpr *this2 = appImage->findVariable("test4");
  if (this2 == NULL) {
    logerror( "**Failed** test #4 (static member)\n");
    logerror( "Unable to find variable \"test4\"\n");
    return FAILED;
  }
  
  BPatch_Vector<BPatch_snippet *> call4_args;
  
  BPatch_funcCallExpr call4Expr(*call4_func, call4_args);
  BPatch_constExpr thisExpr((void *)this2->getBaseAddr());
  call4_args.push_back(&thisExpr);
  checkCost(call4Expr);
  appThread->insertSnippet(call4Expr, *point4_3);
  
  return PASSED;
#else
  // Not a platform we support this test on
  return SKIPPED;
#endif
}

// External Interface
// extern "C" TEST_DLL_EXPORT int test5_4_mutatorMAIN(ParameterDict &param)
// {
//     BPatch *bpatch;
//     bpatch = (BPatch *)(param["bpatch"]->getPtr());
//     BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());

//     // Get log file pointers
//     FILE *outlog = (FILE *)(param["outlog"]->getPtr());
//     FILE *errlog = (FILE *)(param["errlog"]->getPtr());
//     setOutputLog(outlog);
//     setErrorLog(errlog);

//     // Read the program's image and get an associated image object
//     BPatch_image *appImage = appThread->getImage();

//     // Run mutator code
//     return mutatorTest(appThread, appImage);
// }
