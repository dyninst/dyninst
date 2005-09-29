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

// $Id: test1_33.C,v 1.1 2005/09/29 20:38:39 bpellin Exp $
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

int mutateeFortran;

//
// Start Test Case #33 - (control flow graphs)
//

bool hasBackEdge(BPatch_basicBlock *bb, BPatch_Set<int> visited)
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

int mutatorTest( BPatch_thread * /*appThread*/, BPatch_image * appImage )
{
    unsigned int i;

    if (mutateeFortran) {
	return 0;
    }

    BPatch_Vector<BPatch_function *> bpfv;
    char *fn = "func33_2";
    if (NULL == appImage->findFunction(fn, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
      fprintf(stderr, "    Unable to find function %s\n", fn);
      return -1;
    }
    
    BPatch_function *func2 = bpfv[0];

    BPatch_flowGraph *cfg = func2->getCFG();
    if (cfg == NULL) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	fprintf(stderr, "  Unable to get control flow graph of func33_2\n");
	return -1;
    }

    /*
     * Test for consistency of entry basic blocks.
     */
    BPatch_Vector<BPatch_basicBlock*> entry_blocks;
    cfg->getEntryBasicBlock(entry_blocks);

    if (entry_blocks.size() != 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected %d entry basic blocks in func33_2, should have been one.\n", entry_blocks.size());
            return -1;
    }

    for (i = 0; i < entry_blocks.size(); i++) {
	BPatch_Vector<BPatch_basicBlock*> sources;
	entry_blocks[i]->getSources(sources);
	if (sources.size() > 0) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  An entry basic block has incoming edges in the control flow graph\n");
	    return -1;
	}

    	BPatch_Vector<BPatch_basicBlock*> targets;
	entry_blocks[i]->getTargets(targets);
	if (targets.size() < 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs\n");
	    fprintf(stderr, "  An entry basic block has no outgoing edges in the control flow graph\n");
	    return -1;
	}
    }

    /*
     * Test for consistency of exit basic blocks.
     */
    BPatch_Vector<BPatch_basicBlock*> exit_blocks;
    cfg->getExitBasicBlock(exit_blocks);

    if (exit_blocks.size() != 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected %d exit basic blocks in func33_2, should have been one.\n", exit_blocks.size());
            return -1;
    }

    for (i = 0; i < exit_blocks.size(); i++) {
	BPatch_Vector<BPatch_basicBlock*> sources;
	exit_blocks[i]->getSources(sources);
	if (sources.size() < 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  An exit basic block has no incoming edges in the control flow graph\n");
	    return -1;
	}

	BPatch_Vector<BPatch_basicBlock*> targets;
	exit_blocks[i]->getTargets(targets);
	if (targets.size() > 0) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  An exit basic block has outgoing edges in the control flow graph\n");
	    return -1;
	}
    }

    /*
     * Check structure of control flow graph.
     */
    BPatch_Set<BPatch_basicBlock*> blocks;
    cfg->getAllBasicBlocks(blocks);
    if (blocks.size() < 4) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	fprintf(stderr, "  Detected %d basic blocks in func33_2, should be at least four.\n", blocks.size());
	return -1;
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
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected a basic block in func33_2 with %d incoming edges and %d\n", in.size(), out.size());
	    fprintf(stderr, "  outgoing edges - neither should be greater than two.\n");
	    return -1;
	} else if (in.size() > 1 && out.size() > 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected a basic block in func33_2 with %d incoming edges and %d\n", in.size(), out.size());
	    fprintf(stderr, "  outgoing edges - only one should be greater than one.\n");
	    return -1;
	} else if (in.size() == 0 && out.size() == 0) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected a basic block in func33_2 with no incoming or outgoing edges.\n");
	    return -1;
	} else if (in.size() == 2) {
	    assert(out.size() <= 1);

	    if (foundInDegreeTwo) {
		fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
		fprintf(stderr, "  Detected two basic blocks in func33_2 with in degree two, there should only\n");
		fprintf(stderr, "  be one.\n");
		return -1;
	    }
	    foundInDegreeTwo = true;

	    if (in[0]->getBlockNumber() == in[1]->getBlockNumber()) {
		fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
		fprintf(stderr, "  Two edges go to the same block (number %d).\n", in[0]->getBlockNumber());
		return -1;
	    }
	} else if (out.size() == 2) {
	    assert(in.size() <= 1);

	    if (foundOutDegreeTwo) {
		fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
		fprintf(stderr, "  Detected two basic blocks in func33_2 with out degree two, there should only\n");
		fprintf(stderr, "  be one.\n");
		return -1;
	    }
	    foundOutDegreeTwo = true;

	    if (out[0]->getBlockNumber() == out[1]->getBlockNumber()) {
		fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
		fprintf(stderr, "  Two edges go to the same block (number %d).\n", out[0]->getBlockNumber());
		return -1;
	    }
	} else if (in.size() > 1 || out.size() > 1) {
	    /* Shouldn't be able to get here. */
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected a basic block in func33_2 with %d incoming edges and %d\n", in.size(), out.size());
	    fprintf(stderr, "  outgoing edges.\n");
	    return -1;
	}
    }

    delete [] block_elements;
    
    if (blocksNoIn > 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected more than one block in func33_2 with no incoming edges.\n");
	    return -1;
    }

    if (blocksNoOut > 1) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Detected more than block in func33_2 with no outgoing edges.\n");
	    return -1;
    }

    if (!foundOutDegreeTwo) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Did not detect the \"if\" statement in func33_2.\n");
	    return -1;
    }

    /*
     * Check for loops (there aren't any in the function we're looking at).
     */
    BPatch_Set<int> empty;
    if (hasBackEdge(entry_blocks[0], empty)) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	fprintf(stderr, "  Detected a loop in func33_2, there should not be one.\n");
	return -1;
    }

    /*
     * Now check a function with a switch statement.
     */
    bpfv.clear();
    char *fn2 = "func33_3";
    if (NULL == appImage->findFunction(fn2, bpfv) || !bpfv.size()
	|| NULL == bpfv[0]){
      fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
      fprintf(stderr, "    Unable to find function %s\n", fn2);
      return -1;
    }
    
    BPatch_function *func3 = bpfv[0];

    BPatch_flowGraph *cfg3 = func3->getCFG();
    if (cfg3 == NULL) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	fprintf(stderr, "  Unable to get control flow graph of func33_3\n");
	return -1;
    }

    BPatch_Set<BPatch_basicBlock*> blocks3;
    cfg3->getAllBasicBlocks(blocks3);
    if (blocks3.size() < 10) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	fprintf(stderr, "  Detected %d basic blocks in func33_3, should be at least ten.\n", blocks3.size());
	return -1;
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
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Found basic block in func33_3 with unexpected number of edges.\n");
	    fprintf(stderr, "  %d incoming edges, %d outgoing edges.\n",
		    in.size(), out.size());
	    return -1;
	}
    }

    if (!foundSwitchIn || !foundSwitchOut) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	if (!foundSwitchIn)
	    fprintf(stderr,"  Did not find \"switch\" statement in func33_3.\n");
	if (!foundSwitchOut)
	    fprintf(stderr,"  Did not find block afer \"switch\" statement.\n");
	return -1;
    }

    /* Check dominator info. */
    BPatch_Vector<BPatch_basicBlock*> entry3;
    cfg3->getEntryBasicBlock(entry3);
    if (entry3.size() != 1) {
	fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	fprintf(stderr, "  Detected %d entry basic blocks in func33_3, should have been one.\n", entry_blocks.size());
	return -1;
    }

    for (i = 0; i < (unsigned int) blocks3.size(); i++) {
	if (!entry3[0]->dominates(block_elements[i])) {
	    fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
	    fprintf(stderr, "  Entry block does not dominate all blocks in func33_3\n");
	    return -1;
	}
    }

    BPatch_Vector<BPatch_basicBlock*> exit3;
    cfg3->getExitBasicBlock(exit3);
    if (exit3.size() != 1) {
       fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
       fprintf(stderr, "  Detected %d exit basic blocks in func33_3, should have been one.\n", exit3.size());
       return -1;
    }

    for (i = 0; i < (unsigned int) exit3.size(); i++) {
       if (!exit3[i]->postdominates(entry3[0])) {
          fprintf(stderr, "**Failed** test #33 (control flow graphs)\n");
          fprintf(stderr, "  Exit block %d does not postdominate all entry blocks in func33_3\n", i);
          return -1;
       }
    }


    delete [] block_elements;

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
