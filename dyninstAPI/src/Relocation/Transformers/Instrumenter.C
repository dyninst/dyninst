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



#include "Relocation/Transformers/Transformer.h"
#include "Relocation/Transformers/Instrumenter.h"
#include "debug.h"
#include "Relocation/Atoms/Atom.h"
#include "Relocation/Atoms/Target.h"
#include "instPoint.h"
#include "baseTramp.h"
#include "Relocation/Atoms/Instrumentation.h"
#include "Relocation/Atoms/CFAtom.h"

using namespace std;
using namespace Dyninst;
using namespace Relocation;
using namespace InstructionAPI;


bool Instrumenter::processTrace(TraceList::iterator &iter) {
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

  AtomList &elements = (*iter)->elements();

  for (AtomList::iterator e_iter = elements.begin();
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

    // Get the stuff we need: a CFAtom for the last instruction
    CFAtom::Ptr cf = dyn_detail::boost::dynamic_pointer_cast<CFAtom>(elements.back());
    assert(cf);

    if (post) {
      relocation_cerr << "   ... fallthrough inst, adding" << endl;
      if (!addEdgeInstrumentation(post,
				  cf,
				  CFAtom::Fallthrough,
				  *iter))
	return false;
    }
    if (target) {
      relocation_cerr << "   ... target inst, adding" << endl;
      if (!addEdgeInstrumentation(target,
				  cf,
				  CFAtom::Taken,
				  *iter))
	return false;
    }
  }
  return true;
}

bool Instrumenter::postprocess(TraceList &bl) {
  // Yuck iteration... anyone have a better idea?

  relocation_cerr << "Instrumenter: postProcess" << endl;
  
  if (edgeTraces_.empty()) {
    relocation_cerr << "  ... nothing to do, returning" << endl;
    return true;
  }

  for (TraceList::iterator iter = bl.begin();
       iter != bl.end(); ++iter) {
    relocation_cerr << "   Testing block " << iter->get() << endl;
    // Try pre-insertion
    EdgeTraces::iterator pre = edgeTraces_.find(std::make_pair(*iter, Before));
    if (pre != edgeTraces_.end()) {
      relocation_cerr << "     Inserting " << pre->second.size() << " pre blocks" << endl;
      bl.insert(iter, pre->second.begin(), pre->second.end());
    }
    // And post-insertion?
    EdgeTraces::iterator post = edgeTraces_.find(std::make_pair(*iter, After));
    if (post != edgeTraces_.end()) {
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
					     CFAtom::Ptr cf,
					     Address dest,
					     Trace::Ptr cur) {
  // We handle edge instrumentation by creating a new Trace and
  // wiring it in in the appropriate place. The actual adding is
  // done later, since we can't modify the list from here. 
  // What we can do is leave a marker of _where_ it should be
  // inserted. 

  Trace::Ptr inst = Trace::create(tramp);
  Target<Trace::Ptr> *t = new Target<Trace::Ptr>(inst);

  // 1) Find the appropriate successor S of the current block B
  // 2) Set the fallthrough successor of the instrumentation block I to S
  // 3) Set the appropriate successor of S to I
  // 4) Add the inst block to the "add this to the list" list.

  // 1)
  CFAtom::DestinationMap::iterator d_iter = cf->destMap_.find(dest);
  // What if someone requested edge instrumentation for an edge that
  // doesn't exist? Oopsie.
  if (d_iter == cf->destMap_.end()) {
    delete t;
    return true;
  }

  // 2)
  // Keep this info for later...
  TargetInt *target = d_iter->second;

  CFAtom::Ptr postCF = CFAtom::create();
  postCF->addDestination(CFAtom::Fallthrough, target);
  inst->elements().push_back(postCF);

  // 3)
  d_iter->second = t;    
  
  // 4) 
  // It's more efficient branch-wise to put a block in
  // before its target rather than after this; 
  // however, if we aren't moving the target then we 
  // fall back.
  Trace::Ptr insertPoint;
  Target<Trace::Ptr> *targ = dynamic_cast<Target<Trace::Ptr> *>(target);
  if (targ) {
    edgeTraces_[std::make_pair(targ->t(), Before)].push_back(inst);
  }
  else {
    edgeTraces_[std::make_pair(cur, After)].push_back(inst);
  }

  return true;
}
