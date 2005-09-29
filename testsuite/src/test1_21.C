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

// $Id: test1_21.C,v 1.1 2005/09/29 20:38:21 bpellin Exp $
/*
 * #Name: test1_21
 * #Desc: findFunction in module
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4,alpha_dec_osf4_0,i386_unknown_solaris2_5,i386_unknown_linux2_0,ia64_unknown_linux2_4,mips_sgi_irix6_4,rs6000_ibm_aix4_1,x86_64_unknown_linux2_4
 * #Notes: This test uses some special magic for libNameA and libNameB that should probably be altered
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
// Start Test Case #21 - mutator side (findFunction in module)
//
// There is no corresponding failure (test2) testing because the only
// bad input to replaceFunction is a non-existent BPatch_function.
// But this is already checked by the "non-existent function" test in
// test2.

int mutatorTest21(BPatch_thread *, BPatch_image *appImage)
{
#if defined(sparc_sun_solaris2_4) \
 || defined(alpha_dec_osf4_0) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4) \
 || defined(mips_sgi_irix6_4) \
 || defined(rs6000_ibm_aix4_1)

    // Lookup the libtestA.so and libtestB.so modules that we've just loaded

    if (mutateeFortran) {
	return 0;
    }

    BPatch_module *modA = NULL;
    BPatch_module *modB = NULL;
    BPatch_Vector<BPatch_module *> *mods = appImage->getModules();
    if (!mods || mods->size() == 0) {
	 fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	 fprintf(stderr, "  Mutator couldn't search modules of mutatee\n");
         for (unsigned int j = 0; j < mods->size(); ++j) {
            char buf2[1024];
            BPatch_module *m = (*mods)[j];
            m->getName(buf2, 1024);
            fprintf(stderr, "%s[%d]:  module: %s\n", __FILE__, __LINE__, buf2);
         }
	 return -1;
    }

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
    if (! modA || ! modB ) {
	 fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	 fprintf(stderr, "  Mutator couldn't find shlib in mutatee\n");
	 fflush(stdout);
	 return -1;
    }

    // Find the function CALL21_1 in each of the modules
    BPatch_Vector<BPatch_function *> bpmv;
    if (NULL == modA->findFunction("call21_1", bpmv, false, false, true) || !bpmv.size()) {
      fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
      fprintf(stderr, "  %s[%d]: Mutator couldn't find a function in %s\n", 
                         __FILE__, __LINE__, libNameA);
      return -1;
    }
    BPatch_function *funcA = bpmv[0];

    bpmv.clear();
    if (NULL == modB->findFunction("call21_1", bpmv, false, false, true) || !bpmv.size()) {
      fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
      fprintf(stderr, "  %s[%d]: Mutator couldn't find a function in %s\n", 
                          __FILE__, __LINE__, libNameB);
      return -1;
    } 
    BPatch_function *funcB = bpmv[0];

    // Kludgily test whether the functions are distinct
    if (funcA->getBaseAddr() == funcB->getBaseAddr()) {
	 fprintf(stderr, "**Failed test #21 (findFunction in module)\n");
	 fprintf(stderr,
	        "  Mutator cannot distinguish two functions from different shlibs\n");
	 return -1;
    }

    //  Now test regex search
    //  Not meant to be an extensive check of all regex usage, just that
    //  the basic mechanism that deals with regexes is not broken

    bpmv.clear();
    //   regex "^cb" should match all functions that begin with "cb"
    //   We dont use existing "call" functions here since (at least on
    //   linux, we also find call_gmon_start().  Thus the dummy fns.
    if (NULL == modB->findFunction("^cb", bpmv, false, false, true) || (bpmv.size() != 2)) {

	 fprintf(stderr, "**Failed test #21 (findFunction in module, regex)\n");
         fprintf(stderr, "  Expected 2 functions matching ^cb, got %d\n",
                            bpmv.size());
         char buf[128];
         for (unsigned int i = 0; i < bpmv.size(); ++i) 
            fprintf(stderr, "  matched function: %s\n", 
                   bpmv[i]->getName(buf, 128));
         return -1;
    }

    bpmv.clear();
    if (NULL == modB->findFunction("^cbll21", bpmv, false, false, true) || (bpmv.size() != 1)) {
	 fprintf(stderr, "**Failed test #21 (findFunction in module, regex)\n");
         fprintf(stderr, "  Expected 1 function matching ^cbll21, got %d\n",
                            bpmv.size());
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

   return mutatorTest21(appThread, appImage);

}

// External Interface
extern "C" int mutatorMAIN(ParameterDict &param)
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
