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

#define BPATCH_FILE

#include <stdio.h>
#include <string.h>
#include <iostream>
#include "common/h/std_namesp.h"
#include "BPatch_basicBlockLoop.h"

//constructors
//internal use only


BPatch_basicBlockLoop::BPatch_basicBlockLoop()
    : parent(NULL) {}

BPatch_basicBlockLoop::BPatch_basicBlockLoop(BPatch_basicBlock* lh) 
    : loopHead(lh), parent(NULL) {}

bool BPatch_basicBlockLoop::containsAddress(unsigned long addr)
{
    BPatch_Vector<BPatch_basicBlock*> blks;
    getLoopBasicBlocksExclusive(blks);

    for(unsigned int i = 0; i < blks.size(); i++) {
	if (addr >= blks[i]->getStartAddress() &&
	    addr <= blks[i]->getEndAddress()) 
	    return true;
    }

    return false;
}


//retrieves the basic blocks which has back edge to the head of the loop
//meaning tail of back edges which defines the loop
void BPatch_basicBlockLoop::getBackEdges(BPatch_Vector<BPatch_basicBlock*>& bes){
	BPatch_basicBlock** elements = 
			new BPatch_basicBlock*[backEdges.size()];
	backEdges.elements(elements);
	for(unsigned i=0;i<backEdges.size();i++)
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

    for(unsigned i=0; i < containedLoops.size(); i++) {
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
	for(unsigned i=0;i<basicBlocks.size();i++)
		bbs.push_back(elements[i]);
	delete[] elements;
}


// returns the basic blocks in this loop, not those of its inner loops
void BPatch_basicBlockLoop::getLoopBasicBlocksExclusive(BPatch_Vector<BPatch_basicBlock*>& bbs) {
    // start with a copy of all this loops basic blocks
    BPatch_Set<BPatch_basicBlock*> allBlocks(basicBlocks);

    // remove the blocks in each contained loop
    BPatch_Vector<BPatch_basicBlockLoop*> contLoops;
    getContainedLoops(contLoops);

    for (unsigned int i = 0; i < contLoops.size(); i++) {
	allBlocks -= contLoops[i]->basicBlocks;
    }

    BPatch_basicBlock** elements = new BPatch_basicBlock*[allBlocks.size()];
    allBlocks.elements(elements);

    for (unsigned int j = 0; j < allBlocks.size(); j++)
	bbs.push_back(elements[j]);

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

#ifdef DEBUG
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
#endif
