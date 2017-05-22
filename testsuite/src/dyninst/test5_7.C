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

// $Id: test5_7.C,v 1.1 2008/10/30 19:21:10 legendre Exp $
/*
 * #Name: test5_7
 * #Desc: Template
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
class test5_7_Mutator : public DyninstMutator {
public:
  virtual test_results_t executeTest();
};
extern "C" DLLEXPORT TestMutator *test5_7_factory() {
  return new test5_7_Mutator();
}

//
// Start Test Case #7 - (template)
//
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
test_results_t test5_7_Mutator::executeTest() {
  BPatch_Vector<BPatch_function *> bpfv;
  const char *fn2 = "template_test::func_cpp";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    logerror("**Failed** test #7 (template)\n");
    logerror("    Unable to find function %s\n", fn2);
    return FAILED;
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point7_1 = f1->findPoint(BPatch_subroutine);
  assert(point7_1);

   int index = 0;
   int flag = 0;
   BPatch_function *func;
   int bound = point7_1->size();
   BPatch_variableExpr *content7_1;
   BPatch_variableExpr *content7_2;

   while (index < bound) {
     if ((func = (*point7_1)[index]->getCalledFunction()) == NULL) {
        logerror("**Failed** test #7 (template)\n");
        logerror("    Can't find the invoked function\n");
        return FAILED;
     }

     char fn[256];
     if (!strcmp("sample_template<int>::content", func->getName(fn, 256))) {
         BPatch_Vector<BPatch_point *> *point7_2 = func->findPoint(BPatch_entry);
         assert(point7_2);

         content7_1 = appImage->findVariable(*(*point7_2)[0], "ret");
         if (!content7_1) {
            logerror("**Failed** test #7 (template)\n");
            logerror("  Can't find local variable ret\n");
            return FAILED;
         }
         flag++;
     } else if (!strcmp("sample_template<char>::content", func->getName(fn, 256))) {
       BPatch_Vector<BPatch_point *> *point7_3 = func->findPoint(BPatch_entry);
       assert(point7_3);

       content7_2 = appImage->findVariable(*(*point7_3)[0], "ret");
       if (!content7_2) {
	 logerror("**Failed** test #7 (template)\n");
	 logerror("  Can't find local variable ret\n");
	 return FAILED;;
       }
       flag++;
     }
     index ++;
  }

  if (flag != 2) {
     logerror("**Failed** test #7 (template)\n");
     return FAILED;
  }

   BPatch_type *type7_0 = appImage->findType("int");
   BPatch_type *type7_1 = const_cast<BPatch_type *> (content7_1->getType());
   BPatch_type *type7_2 = appImage->findType("char");
   BPatch_type *type7_3 = const_cast<BPatch_type *> (content7_2->getType());

   if (!type7_0->isCompatible(type7_1)) {
      logerror("**Failed** test #7 (template)\n");
      logerror("    type7_0 reported as incompatibile with type7_1\n");
      logerror("    '%s' reported as incompatibile with '%s'\n", type7_0->getName(), type7_1->getName());
      logerror("    dataClasses: '%d'/ '%d', dataScalar = %d, dataUnknownType = %d\n", (int) type7_0->getDataClass(), (int)type7_1->getDataClass(), (int) BPatch_dataScalar, (int) BPatch_dataUnknownType);
      return FAILED;;
   }

   if (!type7_2->isCompatible(type7_3)) {
      logerror("**Failed** test #7 (template)\n");
      logerror("    type7_2 reported as incompatibile with type7_3\n");
      return FAILED;;
   }
   
   bpfv.clear();
   const char *fn3 = "test5_7_passed";
   if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
       || NULL == bpfv[0]){
     logerror("**Failed** test #7 (template)\n");
     logerror("    Unable to find function %s\n", fn3);
     return FAILED;;
   }
   if (bpfv.size() > 1) {
     logerror("WARNING:  found %d functions matching '%s'\n", bpfv.size(), fn3);
   }
   BPatch_function *call7_func = bpfv[0];  

   BPatch_Vector<BPatch_point *> *point7_4 = f1->findPoint(BPatch_exit);
   assert(point7_4);

   BPatch_Vector<BPatch_snippet *> call7_args;
   BPatch_funcCallExpr call7Expr(*call7_func, call7_args);

   checkCost(call7Expr);
   appAddrSpace->insertSnippet(call7Expr, *point7_4);
   return PASSED;
}
