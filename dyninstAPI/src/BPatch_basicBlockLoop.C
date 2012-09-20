/*
 * See the dyninst/COPYRIGHT file for copyright information.
 * 
 * We provide the Paradyn Tools (below described as "Paradyn")
 * on an AS IS basis, and do not warrant its validity or performance.
 * We reserve the right to update, modify, or discontinue this
 * software at any time.  We shall have no obligation to supply such
 * updates or modifications or any other form of support to you.
 * 
 * By your use of Paradyn, you understand and agree that we (or any
 * other person or entity with proprietary rights in Paradyn) are
 * under no obligation to provide either maintenance services,
 * update services, notices of latent defects, or correction of
 * defects for Paradyn.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#define BPATCH_FILE

#include <stdio.h>
#include <string.h>
#include <iostream>
#include "common/h/std_namesp.h"
#include "BPatch_basicBlockLoop.h"
#include <iterator>

#include "BPatch_edge.h"

//constructors
//internal use only


BPatch_basicBlockLoop::BPatch_basicBlockLoop(BPatch_flowGraph *fg)
    : flowGraph(fg), parent(NULL) {}

BPatch_basicBlockLoop::BPatch_basicBlockLoop(BPatch_edge *be, 
					     BPatch_flowGraph *fg) 
    : flowGraph(fg), parent(NULL) 
{
    backEdges.insert(be);
}

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
  return  * backEdges.begin();
}
int BPatch_basicBlockLoop::getBackEdgesInt(BPatch_Vector<BPatch_edge*> &edges)
{
   std::copy(backEdges.begin(), backEdges.end(), std::back_inserter(edges));
   return edges.size();

#if 0
    for (unsigned idx =0; idx < edges.size(); idx++) {
        backEdges.insert(edges[idx]);
    }
    return edges.size();
#endif
}

// this is a private function, invoked by BPatch_flowGraph::createLoops
void BPatch_basicBlockLoop::addBackEdges
(std::vector< BPatch_edge*> &newEdges)
{
    backEdges.insert(newEdges.begin(),newEdges.end());
}

bool 
BPatch_basicBlockLoop::hasAncestorInt(BPatch_basicBlockLoop* l) {
    return l->containedLoops.contains(this); 
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
    assert(backEdges.size());
    return (* backEdges.begin())->getTarget();
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

