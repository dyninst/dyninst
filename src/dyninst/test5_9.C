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

// $Id: test5_9.C,v 1.1 2008/10/30 19:21:14 legendre Exp $
/*
 * #Name: test5_9
 * #Desc: Derivation
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4_test,i386_unknown_linux2_0_test,x86_64_unknown_linux2_4_test
 * #Notes:  There are additional test5 subtests, but they weren't enabled
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"
#include "BPatch_point.h"

#include "test_lib.h"

#include "dyninst_comp.h"
class test5_9_Mutator : public DyninstMutator {
public:
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test5_9_factory() {
  return new test5_9_Mutator();
}

//
// Start Test Case #9 - (derivation)
//
// static int mutatorTest(BPatch_thread *, BPatch_image *appImage)
test_results_t test5_9_Mutator::executeTest() {
   bool found = false;
   
  BPatch_Vector<BPatch_function *> bpfv;
  const char *fn = "derivation_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror("**Failed** test #9 (derivation)\n");
    logerror("    Unable to find function %s\n", fn);
    return FAILED;
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point9_1 = f1->findPoint(BPatch_exit);
  if (!point9_1 || (point9_1->size() < 1)) {
    logerror("Unable to find point derivation_test::func_cpp - exit.\n");
    return FAILED;
  }
   
  bpfv.clear();
  const char *fn2 = "main";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror("**Failed** test #9 (derivation)\n");
    logerror("    Unable to find function %s\n", fn2);
    return FAILED;
  }
  BPatch_function *f2 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point9_2 = f2->findPoint(BPatch_allLocations);
  if (!point9_2 || (point9_2->size() < 1)) {
    logerror("Unable to find point in main.\n");
    return FAILED;
  }

   BPatch_variableExpr *expr9_0=appImage->findVariable(*(*point9_2)[0], "test5_9_test9");
   if (!expr9_0) {
      logerror("**Failed** test #9 (derivation)\n");
      logerror("    Unable to locate one of variables\n");
      return FAILED;
   }

   BPatch_Vector<BPatch_variableExpr *> *fields = expr9_0->getComponents();
   if (!fields || fields->size() == 0 ) {
         logerror("**Failed** test #9 (derivation)\n");
         logerror("  struct lacked correct number of elements\n");
         return FAILED;
   }

   int index = 0;
   while ( index < fields->size() ) {
       if ( !strcmp("call_cpp", (*fields)[index]->getName()) ||
           !strcmp("cpp_test_util::call_cpp", (*fields)[index]->getName())) {
          found = true;
          break;
       }
       index ++;
   }
   
   if ( !found ) {
     logerror("**Failed** test #9 (derivation)\n");
     logerror("    Can't find inherited class member functions\n");
     logerror("    Expected call_cpp or cpp_test_util::call_cpp\n");
     index = 0;
     while ( index < fields->size() ) {
       logerror("    Field %d: %s\n", index, (*fields)[index]->getName());
       ++index;
     }
     return FAILED;
   }

   // TODO pass a success message to the mutatee
   const char *passfn = "test5_9_passed";
   BPatch_Vector<BPatch_function *> passfv;
   if ((NULL == appImage->findFunction(passfn, passfv))
       || (passfv.size() < 1) || (NULL == passfv[0])) {
     logerror("**Failed** test #9 (derivation)\n");
     logerror("    Can't find function %s\n", passfn);
     return FAILED;
   }
   BPatch_function *pass_func = passfv[0];
   BPatch_Vector<BPatch_snippet *> pass_args;
   BPatch_funcCallExpr pass_expr(*pass_func, pass_args);
   appAddrSpace->insertSnippet(pass_expr, *point9_1);
   
  return PASSED;
}

// External Interface
// extern "C" TEST_DLL_EXPORT int test5_9_mutatorMAIN(ParameterDict &param)
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
