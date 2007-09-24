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

// $Id: test1_20.C,v 1.1 2007/09/24 16:37:17 cooksey Exp $
/*
 * #Name: test1_20
 * #Desc: Mutator Side - Instrumentation at arbitrary points
 * #Dep: 
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"
#include "Callbacks.h"

static BPatch *bpatch;

#include "TestMutator.h"
class test1_20_Mutator : public TestMutator {
  BPatch *bpatch;

  virtual test_results_t setup(ParameterDict &param);
  virtual test_results_t preExecution();
};
extern "C" TEST_DLL_EXPORT TestMutator *test1_20_factory() {
  return new test1_20_Mutator();
}

//
// Start Test Case #20 - mutator side (instrumentation at arbitrary points)
//
// static int mutatorTest(BPatch_thread *appThread, BPatch_image *appImage)
// {
test_results_t test1_20_Mutator::preExecution() {
    BPatch_Vector<BPatch_function *> bpfv;
    char *fn = "test1_20_call1";
    if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
         || NULL == bpfv[0]){
         logerror("    Unable to find function %s\n", fn);
         return FAILED;
  }


    BPatch_function *call20_1_func = bpfv[0];

    BPatch_Vector<BPatch_snippet *> nullArgs;
    BPatch_funcCallExpr call20_1Expr(*call20_1_func, nullArgs);
    checkCost(call20_1Expr);

    bpfv.clear();
    char *fn2 = "test1_20_func2";
    if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size() ||
	    NULL == bpfv[0]){
      logerror("    Unable to find function %s\n", fn2);
      return FAILED;
    }

    BPatch_function *f = bpfv[0];

    // We really don't want to use function size... grab a flowgraph and
    // do this right.
    BPatch_flowGraph *cfg = f->getCFG();
    if (!cfg) {
	logerror("**Failed** test #20 (instrumentation at arbitrary points)\n");
	logerror("    no flowgraph for %s\n", fn2);
	return FAILED;
    }

    BPatch_point *p = NULL;
    bool found_one = false;

    /* We expect certain errors from createInstPointAtAddr. */
    BPatchErrorCallback oldError =
	bpatch->registerErrorCallback(createInstPointError);


    BPatch_Set<BPatch_basicBlock *> blocks;
    if (!cfg->getAllBasicBlocks(blocks))
        assert(0); // This can't return false :)
    if (blocks.size() == 0) {
        logerror("**Failed** test #20 (instrumentation at arbitrary points)\n");
        logerror("    no blocks for %s\n", fn2);
        return FAILED;
    }
    
    appThread->getProcess()->beginInsertionSet();
    dprintf("%s[%d]:  about to instrument %d basic blocks\n", __FILE__, __LINE__, blocks.size());

    BPatch_Set<BPatch_basicBlock *>::iterator blockIter = blocks.begin();
    for (; blockIter != blocks.end(); blockIter++) {
        BPatch_basicBlock *block = *blockIter;
        assert(block);

        dprintf("%s[%d]:  inserting arbitrary instrumentation in basic block starting at addr %p\n", __FILE__, __LINE__, (void *) block->getStartAddress());
        BPatch_Vector<BPatch_instruction *> *insns = block->getInstructions();
        assert(insns);
        for (unsigned int i = 0; i < insns->size(); ++i) {
          BPatch_instruction *insn = (*insns)[i];
          BPatch_point *pt = insn->getInstPoint();
          if (pt) {
            if (pt->getPointType() == BPatch_arbitrary) {
                found_one = true;
                if (appThread->insertSnippet(call20_1Expr, *pt) == NULL) {
                    logerror("%s[%d]: Unable to insert snippet into function \"func20_2.\"\n",
                             __FILE__, __LINE__);
                    return FAILED;
                }
                dprintf("%s[%d]:  SUCCESS installing inst at address %p/%p\n", __FILE__, __LINE__, insn->getAddress(), pt->getAddress());
            }
            else
              logerror("%s[%d]:  non-arbitrary point (%d) being ignored\n", __FILE__, __LINE__);

          }
          else {
             logerror("%s[%d]:  no instruction for point\n", __FILE__, __LINE__);
          }
        }

    }

    appThread->getProcess()->finalizeInsertionSet(false);

    bpatch->registerErrorCallback(oldError);

    if (!found_one) {
	logerror("Unable to find a point to instrument in function \"%s.\"\n",
		 fn2);
	return FAILED;
    }

    return PASSED;
} // test1_20_Mutator::preExecution()

// External Interface
// extern "C" TEST_DLL_EXPORT int test1_20_mutatorMAIN(ParameterDict &param)
// {
test_results_t test1_20_Mutator::setup(ParameterDict &param) {
    bool useAttach = param["useAttach"]->getInt();
    bpatch = (BPatch *)(param["bpatch"]->getPtr());
    appThread = (BPatch_thread *)(param["appThread"]->getPtr());

    // Read the program's image and get an associated image object
    appImage = appThread->getImage();

    if ( useAttach )
    {
      if ( ! signalAttached(appThread, appImage) )
         return FAILED;
    }

    return PASSED;
} // test1_20_Mutator::setup(ParameterDict &)
