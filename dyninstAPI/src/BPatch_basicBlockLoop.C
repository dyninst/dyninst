#define BPATCH_FILE

#include <stdio.h>
#include <iostream>
#include "BPatch_basicBlockLoop.h"

//constructors
//internal use only

BPatch_basicBlockLoop::BPatch_basicBlockLoop()
    : parent(NULL) {}

BPatch_basicBlockLoop::BPatch_basicBlockLoop(BPatch_basicBlock* lh) 
    : loopHead(lh), parent(NULL) {}

//retrieves the basic blocks which has back edge to the head of the loop
//meaning tail of back edges which defines the loop
void BPatch_basicBlockLoop::getBackEdges(BPatch_Vector<BPatch_basicBlock*>& bes){
	BPatch_basicBlock** elements = 
			new BPatch_basicBlock*[backEdges.size()];
	backEdges.elements(elements);
	for(int i=0;i<backEdges.size();i++)
		bes.push_back(elements[i]);
	delete[] elements;
}

bool 
BPatch_basicBlockLoop::hasAncestor(BPatch_basicBlockLoop* l) {
    // walk up this loop's chain of parents looking for l
    BPatch_basicBlockLoop* p = parent;
    while (p != NULL) {
	if (p==l) return true;
	p = p->parent;
    }
    return false;
}


void 
BPatch_basicBlockLoop::getLoops(BPatch_Vector<BPatch_basicBlockLoop*>& nls, 
				bool outerMostOnly)
{
    BPatch_basicBlockLoop** elements = 
	new BPatch_basicBlockLoop* [containedLoops.size()];

    containedLoops.elements(elements);

    for(int i=0; i < containedLoops.size(); i++) {
	// only return a contained loop if this loop is its parent
	if (outerMostOnly) {
	    if (this == elements[i]->parent) {
		nls.push_back(elements[i]);
	    }
	}
	else {
	    nls.push_back(elements[i]);
	}
    }

    delete[] elements;
}

//method that returns the nested loops inside the loop. It returns a set
//of basicBlockLoop that are contained. It might be useful to add nest 
//as a field of this class but it seems it is not necessary at this point
void 
BPatch_basicBlockLoop::getContainedLoops(BPatch_Vector<BPatch_basicBlockLoop*>& nls)
{
    getLoops(nls, false);
}

// get the outermost loops nested under this loop
void 
BPatch_basicBlockLoop::getOuterLoops(BPatch_Vector<BPatch_basicBlockLoop*>& nls)
{
    getLoops(nls, true);
}

//returns the basic blocks in the loop
void BPatch_basicBlockLoop::getLoopBasicBlocks(BPatch_Vector<BPatch_basicBlock*>& bbs){
	BPatch_basicBlock** elements = 
			new BPatch_basicBlock*[basicBlocks.size()];
	basicBlocks.elements(elements);
	for(int i=0;i<basicBlocks.size();i++)
		bbs.push_back(elements[i]);
	delete[] elements;
}

//method that returns the head of the loop. Which is also
//head of the back edge which defines the natural loop
BPatch_basicBlock* BPatch_basicBlockLoop::getLoopHead(){
	return loopHead;
}

//we did not implement this method yet. It needs some deeper
//analysis and some sort of uniform dataflow framework. It is a method
//that returns the iterator of the loop. To find it we have to do 
//invariant code analysis which needs reaching definition information
//live variable analysis etc. Since these algorithms are not
//machine independent and needs more inner level machine dependent
//functions and we do not need at this moment for our project we did not 
//implement the function. It returns NULL for now.
BPatch_Set<BPatch_variableExpr*>* BPatch_basicBlockLoop::getLoopIterators(){
	cerr<<"WARNING : BPatch_basicBlockLoop::getLoopIterators is not";
	cerr<<" implemented yet\n";
	return NULL;
}

//print method
ostream& operator<<(ostream& os,BPatch_basicBlockLoop& bbl){
	int i;

	os << "Begin LOOP :\n";
	os << "HEAD : " << bbl.loopHead->getBlockNumber() << "\n";

	os << "BACK EDGES :\n";
	BPatch_basicBlock** belements = 
			new BPatch_basicBlock*[bbl.backEdges.size()];
	bbl.backEdges.elements(belements);
	for(i=0;i<bbl.backEdges.size();i++)
		os << belements[i]->getBlockNumber() << " " ;
	delete[] belements;
	os << "\n";

	os << "BASIC BLOCKS :\n";
	belements = new BPatch_basicBlock*[bbl.basicBlocks.size()];
	bbl.basicBlocks.elements(belements);
	for(i=0;i<bbl.basicBlocks.size();i++)
		os << belements[i]->getBlockNumber() << " ";
	delete[] belements;
	os << "\n";

	os << "CONTAINED LOOPS WITH HEAD :\n";
	BPatch_basicBlockLoop** lelements = 
			new BPatch_basicBlockLoop*[bbl.containedLoops.size()]; 
	bbl.containedLoops.elements(lelements);
	for(i=0;i<bbl.containedLoops.size();i++)
		os << "\t" << lelements[i]->loopHead->getBlockNumber() << "\n";
	delete[] lelements;
	os << "End LOOP :\n";

	return os;
}
