/*
 * Copyright (c) 1996-2009 Barton P. Miller
 * 
 * We provide the Paradyn Parallel Performance Tools (below
 * described as "Paradyn") on an AS IS basis, and do not warrant its
 * validity or performance.  We reserve the right to update, modify,
 * or discontinue this software at any time.  We shall have no
 * obligation to supply such updates or modifications or any other
 * form of support to you.
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



#include "r_t_Base.h"
#include "r_t_Instrumenter.h"
#include "debug.h"
#include "r_e_Base.h"
#include "r_e_Target.h"
#include "instPoint.h"
#include "baseTramp.h"
#include "r_e_Instrumentation.h"
#include "r_e_ControlFlow.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;


bool Instrumenter::processBlock(BlockList::iterator &iter) {
  relocation_cerr << "Instrumenter, processing block " 
		  << std::hex << (*iter)->origAddr() << std::dec << endl;

  if ((*iter)->bbl() == NULL)
    return true;

  // Basic concept: iterate over all of our instructions and look
  // up the instPoint for that instruction. If it exists, prepend
  // or append an instrumentation element at the appropriate
  // point in the list.
  
  // TODO: edge instrumentation... that will require modifying
  // the block list as well as elements within a block. 

  baseTramp *pre = NULL;
  baseTramp *post = NULL;
  instPoint *point = NULL;

  ElementList &elements = (*iter)->elements();

  for (ElementList::iterator e_iter = elements.begin();
       e_iter != elements.end(); ++e_iter) {
    assert(pre == NULL);

    // We're inserting an Inst element before us (if there is a baseTramp
    // with something interesting, that is). 

    // Assertion: we have no Inst elements already
    Address addr = (*e_iter)->addr();
    if (addr == 0) continue;
    relocation_cerr << "  Checking for point at " << std::hex << addr << std::dec << endl;

    point = (*iter)->bbl()->func()->findInstPByAddr(addr);
    if (!point) continue;

    pre = point->preBaseTramp();

    relocation_cerr << "   Found instrumentation at addr " 
		    << std::hex << addr << std::dec
		    << ((post && !post->empty()) ? "<POST CARRYOVER>" : "")
		    << ((pre && !pre->empty()) ? "<PRE>" : "") << endl;
    
    Inst::Ptr inst = Inst::create();
    if (post && !post->empty())
      inst->addBaseTramp(post);

    if (pre && !pre->empty())
      inst->addBaseTramp(pre);

    if (!inst->empty())
      elements.insert(e_iter, inst);
    // Otherwise it silently disappears...

    post = point->postBaseTramp();
    pre = NULL;
  }

  // Edge instrumentation time
  // Only the final point can have edge instrumentation;
  // this includes the postBaseTramp (for fallthrough)
  // or a targetBaseTramp (for taken edges)

  if (point) {
    relocation_cerr << "   Trailing <point>, checking edge instrumentation" << endl;
    baseTramp *target = point->targetBaseTramp();
    // post is still assigned from above
    if (!target &&
	!post) {
      relocation_cerr << "   ... neither target nor post, no edge" << endl;
      return true;
    }

    // Get the stuff we need: a CFElement for the last instruction
    CFElement::Ptr cf = dyn_detail::boost::dynamic_pointer_cast<CFElement>(elements.back());
    assert(cf);

    if (post) {
      relocation_cerr << "   ... fallthrough inst, adding" << endl;
      if (!addEdgeInstrumentation(post,
				  cf,
				  CFElement::Fallthrough,
				  *iter))
	return false;
    }
    if (target) {
      relocation_cerr << "   ... target inst, adding" << endl;
      if (!addEdgeInstrumentation(target,
				  cf,
				  CFElement::Taken,
				  *iter))
	return false;
    }
  }
  return true;
}

bool Instrumenter::postprocess(BlockList &bl) {
  // Yuck iteration... anyone have a better idea?

  relocation_cerr << "Instrumenter: postProcess" << endl;
  
  if (edgeBlocks_.empty()) {
    relocation_cerr << "  ... nothing to do, returning" << endl;
    return true;
  }

  for (BlockList::iterator iter = bl.begin();
       iter != bl.end(); ++iter) {
    relocation_cerr << "   Testing block " << iter->get() << endl;
    // Try pre-insertion
    EdgeBlocks::iterator pre = edgeBlocks_.find(std::make_pair(*iter, Before));
    if (pre != edgeBlocks_.end()) {
      relocation_cerr << "     Inserting " << pre->second.size() << " pre blocks" << endl;
      bl.insert(iter, pre->second.begin(), pre->second.end());
    }
    // And post-insertion?
    EdgeBlocks::iterator post = edgeBlocks_.find(std::make_pair(*iter, After));
    if (post != edgeBlocks_.end()) {
      relocation_cerr << "    Inserting " << post->second.size() << " post blocks" << endl;
      // Game the main iterator here...
      ++iter; // To get successor
      bl.insert(iter, post->second.begin(), post->second.end());
      // We're now one too far; back up so the for loop will
      // move us forward.
      --iter;
    }
  }
  return true;
}


bool Instrumenter::addEdgeInstrumentation(baseTramp *tramp,
					     CFElement::Ptr cf,
					     Address dest,
					     Block::Ptr cur) {
  // We handle edge instrumentation by creating a new Block and
  // wiring it in in the appropriate place. The actual adding is
  // done later, since we can't modify the list from here. 
  // What we can do is leave a marker of _where_ it should be
  // inserted. 

  Block::Ptr inst = Block::create(tramp);
  Target<Block::Ptr> *t = new Target<Block::Ptr>(inst);

  // 1) Find the appropriate successor S of the current block B
  // 2) Set the fallthrough successor of the instrumentation block I to S
  // 3) Set the appropriate successor of S to I
  // 4) Add the inst block to the "add this to the list" list.

  // 1)
  CFElement::DestinationMap::iterator d_iter = cf->destMap_.find(dest);
  // What if someone requested edge instrumentation for an edge that
  // doesn't exist? Oopsie.
  if (d_iter == cf->destMap_.end()) {
    delete t;
    return true;
  }

  // 2)
  // Keep this info for later...
  TargetInt *target = d_iter->second;

  CFElement::Ptr postCF = CFElement::create();
  postCF->addDestination(CFElement::Fallthrough, target);
  inst->elements().push_back(postCF);

  // 3)
  d_iter->second = t;    
  
  // 4) 
  // It's more efficient branch-wise to put a block in
  // before its target rather than after this; 
  // however, if we aren't moving the target then we 
  // fall back.
  Block::Ptr insertPoint;
  Target<Block::Ptr> *targ = dynamic_cast<Target<Block::Ptr> *>(target);
  if (targ) {
    edgeBlocks_[std::make_pair(targ->t(), Before)].push_back(inst);
  }
  else {
    edgeBlocks_[std::make_pair(cur, After)].push_back(inst);
  }

  return true;
}
