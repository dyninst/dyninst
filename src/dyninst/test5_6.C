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

// $Id: test5_6.C,v 1.1 2008/10/30 19:21:08 legendre Exp $
/*
 * #Name: test5_6
 * #Desc: Exception
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
class test5_6_Mutator : public DyninstMutator {
public:
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test5_6_factory() {
  return new test5_6_Mutator();
}

//  
// Start Test Case #6 - (exception)
// 
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test5_6_Mutator::executeTest() {
  BPatch_Vector<BPatch_function *> bpfv;
  char *fn2 = "exception_test::func_cpp";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror("**Failed** test #6 (exception)\n");
    logerror("    Unable to find function %s\n", fn2);
    return FAILED;
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point6_1 = f1->findPoint(BPatch_subroutine);
  assert(point6_1);
   
  int index = 0;
  BPatch_function *func;
  int bound = point6_1->size();
  // BUG We don't check that (point6_1->size() > 0)
  BPatch_variableExpr *testno = appImage->findVariable(*(*point6_1)[0],
						       "testno");
  if (!testno) {
    logerror("**Failed** test #6 (exception)\n");
    logerror("    Can't find the variable in try branch of exception statement\n");
    return FAILED;
  }

  while (index < bound) {
    if ((func = (*point6_1)[index]->getCalledFunction()) == NULL) {
      index++;
      continue;
    }
    char fn[256];
    func->getName(fn, 256);
    if (!strcmp("sample_exception::response", func->getName(fn, 256))) {
      BPatch_Vector<BPatch_point *> *point6_2 = func->findPoint(BPatch_exit);
      assert(point6_2);

      bpfv.clear();
      char *fn3 = "test5_6_passed";
      if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
	  || NULL == bpfv[0]){
	logerror("**Failed** test #6 (exception)\n");
	logerror("    Unable to find function %s\n", fn3);
	return FAILED;
      }
      BPatch_function *call6_func = bpfv[0];
  
      BPatch_Vector<BPatch_snippet *> call6_args;
      BPatch_constExpr expr6_0(6);
      call6_args.push_back(&expr6_0);
      BPatch_funcCallExpr call6Expr(*call6_func, call6_args);

      checkCost(call6Expr);
      appAddrSpace->insertSnippet(call6Expr, *point6_2);
      return PASSED;
    }
    index++;
  }

  // If we get here, it's because we didn't find the call to
  // sample_exception::response().  That's an error.
  logerror("**Failed** test #6 (exception)\n");
  logerror("    Can't find call to exception object method in catch block\n");
  return FAILED; // FIXME This doesn't look right..
}

// External Interface
// extern "C" TEST_DLL_EXPORT int test5_6_mutatorMAIN(ParameterDict &param)
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
