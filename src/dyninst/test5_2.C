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

// $Id: test5_2.C,v 1.1 2008/10/30 19:21:00 legendre Exp $
/*
 * #Name: test5_2
 * #Desc: Overload Functions
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
class test5_2_Mutator : public DyninstMutator {
public:
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test5_2_factory() {
  return new test5_2_Mutator();
}

//
// Start Test Case #2 - (overload function)
// 
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test5_2_Mutator::executeTest() {

  BPatch_Vector<BPatch_function *> bpfv;
  const char *fn = "overload_func_test::func_cpp";
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
                 logerror( "    The overloaded function %s has wrong number of parameters\n", func->getMangledName().c_str());
		 logerror( "    expected 1, actual %d, possible 'this' parameter %s\n", param->size(), 
			 param->size() ? (*param)[0]->getName() : "<NO PARAMS>");
		 if(param->size() == 2) logerror( "    second parameter %s\n", (*param)[1]->getName());
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
    const char *fn2 = "overload_func_test::pass";
    if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      logerror( "**Failed** test #2 (overloaded functions)\n");
      logerror( "    Unable to find function %s\n", fn2);
      return FAILED;
    }
    BPatch_function *call2_func = bpfv[0];  

    BPatch_variableExpr *this2 = appImage->findVariable("test5_2_test2");
    if (this2 == NULL) {
       logerror( "**Failed** test #2 (overloaded functions)\n");
       logerror( "Unable to find variable \"test5_2_test2\"\n");
       return FAILED;
    }

    BPatch_Vector<BPatch_snippet *> call2_args;
    BPatch_constExpr expr2_0((void *)this2->getBaseAddr());
    call2_args.push_back(&expr2_0);
    BPatch_funcCallExpr call2Expr(*call2_func, call2_args);

    checkCost(call2Expr);
    if (appAddrSpace->insertSnippet(call2Expr, *point2_3) == NULL) {
    }
    return PASSED;
}
