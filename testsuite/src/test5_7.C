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

// $Id: test5_7.C,v 1.2 2005/11/22 19:42:33 bpellin Exp $
/*
 * #Name: test5_7
 * #Desc: Template
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4,i386_unknown_linux2_0,x86_64_unknown_linux2_4,ia64_unknown_linux2_4
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"


//
// Start Test Case #7 - (template)
//
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4)

  BPatch_Vector<BPatch_function *> bpfv;
  char *fn2 = "template_test::func_cpp";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #7 (template)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn2);
    return FAIL;
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
        fprintf(stderr, "**Failed** test #7 (template)\n");
        fprintf(stderr, "    Can't find the invoked function\n");
        return FAIL;
     }

     char fn[256];
     if (!strcmp("sample_template<int>::content", func->getName(fn, 256))) {
         BPatch_Vector<BPatch_point *> *point7_2 = func->findPoint(BPatch_entry);
         assert(point7_2);

         content7_1 = appImage->findVariable(*(*point7_2)[0], "ret");
         if (!content7_1) {
            fprintf(stderr, "**Failed** test #7 (template)\n");
            fprintf(stderr, "  Can't find local variable ret\n");
            return FAIL;
         }
         flag++;
     } else if (!strcmp("sample_template<char>::content", func->getName(fn, 256))) {

            BPatch_Vector<BPatch_point *> *point7_3 = func->findPoint(BPatch_entry);
            assert(point7_3);

            content7_2 = appImage->findVariable(*(*point7_3)[0], "ret");
            if (!content7_2) {
               fprintf(stderr, "**Failed** test #7 (template)\n");
               fprintf(stderr, "  Can't find local variable ret\n");
	       return FAIL;;
            }
            flag++;
     }
     index ++;
  }

  if (flag != 2) {
     fprintf(stderr, "**Failed** test #7 (template)\n");
     return FAIL;;
  }

   BPatch_type *type7_0 = appImage->findType("int");
   BPatch_type *type7_1 = const_cast<BPatch_type *> (content7_1->getType());
   BPatch_type *type7_2 = appImage->findType("char");
   BPatch_type *type7_3 = const_cast<BPatch_type *> (content7_2->getType());

   if (!type7_0->isCompatible(type7_1)) {
      fprintf(stderr, "**Failed** test #7 (template)\n");
      fprintf(stderr,"    type7_0 reported as incompatibile with type7_1\n");
      fprintf(stderr,"    '%s' reported as incompatibile with '%s'\n", type7_0->getName(), type7_1->getName());
      fprintf(stderr,"    dataClasses: '%d'/ '%d', dataScalar = %d, dataUnknownType = %d\n", (int) type7_0->getDataClass(), (int)type7_1->getDataClass(), (int) BPatch_dataScalar, (int) BPatch_dataUnknownType);
      return FAIL;;
   }

   if (!type7_2->isCompatible(type7_3)) {
      fprintf(stderr, "**Failed** test #7 (template)\n");
      fprintf(stderr,"    type7_2 reported as incompatibile with type7_3\n");
      return FAIL;;
   }
   
   bpfv.clear();
   char *fn3 = "template_test_call_cpp";
   if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
       || NULL == bpfv[0]){
     fprintf(stderr, "**Failed** test #7 (template)\n");
     fprintf(stderr, "    Unable to find function %s\n", fn3);
     return FAIL;;
   }
   if (bpfv.size() > 1) {
     fprintf(stderr, "WARNING:  found %d functions matching '%s'\n", bpfv.size(), fn3);
   }
   BPatch_function *call7_func = bpfv[0];  

   BPatch_Vector<BPatch_point *> *point7_4 = f1->findPoint(BPatch_exit);
   assert(point7_4);

   BPatch_Vector<BPatch_snippet *> call7_args;
   BPatch_constExpr expr7_0(7);
   call7_args.push_back(&expr7_0);
   BPatch_funcCallExpr call7Expr(*call7_func, call7_args);

   checkCost(call7Expr);
   appThread->insertSnippet(call7Expr, *point7_4);
#endif

   return PASS;
}

// External Interface
extern "C" TEST_DLL_EXPORT int mutatorMAIN(ParameterDict &param)
{
    BPatch *bpatch;
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());


    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
