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

// $Id: test1_22.C,v 1.2 2005/11/22 19:41:38 bpellin Exp $
/*
 * #Name: test1_22
 * #Desc: Mutator Side - Replace Function
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4,i386_unknown_linux2_0,alpha_dec_osf4_0,ia64_unknown_linux2_4,x86_64_unknown_linux2_4
 * #Notes: This test uses libNameA/B magic like test1_21
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

const char *libNameAroot = "libtestA";
const char *libNameBroot = "libtestB";
int mutateeFortran;

char libNameA[128], libNameB[128];
//
// Start Test Case #22 - mutator side (replace function)
//
// There is no corresponding failure (test2) testing because the only
// invalid input to replaceFunction is a non-existent BPatch_function.
// But this is already checked by the "non-existent function" test in
// test2.
int mutatorTest22(BPatch_thread *appThread, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(alpha_dec_osf4_0) \
 || defined(ia64_unknown_linux2_4)

    if (mutateeFortran) {
	return 0;
    }

    char errbuf[1024]; errbuf[0] = '\0';
    BPatch_module *modA = NULL;
    BPatch_module *modB = NULL;

    // Assume that a prior test (mutatorTest21) has loaded the
    // libraries libtestA.so and libtestB.so into the mutator.
    BPatch_Vector<BPatch_module *> *mods = appImage->getModules();
    if (!mods || mods->size() == 0) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't find shlib in mutatee\n");
         return -1;
    }
    // Lookup the libtestA.so and libtestB.so modules
    for (unsigned int i = 0; i < mods->size() && !(modA && modB); i++) {
	 char buf[1024];
	 BPatch_module *m = (*mods)[i];
	 m->getName(buf, 1024);
	 // module names sometimes have "_module" appended
	 if (!strncmp(libNameA, buf, strlen(libNameA)))
	      modA = m;
	 else if (!strncmp(libNameB, buf, strlen(libNameB)))
	      modB = m;
    }
    if (! modA || ! modB) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't find dynamically loaded modules\n");
	 return -1;
    }
    
    //  Mutatee function replacement scheme:

    //  function      module     replaced    global
    //                         or called?   

    //  call22_1       a.out     replaced         1       global is the index
    //  call22_2       a.out       called         1       of the global variable
    //  call22_3       a.out     replaced         2       in test1.mutatee updated
    //  call22_4    libtestA       called         2       by the function
    //  call22_5A   libtestA     replaced         3
    //  call22_5B   libtestB       called         3
    //  call22_6    libtestA     replaced         4
    //  call22_7       a.out       called         4

    // Both of each pair of functions (e.g., call22_1, call22_2)
    // increments a global variable.  The mutatee will test that the
    // variable has been updated only be the "called" function.

    // Replace an a.out with another a.out function

    BPatch_Vector<BPatch_function *> bpfv;
    char *fn = "call22_1";
    char *fn2 = "call22_2";

    if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      fprintf(stderr, "**Failed test #22 (replace function)\n");
      fprintf(stderr, "    Unable to find function %s\n", fn);
      return -1;
    }

    BPatch_function *call22_1func = bpfv[0];

    bpfv.clear();
    if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      fprintf(stderr, "**Failed test #22 (replace function)\n");
      fprintf(stderr, "    Unable to find function %s\n", fn2);
      return -1;
    }

    BPatch_function *call22_2func = bpfv[0];

    if (! appThread->replaceFunction(*call22_1func, *call22_2func)) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't replaceFunction (a.out -> a.out)\n");
	 return -1;
    }

    // Replace an a.out function with a shlib function
    bpfv.clear();
    char *fn3 = "call22_3";
    if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      fprintf(stderr, "**Failed test #22 (replace function)\n");
      fprintf(stderr, "    Unable to find function %s\n", fn3);
      return -1;
    }

    BPatch_function *call22_3func = bpfv[0];

    BPatch_Vector<BPatch_function *> bpmv;
    if (NULL == modA->findFunction("call22_4", bpmv) || !bpmv.size()) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't find functions in mutatee\n");
	 return -1;
    }
    BPatch_function *call22_4func = bpmv[0];
    
    if (! appThread->replaceFunction(*call22_3func, *call22_4func)) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't replaceFunction (a.out -> shlib)\n");
	 return -1;
    }

    // Replace a shlib function with a shlib function
    bpmv.clear();
    if (NULL == modA->findFunction("call22_5", bpmv) || !bpmv.size()) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't find functions in mutatee\n");
	 return -1;
    }
    BPatch_function *call22_5Afunc = bpmv[0];

    bpmv.clear();
    if (NULL == modB->findFunction("call22_5", bpmv) || !bpmv.size()) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't find functions in mutatee\n");
	 return -1;
    }
    BPatch_function *call22_5Bfunc = bpmv[0];

    if (! appThread->replaceFunction(*call22_5Afunc, *call22_5Bfunc)) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't replaceFunction (shlib -> shlib)\n");
    }

    // Replace a shlib function with an a.out function
    bpmv.clear();
    if (NULL == modA->findFunction("call22_6", bpmv) || !bpmv.size()) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't find functions in mutatee\n");
	 return -1;
    }
    BPatch_function *call22_6func = bpmv[0];

    bpfv.clear();
    char *fn4 = "call22_7";
    if (NULL == appImage->findFunction(fn4, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      fprintf(stderr, "**Failed test #22 (replace function)\n");
      fprintf(stderr, "    Unable to find function %s\n", fn4);
      return -1;
    }
    BPatch_function *call22_7func = bpfv[0];

    if (! appThread->replaceFunction(*call22_6func, *call22_7func)) {
	 fprintf(stderr, "**Failed test #22 (replace function)\n");
	 fprintf(stderr, "  Mutator couldn't replaceFunction (shlib -> a.out)\n");
	 return -1;
    }

#endif
    return 0;
}


// Wrapper to call readyTest
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
   strncpy(libNameA, libNameAroot, 128);
   addLibArchExt(libNameA,128);
   strncpy(libNameB, libNameBroot, 128);
   addLibArchExt(libNameB,128);
   RETURNONFAIL(readyTest21or22(appThread, libNameA, libNameB));
   return mutatorTest22(appThread, appImage);

}

// External Interface
extern "C" TEST_DLL_EXPORT int mutatorMAIN(ParameterDict &param)
{
    BPatch *bpatch;
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());


    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    mutateeFortran = isMutateeFortran(appImage);

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
