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

// $Id: test5_3.C,v 1.1 2008/10/30 19:21:02 legendre Exp $
/*
 * #Name: test5_3
 * #Desc: Overload Operatior
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"

#include "test_lib.h"

#include "dyninst_comp.h"
class test5_3_Mutator : public DyninstMutator {
public:
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test5_3_factory() {
  return new test5_3_Mutator();
}

//
// Start Test Case #3 - (overload operator)
//      
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test5_3_Mutator::executeTest() {

  BPatch_Vector<BPatch_function *> bpfv;
  const char *fn = "overload_op_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror("**Failed** test #3 (overloaded operation)\n");
    logerror("    Unable to find function %s\n", fn);
    return FAILED;
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point3_1 = f1->findPoint(BPatch_subroutine);

  assert(point3_1);
  if (point3_1->size() == 0) {
    logerror("**Failed test5_3 (overload operation)\n");
    logerror("    Can't find overloaded operator call points\n");
    return FAILED;
  }

  unsigned int index = 0;
  char fn3[256];
  BPatch_function *func;
  while (index < point3_1->size()) {
     if ((func = (*point3_1)[index]->getCalledFunction()) != NULL &&
         !strcmp("overload_op_test::operator++", func->getName(fn3, 256)))
     {
        break;
     }
     index ++;
  }
  if(!func) {
    logerror("**Failed** test #3 (overload operation)\n");
    logerror("    Can't find the overload operator\n");
    return FAILED;
  }

  if (0 != strcmp("overload_op_test::operator++", func->getName(fn3, 256))) {
    logerror("**Failed** test #3 (overload operation)\n");
    logerror("    Can't find the overloaded operator\n");
    return FAILED;
  }
  // FIXME It caught fprintf...

  BPatch_Vector<BPatch_point *> *point3_2 = func->findPoint(BPatch_exit);
  assert(point3_2);
  
  bpfv.clear();
  const char *fn2 = "overload_op_test::call_cpp";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror("**Failed** test #3 (overloaded operation)\n");
    logerror("    Unable to find function %s\n", fn2);
    return FAILED;
  }
  BPatch_function *call3_1 = bpfv[0];  
  
  BPatch_variableExpr *this2 = appImage->findVariable("test5_3_test3");
  if (this2 == NULL) {
    logerror( "**Failed** test #3 (overloaded operation)\n");
    logerror( "Unable to find variable \"test5_3_test3\"\n");
    return FAILED;
  }

  BPatch_Vector<BPatch_snippet *> opArgs;
  BPatch_constExpr expr2_0((void *)this2->getBaseAddr());
  opArgs.push_back(&expr2_0);
  opArgs.push_back(new BPatch_retExpr());
  BPatch_funcCallExpr call3_1Expr(*call3_1, opArgs);
  
  checkCost(call3_1Expr);
  appAddrSpace->insertSnippet(call3_1Expr, *point3_2);
  //  int tag = 1;
  //  while (tag != 0) {}
  
  return PASSED;
}

// External Interface
// extern "C" TEST_DLL_EXPORT int test5_3_mutatorMAIN(ParameterDict &param)
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
