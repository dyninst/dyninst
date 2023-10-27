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


#include <stdio.h>
#include <string.h>
#include <iostream>
#include <iterator>
#include <algorithm>
#include "CFG.h"
#include "PatchCFG.h"
#include <iostream>
using namespace std;
using namespace Dyninst;
using namespace Dyninst::PatchAPI;
//constructors
//internal use only


PatchLoop::PatchLoop(PatchObject *obj, ParseAPI::Loop *loop): loop_(loop), parent(NULL) {
    //parent pointer and containedLoops vectors are set in PatchFunction::createLoops
    
    //set backEdges
    vector<ParseAPI::Edge*> be;
    loop->getBackEdges(be);
    for (auto eit = be.begin(); eit != be.end(); ++eit) {
        backEdges.insert(obj->getEdge(*eit, obj->getBlock((*eit)->src()), obj->getBlock((*eit)->trg())));
    }
      
    //set func
    func = obj->getFunc(const_cast<ParseAPI::Function*>(loop->getFunction()));

    //set basicBlocks
    vector<ParseAPI::Block*> b;
    loop->getLoopBasicBlocks(b);
    for (auto bit = b.begin(); bit != b.end(); ++bit)
        basicBlocks.insert(obj->getBlock(*bit));

    //set entries;
    vector<ParseAPI::Block*> eb;
    loop->getLoopEntries(eb);
    for (auto bit = eb.begin(); bit != eb.end(); ++bit)
        entries.insert(obj->getBlock(*bit));

}


bool PatchLoop::containsAddress(Address addr)
{
    vector<PatchBlock*> blks;
    getLoopBasicBlocksExclusive(blks);

    for(unsigned i = 0; i < blks.size(); i++) {
	if (addr >= blks[i]->start() &&
	    addr < blks[i]->end() ) 
	    return true;
    }

    return false;
}

bool PatchLoop::containsAddressInclusive(Address addr)
{
    vector<PatchBlock*> blks;
    getLoopBasicBlocks(blks);

    for(unsigned i = 0; i < blks.size(); i++) {
	if (addr >= blks[i]->start() &&
	    addr < blks[i]->end() ) 
	    return true;
    }

    return false;
}

int PatchLoop::getBackEdges(vector<PatchEdge*> &edges)
{
   edges.insert(edges.end(), backEdges.begin(), backEdges.end());
   return edges.size();

}

int PatchLoop::getLoopEntries(vector<PatchBlock*> &e) {
   e.insert(e.end(), entries.begin(), entries.end());
   return e.size();
}


bool PatchLoop::hasAncestor(PatchLoop* l) {
   return (l->containedLoops.find(this) != l->containedLoops.end());
}


bool PatchLoop::getLoops(vector<PatchLoop*>& nls, bool outerMostOnly) const
{
   for (auto iter = containedLoops.begin(); iter != containedLoops.end(); ++iter) {
      // only return a contained loop if this loop is its parent
      if (outerMostOnly && (this != (*iter)->parent)) continue;
      nls.push_back(*iter);
   }
   
   return true;
}

//method that returns the nested loops inside the loop. It returns a set
//of Loop that are contained. It might be useful to add nest 
//as a field of this class but it seems it is not necessary at this point
bool PatchLoop::getContainedLoops(vector<PatchLoop*>& nls)
{
  return getLoops(nls, false);
}

// get the outermost loops nested under this loop
bool 
PatchLoop::getOuterLoops(vector<PatchLoop*>& nls)
{
  return getLoops(nls, true);
}

//returns the basic blocks in the loop
bool PatchLoop::getLoopBasicBlocks(vector<PatchBlock*>& bbs) {
   bbs.insert(bbs.end(), basicBlocks.begin(), basicBlocks.end());
  return true;
}


// returns the basic blocks in this loop, not those of its inner loops
bool PatchLoop::getLoopBasicBlocksExclusive(vector<PatchBlock*>& bbs) {
    // start with a copy of all this loops basic blocks
   std::set<PatchBlock*> allBlocks(basicBlocks);


   // remove the blocks in each contained loop
   vector<PatchLoop*> contLoops;
   getContainedLoops(contLoops);


   std::set<PatchBlock *> toRemove;

   for (unsigned int i = 0; i < contLoops.size(); i++) {
      std::copy(contLoops[i]->basicBlocks.begin(),
                contLoops[i]->basicBlocks.end(),
                std::inserter(toRemove, toRemove.end()));
   }
   
   std::set_difference(allBlocks.begin(), allBlocks.end(),
                       toRemove.begin(), toRemove.end(),
                       std::back_inserter(bbs),
                       std::less<PatchBlock *>());

   return true;
}



bool PatchLoop::hasBlock(PatchBlock* block) 
{
    vector<PatchBlock*> blks;
    getLoopBasicBlocks(blks);

    for(unsigned i = 0; i < basicBlocks.size(); i++)
        if (blks[i]->start() == block->start())
            return true;
    return false;
}


bool PatchLoop::hasBlockExclusive(PatchBlock*block) 
{
    vector<PatchBlock*> blks;
    getLoopBasicBlocksExclusive(blks);

    for(unsigned i = 0; i < basicBlocks.size(); i++)
        if (blks[i]->start() == block->start())
            return true;
    return false;
}



PatchFunction* PatchLoop::getFunction() 
{
    return func;
}




std::string PatchLoop::format() const {
   std::stringstream ret;
   
   ret << hex << "(PatchLoop " << this << ": ";
   for (std::set<PatchBlock *>::iterator iter = basicBlocks.begin();
        iter != basicBlocks.end(); ++iter) {
      ret << (*iter)->start() << ", ";
   }
   ret << ")" << dec << endl;

   return ret.str();
}
