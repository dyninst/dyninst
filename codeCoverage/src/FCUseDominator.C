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
			       const char* funcN,const char* fileN) 
		: FunctionCoverage(f,t,i,funcN,fileN) {}

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
			BPatch_sourceBlock* sb = id->getSourceBlock();
			updateLinesCovered(sb);
		}
		executionCounts[id->getBlockNumber()] += ec;
	}
        return Error_OK;
}

