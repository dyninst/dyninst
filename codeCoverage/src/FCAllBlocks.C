#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream.h>
#include <fstream.h>

#include <CCcommon.h>
#include <FCAllBlocks.h>

/** constructor */
FCAllBlocks::FCAllBlocks() : FunctionCoverage() {}

/** constructor */
FCAllBlocks::FCAllBlocks(BPatch_function* f,BPatch_thread* t,
			 BPatch_image* i,
			 const char* funcN,const char* fileN) 
	: FunctionCoverage(f,t,i,funcN,fileN) {}

/* destructor */
FCAllBlocks::~FCAllBlocks() {}

/** since for all basic block instrumentation we do not need 
  * dominator information this method does nothing
  */
void FCAllBlocks::fillDominatorInfo(){
}

/** For all basic block instrumentation every basic blocks instrumented
  * so this method simply returns true for any given argument
  */ 
bool FCAllBlocks::validateBasicBlock(BPatch_basicBlock* bb){
	if(bb)
		return true;
	return false;
}

/** for all basic block instrumentation every basic block
  * is updated for its execution count. It does not infer any other
  * execution of any other basic block
  */ 
int FCAllBlocks::updateExecutionCounts(BPatch_basicBlock* bb,int ec){
	if(bb)
		executionCounts[bb->getBlockNumber()] += ec;
	return Error_OK;
}
