/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

// $Id: test5_4.C,v 1.1 2008/10/30 19:21:04 legendre Exp $
/*
 * #Name: test5_4
 * #Desc: Static Member
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4_test,i386_unknown_linux2_0_test,x86_64_unknown_linux2_4_test,ia64_unknown_linux2_4_test
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"

#include "test_lib.h"

#include "dyninst_comp.h"
class test5_4_Mutator : public DyninstMutator {
public:
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test5_4_factory() {
  return new test5_4_Mutator();
}

//  
// Start Test Case #4 - (static member)
// 
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test5_4_Mutator::executeTest() {

  BPatch_Vector<BPatch_function *> bpfv;
  const char *fn = "static_test::func_cpp";
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
  bool found_func = false;
  
  while ((index < bound) && (vect4_1.size() < 2)) {
    char fn[256];

    // Iterating over function calls in static_test::func_cpp()
    if ((func = (*point4_1)[index]->getCalledFunction()) == NULL) {
      //logerror("**Failed** test #4 (static member)\n");
      //logerror("    Can't find the invoked function\n");
      //return FAILED;
      // not-a-bug
    }
    else if (!strcmp("static_test::call_cpp", func->getName(fn, 256))) {
      found_func = true;
      BPatch_Vector<BPatch_point *> *point4_2 = func->findPoint(BPatch_exit);
      assert(point4_2);
      assert(!point4_2->empty());
      
      // use getComponent to access this "count". However, getComponent is
      // causing core dump at this point
      BPatch_variableExpr *var4_1 = appImage->findVariable(*(*point4_2)[0],
							   "count");
      if (!var4_1) {
	logerror("**Failed** test #4 (static member)\n");
	logerror("  Can't find static variable count\n");
	return FAILED;
      }
      vect4_1.push_back(var4_1);
    }

    index ++;
  }
  if(!found_func) {
    // this is what the above not-a-bug was trying to catch
    logerror("**Failed** test #4 (static member)\n");
    logerror("    Can't find the invoked function\n");
    return FAILED;
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
  const char *fn2 = "static_test::pass";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror("**Failed** test #4 (static member)\n");
    logerror("    Unable to find function %s\n", fn2);
    return FAILED;
  }
  BPatch_function *call4_func = bpfv[0];  

  BPatch_variableExpr *this2 = appImage->findVariable("test5_4_test4");
  if (this2 == NULL) {
    logerror( "**Failed** test #4 (static member)\n");
    logerror( "Unable to find variable \"test5_4_test4\"\n");
    return FAILED;
  }
  
  BPatch_Vector<BPatch_snippet *> call4_args;
  
  BPatch_funcCallExpr call4Expr(*call4_func, call4_args);
  BPatch_constExpr thisExpr((void *)this2->getBaseAddr());
  call4_args.push_back(&thisExpr);
  checkCost(call4Expr);
  appAddrSpace->insertSnippet(call4Expr, *point4_3);
  
  return PASSED;
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
