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
#include "common/src/std_namesp.h"
#include <iterator>
#include <algorithm>
#include "CFG.h"
#include <iostream>
using namespace std;
using namespace Dyninst;
using namespace Dyninst::ParseAPI;
//constructors
//internal use only


Loop::Loop(const Function *f)
    : func(f), parent(NULL) {
    }

Loop::Loop(Edge *be, const Function *f)
    : func(f), parent(NULL) 
{
    backEdges.insert(be);
}

bool Loop::containsAddress(Address addr)
{
    vector<Block*> blks;
    getLoopBasicBlocksExclusive(blks);

    for(unsigned i = 0; i < blks.size(); i++) {
	if (addr >= blks[i]->start() &&
	    addr < blks[i]->end() ) 
	    return true;
    }

    return false;
}

bool Loop::containsAddressInclusive(Address addr)
{
    vector<Block*> blks;
    getLoopBasicBlocks(blks);

    for(unsigned i = 0; i < blks.size(); i++) {
	if (addr >= blks[i]->start() &&
	    addr < blks[i]->end() ) 
	    return true;
    }

    return false;
}

int Loop::getBackEdges(vector<Edge*> &edges)
{
   edges.insert(edges.end(), backEdges.begin(), backEdges.end());
   return edges.size();

}

bool 
Loop::hasAncestor(Loop* l) {
   return (l->containedLoops.find(this) != l->containedLoops.end());
}


bool
Loop::getLoops(vector<Loop*>& nls, bool outerMostOnly) const
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
bool
Loop::getContainedLoops(vector<Loop*>& nls)
{
  return getLoops(nls, false);
}

// get the outermost loops nested under this loop
bool 
Loop::getOuterLoops(vector<Loop*>& nls)
{
  return getLoops(nls, true);
}

//returns the basic blocks in the loop
bool Loop::getLoopBasicBlocks(vector<Block*>& bbs) {
   bbs.insert(bbs.end(), basicBlocks.begin(), basicBlocks.end());
  return true;
}


// returns the basic blocks in this loop, not those of its inner loops
bool Loop::getLoopBasicBlocksExclusive(vector<Block*>& bbs) {
    // start with a copy of all this loops basic blocks
   std::set<Block*> allBlocks(basicBlocks);


   // remove the blocks in each contained loop
   vector<Loop*> contLoops;
   getContainedLoops(contLoops);


   std::set<Block *> toRemove;

   for (unsigned int i = 0; i < contLoops.size(); i++) {
      std::copy(contLoops[i]->basicBlocks.begin(),
                contLoops[i]->basicBlocks.end(),
                std::inserter(toRemove, toRemove.end()));
   }
   
   std::set_difference(allBlocks.begin(), allBlocks.end(),
                       toRemove.begin(), toRemove.end(),
                       std::back_inserter(bbs),
                       std::less<Block *>());

   return true;
}



bool Loop::hasBlock(Block* block) 
{
    vector<Block*> blks;
    getLoopBasicBlocks(blks);

    for(unsigned i = 0; i < blks.size(); i++)
        if (blks[i]->start() == block->start())
            return true;
    return false;
}


bool Loop::hasBlockExclusive(Block*block) 
{
    vector<Block*> blks;
    getLoopBasicBlocksExclusive(blks);

    for(unsigned i = 0; i < blks.size(); i++)
        if (blks[i]->start() == block->start())
            return true;
    return false;
}




int Loop::getLoopEntries(vector<Block*> &e) {
    e.insert(e.end(), entries.begin(), entries.end());
    return e.size();
}


const Function* Loop::getFunction()
{
    return func;
}




std::string Loop::format() const {
   std::stringstream ret;
   
   ret << hex << "(Loop " << this << ": ";
   for (std::set<Block *>::iterator iter = basicBlocks.begin();
        iter != basicBlocks.end(); ++iter) {
      ret << (*iter)->start() << ", ";
   }
   ret << ")" << dec << endl;

   return ret.str();
}
