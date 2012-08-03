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

// $Id: test5_8.C,v 1.1 2008/10/30 19:21:12 legendre Exp $
/*
 * #Name: test5_8
 * #Desc: Declaration
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
class test5_8_Mutator : public DyninstMutator {
public:
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test5_8_factory() {
  return new test5_8_Mutator();
}

//
// Start Test Case #8 - (declaration)
//   
test_results_t test5_8_Mutator::executeTest() {
  // Find the exit point to the procedure "func_cpp"
  BPatch_Vector<BPatch_function *> bpfv;
  const char *fn = "decl_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror( "**Failed** test #8 (declaration)\n");
    logerror( "    Unable to find function %s\n", fn);
    return FAILED;
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point8_1 = f1->findPoint(BPatch_exit);
  if (!point8_1 || (point8_1->size() < 1)) {
    logerror( "Unable to find point decl_test::func_cpp - exit.\n");
    return FAILED;
  }

  bpfv.clear();
  const char *fn2 = "main";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror( "**Failed** test #8 (declaration)\n");
    logerror( "    Unable to find function %s\n", fn2);
    return FAILED;
  }
  BPatch_function *f2 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point8_2 = f2->findPoint(BPatch_allLocations);
  if (!point8_2 || (point8_2->size() < 1)) {
    logerror( "Unable to find point in main.\n");
    return FAILED;
  }

  bpfv.clear();
  const char *fn3 = "decl_test::call_cpp";
  if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror( "**Failed** test #8 (declaration)\n");
    logerror( "    Unable to find function %s\n", fn3);
    return FAILED;
  }
  BPatch_function *call8_func = bpfv[0];  

  BPatch_variableExpr *this8 = appImage->findVariable("test5_8_test8");
  if (this8 == NULL) {
    logerror( "**Failed** test #8 (declaration)\n");
    logerror( "Unable to find variable \"test5_8_test8\"\n");
    return FAILED;
  }

  BPatch_Vector<BPatch_snippet *> call8_args;
  BPatch_constExpr expr8_0((void *)this8->getBaseAddr());
  call8_args.push_back(&expr8_0);
  BPatch_constExpr expr8_1(8);
  call8_args.push_back(&expr8_1);
  BPatch_funcCallExpr call8Expr(*call8_func, call8_args);

  // find the variables of different scopes
  // What *exactly* are we testing here?  Just finding variables of various
  // types with different point parameters to findVariable()?
  BPatch_variableExpr *expr8_2=appImage->findVariable("CPP_DEFLT_ARG");
  BPatch_variableExpr *expr8_3=appImage->findVariable(*(*point8_2)[0], "test5_8_test8");
  BPatch_variableExpr *expr8_4=appImage->findVariable(*(*point8_1)[0], "CPP_DEFLT_ARG");
  if (!expr8_2 || !expr8_3 || !expr8_4) {
    logerror( "**Failed** test #8 (delcaration)\n");
    logerror( "    Unable to locate one of variables\n");
    return FAILED;
  }

    BPatch_Vector<BPatch_variableExpr *> *fields = expr8_3->getComponents();
    if (!fields || fields->size() == 0 ) {
          logerror( "**Failed** test #8 (declaration)\n");
          logerror( "  struct lacked correct number of elements\n");
          return FAILED;
     }

    unsigned int index = 0;
    while ( index < fields->size() ) {
	char fieldName[100];
	strcpy(fieldName, (*fields)[index]->getName());
       if ( !strcmp("CPP_TEST_UTIL_VAR", (*fields)[index]->getName()) ) {
           dprintf("Inserted snippet2\n");
           checkCost(call8Expr);
	   BPatchSnippetHandle *handle;
           handle = appAddrSpace->insertSnippet(call8Expr, *point8_1);
           return PASSED;
       }
       
       index ++;
    }
    logerror( "**Failed** test #8 (declaration)\n");
    logerror( "    Can't find inherited class member variables\n");
    return FAILED;
}
