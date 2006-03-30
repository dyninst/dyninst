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

// $Id: test1_37.C,v 1.5 2006/03/30 04:57:50 nater Exp $
/*
 * #Name: test1_37
 * #Desc: Instrument Loops
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"

int mutateeFortran;
BPatch *bpatch;

//
// Start Test Case #37 - (loop instrumentation)
//

// sort basic blocks ascending by block number
void sort_blocks(BPatch_Vector<BPatch_basicBlock*> &a, int n) {
    for (int i=0; i<n-1; i++) {
	for (int j=0; j<n-1-i; j++)
	    if (a[j+1]->getBlockNumber() < a[j]->getBlockNumber()) {    
		BPatch_basicBlock* tmp = a[j]; 
		a[j] = a[j+1];
		a[j+1] = tmp;
	    }
    }
}

/* This method instruments the entry and exit edges of a loop with 
   the passed-in function. It accomplishes this by looking up the entry
   and exit blocks of the loop, finding the edges that do not come from
   or target other blocks in the loop (respectively), and instrumenting
   those edges. So effectively, this is a test of both our loop detection
   and edge instrumentation facilities. Two for one, yay!
*/
void instrumentLoops(BPatch_thread *appThread, BPatch_image *appImage,
             BPatch_Vector<BPatch_basicBlockLoop*> &loops,
             BPatch_funcCallExpr &callInc) 
{            
    // for each loop (set of basic blocks)
    for (unsigned int i = 0; i < loops.size(); i++) {
        BPatch_flowGraph *cfg; 
        BPatch_Vector<BPatch_point*> * exits;
        BPatch_Vector<BPatch_point*> * entries;
        
        cfg = loops[i]->getFlowGraph();
        
        // Find loop entry and exit points
        entries = cfg->findLoopInstPoints(BPatch_locLoopEntry,
                                          loops[i]);
        exits = cfg->findLoopInstPoints(BPatch_locLoopExit,
                                        loops[i]);
        // instrument those points      
        
        if(entries->size() == 0) {
            fprintf(stderr,"**Failed** test #37 (instrument loops)\n");
            fprintf(stderr,"   Unable to find loop entry inst point.\n");
        }
        if(exits->size() == 0) {
            fprintf(stderr,"**Failed** test #37 (instrument loops)\n");
            fprintf(stderr,"   Unable to find loop exit inst point.\n");
        }

        unsigned int j;
        BPatch_point *p = NULL;
        for(j=0;j<entries->size();j++) {
            p = (*entries)[j];

            BPatchSnippetHandle * han =
            appThread->insertSnippet(callInc, *p, BPatch_callBefore);

            // did we insert the snippet?
            if (han == NULL) {
                fprintf(stderr,"**Failed** test #37 (instrument loops)\n");
                fprintf(stderr,"   Unable to insert snippet at loop entry.\n");
            }
        }
        for(j=0;j<exits->size();j++) {
            p = (*exits)[j];

            BPatchSnippetHandle * han =
            appThread->insertSnippet(callInc, *p, BPatch_callBefore);

            // did we insert the snippet?
            if (han == NULL) {
                fprintf(stderr,"**Failed** test #37 (instrument loops)\n");
                fprintf(stderr,"   Unable to insert snippet at loop exit.\n");
            }
        }

        // we are responsible for releasing the point vectors
        delete entries;
        delete exits;

        BPatch_Vector<BPatch_basicBlockLoop*> lps;
        loops[i]->getOuterLoops(lps);

        // recur with this loop's outer loops
        instrumentLoops(appThread, appImage, lps, callInc);
    }
}

int instrumentFuncLoopsWithCall(BPatch_thread *appThread, 
				 BPatch_image *appImage,
				 char *call_func,
				 char *inc_func)
{
    // get function * for call_func
    BPatch_Vector<BPatch_function *> funcs;

    appImage->findFunction(call_func, funcs);
    BPatch_function *func = funcs[0];

    // get function * for inc_func
    BPatch_Vector<BPatch_function *> funcs2;
    appImage->findFunction(inc_func, funcs2);
    BPatch_function *incVar = funcs2[0];

    if (func == NULL || incVar == NULL) {
	fprintf(stderr,"**Failed** test #37 (instrument loops)\n");
	fprintf(stderr,"    Unable to get funcions.\n");
        return -1;
    }

    // create func expr for incVar
    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_funcCallExpr callInc(*incVar, nullArgs);
    checkCost(callInc);

    // instrument the function's loops
    BPatch_flowGraph *cfg = func->getCFG();
    BPatch_Vector<BPatch_basicBlockLoop*> loops;
    cfg->getOuterLoops(loops);

    instrumentLoops(appThread, appImage, loops, callInc);
}


int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
{
    if (mutateeFortran) {
	return 0;
    } 

    RETURNONFAIL(instrumentFuncLoopsWithCall(appThread, appImage,"call37_1", "inc37_1"));

    RETURNONFAIL(instrumentFuncLoopsWithCall(appThread, appImage,"call37_2", "inc37_2"));

    RETURNONFAIL(instrumentFuncLoopsWithCall(appThread, appImage,"call37_3", "inc37_3"));

    return 0;
}

// External Interface
extern "C" TEST_DLL_EXPORT int mutatorMAIN(ParameterDict &param)
{
    bool useAttach = param["useAttach"]->getInt();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    BPatch_thread *appThread = (BPatch_thread *)(param["appThread"]->getPtr());


    // Read the program's image and get an associated image object
    BPatch_image *appImage = appThread->getImage();

    if ( useAttach )
    {
      if ( ! signalAttached(appThread, appImage) )
         return -1;
    }

    mutateeFortran = isMutateeFortran(appImage);

    // Run mutator code
    return mutatorTest(appThread, appImage);
}
