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

// $Id: test1_39.C,v 1.2 2005/11/22 19:41:56 bpellin Exp $
/*
 * #Name: test1_39
 * #Desc: Mutator Side - If without else
 * #Dep: 
 * #Arch: sparc_sun_solaris2_4,alpha_dec_osf4_0,i386_unknown_solaris2_5,,i386_unknown_linux2_0,ia64_unknown_linux2_4,mips_sgi_irix6_4,rs6000_ibm_aix4_1,x86_64_unknown_linux2_4
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

int mutateeFortran;

//
//  Test case 39:  verify that regex search is working
//

int mutatorTest(BPatch_thread * /*appThread*/, BPatch_image *appImage)
{
  //  Note:  regex search by module is covered in test 21
#if defined(sparc_sun_solaris2_4) \
 || defined(alpha_dec_osf4_0) \
 || defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(ia64_unknown_linux2_4) \
 || defined(mips_sgi_irix6_4) \
 || defined(rs6000_ibm_aix4_1)

   BPatch_Vector<BPatch_function *> bpmv;

   //  Not meant to be an extensive check of all regex usage, just that
    //  the basic mechanism that deals with regexes is not broken

    //   regex "^fucn12" should match all functions that begin with "func12"
    if (NULL == appImage->findFunction("^func12", bpmv) || (bpmv.size() != 2)) {

         fprintf(stderr, "**Failed test #39 (regex function search)\n");
         fprintf(stderr, "  Expected 2 functions matching ^func12, got %d\n",
                            bpmv.size());
        char buf[128];
         for (unsigned int i = 0; i < bpmv.size(); ++i)
            fprintf(stderr, "  matched function: %s\n",
                   bpmv[i]->getName(buf, 128));
         return -1;
    }

    bpmv.clear();
    if (NULL == appImage->findFunction("^func12_1", bpmv) 
       || (bpmv.size() != 1)) {
         fprintf(stderr, "**Failed test #39 (regex function search)\n");
         fprintf(stderr, "  Expected 1 function matching ^func12_1, got %d\n",
                            bpmv.size());
         return -1;
    }

    if (mutateeFortran) return 0;

    //  Now lets look for a pattern that ought to match something
    //  in libc (can't check number of hits since libc may vary,
    //  but can check existence)
    bpmv.clear();
    //const char *libc_regex = "^inet_n";
    const char *libc_regex = "^sp";
    if (NULL == appImage->findFunction(libc_regex, bpmv) 
       || (!bpmv.size())) {
         fprintf(stderr, "**Failed test #39 (regex function search)\n");
         fprintf(stderr, "  Expected function(s) matching %s\n",libc_regex);
         return -1;
    }

#endif

    return 0;
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
