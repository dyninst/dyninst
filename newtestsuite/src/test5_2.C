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

// $Id: test5_2.C,v 1.1 2007/09/24 16:40:01 cooksey Exp $
/*
 * #Name: test5_2
 * #Desc: Overload Functions
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
class test5_2_Mutator : public TestMutator {
public:
  virtual test_results_t preExecution();
};
extern "C" TEST_DLL_EXPORT TestMutator *test5_2_factory() {
  return new test5_2_Mutator();
}

//
// Start Test Case #2 - (overload function)
// 
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test5_2_Mutator::preExecution() {

#if defined(sparc_sun_solaris2_4) \
    || defined(os_linux) \
    || defined(os_windows)

  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "overload_func_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror( "**Failed** test #2 (overloaded functions)\n");
    logerror( "    Unable to find function %s\n", fn);
    return FAILED;
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point2_1 = f1->findPoint(BPatch_subroutine);

  // Shouldn't this size comparison be < 3?  There's three function calls..
  if (!point2_1 || (point2_1->size() < 3)) {
    logerror( "Unable to find point overload_func_test::func_cpp - calls. \n");
    return FAILED;
  }

  BPatch_Vector<BPatch_point *> *point2_3 = f1->findPoint(BPatch_exit);
  if (!point2_3 || point2_3->size() <1) {
    logerror( "Unable to find point overload_func_test::func_cpp - exit.\n");
    return FAILED;
  }

    for (unsigned int n=0; n<point2_1->size(); n++) {
       BPatch_function *func;
    
       if ((func = (*point2_1)[n]->getCalledFunction()) == NULL) continue;

       char fn[256];
       if (func->getName(fn, 256) == NULL) {
            logerror( "**Failed** test #2 (overloaded functions)\n");
            logerror( "    Can't get name of called function in overload_func_test::func_cpp\n");
            return FAILED;
       }
       if (strcmp(fn, "overload_func_test::call_cpp")) {
           logerror( "**Failed** test #2 (overloaded functions)\n");
           logerror( "    The called function was named \"%s\""
                           " not \"overload_func_test::call_cpp\"\n", fn);
           //return FAILED;
           continue;
       }       
       BPatch_Vector<BPatch_point *> *point2_2 = func->findPoint(BPatch_entry);
       BPatch_Vector<BPatch_localVar *> *param = func->getParams();
       assert(point2_2 && param);

       switch (n) {
          case 0 : {

	      if ( (param->size() == 1) ||
	           ((param->size() == 2) && (!strcmp((*param)[0]->getName(), "this"))) ) 
		 //First param might be "this"!
		 break;
	      else {
                 logerror( "**Failed** test #2 (overloaded functions)\n");
                 logerror( "    The overloaded function has wrong number of parameters %d\n", param->size());
                 return FAILED;
              }
          }
          case 1 : {
              if ( (param->size() == 1) ||
                   ((param->size() == 2) && (!strcmp((*param)[0]->getName(), "this"))) )
                 //First param might be "this"!
                 break;
              else {
                 logerror( "**Failed** test #2 (overloaded functions)\n"); 
                 logerror( "    The overloaded function has wrong number of parameters\n");
                 return FAILED;
              }
          }
          case 2 : {
              if ( (param->size() == 2) ||
                   ((param->size() == 3) && (!strcmp((*param)[0]->getName(), "this"))) )
                 //First param might be "this"!
                 break;
              else {
                 logerror( "**Failed** test #2 (overloaded functions)\n"); 
                 logerror( "    The overloaded function has wrong number of parameters\n");
                 return FAILED;
              }
          }
          default : {
              logerror( "**Failed** test #2 (overloaded functions)\n");
              logerror( "    Incorrect number of subroutine calls from overload_func_test::func_cpp\n");
              return FAILED;
          }
       };
    }

    bpfv.clear();  
    char *fn2 = "overload_func_test::pass";
    if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      logerror( "**Failed** test #2 (overloaded functions)\n");
      logerror( "    Unable to find function %s\n", fn2);
      return FAILED;
    }
    BPatch_function *call2_func = bpfv[0];  

    BPatch_variableExpr *this2 = appImage->findVariable("test2");
    if (this2 == NULL) {
       logerror( "**Failed** test #2 (overloaded functions)\n");
       logerror( "Unable to find variable \"test2\"\n");
       return FAILED;
    }

    BPatch_Vector<BPatch_snippet *> call2_args;
    BPatch_constExpr expr2_0((void *)this2->getBaseAddr());
    call2_args.push_back(&expr2_0);
    BPatch_funcCallExpr call2Expr(*call2_func, call2_args);

    checkCost(call2Expr);
    appThread->insertSnippet(call2Expr, *point2_3);
    return PASSED;
#else // Skipped on unsupported platforms
    return SKIPPED;
#endif
}

// External Interface
// extern "C" TEST_DLL_EXPORT int test5_2_mutatorMAIN(ParameterDict &param)
// {
//     BPatch *bpatch;
//     bpatch = (BPatch *)(param["bpatch"]->getPtr());
//     BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());

//     // Read the program's image and get an associated image object
//     BPatch_image *appImage = appThread->getImage();

//     // Run mutator code
//     return mutatorTest(appThread, appImage);
// }
