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

// $Id: test5_5.C,v 1.1 2008/10/30 19:21:06 legendre Exp $
/*
 * #Name: test5_5
 * #Desc: Namespace
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
class test5_5_Mutator : public DyninstMutator {
public:
  test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test5_5_factory() {
  return new test5_5_Mutator();
}

//  
// Start Test Case #5 - (namespace)
// 
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test5_5_Mutator::executeTest() {
  BPatch_Vector<BPatch_function *> bpfv;
  const char *fn = "namespace_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror("**Failed** test #5 (namespace)\n");
    logerror("    Unable to find function %s\n", fn);
    return FAILED;
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point5_1 = f1->findPoint(BPatch_exit);

   assert(point5_1);
   BPatch_variableExpr *var1 = appImage->findVariable(*(*point5_1)[0],
                                                      "local_fn_var");
   BPatch_variableExpr *var2 = appImage->findVariable(*(*point5_1)[0],
                                                      "local_file_var_5_5");
   BPatch_variableExpr *var3 = appImage->findVariable(*(*point5_1)[0],
                                                      "CPP_DEFLT_ARG");
   
   if (!var1 || !var3) {
      logerror("**Failed** test #5 (namespace)\n");
      if (!var1)
         logerror("  can't find local variable local_fn_var\n");
      if (!var2)
         logerror("  can't find file local variable local_file_var_5_5\n");
      if (!var3)
         logerror("  can't find global variable CPP_DEFLT_ARG\n");
      return FAILED;
    }

   // AIX doesn't keep symbols for local variables; however, we can test the
   // remainder of the functionality.
#if !defined(os_aix_test) 
   if (!var2) {
      logerror("**Failed** test #5 (namespace)\n");
      if (!var2)
         logerror("  can't find file local variable local_file_var_5_5\n");
      return FAILED;
   }
#endif

   bpfv.clear();
   const char *fn2 = "main";
   if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	 || NULL == bpfv[0]){
    logerror("**Failed** test #5 (namespace)\n");
    logerror("    Unable to find function %s\n", fn2);
    return FAILED;
  }
  BPatch_function *f2 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point5_2 = f2->findPoint(BPatch_allLocations);

   if (!point5_2 || (point5_2->size() < 1)) {
      logerror("Unable to find point in main.\n");
      return FAILED;
   }
   BPatch_variableExpr *expr5_1=appImage->findVariable(*(*point5_2)[0], "test5_5_test5");
   if (!expr5_1) {
      logerror("**Failed** test #5 (namespace)\n");
      logerror("    Unable to locate test5_5_test5 in main\n");
	  return FAILED;
   }
   
   BPatch_Vector<BPatch_variableExpr *> *fields = expr5_1->getComponents();
   if (!fields || fields->size() == 0 ) {
      logerror("**Failed** test #5 (namespace)\n");
      logerror("  struct lacked correct number of elements\n");
      return FAILED;
   }
   
   logerror (" fields.size () is %d \n", fields->size());

   for(unsigned j = 0; j < fields->size() ; j++) {
      logerror (" field %d name %s \n", j, (*fields)[j]->getName());
   }

   unsigned int index = 0;
   while ( index < fields->size() ) {
      if (!strcmp("class_variable", (*fields)[index]->getName()) ) {

 	BPatch_Vector<BPatch_function *> bpfv4;
	const char *fn4 = "namespace_test::namespace_test";
	if (NULL == appImage->findFunction(fn4, bpfv4) || !bpfv4.size()
	    || NULL == bpfv4[0]){
	  logerror("**Failed** test #5 (namespace)\n");
	  logerror("    Unable to find constructor %s\n", fn4);
	  return FAILED;
	}

	BPatch_Vector<BPatch_function *> bpfv2;
	const char *fn3 = "namespace_test::pass";
	if (NULL == appImage->findFunction(fn3, bpfv2) || !bpfv2.size()
	    || NULL == bpfv2[0]){
	  logerror("**Failed** test #5 (namespace)\n");
	  logerror("    Unable to find function %s\n", fn3);
	  return FAILED;
	}
	BPatch_function *call5_func = bpfv2[0];  
            
         BPatch_variableExpr *this5 = appImage->findVariable("test5_5_test5");
         if (this5 == NULL) {
            logerror("**Failed** test #5 (namespace)\n");
            logerror("Unable to find variable \"test5_5_test5\"\n");
            return FAILED;
         }
         
         BPatch_Vector<BPatch_snippet *> call5_args;
         
         BPatch_constExpr expr5_0((void *)this5->getBaseAddr());
         call5_args.push_back(&expr5_0);
         BPatch_funcCallExpr call5Expr(*call5_func, call5_args);
         checkCost(call5Expr);
         appAddrSpace->insertSnippet(call5Expr, *point5_1);
         return PASSED;
      }
      index ++;
   }
   logerror("**Failed** test #5 (namespace)\n");
   logerror("    Can't find class member variables\n");
   return FAILED;
}

// External Interface
// extern "C" TEST_DLL_EXPORT int test5_5_mutatorMAIN(ParameterDict &param)
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
