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

// $Id: test5_1.C,v 1.1 2008/10/30 19:20:58 legendre Exp $
/*
 * #Name: test5_1
 * #Desc: C++ Argument Pass
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

class test5_1_Mutator : public DyninstMutator {
public:
  //  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t executeTest();
};

extern "C" DLLEXPORT TestMutator *test5_1_factory() {
  return new test5_1_Mutator();
}

//  
// Start Test Case #1 - (C++ argument pass)
//       
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test5_1_Mutator::executeTest() {

  BPatch_Vector<BPatch_function *> bpfv;
  const char *fn = "arg_test::call_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror("**Failed** test #1\n");
    logerror("    Unable to find function %s\n", fn);
    return FAILED;
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point1_1 = f1->findPoint(BPatch_subroutine);
       
   assert(point1_1);
   assert((*point1_1)[0]);

   // check the paramter passing modes
   BPatch_variableExpr *arg0 = appImage->findVariable(*(*point1_1)[0],
       "reference");
   BPatch_variableExpr *arg1 = appImage->findVariable(*(*point1_1)[0],
       "arg1");
   BPatch_variableExpr *arg2 = appImage->findVariable(*(*point1_1)[0],
       "arg2");
   BPatch_variableExpr *arg3 = appImage->findVariable(*(*point1_1)[0],
       "arg3");
   BPatch_variableExpr *arg4 = appImage->findVariable(*(*point1_1)[0],
       "m");

   if (!arg0 || !arg1 || !arg2 || !arg3 || !arg4) {
      logerror("**Failed** test #1 (argument passing)\n");
      if ( !arg0 )
         logerror("  can't find local variable 'reference'\n");
      if ( !arg1 )
         logerror("  can't find local variable 'arg1'\n");
      if ( !arg2 )
         logerror("  can't find local variable 'arg2'\n");
      if ( !arg3 )
         logerror("  can't find local variable 'arg3'\n");
      if ( !arg4 )
         logerror("  can't find local variable 'm'\n");
      return FAILED;
   }

   BPatch_type *type1_0 = const_cast<BPatch_type *> (arg0->getType());
   BPatch_type *type1_1 = const_cast<BPatch_type *> (arg1->getType());
   BPatch_type *type1_2 = const_cast<BPatch_type *> (arg2->getType());
   BPatch_type *type1_3 = const_cast<BPatch_type *> (arg4->getType());
   assert(type1_0 && type1_1 && type1_2 && type1_3);

   if (!type1_1->isCompatible(type1_3)) {
       logerror("**Failed** test #1 (C++ argument pass)\n");
       logerror("    type1_1 reported as incompatibile with type1_3\n");
       return FAILED;
   }

   if (!type1_2->isCompatible(type1_0)) {
        logerror("**Failed** test #1 (C++ argument pass)\n");
        logerror("    type1_2 reported as incompatibile with type1_0\n");
        return FAILED;
   }

   BPatch_arithExpr expr1_1(BPatch_assign, *arg3, BPatch_constExpr(1));
   checkCost(expr1_1);
   appAddrSpace->insertSnippet(expr1_1, *point1_1);

   // pass a paramter to a class member function
   bpfv.clear();
   const char *fn2 = "arg_test::func_cpp";
   if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
       || NULL == bpfv[0]){
     logerror("**Failed** test #1 (C++ argument pass)\n");
     logerror("    Unable to find function %s\n", fn2);
     return FAILED;
   }
   BPatch_function *f2 = bpfv[0];  
   BPatch_Vector<BPatch_point *> *point1_2 = f2->findPoint(BPatch_subroutine);

   if (!point1_2 || (point1_2->size() < 1)) {
     logerror("**Failed** test #1 (C++ argument pass)\n");
      logerror("Unable to find point arg_test::func_cpp - exit.\n");
      return FAILED;
   }

   bpfv.clear();
   const char *fn3 = "arg_test::arg_pass";
   if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
       || NULL == bpfv[0]) {
     logerror("**Failed** test #1 (C++ argument pass)\n");
     logerror("    Unable to find function %s\n", fn3);
     return FAILED;
   }
   BPatch_function *call1_func = bpfv[0];  

   BPatch_variableExpr *this1 = appImage->findVariable("test1");
   if (this1 == NULL) {
      logerror("**Failed** test #1 (C++ argument pass)\n");
      logerror("Unable to find variable \"test1\"\n");
      return FAILED;
   }

   BPatch_Vector<BPatch_snippet *> call1_args;
   BPatch_constExpr expr1_2((const void *)this1->getBaseAddr());
   call1_args.push_back(&expr1_2);
   BPatch_constExpr expr1_3(1);
   call1_args.push_back(&expr1_3);
   BPatch_funcCallExpr call1Expr(*call1_func, call1_args);

   checkCost(call1Expr);
   appAddrSpace->insertSnippet(call1Expr, *point1_2);
   return PASSED;
}
