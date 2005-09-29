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

// $Id: test1_35.C,v 1.1 2005/09/29 20:38:41 bpellin Exp $
/*
 * #Name: test1_35
 * #Desc: Function Relocation
 * #Dep: 
 * #Arch:
 * #Notes: i386_unknown_solaris2_5,i386_unknown_linux2_0,sparc_sun_solaris2_4,x86_64_unknown_linux2_4
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

int mutateeFortran;

// Start Test Case #35 - (function relocation)
int mutatorTest( BPatch_thread * appThread, BPatch_image * appImage )
{
#if defined(i386_unknown_solaris2_5) \
 || defined(i386_unknown_linux2_0) \
 || defined(x86_64_unknown_linux2_4) /* Blind duplication - Ray */ \
 || defined(sparc_sun_solaris2_4)

    if (mutateeFortran)
	return 0;

    BPatch_Vector<BPatch_function *> bpfv;
    char *fn = "call35_1";
    if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      fprintf(stderr, "**Failed** test #34 (function relocation)\n");
      fprintf(stderr, "    Unable to find function %s\n", fn);
      return -1;
    }
    
    BPatch_function *foo_function = bpfv[0];

    BPatch_Vector<BPatch_point *> *point35_1 =  
	foo_function->findPoint(BPatch_subroutine);

    assert(point35_1);

    BPatch_variableExpr *var1 = appImage->findVariable(*(*point35_1)[0], 
	"localVariable35_1");
    BPatch_variableExpr *var2 = appImage->findVariable(*(*point35_1)[0], 
	"localVariable35_2");
    BPatch_variableExpr *var3 = appImage->findVariable(*(*point35_1)[0], 
	"total35_1");
    BPatch_variableExpr *var4 = appImage->findVariable(*(*point35_1)[0], 
	"total35_2");

    if (!var1 || !var2 || !var3 || !var4 ) {
	fprintf(stderr, "**Failed** test #35 (function relocation)\n");
	if (!var1) 
	    fprintf(stderr, "  can't find local variable localVariable35_1\n");
	if (!var2) 
	    fprintf(stderr, "  can't find local variable localVariable35_2\n");
        if (!var3) 
	    fprintf(stderr, "  can't find local variable total35_1\n");
        if (!var4) 
	    fprintf(stderr, "  can't find local variable total35_2\n");
	return -1;
    }

    BPatch_snippet * snippet35_1 = 
      new BPatch_arithExpr(BPatch_assign, *var1, BPatch_constExpr(7));

    BPatch_snippet * snippet35_2 = 
      new BPatch_arithExpr(BPatch_assign, *var2, BPatch_constExpr(5));

    BPatch_snippet * snippet35_3 = 
      new BPatch_arithExpr(BPatch_assign, *var4, *var3);

    BPatch_point * call_1 = ( (* point35_1)[0] );
    assert( call_1 != 0 );
    
    BPatch_point * call_2 = ( (* point35_1)[2] );
    assert( call_2 != 0 );
    
    appThread->insertSnippet( * snippet35_3, * call_2, BPatch_callAfter, BPatch_firstSnippet );
    appThread->insertSnippet( * snippet35_2, * call_1, BPatch_callBefore, BPatch_firstSnippet );
    appThread->insertSnippet( * snippet35_1, * call_1, BPatch_callBefore, BPatch_firstSnippet );

#endif   

    return 0;
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
