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
#include "BPatch_basicBlockLoop.h"
#include <iterator>
#include <algorithm>
#include "BPatch_edge.h"
#include <iostream>
#include "PatchCFG.h"
#include "Relocation/DynCommon.h"
#include "block.h"
using namespace std;
using namespace Dyninst::PatchAPI;
//constructors
//internal use only

BPatch_basicBlockLoop::BPatch_basicBlockLoop(BPatch_flowGraph *fg, PatchLoop* loop) 
    : flowGraph(fg), parent(NULL) 
{
    //parent pointer and containedLoops vectors are set in BPatch_flowgraph::createLoops
    
    //set backEdges
    vector<PatchEdge*> be;
    loop->getBackEdges(be);
    for (auto eit = be.begin(); eit != be.end(); ++eit) {
        backEdges.insert(fg->findEdge(SCAST_EI(*eit)));
    }
      
    //set basicBlocks
    vector<PatchBlock*> b;
    loop->getLoopBasicBlocks(b);
    for (auto bit = b.begin(); bit != b.end(); ++bit)
        basicBlocks.insert(fg->findBlock(SCAST_BI(*bit)));

    //set entries
    vector<PatchBlock*> eb;
    loop->getLoopEntries(eb);
    for (auto bit = eb.begin(); bit != eb.end(); ++bit)
        entries.insert(fg->findBlock(SCAST_BI(*bit)));
}

bool BPatch_basicBlockLoop::containsAddress(unsigned long addr)
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

bool BPatch_basicBlockLoop::containsAddressInclusive(unsigned long addr)
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

int BPatch_basicBlockLoop::getLoopEntries(BPatch_Vector<BPatch_basicBlock*> &e) {
    e.insert(e.end(), entries.begin(), entries.end());
    return e.size();
}

int BPatch_basicBlockLoop::getBackEdges(BPatch_Vector<BPatch_edge*> &edges)
{
   edges.insert(edges.end(), backEdges.begin(), backEdges.end());
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
BPatch_basicBlockLoop::hasAncestor(BPatch_basicBlockLoop* l) {
   return (l->containedLoops.find(this) != l->containedLoops.end());
}


bool
BPatch_basicBlockLoop::getLoops(BPatch_Vector<BPatch_basicBlockLoop*>& nls, 
				bool outerMostOnly) const
{
   for (std::set<BPatch_basicBlockLoop *>::iterator iter = containedLoops.begin();
        iter != containedLoops.end(); ++iter) {
      // only return a contained loop if this loop is its parent
      if (outerMostOnly && (this != (*iter)->parent)) continue;
      nls.push_back(*iter);
   }
   
   return true;
}

//method that returns the nested loops inside the loop. It returns a set
//of basicBlockLoop that are contained. It might be useful to add nest 
//as a field of this class but it seems it is not necessary at this point
bool
BPatch_basicBlockLoop::getContainedLoops(BPatch_Vector<BPatch_basicBlockLoop*>& nls)
{
  return getLoops(nls, false);
}

// get the outermost loops nested under this loop
bool 
BPatch_basicBlockLoop::getOuterLoops(BPatch_Vector<BPatch_basicBlockLoop*>& nls)
{
  return getLoops(nls, true);
}

//returns the basic blocks in the loop
bool BPatch_basicBlockLoop::getLoopBasicBlocks(BPatch_Vector<BPatch_basicBlock*>& bbs) {
   bbs.insert(bbs.end(), basicBlocks.begin(), basicBlocks.end());
  return true;
}


// returns the basic blocks in this loop, not those of its inner loops
bool BPatch_basicBlockLoop::getLoopBasicBlocksExclusive(BPatch_Vector<BPatch_basicBlock*>& bbs) {
    // start with a copy of all this loops basic blocks
   std::set<BPatch_basicBlock*> allBlocks(basicBlocks);


   // remove the blocks in each contained loop
   BPatch_Vector<BPatch_basicBlockLoop*> contLoops;
   getContainedLoops(contLoops);


   std::set<BPatch_basicBlock *> toRemove;

   for (unsigned int i = 0; i < contLoops.size(); i++) {
      std::copy(contLoops[i]->basicBlocks.begin(),
                contLoops[i]->basicBlocks.end(),
                std::inserter(toRemove, toRemove.end()));
   }
   
   std::set_difference(allBlocks.begin(), allBlocks.end(),
                       toRemove.begin(), toRemove.end(),
                       std::back_inserter(bbs),
                       std::less<BPatch_basicBlock *>());

   return true;
}



bool BPatch_basicBlockLoop::hasBlock(BPatch_basicBlock*block) 
{
    BPatch_Vector<BPatch_basicBlock*> blks;
    getLoopBasicBlocks(blks);

    for(unsigned i = 0; i < basicBlocks.size(); i++)
        if (blks[i]->getBlockNumber() == block->getBlockNumber())
            return true;
    return false;
}


bool BPatch_basicBlockLoop::hasBlockExclusive(BPatch_basicBlock*block) 
{
    BPatch_Vector<BPatch_basicBlock*> blks;
    getLoopBasicBlocksExclusive(blks);

    for(unsigned i = 0; i < basicBlocks.size(); i++)
        if (blks[i]->getBlockNumber() == block->getBlockNumber())
            return true;
    return false;
}



BPatch_flowGraph* BPatch_basicBlockLoop::getFlowGraph() 
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
std::set<BPatch_variableExpr*>* BPatch_basicBlockLoop::getLoopIterators(){
	cerr<<"WARNING : BPatch_basicBlockLoop::getLoopIterators is not";
	cerr<<" implemented yet\n";
	return NULL;
}

std::string BPatch_basicBlockLoop::format() const {
   std::stringstream ret;
   
   ret << hex << "(Loop " << this << ": ";
   for (std::set<BPatch_basicBlock *>::iterator iter = basicBlocks.begin();
        iter != basicBlocks.end(); ++iter) {
      ret << (*iter)->getStartAddress() << ", ";
   }
   ret << ")" << dec << endl;

   return ret.str();
}
