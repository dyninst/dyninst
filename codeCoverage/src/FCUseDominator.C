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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream.h>
#include <fstream.h>

#include <CCcommon.h>
#include <FCUseDominator.h>

/** constructor */
FCUseDominator::FCUseDominator() : FunctionCoverage() {}

/** constructor */
FCUseDominator::FCUseDominator(BPatch_function* f,BPatch_thread* t,
			       BPatch_image* i,
			       const char* funcN) 
		: FunctionCoverage(f,t,i,funcN) {}

/* destructor */
FCUseDominator::~FCUseDominator() {}

/** fills the dominator information of the control flow graph
  * of the function represented by this object
  */
void FCUseDominator::fillDominatorInfo(){
	if(cfg)
		cfg->fillDominatorInfo();
}

/** a basic block is instrumented if it is one of the leaf nodes
  * in the immediate dominator tree of the control flow graph 
  * or it has an outgoing edge to a basic block that it does 
  * not dominate
  */
bool FCUseDominator::validateBasicBlock(BPatch_basicBlock* bb){
	bool ret = false;

	BPatch_Vector<BPatch_basicBlock*> immediateDominates;
	bb->getImmediateDominates(immediateDominates);

	/** if it is a leaf node */
	if(!immediateDominates.size())
		ret = true;
	else{
		/** or it has outgoing edge to basic block
		  * it does not dominate
		  */
		BPatch_Set<BPatch_basicBlock*> allDom;
		bb->getAllDominates(allDom);

		BPatch_Vector<BPatch_basicBlock*> targets;
		bb->getTargets(targets);

		for(unsigned int j=0;j<targets.size();j++)
			if(!allDom.contains(targets[j])){
				/** target is not in the set 
				  * basic block dominates
				  */
				ret = true;
				break;
			}
	}
	return ret;
}

/** the execution counts of the basic block given and the basic blocks
  * whose execution can be deduced from the given basic block is updated.
  * If a is immediate dominator of b and if b is executed then we can
  * deduce a is also executed. So this method iteratively updates
  * execution counts of the basic blocks through the path towards the root
  * of the immediate dominator tree starting from the basic block given
  */
int FCUseDominator::updateExecutionCounts(BPatch_basicBlock* bb,int ec){
	for(BPatch_basicBlock* id = bb;
	    id != NULL;
	    id = id->getImmediateDominator())
	{
		if(!executionCounts[id->getBlockNumber()]){
			BPatch_Vector<BPatch_sourceBlock*> sb;
			id->getSourceBlocks(sb);
			for(unsigned int i=0;i<sb.size();i++)
				updateLinesCovered(sb[i]);
		}
		executionCounts[id->getBlockNumber()] += ec;
	}
    return Error_OK;
}

