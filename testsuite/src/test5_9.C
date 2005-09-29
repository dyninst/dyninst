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

// $Id: test5_9.C,v 1.1 2005/09/29 20:39:32 bpellin Exp $
/*
 * #Name: test5_9
 * #Desc: Derivation
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4,i386_unknown_linux2_0,x86_64_unknown_linux2_4
 * #Notes:  There are additional test5 subtests, but they weren't enabled
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

//
// Start Test Case #9 - (derivation)
//
int mutatorTest(BPatch_thread *, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */
   bool found = false;
   
  BPatch_Vector<BPatch_function *> bpfv;
  char *fn = "derivation_test::func_cpp";
  if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #9 (derivation)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn);
    return FAIL;
  }
  BPatch_function *f1 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point9_1 = f1->findPoint(BPatch_exit);
  if (!point9_1 || (point9_1->size() < 1)) {
    fprintf(stderr, "Unable to find point derivation_test::func_cpp - exit.\n");
    return FAIL;
  }
   
  bpfv.clear();
  char *fn2 = "main";
  if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
      || NULL == bpfv[0]){
    fprintf(stderr, "**Failed** test #9 (derivation)\n");
    fprintf(stderr, "    Unable to find function %s\n", fn2);
    return FAIL;
  }
  BPatch_function *f2 = bpfv[0];  
  BPatch_Vector<BPatch_point *> *point9_2 = f2->findPoint(BPatch_allLocations);
  if (!point9_2 || (point9_2->size() < 1)) {
    fprintf(stderr, "Unable to find point in main.\n");
    return FAIL;
  }

   BPatch_variableExpr *expr9_0=appImage->findVariable(*(*point9_2)[0], "test9");
   if (!expr9_0) {
      fprintf(stderr, "**Failed** test #9 (derivation)\n");
      fprintf(stderr, "    Unable to locate one of variables\n");
      return FAIL;
   }

   BPatch_Vector<BPatch_variableExpr *> *fields = expr9_0->getComponents();
   if (!fields || fields->size() == 0 ) {
         fprintf(stderr, "**Failed** test #9 (derivation)\n");
         fprintf(stderr, "  struct lacked correct number of elements\n");
         return FAIL;
   }

   int index = 0;
   while ( index < fields->size() ) {
       if ( !strcmp("call_cpp", (*fields)[index]->getName()) ) {
          found = true;
          break;
       }
       index ++;
   }
   
   if ( !found ) {
     fprintf(stderr, "**Failed** test #9 (derivation)\n");
     fprintf(stderr, "    Can't find inherited class member functions\n");
     return FAIL;
  }
#endif
  return PASS;
}

// External Interface
extern "C" int mutatorMAIN(ParameterDict &param)
{
    BPatch *bpatch;
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());


    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
