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


BPatch_basicBlockLoop::BPatch_basicBlockLoop(BPatch_flowGraph *fg)
    : flowGraph(fg), parent(NULL) {}

BPatch_basicBlockLoop::BPatch_basicBlockLoop(BPatch_edge *be, 
					     BPatch_flowGraph *fg) 
    : backEdge(be), flowGraph(fg), parent(NULL) {}

bool BPatch_basicBlockLoop::containsAddressInt(unsigned long addr)
{
    BPatch_Vector<BPatch_basicBlock*> blks;
    getLoopBasicBlocksExclusive(blks);

    for(unsigned i = 0; i < blks.size(); i++) {
	if (addr >= blks[i]->getStartAddress() &&
	    addr < blks[i]->getStartAddress() + blks[i]->size() ) 
	    return true;
    }

    return false;
}

bool BPatch_basicBlockLoop::containsAddressInclusiveInt(unsigned long addr)
{
    BPatch_Vector<BPatch_basicBlock*> blks;
    getLoopBasicBlocks(blks);

    for(unsigned i = 0; i < blks.size(); i++) {
	if (addr >= blks[i]->getStartAddress() &&
	    addr < blks[i]->getStartAddress() + blks[i]->size() ) 
	    return true;
    }

    return false;
}

BPatch_edge *BPatch_basicBlockLoop::getBackEdgeInt()
{
  return backEdge;
}

bool 
BPatch_basicBlockLoop::hasAncestorInt(BPatch_basicBlockLoop* l) {
    // walk up this loop's chain of parents looking for l
    BPatch_basicBlockLoop* p = parent;
    while (p != NULL) {
        //        fprintf(stderr,"hasAncestor 0x%x 0x%x\n", p, p->parent);
	if (p==l) return true;
	p = p->parent;
    }
    return false;
}


bool
BPatch_basicBlockLoop::getLoops(BPatch_Vector<BPatch_basicBlockLoop*>& nls, 
				bool outerMostOnly) const
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
    return true;
}

//method that returns the nested loops inside the loop. It returns a set
//of basicBlockLoop that are contained. It might be useful to add nest 
//as a field of this class but it seems it is not necessary at this point
bool
BPatch_basicBlockLoop::getContainedLoopsInt(BPatch_Vector<BPatch_basicBlockLoop*>& nls)
{
  return getLoops(nls, false);
}

// get the outermost loops nested under this loop
bool 
BPatch_basicBlockLoop::getOuterLoopsInt(BPatch_Vector<BPatch_basicBlockLoop*>& nls)
{
  return getLoops(nls, true);
}

//returns the basic blocks in the loop
bool BPatch_basicBlockLoop::getLoopBasicBlocksInt(BPatch_Vector<BPatch_basicBlock*>& bbs) {
  BPatch_basicBlock** elements = 
    new BPatch_basicBlock*[basicBlocks.size()];
  basicBlocks.elements(elements);
  for(unsigned i=0;i<basicBlocks.size();i++)
    bbs.push_back(elements[i]);
  delete[] elements;
  return true;
}


// returns the basic blocks in this loop, not those of its inner loops
bool BPatch_basicBlockLoop::getLoopBasicBlocksExclusiveInt(BPatch_Vector<BPatch_basicBlock*>& bbs) {
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
    return true;
}



bool BPatch_basicBlockLoop::hasBlockInt(BPatch_basicBlock*block) 
{
    BPatch_Vector<BPatch_basicBlock*> blks;
    getLoopBasicBlocks(blks);

    for(unsigned i = 0; i < basicBlocks.size(); i++)
        if (blks[i]->getBlockNumber() == block->getBlockNumber())
            return true;
    return false;
}


bool BPatch_basicBlockLoop::hasBlockExclusiveInt(BPatch_basicBlock*block) 
{
    BPatch_Vector<BPatch_basicBlock*> blks;
    getLoopBasicBlocksExclusive(blks);

    for(unsigned i = 0; i < basicBlocks.size(); i++)
        if (blks[i]->getBlockNumber() == block->getBlockNumber())
            return true;
    return false;
}


//method that returns the head of the loop. Which is also
//head of the back edge which defines the natural loop
BPatch_basicBlock* BPatch_basicBlockLoop::getLoopHeadInt()
{
    assert(backEdge != NULL);
    return backEdge->target;
}


BPatch_flowGraph* BPatch_basicBlockLoop::getFlowGraphInt() 
{
    return flowGraph;
}


//we did not implement this method yet. It needs some deeper
//analysis and some sort of uniform dataflow framework. It is a method
//that returns the iterator of the loop. To find it we have to do 
//invariant code analysis which needs reaching definition information
//live variable analysis etc. Since these algorithms are not
//machine independent and needs more inner level machine dependent
//functions and we do not need at this moment for our project we did not 
//implement the function. It returns NULL for now.
BPatch_Set<BPatch_variableExpr*>* BPatch_basicBlockLoop::getLoopIteratorsInt(){
	cerr<<"WARNING : BPatch_basicBlockLoop::getLoopIterators is not";
	cerr<<" implemented yet\n";
	return NULL;
}

