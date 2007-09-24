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

// $Id: test1_33.C,v 1.1 2007/09/24 16:38:06 cooksey Exp $
/*
 * #Name: test1_33 
 * #Desc: Control Flow Graphs
 * #Dep: 
 * #Arch:
 * #Notes:
 */

#include "BPatch.h"
#include "BPatch_Vector.h"
#include "BPatch_thread.h"
#include "BPatch_snippet.h"

#include "test_lib.h"

//static int mutateeFortran;

#include "TestMutator.h"
class test1_33_Mutator : public TestMutator {
  virtual test_results_t preExecution();
};
extern "C" TEST_DLL_EXPORT TestMutator *test1_33_factory() {
  return new test1_33_Mutator();
}

//
// Start Test Case #33 - (control flow graphs)
//

static bool hasBackEdge(BPatch_basicBlock *bb, BPatch_Set<int> visited)
{
    if (visited.contains(bb->getBlockNumber()))
	return true;

    visited.insert(bb->getBlockNumber());

    BPatch_Vector<BPatch_basicBlock*> targets;
    bb->getTargets(targets);

    unsigned int i;
    for (i = 0; i < targets.size(); i++) {
	if (hasBackEdge(targets[i], visited))
	    return true;
    }

    return false;
}

// static int mutatorTest( BPatch_thread * /*appThread*/, BPatch_image * appImage )
// {
test_results_t test1_33_Mutator::preExecution() {
    int pvalue;
    unsigned int i;

    if (isMutateeFortran(appImage)) {
	return SKIPPED;
    }

    BPatch_Vector<BPatch_function *> bpfv;
    char *fn = "test1_33_func2";
    if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      logerror("**Failed** test #33 (control flow graphs)\n");
      logerror("    Unable to find function %s\n", fn);
      return FAILED;
    }
    
    BPatch_function *func2 = bpfv[0];

    BPatch_flowGraph *cfg = func2->getCFG();
    if (cfg == NULL) {
	logerror("**Failed** test #33 (control flow graphs)\n");
	logerror("  Unable to get control flow graph of %s\n", fn);
	return FAILED;
    }

    /*
     * Test for consistency of entry basic blocks.
     */
    BPatch_Vector<BPatch_basicBlock*> entry_blocks;
    cfg->getEntryBasicBlock(entry_blocks);

    if (entry_blocks.size() != 1) {
	    logerror("**Failed** test #33 (control flow graphs)\n");
	    logerror("  Detected %d entry basic blocks in %s, should have been one.\n", entry_blocks.size(), fn);
            return FAILED;
    }

    for (i = 0; i < entry_blocks.size(); i++) {
	BPatch_Vector<BPatch_basicBlock*> sources;
	entry_blocks[i]->getSources(sources);
	if (sources.size() > 0) {
	    logerror("**Failed** test #33 (control flow graphs)\n");
	    logerror("  An entry basic block has incoming edges in the control flow graph\n");
	    return FAILED;
	}

    	BPatch_Vector<BPatch_basicBlock*> targets;
	entry_blocks[i]->getTargets(targets);
	if (targets.size() < 1) {
	    logerror("**Failed** test #33 (control flow graphs\n");
	    logerror("  An entry basic block has no outgoing edges in the control flow graph\n");
	    return FAILED;
	}
    }

    /*
     * Test for consistency of exit basic blocks.
     */
    BPatch_Vector<BPatch_basicBlock*> exit_blocks;
    cfg->getExitBasicBlock(exit_blocks);

    if (exit_blocks.size() != 1) {
	    logerror("**Failed** test #33 (control flow graphs)\n");
	    logerror("  Detected %d exit basic blocks in %s, should have been one.\n", exit_blocks.size(), fn);
            return FAILED;
    }

    for (i = 0; i < exit_blocks.size(); i++) {
	BPatch_Vector<BPatch_basicBlock*> sources;
	exit_blocks[i]->getSources(sources);
	if (sources.size() < 1) {
	    logerror("**Failed** test #33 (control flow graphs)\n");
	    logerror("  An exit basic block has no incoming edges in the control flow graph\n");
	    return FAILED;
	}

	BPatch_Vector<BPatch_basicBlock*> targets;
	exit_blocks[i]->getTargets(targets);
	if (targets.size() > 0) {
	    logerror("**Failed** test #33 (control flow graphs)\n");
	    logerror("  An exit basic block has outgoing edges in the control flow graph\n");
	    return FAILED;
	}
    }

    /*
     * Check structure of control flow graph.
     */
    BPatch_Set<BPatch_basicBlock*> blocks;
    cfg->getAllBasicBlocks(blocks);
    if (blocks.size() < 4) {
	logerror("**Failed** test #33 (control flow graphs)\n");
	logerror("  Detected %d basic blocks in %s, should be at least four.\n", blocks.size(), fn);
	return FAILED;
    }

    BPatch_basicBlock **block_elements = new BPatch_basicBlock*[blocks.size()];
    blocks.elements(block_elements);

    bool foundOutDegreeTwo = false;
    bool foundInDegreeTwo = false;
    int blocksNoIn = 0, blocksNoOut = 0;
    
    for (i = 0; i < (unsigned int) blocks.size(); i++) {
	BPatch_Vector<BPatch_basicBlock*> in;
	BPatch_Vector<BPatch_basicBlock*> out;

	block_elements[i]->getSources(in);
	block_elements[i]->getTargets(out);

	if (in.size() == 0)
	    blocksNoIn++;

	if (out.size() == 0)
	    blocksNoOut++;

	if (in.size() > 2 || out.size() > 2) {
	    logerror("**Failed** test #33 (control flow graphs)\n");
	    logerror("  Detected a basic block in %s with %d incoming edges and %d\n", fn, in.size(), out.size());
	    logerror("  outgoing edges - neither should be greater than two.\n");
	    return FAILED;
	} else if (in.size() > 1 && out.size() > 1) {
	    logerror("**Failed** test #33 (control flow graphs)\n");
	    logerror("  Detected a basic block in %s with %d incoming edges and %d\n", fn, in.size(), out.size());
	    logerror("  outgoing edges - only one should be greater than one.\n");
	    return FAILED;
	} else if (in.size() == 0 && out.size() == 0) {
	    logerror("**Failed** test #33 (control flow graphs)\n");
	    logerror("  Detected a basic block in %s with no incoming or outgoing edges.\n", fn);
	    return FAILED;
	} else if (in.size() == 2) {
	    assert(out.size() <= 1);

	    if (foundInDegreeTwo) {
		logerror("**Failed** test #33 (control flow graphs)\n");
		logerror("  Detected two basic blocks in %s with in degree two, there should only\n", fn);
		logerror("  be one.\n");
		return FAILED;
	    }
	    foundInDegreeTwo = true;

	    if (in[0]->getBlockNumber() == in[1]->getBlockNumber()) {
		logerror("**Failed** test #33 (control flow graphs)\n");
		logerror("  Two edges go to the same block (number %d).\n", in[0]->getBlockNumber());
		return FAILED;
	    }
	} else if (out.size() == 2) {
	    assert(in.size() <= 1);

	    if (foundOutDegreeTwo) {
		logerror("**Failed** test #33 (control flow graphs)\n");
		logerror("  Detected two basic blocks in %s with out degree two, there should only\n", fn);
		logerror("  be one.\n");
		return FAILED;
	    }
	    foundOutDegreeTwo = true;

	    if (out[0]->getBlockNumber() == out[1]->getBlockNumber()) {
		logerror("**Failed** test #33 (control flow graphs)\n");
		logerror("  Two edges go to the same block (number %d).\n", out[0]->getBlockNumber());
		return FAILED;
	    }
	} else if (in.size() > 1 || out.size() > 1) {
	    /* Shouldn't be able to get here. */
	    logerror("**Failed** test #33 (control flow graphs)\n");
	    logerror("  Detected a basic block in %s with %d incoming edges and %d\n", fn, in.size(), out.size());
	    logerror("  outgoing edges.\n");
	    return FAILED;
	}
    }

    delete [] block_elements;
    
    if (blocksNoIn > 1) {
	    logerror("**Failed** test #33 (control flow graphs)\n");
	    logerror("  Detected more than one block in %s with no incoming edges.\n", fn);
	    return FAILED;
    }

    if (blocksNoOut > 1) {
	    logerror("**Failed** test #33 (control flow graphs)\n");
	    logerror("  Detected more than block in %s with no outgoing edges.\n", fn);
	    return FAILED;
    }

    if (!foundOutDegreeTwo) {
	    logerror("**Failed** test #33 (control flow graphs)\n");
	    logerror("  Did not detect the \"if\" statement in %s.\n", fn);
	    return FAILED;
    }

    /*
     * Check for loops (there aren't any in the function we're looking at).
     */
    BPatch_Set<int> empty;
    if (hasBackEdge(entry_blocks[0], empty)) {
	logerror("**Failed** test #33 (control flow graphs)\n");
	logerror("  Detected a loop in %s, there should not be one.\n", fn);
	return FAILED;
    }

    /*
     * Now check a function with a switch statement.
     */
    bpfv.clear();
    char *fn2 = "test1_33_func3";
    // Bernat, 8JUN05 -- include uninstrumentable here...
    if (NULL == appImage->findFunction(fn2, bpfv, false, false, true) || !bpfv.size()
	|| NULL == bpfv[0]){
      logerror("**Failed** test #33 (control flow graphs)\n");
      logerror("    Unable to find function %s\n", fn2);
      return FAILED;
    }
    
    BPatch_function *func3 = bpfv[0];

    BPatch_flowGraph *cfg3 = func3->getCFG();
    if (cfg3 == NULL) {
	logerror("**Failed** test #33 (control flow graphs)\n");
	logerror("  Unable to get control flow graph of %s\n", fn2);
	return FAILED;
    }

    BPatch_Set<BPatch_basicBlock*> blocks3;
    cfg3->getAllBasicBlocks(blocks3);
    if (blocks3.size() < 10) {
	logerror("**Failed** test #33 (control flow graphs)\n");
	logerror("  Detected %d basic blocks in %s, should be at least ten.\n", blocks3.size(), fn2);
	return FAILED;
    }

    block_elements = new BPatch_basicBlock*[blocks3.size()];
    blocks3.elements(block_elements);

    bool foundSwitchIn = false;
    bool foundSwitchOut = false;
    bool foundRangeCheck = false;
    for (i = 0; i < (unsigned int)blocks3.size(); i++) {
	BPatch_Vector<BPatch_basicBlock*> in;
	BPatch_Vector<BPatch_basicBlock*> out;

	block_elements[i]->getSources(in);
	block_elements[i]->getTargets(out);

	if (!foundSwitchOut && out.size() >= 10 && in.size() <= 1) {
	    foundSwitchOut = true;
	} else if (!foundSwitchIn && in.size() >= 10 && out.size() <= 1) {
	    foundSwitchIn = true;
	} else if (!foundRangeCheck && out.size() == 2 && in.size() <= 1) {
	    foundRangeCheck = true;
	} else if (in.size() > 1 && out.size() > 1) {
	    logerror("**Failed** test #33 (control flow graphs)\n");
	    logerror("  Found basic block in %s with unexpected number of edges.\n", fn2);
	    logerror("  %d incoming edges, %d outgoing edges.\n",
		    in.size(), out.size());
	    return FAILED;
	}
    }

    if (!foundSwitchIn || !foundSwitchOut) {
	logerror("**Failed** test #33 (control flow graphs)\n");
	if (!foundSwitchIn)
	    logerror("  Did not find \"switch\" statement in %s.\n", fn2);
	if (!foundSwitchOut)
	    logerror("  Did not find block after \"switch\" statement.\n");
	return FAILED;
    }

    /* Check dominator info. */
    BPatch_Vector<BPatch_basicBlock*> entry3;
    cfg3->getEntryBasicBlock(entry3);
    if (entry3.size() != 1) {
	logerror("**Failed** test #33 (control flow graphs)\n");
	logerror("  Detected %d entry basic blocks in %s, should have been one.\n", entry_blocks.size(), fn2);
	return FAILED;
    }

    for (i = 0; i < (unsigned int) blocks3.size(); i++) {
	if (!entry3[0]->dominates(block_elements[i])) {
	    logerror("**Failed** test #33 (control flow graphs)\n");
	    logerror("  Entry block does not dominate all blocks in %s\n", fn2);
	    return FAILED;
	}
    }

    BPatch_Vector<BPatch_basicBlock*> exit3;
    cfg3->getExitBasicBlock(exit3);
    if (exit3.size() != 1) {
       logerror("**Failed** test #33 (control flow graphs)\n");
       logerror("  Detected %d exit basic blocks in , should have been one.\n", exit3.size(), fn2);
       return FAILED;
    }

    for (i = 0; i < (unsigned int) exit3.size(); i++) {
       if (!exit3[i]->postdominates(entry3[0])) {
          logerror("**Failed** test #33 (control flow graphs)\n");
          logerror("  Exit block %d does not postdominate all entry blocks in %s\n", i, fn2);
          return FAILED;
       }
    }

    BPatch_variableExpr *expr33_1 = 
      appImage->findVariable("test1_33_globalVariable1");
    if (expr33_1 == NULL) {
          logerror("**Failed** test #33 (control flow graphs)\n");
          logerror("    Unable to locate test1_33_globalVariable1\n");
          return FAILED;
    } 

    pvalue = 1;
    expr33_1->writeValue(&pvalue);


    delete [] block_elements;

    return PASSED;
}
