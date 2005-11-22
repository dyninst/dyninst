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

// $Id: test1_25.C,v 1.2 2005/11/22 19:41:41 bpellin Exp $
/*
 * #Name: test1_25
 * #Desc: Unary Operators
 * #Dep: 
 * #Arch: !mips_sgi_irix6_4
 * #Notes: A small part of this test is excluded on most platforms
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

BPatch *bpatch;
//
// Start Test Case #25 - unary operators
//
int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
	// Used as hack for Fortran to allow assignment of a pointer to an int
	bpatch->setTypeChecking (false);
#if !defined(mips_sgi_irix6_4)
    //     First verify that we can find a local variable in call25_1
  BPatch_Vector<BPatch_function *> found_funcs;
    if ((NULL == appImage->findFunction("call25_1", found_funcs)) || !found_funcs.size()) {
       fprintf(stderr, "    Unable to find function %s\n",
	      "call25_1");
      return -1;
    }

    if (1 < found_funcs.size()) {
      fprintf(stderr, "%s[%d]:  WARNING  : found %d functions named %s.  Using the first.\n", 
	      __FILE__, __LINE__, found_funcs.size(), "call25_1");
    }

    BPatch_Vector<BPatch_point *> *point25_1 = found_funcs[0]->findPoint(BPatch_entry);

    assert(point25_1);
//    assert(point25_1 && (point25_1->size() == 1));

    BPatch_variableExpr *gvar[8];

    for (int i=1; i <= 7; i++) {
        char name[80];

        sprintf (name, "globalVariable25_%d", i);
        gvar [i] = findVariable (appImage, name, point25_1);

	if (!gvar[i]) {
	    fprintf(stderr, "**Failed** test #25 (unary operaors)\n");
	    fprintf(stderr, "  can't find variable globalVariable25_%d\n", i);
	    return -1;
	}
    }

    //     globalVariable25_2 = &globalVariable25_1
#if !defined(sparc_sun_solaris2_4) \
 && !defined(rs6000_ibm_aix4_1) \
 && !defined(alpha_dec_osf4_0) \
 && !defined(i386_unknown_linux2_0) \
 && !defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 && !defined(ia64_unknown_linux2_4) \
 && !defined(i386_unknown_solaris2_5) \
 && !defined(i386_unknown_nt4_0)

    // without type info need to inform
    BPatch_type *type = appImage->findType("void *");
    assert(type);
    gvar[2]->setType(type);
#endif

    BPatch_arithExpr assignment1(BPatch_assign, *gvar[2],
	BPatch_arithExpr(BPatch_addr, *gvar[1]));

    appThread->insertSnippet(assignment1, *point25_1);

    // 	   globalVariable25_3 = *globalVariable25_2
    //		Need to make sure this happens after the first one
    BPatch_arithExpr assignment2(BPatch_assign, *gvar[3],
	BPatch_arithExpr(BPatch_deref, *gvar[2]));
    appThread->insertSnippet(assignment2, *point25_1,  BPatch_callBefore,
	    BPatch_lastSnippet);

    // 	   globalVariable25_5 = -globalVariable25_4
    BPatch_arithExpr assignment3(BPatch_assign, *gvar[5],
	BPatch_arithExpr(BPatch_negate, *gvar[4]));
    appThread->insertSnippet(assignment3, *point25_1);

    // 	   globalVariable25_7 = -globalVariable25_6
    BPatch_arithExpr assignment4(BPatch_assign, *gvar[7],
	BPatch_arithExpr(BPatch_negate, *gvar[6]));
    appThread->insertSnippet(assignment4, *point25_1);

#endif
	bpatch->setTypeChecking (true);
        return 0;
}

// External Interface
extern "C" TEST_DLL_EXPORT int mutatorMAIN(ParameterDict &param)
{
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());


    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
