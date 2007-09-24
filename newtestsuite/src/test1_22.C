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

// $Id: test1_22.C,v 1.1 2007/09/24 16:37:24 cooksey Exp $
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

// static const char *libNameAroot = "libtestA";
// static const char *libNameBroot = "libtestB";
// static int mutateeFortran;

// static char libNameA[128], libNameB[128];

#include "TestMutator.h"

class test1_22_Mutator : public TestMutator {
  const char *libNameAroot;
  const char *libNameBroot;
  char libNameA[128], libNameB[128];
  
public:
  test1_22_Mutator();
  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t preExecution();
  virtual test_results_t mutatorTest22();
};
test1_22_Mutator::test1_22_Mutator() : libNameAroot("libtestA"),
				       libNameBroot("libtestB") {
}
extern "C" TEST_DLL_EXPORT TestMutator *test1_22_factory() {
  return new test1_22_Mutator();
}

//
// Start Test Case #22 - mutator side (replace function)
//
// There is no corresponding failure (test2) testing because the only
// invalid input to replaceFunction is a non-existent BPatch_function.
// But this is already checked by the "non-existent function" test in
// test2.
// static int mutatorTest22(BPatch_thread *appThread, BPatch_image *appImage)
// {
test_results_t test1_22_Mutator::mutatorTest22() {
#if defined(os_solaris) \
 || defined(alpha_dec_osf4_0) \
 || defined(os_linux) \
 || defined(os_windows)

    char errbuf[1024]; errbuf[0] = '\0';
    BPatch_module *modA = NULL;
    BPatch_module *modB = NULL;

    // Assume that a prior test (mutatorTest21) has loaded the
    // libraries libtestA.so and libtestB.so into the mutator.
    BPatch_Vector<BPatch_module *> *mods = appImage->getModules();
    if (!mods || mods->size() == 0) {
	 logerror("**Failed test #22 (replace function)\n");
	 logerror("  Mutator couldn't find shlib in mutatee\n");
         return FAILED;
    }
    // Lookup the libtestA.so and libtestB.so modules
    for (unsigned int i = 0; i < mods->size() && !(modA && modB); i++) {
	 char buf[1024];
	 BPatch_module *m = (*mods)[i];
	 m->getName(buf, 1024);
	 // module names sometimes have "_module" appended
	 if (!strcmpcase(libNameA, buf))
	      modA = m;
	 else if (!strcmpcase(libNameB, buf))
	      modB = m;
    }
    if (! modA || ! modB) {
	 logerror("**Failed test #22 (replace function)\n");
	 logerror("  Mutator couldn't find dynamically loaded modules\n");
	 return FAILED;
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
    char *fn = "test1_22_call1";
    char *fn2 = "test1_22_call2";

    if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      logerror("**Failed test #22 (replace function)\n");
      logerror("    Unable to find function %s\n", fn);
      return FAILED;
    }

    BPatch_function *call22_1func = bpfv[0];

    bpfv.clear();
    if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      logerror("**Failed test #22 (replace function)\n");
      logerror("    Unable to find function %s\n", fn2);
      return FAILED;
    }

    BPatch_function *call22_2func = bpfv[0];

    if (! appThread->replaceFunction(*call22_1func, *call22_2func)) {
	 logerror("**Failed test #22 (replace function)\n");
	 logerror("  Mutator couldn't replaceFunction (a.out -> a.out)\n");
	 return FAILED;
    }

    // Replace an a.out function with a shlib function
    bpfv.clear();
    char *fn3 = "test1_22_call3";
    if (NULL == appImage->findFunction(fn3, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      logerror("**Failed test #22 (replace function)\n");
      logerror("    Unable to find function %s\n", fn3);
      return FAILED;
    }

    BPatch_function *call22_3func = bpfv[0];

    BPatch_Vector<BPatch_function *> bpmv;
    if (NULL == modA->findFunction("call22_4", bpmv) || !bpmv.size()) {
	 logerror("**Failed test #22 (replace function)\n");
	 logerror("  Mutator couldn't find functions in mutatee\n");
	 return FAILED;
    }
    BPatch_function *call22_4func = bpmv[0];
    
    if (! appThread->replaceFunction(*call22_3func, *call22_4func)) {
	 logerror("**Failed test #22 (replace function)\n");
	 logerror("  Mutator couldn't replaceFunction (a.out -> shlib)\n");
	 return FAILED;
    }

    // Replace a shlib function with a shlib function
    bpmv.clear();
    if (NULL == modA->findFunction("call22_5", bpmv) || !bpmv.size()) {
	 logerror("**Failed test #22 (replace function)\n");
	 logerror("  Mutator couldn't find functions in mutatee\n");
	 return FAILED;
    }
    BPatch_function *call22_5Afunc = bpmv[0];

    bpmv.clear();
    if (NULL == modB->findFunction("call22_5", bpmv) || !bpmv.size()) {
	 logerror("**Failed test #22 (replace function)\n");
	 logerror("  Mutator couldn't find functions in mutatee\n");
	 return FAILED;
    }
    BPatch_function *call22_5Bfunc = bpmv[0];

    if (! appThread->replaceFunction(*call22_5Afunc, *call22_5Bfunc)) {
	 logerror("**Failed test #22 (replace function)\n");
	 logerror("  Mutator couldn't replaceFunction (shlib -> shlib)\n");
    }

    // Replace a shlib function with an a.out function
    bpmv.clear();
    if (NULL == modA->findFunction("call22_6", bpmv) || !bpmv.size()) {
	 logerror("**Failed test #22 (replace function)\n");
	 logerror("  Mutator couldn't find functions in mutatee\n");
	 return FAILED;
    }
    BPatch_function *call22_6func = bpmv[0];

    bpfv.clear();
    char *fn4 = "test1_22_call7";
    if (NULL == appImage->findFunction(fn4, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      logerror("**Failed test #22 (replace function)\n");
      logerror("    Unable to find function %s\n", fn4);
      return FAILED;
    }
    BPatch_function *call22_7func = bpfv[0];
    if (! appThread->replaceFunction(*call22_6func, *call22_7func)) {
	 logerror("**Failed test #22 (replace function)\n");
	 logerror("  Mutator couldn't replaceFunction (shlib -> a.out)\n");
	 return FAILED;
    }
    return PASSED;
#else // Not running on one of the specified platforms
    return SKIPPED;
#endif
}


// Wrapper to call readyTest
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
// {
test_results_t test1_22_Mutator::preExecution() {
   int pointer_size = 0;
#if defined(arch_x86_64)
   pointer_size = pointerSize(appImage);
#endif
   strncpy(libNameA, libNameAroot, 128);
   addLibArchExt(libNameA,128, pointer_size);
   strncpy(libNameB, libNameBroot, 128);
   addLibArchExt(libNameB,128, pointer_size);

//    RETURNONFAIL(readyTest21or22(appThread, libNameA, libNameB,
//             mutateeFortran));
   char libA[128], libB[128];
   snprintf(libA, 128, "./%s", libNameA);
   snprintf(libB, 128, "./%s", libNameB);
   // if (!mutateeFortran) {
   if (! appThread->loadLibrary(libA)) {
     logerror("**Failed test1_22 (findFunction in module)\n");
     logerror("  Mutator couldn't load %s into mutatee\n", libNameA);
     return FAILED;
   }
   if (! appThread->loadLibrary(libB)) {
     logerror("**Failed test1_22 (findFunction in module)\n");
     logerror("  Mutator couldn't load %s into mutatee\n", libNameB);
     return FAILED;
   }
   // }

   return mutatorTest22();
} // test1_22_Mutator::preExecution()

// External Interface
// extern "C" TEST_DLL_EXPORT int test1_22_mutatorMAIN(ParameterDict &param)
// {
test_results_t test1_22_Mutator::setup(ParameterDict &param) {
    BPatch *bpatch;
    bool useAttach = param["useAttach"]->getInt();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    appThread = (BPatch_thread *)(param["appThread"]->getPtr());

    // Get log file pointers
//     FILE *outlog = (FILE *)(param["outlog"]->getPtr());
//     FILE *errlog = (FILE *)(param["errlog"]->getPtr());
//     setOutputLog(outlog);
//     setErrorLog(errlog);

    // Read the program's image and get an associated image object
    appImage = appThread->getImage();

    if (isMutateeFortran(appImage))
    {
      return SKIPPED;
    }

    if ( useAttach )
    {
      if ( ! signalAttached(appThread, appImage) )
         return FAILED;
    }

    // Run mutator code
    return PASSED;
}
